# Somfy RTS Controller for Flipper Zero

Control Somfy RTS motorized blinds directly from your Flipper Zero. Supports up to 16 blinds with individual Up/Down/Stop controls and persistent rolling codes.

## Features

- Individual control of multiple Somfy RTS blinds
- Up / Down / Stop commands via hardware buttons
- PROG command for pairing (long-press OK)
- Rolling codes persisted on SD card across reboots
- Works alongside your existing Somfy remote

## Installation

### Prerequisites

- [uv](https://docs.astral.sh/uv/) package manager
- Flipper Zero connected via USB

### Build & Deploy

```bash
uv run ufbt          # Build
uv run ufbt launch   # Deploy and launch on Flipper
```

The app appears under **Sub-GHz > Somfy RTS** on the Flipper.

## Pairing a Blind

Each blind must be paired with the Flipper individually. The Flipper acts as an additional controller — your existing remote continues to work.

1. In the app, select **Add Blind** and enter a name
2. On your **existing Somfy remote**, long-press **PROG** (~3 seconds) until the motor jogs
3. In the app, select the new blind and **long-press OK** to send the PROG command
4. The motor jogs again — pairing is complete

Repeat for each blind.

## Controls

| Button | Action |
|--------|--------|
| UP | Open blind |
| DOWN | Close blind |
| OK | Stop / My |
| Long OK | PROG (pairing) |
| Back | Return to menu |

## How It Works

The app implements the Somfy RTS (Remote Technology Somfy) protocol:

- **Frequency**: 433.42 MHz OOK via CC1101
- **Frame format**: 7-byte encrypted frame with rolling code
- **Encoding**: Manchester encoding with 640us symbol time
- **TX sequence**: Wake-up pulse, preamble, hardware sync, data frame (repeated 3x)

Configuration is stored at `/ext/apps_data/somfy_rts/config.txt` on the SD card.

## Project Structure

```
├── application.fam            # App manifest
├── somfy_rts.c/h              # Entry point, app lifecycle
├── somfy_rts_protocol.c/h     # RTS frame encoding & radio TX
├── somfy_rts_store.c/h        # Blind config persistence
└── scenes/
    ├── somfy_rts_scene.h          # Scene declarations
    ├── somfy_rts_scene_menu.c     # Blind list menu
    ├── somfy_rts_scene_control.c  # Control screen
    └── somfy_rts_scene_add.c      # Add new blind
```
