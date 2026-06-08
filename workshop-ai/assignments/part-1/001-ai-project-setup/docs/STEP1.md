# STEP 1 — Project Scaffold

---

At this very first step, the project scaffold must be created without any other hardware initialization.

---

## Create the project scaffold

Create a new ESP-IDF project named `esp32s3-eye-display` for the ESP32-S3-EYE development board.
Conside the `REQUIREMENTS.md` file as the base of this first step.

## Add all the necessary components from Registry

Any addtional component must be included in the `main/isdf_component.yml`.

## Hardware initialization

No additional hardware initialization is required.

### Verification

After creating all files, set the target and build:

```sh
idf.py set-target esp32s3
idf.py build
```

The build must complete without errors. Warnings are acceptable.

After the build, the project must be flashed and monitored without any errors.

```sh
idf.py flash monitor
```

Monitor for maximum of 15 seconds.

> **Important:** If a `sdkconfig` file already exists in the project root, delete it before building so that `sdkconfig.defaults` is applied cleanly.

Also, run `idf.py reconfigure` if any change in the SDKConfig is done or if any new component is added.

Verify on the log output if the flash memory is the selected one. If needed, create the `sdkconfig.defaults`.
