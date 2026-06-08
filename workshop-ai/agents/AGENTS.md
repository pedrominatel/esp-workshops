---
name: esp-idf-agent-instructions
description: Repository-specific coding agent rules for ESP-IDF projects. Covers environment setup, build/flash workflows, component management, Kconfig conventions, and safety guidelines.
compatibility: Applies to all coding agents working in this ESP-IDF repository.
---

# ESP-IDF Agent Instructions

These instructions apply to this repository and should be followed by coding agents working on ESP-IDF projects here.

## Environment and Tooling
- Always run ESP-IDF commands from the project directory, unless explicitly needed elsewhere.
- If no ESP-IDF project is detected in the current folder, first locate the correct project root in the workspace. Only create a new project scaffold when the user explicitly asks for a new project or the task clearly requires creating one.
- Before `idf.py` commands, ensure ESP-IDF environment is loaded (for example, by sourcing the project/user ESP-IDF export script in the shell session).
- Prefer `idf.py` workflows over calling CMake/Ninja directly.
- Create the project scaffold by using the command `idf.py create-project <PROJECT-NAME>`
- For documentation, always use the Documentation MCP Server if available.
- When searching ESP-IDF documentation, always use docs that match the specified ESP-IDF version for the task.
- When converting APIs or checking API compatibility, always review the ESP-IDF migration guides for the specified version.

## Build and Run Commands
- Build: `idf.py build`
- Clean build artifacts (when needed): `idf.py fullclean`
- Set/confirm target: `idf.py set-target <TARGET>`
- Flash: `idf.py -p <PORT> flash`
- Serial monitor: `idf.py -p <PORT> monitor`
- Flash + monitor: `idf.py -p <PORT> flash monitor`

Notes:
- Do not assume serial port; detect it or ask the user when required.
- Default monitor baud is typically `115200`.
- After changing target with `idf.py set-target`, run `idf.py fullclean` before `idf.py build` when the cleanup is appropriate for the task and permitted by the safety rules below.
- When an interaction includes a successful build for firmware-affecting changes and a board is detected/connected, the agent should also flash and capture monitor logs.
  - Required verification flow: `idf.py -p <PORT> flash monitor`
  - If hardware verification is not relevant to the change, state that it was intentionally skipped.
  - If no board is present, explicitly report that flash/log capture could not be performed.

## Editing Rules
- Edit source files in `/main/`, `/components/`, and project CMake files as needed.
- Do not manually edit generated/build outputs under `/build/`.
- Do not commit transient logs or generated binaries unless the user explicitly asks.
- Treat `sdkconfig` changes as intentional configuration updates: keep them minimal and explain why they changed.
- Only edit `sdkconfig` directly when explicitly requested by the user.
- After any direct `sdkconfig` edit, run `idf.py reconfigure`.

## Default Configuration Files (`sdkconfig*`) Rules
- Prefer storing default values in `sdkconfig.defaults` (or target-specific files such as `sdkconfig.defaults.esp32c6`) instead of manually maintaining `sdkconfig`.
- Treat `sdkconfig` as generated/resolved configuration for the current environment.
- Only change `sdkconfig` directly when the user explicitly requests a direct edit.
- When a user asks to change default configuration values, update `sdkconfig.defaults*` first, then run:
  - `idf.py reconfigure`
- Keep defaults deterministic and minimal:
  - add only required keys
  - avoid unrelated key churn
  - keep target-specific defaults separated by target file when applicable (for example, `sdkconfig.defaults.esp32c6`)

## Adding New Components
- Before creating a new component
  - Search the ESP Component Registry using the public REST API or the registry website to check whether an existing component already fits the need.
- Before creating a new component, search for the components manifest file inside `main/` (for example `main/idf_component.yml`) and check whether the component should be added there first.
- If the agent cannot search for components, ask the user to manually add it to the components manifest or share the component link to be added to the manifest file.
- Use this structure when adding a new component:
  - `components/<component_name>/CMakeLists.txt`
  - `components/<component_name>/include/<component_name>.h`
  - `components/<component_name>/<component_name>.c` (and additional source files as needed)
- Local/private component:
  - Add it under the project `components/` directory.
  - Expose public headers through `include/`.
  - Register sources/includes with `idf_component_register(...)`.
  - Add `REQUIRES`/`PRIV_REQUIRES` explicitly when depending on other components.
- Published/reusable component:
  - Keep component self-contained and ready for registry/reuse.
  - Include required metadata and docs:
    - `idf_component.yml`
    - `README.md`
    - `LICENSE`
    - `Kconfig` and/or `Kconfig.projbuild` when configuration options are needed
  - Include quality/automation assets for CI/CD:
    - Component-level tests (for example under `test/` or `test_apps/`)
    - A CI pipeline definition (`.gitlab-ci.yml` or equivalent workflow used by the target repo)
    - Formatting/lint configuration used by the project
  - Ensure CI covers at least: build, lint/format checks, and tests for supported ESP-IDF targets.

## Modularity Rules
- Use components to keep projects modular.
- Split self-contained functionality (for example: sensor drivers, protocol handlers) into separate ESP-IDF components instead of a single monolithic app to improve reuse and maintenance.
- If a component is planned to be shared across multiple applications, create it as a standalone project/repository with examples, rather than only inside one app.

## Kconfig.projbuild Rules
- Use `Kconfig.projbuild` only for project-level menu entries that must appear at the top/project scope in `menuconfig`.
- Place `Kconfig.projbuild` in the component root: `components/<component_name>/Kconfig.projbuild`.
- Prefer regular `Kconfig` for component-local options; do not move options to `Kconfig.projbuild` unless project-scope visibility is required.
- If the user asks to change a project-scope Kconfig value, apply the change in `Kconfig.projbuild`; use component `Kconfig` for component-local values.
- For HAL definitions, including peripheral GPIO assignments, define values in Kconfig and reference them in code; avoid hardcoding these values directly in source files.
- Keep option names namespaced (for example: `MY_COMPONENT_*`) to avoid symbol collisions.
- Every new config option must include:
  - clear prompt text
  - sensible default
  - help text explaining impact and valid values
- After changing any value in `Kconfig.projbuild`, run `idf.py reconfigure` before build/testing.

## Validation Checklist (Before Hand-off)
- Build succeeds with `idf.py build`.
- If the change affects firmware behavior and a board is connected, flash and capture runtime logs in the same interaction (`idf.py -p <PORT> flash monitor`).
- If code behavior changed, provide exact flash/monitor command used for verification.
- Summarize any configuration changes (especially `sdkconfig`, partition table, or target settings).
- Report anything not validated (for example, if flashing hardware was not available).

## Safety and Collaboration
- Ask before destructive actions (for example: `idf.py fullclean`, deleting files, rewriting configs broadly) unless the user explicitly requested that action or these instructions already require it for the task.
- If unexpected unrelated workspace changes are detected, pause and ask how to proceed.
- Keep changes narrowly scoped to the user request.
- Do not hardcode sensitive information in source or config files (for example: SSID, passwords, API keys, certificates, private keys, tokens, or device secrets).
- Use placeholders, `sdkconfig`/`sdkconfig.defaults*`, environment variables, or secure provisioning flows instead of committing real secrets.

## Documentation Recommendation
- Recommend adding a `README.md` for every project and reusable component.
- At minimum, the README should include:
  - purpose/overview
  - build and run/flash instructions
  - configuration notes (`sdkconfig.defaults*`, target-specific details)
  - usage examples and dependencies (for reusable components)
- If the project is hosted on GitHub, recommend adding a `.gitignore` to exclude transient logs, build artifacts, and generated binaries.

## Additional Recommendations
- Keep builds reproducible: commit and maintain `dependencies.lock` when using the ESP-IDF Component Manager.
- Require explicit handling of `esp_err_t` results; avoid ignoring return codes from ESP-IDF APIs.
- Review partition table and OTA/rollback strategy whenever application size or update flow changes.
- Define logging policy per build type (more verbose in Debug, reduced in Release).
