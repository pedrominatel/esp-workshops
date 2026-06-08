# Hand Gesture Recognition Camera — Architecture

## Overview

Live hand gesture recognition on the **ESP32-S3-EYE** board.  
The OV2640 camera captures VGA frames, a software resize node scales them to 480×480, and a gesture task runs `HandDetect` + `HandGestureRecognizer` from esp-dl on every frame.  
Results are printed over UART; there is no display.

---

## Pipeline

```
OV2640 (VGA 640×480 RGB565)
        │
        ▼
  WhoFetchNode        — "FrameCapFetch"
  (dequeues DMA frames from camera driver ring buffer)
        │
        ▼
  WhoSwResizeNode     — "FrameCap480"
  (software resize 640×480 → 480×480 RGB565,
   converts big-endian DVP pixels to little-endian,
   output buffered in PSRAM ring buffer)
        │  new-frame event
        ▼
  WhoGestureRecognize — "GestureRecognize"
  (copies frame to private PSRAM buffer,
   calls HandDetect::run() → bounding boxes,
   calls HandGestureRecognizer::recognize() → gesture label,
   logs category + score via ESP_LOGI at ≤1 fps)
```

All three tasks run on **Core 0** (`tskNO_AFFINITY` is irrelevant — unicore mode is active).

---

## Key Files

| File | Role |
|---|---|
| `main/app_main.cpp` | Entry point: init BSP LEDs, build pipeline, preload models, start tasks |
| `main/frame_cap_pipeline.hpp/.cpp` | Defines `PROCESS_FRAME_W/H` (480×480), builds the `WhoFrameCap` pipeline |
| `main/who_sw_resize_node.hpp/.cpp` | `WhoFrameCapNode` that resizes camera frames using `dl::image::ImageTransformer` |
| `main/who_gesture_recognize.hpp/.cpp` | `WhoTask` that runs HandDetect + HandGestureRecognizer on each new frame |
| `main/idf_component.yml` | Pins `espressif/esp-dl:3.2.4`, `espressif/esp32_s3_eye_noglib:~5.0`, `espressif/esp32-camera:^2.0.13` |
| `sdkconfig.bsp.esp32_s3_eye` | Board-specific build configuration |

---

## Critical Design Decisions

### 1. `CONFIG_FREERTOS_UNICORE=y` — the crash fix

**Problem:** Both `WhoSwResizeNode` and `HandDetect` use the ESP32-S3's EE (Enhanced Execution) SIMD unit (`dl_tie728_s8_*` assembly). FreeRTOS's *lazy* CP0 coprocessor save/restore has a race condition in SMP mode — even when both tasks are pinned to the same core — because the cross-core ownership tracking code is still active.  
This caused `IllegalInstruction` (EXCCAUSE=0) inside `dl_tie728_s8_depthwise_conv2d_per_layer_33c1_relu` after 2–20 iterations, non-deterministically.

**Fix:** `CONFIG_FREERTOS_UNICORE=y` halts Core 1 and activates the single-core FreeRTOS path, which has correct and well-tested CP0 lazy save/restore. Verified stable for 90+ seconds with zero crashes.

### 2. `CONFIG_SPIRAM_RODATA` removed

With `CONFIG_SPIRAM_RODATA=y` the model flatbuffer is copied to PSRAM at boot and inference reads model weights from PSRAM. This creates heavy PSRAM bus contention alongside the camera DMA writes. Removing this option keeps model weights in flash (.rodata), so inference reads go through the flash cache (separate SPI channel) and do not compete with camera DMA traffic on the PSRAM bus.

### 3. Private copy buffer in `WhoGestureRecognize`

The resize node rotates between `RESIZE_RINGBUF + 1` PSRAM buffers. Without a private copy, the gesture task could be reading a buffer while the resize node overwrites it. The constructor allocates a dedicated `m_copy_buf` (`480×480×2 = 460 KB`) in PSRAM and `memcpy`s the frame before calling inference.

### 4. Models preloaded before tasks start

`HandDetect` and `HandGestureRecognizer` are constructed in `app_main()` before any tasks are created (`WhoGestureRecognize::preload_models()`). This avoids a `StoreProhibited` crash that occurs when FreeRTOS lazy-loads a model for the first time inside a task that doesn't own the necessary memory regions.

### 5. LVGL / LCD removed entirely

The `esp32_s3_eye_noglib` BSP variant (no LVGL) is used. All LCD/display code was removed. Output is serial only. This freed ~800 KB of PSRAM that LVGL's frame buffer required.

### 6. Camera set to VGA (640×480), 3 frame buffers

`FRAMESIZE_VGA` with `CAM_FB_COUNT=3` is the minimum stable configuration.  
`WhoFetchNode` computes its ring buffer length as `fb_count - 2`, so at least 3 buffers are required to avoid an `assert failed: ringbuf_len >= 1` crash.

---

## sdkconfig changes (vs project defaults)

| Option | Value | Reason |
|---|---|---|
| `CONFIG_FREERTOS_UNICORE` | `y` | Fix EE SIMD coprocessor race in SMP FreeRTOS |
| `CONFIG_SPIRAM_RODATA` | *removed* | Reduce PSRAM bus contention during inference |
| `CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY` | `y` | Runtime stack overflow detection (debug aid) |
| `CONFIG_SPIRAM_MODE_OCT` | `y` | Required for 8 MB octal PSRAM on ESP32-S3-EYE |
| `CONFIG_ESP32S3_DATA_CACHE_64KB` | `y` | Larger D-cache improves PSRAM access latency |
| `CONFIG_ESP32S3_DATA_CACHE_LINE_64B` | `y` | 64-byte cache lines for PSRAM efficiency |
| `CONFIG_COMPILER_OPTIMIZATION_PERF` | `y` | `-O2` compilation for inference throughput |

---

## Dependencies (`main/idf_component.yml`)

```yaml
espressif/esp-dl: "3.2.4"                  # pinned — HandDetect + HandGestureRecognizer
espressif/hand_gesture_recognition: "*"
espressif/esp32_s3_eye_noglib: "~5.0"      # pinned — without LVGL
espressif/esp32-camera: "^2.0.13"          # explicit — newer BSP dropped implicit dep
espressif/who_task: "*"
espressif/who_frame_cap: "*"
```

---

## Memory layout at runtime

| Region | Usage |
|---|---|
| Internal DRAM (~310 KB) | FreeRTOS task stacks (32 KB each × 3 tasks), system data |
| PSRAM (~8 MB, ~4.4 MB free after models) | Camera frame buffers (3 × 630 KB), resize output buffers (2 × 460 KB), inference copy buffer (460 KB), model heap allocations (~900 KB) |
| Flash (8 MB) | Application code + model flatbuffer (in .rodata, not copied to PSRAM) |
