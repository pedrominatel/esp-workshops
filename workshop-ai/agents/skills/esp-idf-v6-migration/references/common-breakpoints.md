# Common Breakpoints Across ESP-IDF 5.x to 6.0 Migration

Use this file to map a failure symptom to the migration area to inspect next. Open the official migration guide chapter for the implicated version and subsystem before making changes.

## Build System

- Linker orphan section errors appear in 6.0 and often point to linker fragment or placement issues.
- Constructor execution order changed in 6.0 due to `__libc_init_array()`, which can break registration logic that assumed reverse ordering.
- Kconfig syntax changes matter in newer versions because ESP-IDF 6.0 uses esp-idf-kconfig v3.
- Default compiler warnings as errors in 6.0 can turn previously tolerated issues into hard build failures.

## GCC and Toolchain

- GCC changed repeatedly across 5.x and 6.0. Treat new warnings as migration work, not random noise.
- GCC 15 in 6.0 can expose include-guard issues, unterminated string initializers, and stricter header usage.
- Header assumptions may break:
  - use `<dirent.h>` instead of relying on `<sys/dirent.h>` for function prototypes
  - use `<signal.h>` instead of `<sys/signal.h>` when Picolibc is enabled
- Check Python and CMake baselines when the environment itself fails before application code builds.

## Components and Dependencies

- Legacy transitive dependencies become less reliable as ESP-IDF moves toward explicit component ownership.
- The legacy `driver` component no longer carries the same public dependencies in 6.0. Add the needed `esp_driver_*` dependencies explicitly.
- Some drivers and tools move out of the core tree and must be pulled in from the ESP Component Registry.
- If a previously in-tree feature disappears, check whether Espressif moved it to a standalone component before replacing it manually.

## Peripheral APIs

- Peripheral migrations often break at compile time because enums, config fields, or driver component boundaries changed.
- Projects that still depend on legacy drivers should be reviewed carefully before removing `driver`; some migrations require temporary coexistence plus explicit new dependencies.
- GPIO-sharing behavior and loopback-related assumptions may need review in newer peripheral drivers.

## Networking, Protocols, and Wi-Fi

- Removed headers or deprecated APIs are common breakpoints:
  - SNTP header changes
  - ping API removals
  - Wi-Fi enum and API removals or renames
  - ESP-NETIF iteration helper changes
- Check registry-backed or moved protocol components when a library appears to have vanished from the core IDF tree.
- If the build passes but runtime behavior changes, verify DHCP, DNS, Wi-Fi startup, and socket-related code paths first.

## System, Storage, and Security

- NVS, partition behavior, security defaults, or system APIs may shift over multiple 5.x releases, not only in 6.0.
- When the app depends on boot order, constructors, startup hooks, or low-level init flow, review system chapters for each intermediate migration guide.
- If storage or security failures appear only after boot, do not assume they are unrelated to the version jump.

## Tools and Environment

- ESP-IDF 6.0 raises environment expectations:
  - Python 3.10 or newer
  - CMake 3.22 or newer
- The first build may warn about an outdated project CMake baseline before compile errors become the main blockers.
- When that warning appears, align the top-level `cmake_minimum_required(...)` in `CMakeLists.txt` with `3.22` before deeper migration triage.
- `idf.py size` output format changes can break scripts that parse legacy JSON.
- `idf.py efuse*` commands now require an explicit serial port.
- If a CI workflow breaks after the version bump, audit the toolchain and helper command usage before changing application code.
