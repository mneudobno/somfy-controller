# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Deploy

```bash
uv run ufbt          # Build → produces dist/somfy_rts.fap
uv run ufbt launch   # Build, deploy to connected Flipper, and launch
```

Uses `uv` to manage Python/ufbt. The Flipper SDK lives at `~/.ufbt/current/`.

## Architecture

Flipper Zero app implementing the Somfy RTS radio protocol for controlling motorized blinds. Written in C against the Flipper firmware SDK.

### Scene-based navigation

The app uses Flipper's `SceneManager` + `ViewDispatcher` pattern. Four scenes, each with `on_enter`/`on_event`/`on_exit` callbacks:

- **Menu** (`scenes/somfy_rts_scene_menu.c`) — Submenu listing blinds. Short-press selects, long-press deletes. Shows "All Blinds" when 2+ blinds exist. Uses `submenu_add_item_ex` for extended input type handling.
- **Control** (`scenes/somfy_rts_scene_control.c`) — Raw `View` (not Widget) with custom draw/input callbacks. Needed for direct Up/Down/OK key handling. The view model stores an `SomfyRtsApp*` pointer, so the draw callback dereferences as `*(SomfyRtsApp**)model`.
- **Add** (`scenes/somfy_rts_scene_add.c`) — TextInput for blind name. Generates random 24-bit address.
- **Confirm Delete** (`scenes/somfy_rts_scene_confirm_delete.c`) — DialogEx confirmation.

Scene handlers are registered as arrays in `somfy_rts.c`, indexed by `SomfyRtsScene` enum.

### Protocol layer (`somfy_rts_protocol.c`)

Implements Somfy RTS frame encoding and async TX via CC1101 at 433.42 MHz OOK. Key details:
- 7-byte frame: key, cmd|checksum, rolling code (BE), address (LE)
- Checksum = XOR all nibbles (computed before obfuscation)
- Obfuscation = **forward** rolling XOR (`frame[i] ^= frame[i-1]` for i=1..6)
- Manchester encoding: 640µs half-symbol, MSB first
- TX uses `subghz_devices_start_async_tx` with a state-machine callback returning `LevelDuration` values
- First frame has 2-cycle preamble + wake-up pulse; repeats (2 more) have 7-cycle preamble, no wake-up

### Storage (`somfy_rts_store.c`)

Blind configs persisted to `APP_DATA_PATH("config.txt")` using FlipperFormat (key-value text). Rolling code is saved after every transmission.

### Key constants

- `SOMFY_RTS_ALL_BLINDS (0xFF)` — special `selected_blind` value for all-blinds mode
- `SOMFY_RTS_MAX_BLINDS (16)` — fixed array, no dynamic allocation
- PROG command disabled in all-blinds mode (pairing must be per-blind)
