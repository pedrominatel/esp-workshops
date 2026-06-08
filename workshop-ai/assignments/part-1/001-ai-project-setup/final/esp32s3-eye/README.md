# esp32s3-eye

ESP-IDF project scaffold for the ESP32-S3-EYE board.

## Toolchain

Use ESP-IDF v5.5.2 from `/Users/pedrominatel/esp/v5.5.2/esp-idf`.

## Build

From the project directory:

```sh
source /Users/pedrominatel/esp/v5.5.2/esp-idf/export.sh
idf.py set-target esp32s3
idf.py build
```

## Notes

- The default target is `esp32s3`.
- `build/` and generated SDK config files are ignored by git.
