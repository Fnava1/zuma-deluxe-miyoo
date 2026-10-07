# Zuma Deluxe — Miyoo Mini & Mini+ Port (OnionOS)

[![Platform](https://img.shields.io/badge/Platform-Miyoo%20Mini%20%2F%20Plus-blue.svg)](https://onionui.github.io/)
[![OS](https://img.shields.io/badge/OS-OnionOS-red.svg)](https://onionui.github.io/)
[![Performance](https://img.shields.io/badge/Performance-60%20FPS%20Stable-brightgreen.svg)]()
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

An optimized, native ARM port of **Zuma Deluxe** for the **Miyoo Mini** and **Miyoo Mini Plus** running **OnionOS**.

This port runs at a rock-solid **60 FPS**, includes full gamepad and analog stick support with sub-pixel aiming, ergonomic shoulder trigger controls, tracker-based background music via `libxmp-lite`, and native integration with OnionOS.

> **IMPORTANT LEGAL NOTICE:** This repository does **NOT** contain any copyrighted game assets, audio files, textures, or level data. You must provide your own legally owned copy of **Zuma Deluxe for PC** (available on [Steam](https://store.steampowered.com/app/3330/Zuma_Deluxe/), EA App, or original CD-ROM) to play.

---

## Features

- **60 FPS Performance**: Optimized ARM Cortex-A7 NEON compilation with hardware texture pools and direct framebuffer presentation via Miyoo Mini's native SDL2 driver.
- **Precision Aiming**: Analog stick support with calibrated deadzones, velocity curves, and D-Pad digital directional controls.
- **Ergonomic Frog Rotation**: Shoulder buttons configured for intuitive aiming:
  - **L Button**: Rotate counter-clockwise.
  - **R Button**: Rotate clockwise.
- **Accurate Cursor Hotspot**: Mouse and gamepad cursor coordinates precisely aligned with clickable UI elements and dialog buttons.
- **Full Soundtrack**: High-fidelity multi-channel audio powered by `libxmp-lite`, playing the iconic soundtrack without CPU overhead.
- **Dual Launcher Support**: Compatible with both OnionOS **Apps** (`App/ZumaDeluxe`) and OnionOS **Ports Collection** (`Roms/PORTS`).
- **Clean System Integration**: Auto-detects OnionOS CPU clock scaling (`1700 MHz`), isolates `audioserver` during gameplay, and cleans up upon exit.

---

## Controls

| Button | In-Game Action | Menu / Dialogs |
| :--- | :--- | :--- |
| **D-Pad / Analog Stick** | Aim frog | Move cursor |
| **A Button** | Shoot ball | Select / Click |
| **B Button** | Swap ball with frog's next color | Back / Cancel |
| **L Shoulder** | Rotate frog counter-clockwise | — |
| **R Shoulder** | Rotate frog clockwise | — |
| **Start** | Pause menu | Confirm |
| **Select** | Options dialog | — |
| **Menu Button** | OnionOS GameSwitcher / Home | Home |

---

## Requirements

1. A **Miyoo Mini** (v2/v3) or **Miyoo Mini Plus** running **OnionOS v4.x** or newer.
2. A legitimate copy of **Zuma Deluxe for PC** (Steam, PopCap CD, or EA App).
3. Python 3.8+ on your computer (for asset preparation).

---

## Quick Setup (Recommended)

An automated script [`tools/prepare_assets.py`](tools/prepare_assets.py) is provided to unpack your PC files, convert the soundtrack, and generate a ready-to-flash SD card archive in one command.

### Step 1: Run the Release Builder
The retail PC version of Zuma Deluxe ships with loose asset folders (`fonts`, `images`, `levels`, `music`, `properties`, `sounds`) and does not include a `main.pak`. 

On the Miyoo Mini, reading hundreds of loose files from an SD card causes severe I/O lag. The included release builder automatically converts the soundtrack, bundles the assets into an optimized `main.pak`, and builds the final ZIP:

```bash
# Modo 1 (Rápido): Usar el binario precompilado incluido (sin necesidad de Docker):
python tools/build_release.py "C:\Program Files (x86)\Steam\steamapps\common\Zuma Deluxe"

# Modo 2 (Desarrollador): Recompilar desde el código fuente C++ con Docker:
python tools/build_release.py "C:\Program Files (x86)\Steam\steamapps\common\Zuma Deluxe" --build
```

The script will:
1. Verify all required PC folders (`fonts`, `images`, `levels`, `properties`, `sounds`, `music`).
2. Convert the proprietary soundtrack (`music/zuma.mo3`) to Impulse Tracker format (`music/zuma.it`) using UNMO3.
3. Pack the loose asset folders into an optimized PopCap container (`main.pak`).
4. Assemble both OnionOS layouts (`App/` and `Roms/PORTS/`) with the precompiled `Zuma` ARM binary and hardware libraries.
5. Generate the flash-ready archive: `dist/Zuma_Deluxe_MiyooMini.zip`.

### Step 2: Copy to MicroSD Card
- Extract `dist/Zuma_Deluxe_MiyooMini.zip` directly to the **ROOT** of your Miyoo Mini microSD card.
- Insert the card into your Miyoo Mini and launch **Zuma Deluxe** from:
  - **Main Menu -> Apps -> Zuma Deluxe**, OR
  - **Main Menu -> Games -> Expert / Ports -> Zuma Deluxe**.

---

## Manual Installation Guide

If you prefer to organize the files manually, see [`INSTALL.md`](INSTALL.md) for step-by-step instructions.

In brief:
1. Convert `music/zuma.mo3` to `music/zuma.it` using `unmo3 -y zuma.mo3 zuma.it`.
2. Copy the folders (`fonts`, `images`, `levels`, `music`, `properties`, `sounds`, `userdata`) and `main.pak` to:
   - `SDCARD/App/ZumaDeluxe/` (for App view), OR
   - `SDCARD/Roms/PORTS/Games/Zuma Deluxe/` (for Ports view).
3. Copy the compiled `Zuma` binary and launcher files from [`packaging/`](packaging/).

---

## Building from Source

To compile the native ARM binary yourself using the official Miyoo Mini Docker toolchain:

### Using Docker (Linux / macOS / Windows)

```bash
# Linux / macOS
chmod +x tools/build_docker.sh
./tools/build_docker.sh

# Windows (Command Prompt / PowerShell)
tools\build_docker.bat
```

For full compilation details, CMake flags, and manual toolchain setups, refer to [`BUILDING.md`](BUILDING.md).

---

## Performance Tuning Details

This port incorporates several hardware-specific configurations developed for the Ingenic/SigmaStar SSD202D SoC:
- **CPU Clock**: Governed at 1700 MHz via `/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`.
- **Texture Pool**: `SDL_MMIYOO_TEXTURE_POOL=1` allocates dedicated video memory pools for instant blits without allocation overhead.
- **Framerate Unlock**: `SDL_MMIYOO_VSYNC_MODE=off` prevents double-buffering stalls, ensuring steady 60 FPS update cycles.
- **Sound Isolation**: Temporarily suspends MainUI's `audioserver` to provide exclusive ALSA direct access to SDL2.

---

## Credits & Acknowledgements

- **PopCap Games / Electronic Arts**: Creators of the original masterpiece *Zuma Deluxe*.
- **PopCap Games Framework Team**: Original open-source release of SexyAppFramework.
- **libxmp Team**: High-performance tracker playback library.
- **Un4seen Developments**: UNMO3 decompression utility.
- **OnionOS Community**: For developing and maintaining the best retro handheld OS.

---

## License

This project is licensed under the **MIT License**. See [`LICENSE`](LICENSE) for full details.
Third-party components retain their respective licenses (SexyAppFramework License, LGPL 2.1, zlib).
