---
name: esp-idf-skills
description: ESP32 firmware development skills including build/flash/monitor workflows, EIM CLI setup, ESP-IDF code conventions, and Component Registry API usage.
compatibility: Works with standard ESP-IDF command-line tools.
---

# ESP-IDF Firmware Engineering

This skill provides a structured framework for developing, compiling, and debugging ESP32 applications. It ensures the environment is correctly targeted to the hardware and provides a robust path for capturing and analyzing device logs.

## When to Use

- Developing new ESP-IDF based firmware applications for ESP32-series chips.
- Troubleshooting compilation errors or dependency issues in ESP-IDF projects.
- Flashing firmware to physical hardware and verifying runtime behavior.
- Debugging run-time crashes using the decoded monitor output.

## Instructions

### 1. Environment & Chip Identification
- **Load the Environment First:** Before any `idf.py` command, ensure the ESP-IDF environment is loaded in the current shell session.
- **Check Version First:** Run `idf.py --version` at the start of a session or inspect the exported `IDF_PATH`. ESP-IDF APIs change significantly between versions (e.g., v4.4 vs v5.x).
- **Targeting:** Use `esptool.py chip_id` to identify the hardware when a device is connected, or use `idf.py set-target <TARGET>` based on confirmed hardware or project context. If the device is not present, ask the user to connect the device first before flashing.
- **Port Management:** If multiple ESP32s are connected, explicitly ask the user which port to use rather than guessing.

#### 1.1 Extract Target Details from `idf.py` / `esptool.py chip_id`
- **Primary hardware probe command:**
  - `esptool.py chip_id`
- **Parse these lines from `chip_id` output:**
  - `Detecting chip type... <TARGET>` -> detected target family
  - `Chip is <CHIP_NAME> (<PACKAGE>) (revision vX.Y)` -> chip variant + package + revision
  - `Features: ... Embedded Flash <SIZE> (<VENDOR>)` -> radio/features + flash package details
  - `Crystal is <FREQ>` -> XTAL frequency
  - `USB mode: <MODE>` (if present) -> active USB transport mode
  - `MAC: xx:xx:xx:xx:xx:xx` -> unique device identity
  - `Warning: ... has no Chip ID. Reading MAC instead.` (if present) -> expected behavior on some targets
- **Use this generic reference pattern:**
  - `esptool.py v<version>`
  - `Detecting chip type... <TARGET>`
  - `Chip is <CHIP_NAME> ... (revision vX.Y)`
  - `Features: ...`
  - `Crystal is <FREQ>`
  - `MAC: xx:xx:xx:xx:xx:xx`
  - `Hard resetting via RTS pin...`
- **From `idf.py build` logs (project target):**
  - Look for: `Building ESP-IDF components for target <chip>`
  - Example: `Building ESP-IDF components for target <target>`
- **From boot logs in monitor (chip revision and flash mode):**
  - Look for:
    - `boot: chip revision: vX.Y`
    - `boot.esp32xx: SPI Flash Size : <size>`
    - `boot.esp32xx: SPI Mode       : <mode>`
    - `boot.esp32xx: SPI Speed      : <freq>`
- **From app startup logs (compiled app metadata):**
  - Prefer standard ESP-IDF startup metadata when available, for example:
    - `I (...) cpu_start: App version: ...`
    - `I (...) cpu_start: ELF file SHA256: ...`
    - `I (...) cpu_start: ESP-IDF: ...`
  - If the project emits custom logs such as `app_init: Project name: ...`, include them as app-specific metadata rather than treating them as required.
- **Report all four layers together when possible:**
  1. Project target (`idf.py build`)
  2. Physical chip identity (`esptool.py`)
  3. Boot-time hardware config (monitor logs)
  4. App metadata (standard startup logs and any app-specific metadata)
  - This avoids mismatches between selected target and connected hardware.

### 2. The Build-Clean Cycle
- **Incremental Builds:** Use `idf.py build` for standard changes.
- **When to Clean:** If the build fails with cryptic CMake errors or after changing major configuration options in `sdkconfig`, use `idf.py fullclean` before rebuilding. If cleanup still fails, verify the project path and `CMakeLists.txt` first. Do not delete `build/` or perform other destructive cleanup without user approval.
- **Log Diagnostics:** If a build fails, do not just report the failure. Inspect the `idf.py build` output and any generated log paths for error count, warning count, and specific error locations.
- **Mandatory Runtime Verification Per Interaction:** After every successful build in an interaction, if a board is detected, immediately run flash + monitor and capture logs.
  - Required command flow: `idf.py -p <PORT> flash monitor`
  - If no board is connected, explicitly state that flashing/log capture was skipped due to missing hardware.

### 3. Flashing
- **Serial Conflict Prevention:** Use `idf.py -p <PORT> flash`. Provide the port if necessary.

### 4. Monitoring & Crash Analysis
- **Don't Use Raw Serial for Crashes:** Use `idf.py -p <PORT> monitor`, which automatically decodes addresses in backtraces into file names and line numbers.
- **The Monitor Lifecycle:**
    1. Start with `idf.py -p <PORT> monitor` and capture the output in the terminal or a log file for analysis.
    2. Send commands only if the firmware exposes a UART console and the current terminal setup supports interactive input.
    3. Let the device run until the target behavior/crash is observed.
    4. Stop the monitor process cleanly and summarize the captured output.
- **Ignore "escape sequences not supported" warning:** When monitoring a firmware that uses the ESP-IDF console component (linenoise), the log will contain a message like "Your terminal application does not support escape sequences. Line editing and history features are disabled." This is expected and harmless. It only disables terminal-side line editing features and should not be treated as a firmware error.

## Best Practices
- **Path Safety:** Always resolve project paths to absolute paths.
- **Memory Safety:** When memory issues are suspected, enable heap tracing before building and running a monitor session to do memory analysis.
- **Error Handling:** If `esptool.py chip_id` fails, verify the device is in "Download Mode" (usually by holding the BOOT button while resetting).
- **Error Handling:** If `esptool.py chip_id` fails, verify the device is in "Download Mode" (usually by holding the BOOT button while resetting).

## Install ESP-IDF with EIM CLI
- Prefer EIM CLI for installing and managing ESP-IDF versions.
- Install EIM CLI with the platform package manager:
  - macOS (Homebrew): `brew tap espressif/eim && brew install eim`
  - Debian/Ubuntu: add Espressif APT repo, then `sudo apt install eim-cli`
  - Fedora/RHEL: add Espressif RPM repo, then `sudo dnf install eim-cli`
  - Windows: `winget install Espressif.EIM-CLI`
- Basic install flow with EIM:
  - Install ESP-IDF: `eim install -i <ESP_IDF_VERSION>`
  - List installed versions: `eim list`
  - Select active version: `eim select <ESP_IDF_VERSION>`
  - Run project commands in selected environment: `eim run "idf.py build"`
- For interactive setup, use: `eim wizard`
- For reproducible or CI installs, prefer non-interactive mode (default): `eim install ...`
- If `eim` is unavailable in `PATH`, fix the installation or shell `PATH` first. Once the binary is discoverable, run `eim --help` to verify the installation before continuing.

## ESP-IDF Code Conventions
- Entry point must remain `void app_main(void)`.
- Prefer ESP-IDF APIs and includes over ad-hoc platform code.
- Use `ESP_LOGI/W/E` logging for non-trivial applications instead of excessive `printf`, unless user requests otherwise.
- Keep components explicit via `idf_component_register(...)` and update `REQUIRES`/`PRIV_REQUIRES` when adding dependencies.
- Favor small, testable functions and avoid blocking long operations in a single task without yielding/delay where appropriate.

## ESP Component Registry REST API

> Use when searching the ESP Component Registry for components, examples, or documentation via HTTP API. Applies when the user asks to find ESP-IDF components, browse examples, or fetch component metadata programmatically.

The registry exposes a public REST API at `https://components.espressif.com/api/`. No authentication required.

## Key endpoints

**Search components:**
```
GET /api/components?q={query}&page=1&per_page=10
```
Returns `name`, `namespace`, `description`, `license`, `downloads`, and a `versions` list per result.

**Component detail (includes examples):**
```
GET /api/components/{namespace}/{name}
```
Returns full metadata including an `examples` array. Each example has `name`, `description`, `url` (`.tgz` archive), and a `docs.readme` URL.

**Example README:**
```
GET https://components-file.espressif.com/components/{namespace}/{name}/{version}/examples/{example_path}/readme.md
```

## Example queries

Find Kalman filter components:
```
GET /api/components?q=kalman&page=1&per_page=10
```

Get all examples for `espressif/esp-dsp`:
```
GET /api/components/espressif/esp-dsp   # inspect .examples[] in response
```

## Notes

- Use the REST API when you need raw example URLs, download links, or component metadata programmatically.
- If a search returns more than one plausible component, do not choose automatically. Present the candidates and ask the user to select one before proceeding.
- Components are added to projects with: `idf.py add-dependency "namespace/component^version"`
