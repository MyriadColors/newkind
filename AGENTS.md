# AGENTS.md — Elite: The New Kind (Raylib Port)

## Build

```sh
make          # builds raylib (auto-clones into deps/), then compiles newkind
make clean    # remove object files and binary
make clean-deps  # remove deps/raylib entirely
make distclean   # clean + clean-deps
```

- Requires **clang** (default CC) and **C23** (`-std=c23`).
- Raylib is fetched automatically from GitHub into `deps/raylib/` if missing, then built as a static lib.
- Platform detected from `$(OS)` / `uname -s`. Windows links `opengl32 gdi32 winmm`; Linux links `GL X11 pthread`; macOS uses Cocoa/IOKit frameworks.
- Output binary: `newkind.exe` (Windows) or `newkind` (Linux/macOS).

## Project Structure

Single flat C project — no subdirectories, no packages. All `.c`/`.h` files are in the repo root.

Key source files:
- `main.c` — game loop, input dispatch, screen management
- `game_state.c/.h` — central `GameState` singleton (`g_state`) holding all runtime state
- `space.c` — universe update, ship AI, docking, hyperspace
- `swat.c` — tactics (AI behavior, combat)
- `gfx.c` — rendering abstraction over Raylib (512×512 virtual coordinate space, `GFX_SCALE=2`)
- `threed.c` — 3D polygon rendering
- `elite.c` — galaxy generation, commander management, core constants
- `shipdata.c` — ship definitions and flight data tables
- `intro.c` — title screens
- `keyboard.c` — input abstraction

## State Access Pattern

Global state lives in `GameState g_state` (defined in `game_state.c`). Access via inline getters — not direct field access:
```c
get_session_state()->current_screen   // not g_state.session.current_screen
get_player_state()->current           // commander struct
get_config_state()->speed_cap         // config
```

`main.c` defines shorthand `#define` macros (`cmdr`, `docked`, `flight_speed`, etc.) that wrap these getters. Follow this pattern when adding new code.

## Code Conventions

- **C23 standard**: uses `nullptr`, `[[nodiscard]]`, `bool` from `<stdbool.h>`, `stdint.h` types.
- Screen constants are `SCR_*` defines (e.g. `SCR_FRONT_VIEW`, `SCR_GALACTIC_CHART`) in `elite.h`.
- Graphics coordinate system is 512×384 logical (512×512 virtual canvas with `GFX_SCALE=2`). All rendering uses `gfx_*` functions, not Raylib directly.
- Color constants are `GFX_COL_*` defines mapping to a palette (not RGB).
- Ship types are `SHIP_*` defines, universe objects are `struct univ_object` in `game_state.h`.

## Runtime Config

- `newkind.cfg` — game settings (loaded at startup by `read_config_file()`).
- `newscan.cfg` — scanner layout coordinates.
- Save files use `.nkc` extension.

## Assets

- `data/` — BMP sprites, PCX backgrounds, MIDI music files.
- Root `.wav` files — sound effects loaded at runtime.
- `scanner.bmp` — scanner overlay bitmap.
- `.gitignore` excludes `*.png`, `*.bmp`, `*.nkc`, `deps/`.

## Gotchas

- Adding a new `.c` file: add it to `OBJS` in the makefile **and** add a dependency line (the makefile has explicit per-file deps).
- The `deps/raylib/` directory is git-ignored and auto-cloned. First build takes longer.
- All game logic runs in a single-threaded loop tied to `speed_cap` (milliseconds per tick). Simulation ticks are rate-limited via `GetTime()` comparisons.
- Screen transitions clear the display (`gfx_clear_display()`). Don't assume buffer contents persist across frames.
