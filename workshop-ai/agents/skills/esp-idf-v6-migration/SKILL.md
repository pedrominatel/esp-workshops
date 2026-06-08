---
name: esp-idf-v6-migration
description: Migrate existing ESP-IDF application repositories from ESP-IDF 5.x to 6.0 using cumulative migration guides, version detection, build failure triage, and subsystem-specific fixes.
compatibility: Works with standard ESP-IDF command-line tools and official ESP-IDF migration guides.
---

# ESP-IDF v6 Migration

Use this skill when upgrading an existing ESP-IDF application repository from ESP-IDF 5.x to 6.0.

This skill is optimized for application repos, not standalone reusable components. Treat migration as cumulative: if the project starts before 5.5, review each intermediate migration guide up to 5.5 before applying the 5.5 to 6.0 migration guide.

## When to Use

- This migration skill is not recommended for projects that are based on older than ESP-IDF v4.4, as the migration path is too complex
- Migrating an application from ESP-IDF 5.5 to 6.0
- Migrating an application from 5.4, 5.3, 5.2, 5.1, or 5.0 by chaining intermediate 5.x guides first
- Migrating from 4.4 application forward through 5.0 and then cumulatively to 6.0
- Triaging build failures after switching the project to ESP-IDF 6.0
- Auditing API removals, driver moves, toolchain changes, and Kconfig/build-system breakages during migration

## Source Priority

- Use the official online ESP-IDF migration guides as the primary source of truth.
- Use a local ESP-IDF 6.0 checkout as supporting context for 6.0-only topics, file paths, and examples when one is available.
- Find a local install by running `eim list` first. If EIM is not available or the correct 6.0 install is still unclear, check `IDF_PATH` and the active `idf.py` environment. If the correct 6.0 checkout still cannot be identified reliably, ask the user to provide the path.
- For projects starting on older 5.x versions, do not compress the older migration history into guesses. Open the relevant official intermediate guides listed in [references/migration-paths.md](references/migration-paths.md).

## Workflow

### 1. Detect the current project baseline

- Confirm the current ESP-IDF version before proposing changes.
- Check `idf.py --version`, `eim list`, exported `IDF_PATH`, project docs, CI config, and lockfiles as needed.
- Confirm the project target and whether runtime validation on hardware is expected.
- If the starting version is unclear, stop and resolve that first.

### 2. Pick the cumulative migration path

- Use [references/migration-paths.md](references/migration-paths.md) to select the exact guide chain.
- Required rule:
  - 5.5 starts with the 5.5 to 6.0 guide
  - 5.4 and earlier must review every intermediate migration guide through 5.5, then 6.0
- Use [references/version-summary.md](references/version-summary.md) to decide which subsystem chapters to inspect first.

### 3. Switch the environment to the target IDF

- Move the app into an ESP-IDF 6.0 environment before judging compatibility.
- Confirm Python, CMake, and toolchain requirements before the first build.
- During the first build on ESP-IDF 6.0, check the log for CMake version warnings before triaging deeper compile failures.
- If the top-level `CMakeLists.txt` still declares an older minimum, update it to `cmake_minimum_required(VERSION 3.22)` as an early build-system migration step.
- Prefer running the first build without broad source edits so the failure set reflects real migration blockers.

### 4. Run an initial build and categorize failures

- Build once on the target ESP-IDF version.
- Review warnings as well as hard failures during this first build, especially CMake baseline warnings that should be resolved before subsystem-level edits.
- Group failures before editing:
  - build system or linker
  - GCC or header/toolchain
  - removed or moved components
  - components not suppoprted
  - driver dependency splits
  - subsystem API changes
  - config and Kconfig syntax issues
- Use [references/common-breakpoints.md](references/common-breakpoints.md) to map symptoms to likely migration areas.
- Use [references/build-errors.md](references/build-errors.md) when the build log already points to a concrete missing header, removed API, linker failure, or tool behavior change.

### 5. Fix migration issues by subsystem

- Build system:
  - check CMake version warnings and align the top-level `cmake_minimum_required(...)` with `3.22` when the project still pins an older baseline
  - check orphan section errors
  - check constructor-order assumptions
  - check Kconfig v3 syntax compatibility
  - check warnings-as-errors defaults
- Toolchain:
  - check GCC 15 warnings and promoted errors
  - replace outdated headers such as `sys/dirent.h` where needed
  - account for Picolibc header differences if enabled
- Components and dependencies:
  - replace removed in-tree drivers with Component Registry dependencies where required
  - update `REQUIRES` and `PRIV_REQUIRES` explicitly when legacy transitive dependencies disappear
  - review `driver` usage and split dependencies to `esp_driver_*` components when needed
- Subsystems:
  - review the relevant migration chapters for networking, peripherals, protocols, Wi-Fi, storage, system, security, provisioning, and Bluetooth
  - do not apply mechanical edits outside the subsystem actually implicated by build or runtime evidence
  - do not edit external components or apply any patches. Patches can only be appied if the user explicitly confirms to do so

### 6. Rebuild and validate behavior

- Rebuild after each coherent migration batch, not after every small edit.
- If firmware behavior changed and hardware is available, flash and capture monitor logs on the target board.
- Compare runtime behavior against the pre-migration app expectations, especially for networking, storage, and peripheral initialization.
- Report what was validated and what was not.

## References

- Migration path matrix: [references/migration-paths.md](references/migration-paths.md)
- Common migration breakpoints: [references/common-breakpoints.md](references/common-breakpoints.md)
- Build error symptom map: [references/build-errors.md](references/build-errors.md)
- Version coverage summary: [references/version-summary.md](references/version-summary.md)

## Avoid

- Jumping straight from 5.4 or earlier to 6.0 without reviewing intermediate migration guides
- Treating a local ESP-IDF 6.0 checkout as more authoritative than the current official migration docs
- Suppressing warnings or linker errors before understanding the underlying breakage
- Assuming legacy `driver` dependencies or moved components still arrive transitively
- Making broad unrelated cleanups while the migration error set is still being established
