# ESP-IDF Migration Paths to 6.0

Use the official online migration guides as the primary source of truth. For 6.0-specific context, a local ESP-IDF 6.0 checkout can help locate examples and docs, but it does not replace the official guide chain. Run `eim list` first to discover installed ESP-IDF versions. If EIM is unavailable or the right 6.0 tree is still unclear, check `IDF_PATH` and the active `idf.py` environment. If the correct local 6.0 tree still cannot be identified reliably, ask the user to provide the path.

## Rule

If the application starts before 5.5, review every intermediate migration guide through 5.5 before applying the 5.5 to 6.0 guide.

## Starting Versions

- 5.5
  - Review: 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html

- 5.4
  - Review: 5.4 to 5.5, then 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.5/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html

- 5.3
  - Review: 5.3 to 5.4, 5.4 to 5.5, then 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.4/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.5/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html

- 5.2
  - Review: 5.2 to 5.3, 5.3 to 5.4, 5.4 to 5.5, then 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.3/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.4/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.5/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html

- 5.1
  - Review: 5.1 to 5.2, 5.2 to 5.3, 5.3 to 5.4, 5.4 to 5.5, then 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.2/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.3/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.4/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.5/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html

- 5.0
  - Review: 5.0 to 5.1, 5.1 to 5.2, 5.2 to 5.3, 5.3 to 5.4, 5.4 to 5.5, then 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.1/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.2/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.3/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.4/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.5/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html

- 4.4
  - Review: 4.4 to 5.0, 5.0 to 5.1, 5.1 to 5.2, 5.2 to 5.3, 5.3 to 5.4, 5.4 to 5.5, then 5.5 to 6.0
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.0/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.1/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.2/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.3/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.4/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.5/index.html
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-6.x/6.0/index.html
