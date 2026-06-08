# STEP 2 — Implement Camera Display

---

Implement the full camera-to-display application for the ESP32-S3-EYE board inside the project. At this step, the implemention must shopw a live stream from the camera on the display.

---

## Hardware initialization

All the hardware initialization must be according to the BSP. No changes to the hardware was done.

## Display

The display is based ST7789 with 240×240 px resolution with backlight.

### Display requiremtens

Key requirements:

- The backlight must be configured as `brightess` from 0% (off) to 100% (full).
- The display orientation must be configured manually.
- No auto rotation is needed at this step.

## Camera

Streaming implementation.

Key requirements:

- The camera sensor should outputs exactly 240×240 RGB565.

### Verification

Build, flash and monitor:

```sh
rm -f sdkconfig
idf.py build
idf.py flash monitor
```

A live 240×240 camera image should appear on the LCD with the power LED off.
