# ESP-IDF Migration Build Errors

Use this file when the first build on a newer ESP-IDF version fails and you need a concrete next step. Treat these mappings as triage guidance, then open the relevant official migration chapter before editing code.

## Linker and Build-System Errors

### Symptom

`orphan section` linker errors during the final link step.

### Check Next

- ESP-IDF 6.0 build-system migration guide online
- Custom linker fragments
- Recently added attributes, sections, or linker script changes

### Likely Fix Direction

- Remove unused code or data that lands in an unplaced section
- Add or fix a linker fragment so the section is placed explicitly
- Avoid downgrading `CONFIG_COMPILER_ORPHAN_SECTIONS` unless you understand why the section is orphaned

## Constructor and Startup Ordering

### Symptom

Tests, registries, or startup-time tables initialize in the wrong order after the version bump.

### Check Next

- ESP-IDF 6.0 build-system migration guide
- Code using constructors or registration macros

### Likely Fix Direction

- Audit assumptions about reverse constructor ordering
- Convert list insertion logic from head insertion to tail insertion where appropriate
- Use constructor priorities when ordering must be explicit

## Kconfig and Configuration Parsing

### Symptom

`Kconfig` parsing errors, invalid syntax, or config options that no longer behave as expected.

### Check Next

- ESP-IDF 6.0 build-system migration guide
- esp-idf-kconfig v2 to v3 migration guide

### Likely Fix Direction

- Update `Kconfig` syntax for esp-idf-kconfig v3 compatibility
- Remove outdated constructs instead of working around parser failures
- Re-run reconfigure after fixing project or component config definitions

## Warnings Promoted to Errors

### Symptom

Build breaks on warnings that did not fail before.

### Check Next

- ESP-IDF 6.0 build-system and toolchain migration guides
- Exact GCC warning names in the build log

### Likely Fix Direction

- Fix the code first where practical
- Use targeted suppression only when the warning is understood and the code is still valid
- Avoid broad suppression as the first response

## `sys/dirent.h` Function Declaration Failures

### Symptom

Errors such as:

```c
implicit declaration of function 'opendir'
```

when the source includes `<sys/dirent.h>`.

### Check Next

- ESP-IDF 6.0 toolchain migration guide

### Likely Fix Direction

- Replace `<sys/dirent.h>` with `<dirent.h>`

## Picolibc `sys/signal.h` Missing

### Symptom

Errors such as:

```c
fatal error: sys/signal.h: No such file or directory
```

with Picolibc enabled.

### Check Next

- ESP-IDF 6.0 toolchain migration guide
- Project libc configuration

### Likely Fix Direction

- Replace `<sys/signal.h>` with `<signal.h>`

## Legacy `driver` Dependency Breakages

### Symptom

Build errors after a component or app still depends on `driver`, but symbols or headers from specific drivers are no longer available transitively.

### Check Next

- ESP-IDF 6.0 peripherals migration guide
- Component `CMakeLists.txt`
- Main project `CMakeLists.txt`

### Likely Fix Direction

- Keep `driver` only where legacy APIs are still required
- Add explicit `esp_driver_*` dependencies for the actual peripherals in use
- Update `REQUIRES` and `PRIV_REQUIRES` instead of assuming old transitive behavior

## Removed or Moved Ethernet Driver APIs

### Symptom

Missing Ethernet PHY or SPI MAC constructors such as `esp_eth_phy_new_*` or `esp_eth_mac_new_*`.

### Check Next

- ESP-IDF 6.0 networking migration guide
- Whether the project now needs a Component Registry dependency

### Likely Fix Direction

- Add the replacement Ethernet driver package from the ESP Component Registry
- Update includes and component dependencies to match the externalized driver layout

## `sntp.h` Missing

### Symptom

Build fails because `sntp.h` no longer exists.

### Check Next

- ESP-IDF 6.0 networking migration guide

### Likely Fix Direction

- Replace `sntp.h` with `esp_sntp.h`

## Ping API Removal

### Symptom

Missing headers or symbols for:

- `esp_ping.h`
- `ping.h`
- `ping_init`
- `ping_deinit`
- `esp_ping_set_target`
- `esp_ping_get_target`
- `esp_ping_result`

### Check Next

- ESP-IDF 6.0 networking migration guide

### Likely Fix Direction

- Migrate to the socket-based ping API from `ping/ping_sock.h`

## Wi-Fi API or Enum Removals

### Symptom

Missing Wi-Fi enums, macros, headers, or functions after the version bump.

### Check Next

- ESP-IDF 6.0 Wi-Fi migration guide

### Likely Fix Direction

- Replace removed enums or macros with their direct modern equivalents
- Update renamed APIs instead of creating local compatibility wrappers immediately
- Audit Wi-Fi event structure field changes before reusing old parsing logic

## `idf.py size --format json` Script Breakage

### Symptom

CI or local tooling fails when parsing size output after the migration.

### Check Next

- ESP-IDF 6.0 tools migration guide
- Build scripts or CI scripts that parse `idf.py size`

### Likely Fix Direction

- Replace `idf.py size --format json` with `idf.py size --format json2`
- Update parsers for the newer hierarchical output structure

## `idf.py efuse*` Command Failures

### Symptom

`idf.py efuse*` commands fail because no port was specified.

### Check Next

- ESP-IDF 6.0 tools migration guide

### Likely Fix Direction

- Pass `--port <PORT>` explicitly or set `ESPPORT`

## Environment Baseline Failures

### Symptom

The migration cannot even reach application compile errors because tool execution fails first.

This can also show up as an early build warning that the project's declared CMake minimum is older than the expected ESP-IDF 6.0 baseline.

### Check Next

- Python version
- CMake version
- `eim list`
- active `idf.py --version`

### Likely Fix Direction

- Upgrade to a supported Python version
- Upgrade CMake to a supported version
- Check the top-level `CMakeLists.txt` and, if the build warns that the project minimum is too old, update it to `cmake_minimum_required(VERSION 3.22)`
- Use this top-level boilerplate shape as the reference:

```cmake
# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(hello_world)
```

- Treat this as a build-system migration fix to apply before deeper code triage
- Switch to the correct ESP-IDF 6.0 installation before changing app code
