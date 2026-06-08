# Simple Camera Example

Captures frames from the board camera and displays them on the LCD.

### Pipeline

```
Camera -> Frame capture -> LCD display
```

### Supported boards

- ESP32-S3-EYE (`sdkconfig.bsp.esp32_s3_eye`)
- ESP32-S3-KORVO-2 (`sdkconfig.bsp.esp32_s3_korvo_2`)
- ESP32-P4-Function-EV-Board (`sdkconfig.bsp.esp32_p4_function_ev_board`)

### Build

Set `IDF_EXTRA_ACTIONS_PATH` to your esp-who `tools` directory, then configure for your board:

```bash
idf.py -D SDKCONFIG_DEFAULTS=sdkconfig.bsp.esp32_s3_eye set-target esp32s3 build
```
