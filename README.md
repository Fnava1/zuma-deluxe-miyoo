# Zuma Deluxe — Miyoo Mini & Mini+ Port (OnionOS)

[![Platform](https://img.shields.io/badge/Platform-Miyoo%20Mini%20%2F%20Plus-blue.svg)](https://onionui.github.io/)
[![OS](https://img.shields.io/badge/OS-OnionOS-red.svg)](https://onionui.github.io/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)


> **IMPORTANT LEGAL NOTICE:** This repository does **NOT** contain any copyrighted game assets, audio files, textures, or level data. You must provide your own legally owned copy of **Zuma Deluxe for PC** (available on [Steam](https://store.steampowered.com/app/3330/Zuma_Deluxe/), EA App, or original CD-ROM) to play.

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

### Option 1: Graphical Launcher (Easiest)
Simply run the included graphical port builder:
- **Windows:** Double-click **`Launcher.bat`** in the root directory.
- **Linux / macOS:** Run `./Launcher.sh` or `python3 tools/gui_builder.py`.

The GUI features:
- **Folder Pickers:** Select your original PC game folder and your preferred output destination.
- **Auto-Detection:** Automatically searches standard Steam and PopCap installation paths.
- **Live Progress Bar:** Tracks music conversion, `main.pak` generation, and archive compression.
- **One-Click Flash Ready:** Generates `Zuma_Deluxe_MiyooMini.zip` ready to extract to your SD card.

---

### Option 2: Command Line Builder
You can also run the builder directly from your terminal:

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
4. Assemble the OnionOS Ports layout (`Roms/PORTS/`) with the precompiled `Zuma` ARM binary and hardware libraries.
5. Generate the flash-ready archive: `dist/Zuma_Deluxe_MiyooMini.zip`.

### Step 2: Copy to MicroSD Card
- Extract `dist/Zuma_Deluxe_MiyooMini.zip` directly to the **ROOT** of your Miyoo Mini microSD card.
- Insert the card into your Miyoo Mini and launch **Zuma Deluxe** from **Games -> Expert / Ports -> Puzzle games -> Zuma Deluxe**.

---

## Manual Installation Guide

If you prefer to organize the files manually, see [`INSTALL.md`](INSTALL.md) for step-by-step instructions.

In brief:
1. Convert `music/zuma.mo3` to `music/zuma.it` using `unmo3 -y zuma.mo3 zuma.it`.
2. Copy the folders (`fonts`, `images`, `levels`, `music`, `properties`, `sounds`, `userdata`) and `main.pak` to:
   - `SDCARD/Roms/PORTS/Games/Zuma Deluxe/`.
3. Copy the compiled `Zuma` binary and launcher files from [`packaging/Roms/PORTS/`](packaging/Roms/PORTS/).

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

## Credits & Acknowledgements

- **PopCap Games / Electronic Arts**: Creators of the original *Zuma Deluxe*.
- **PopCap Games Framework Team**: Original open-source release of SexyAppFramework.
- **libxmp Team**: High-performance tracker playback library.
- **Un4seen Developments**: UNMO3 decompression utility.
- **OnionOS Community**: For developing and maintaining the OS.

---

## License

This project is licensed under the **MIT License**. See [`LICENSE`](LICENSE) for full details.
Third-party components retain their respective licenses (SexyAppFramework License, LGPL 2.1, zlib).
