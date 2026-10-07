# Installation & Asset Guide — Zuma Deluxe for Miyoo Mini

This guide explains in detail how to prepare the necessary assets from your PC version of **Zuma Deluxe** and install them onto your **Miyoo Mini** or **Miyoo Mini Plus** running **OnionOS**.

---

## 1. Prerequisites

### Original Game Files
You need a legally purchased copy of Zuma Deluxe for PC. Compatible releases:
- **Steam Version**: `steamapps/common/Zuma Deluxe/`
- **Original CD-ROM**: Retail installation folder.
- **EA App / Origin**: Electronic Arts installation folder.
- **PopCap Web Edition**: Original digital installer folder.

> **Note on Assets & `main.pak`:** The retail PC version of Zuma Deluxe does **NOT** contain a `main.pak` file. PopCap installed the game as loose folders: `fonts/`, `images/`, `levels/`, `music/` (containing `zuma.mo3`), `properties/`, and `sounds/`. 
>
> On the Miyoo Mini's FAT32/exFAT microSD card, reading hundreds of loose files individually causes heavy I/O latency and slow loading screens. Our tools automatically bundle these loose folders into an encrypted `main.pak` container and convert `zuma.mo3` to `zuma.it` for maximum performance.

---

## 2. Automated Release Build (Recommended)

### Option A: Using the Graphical Launcher (Easiest)
Simply run the included GUI builder:
- **Windows:** Double-click **`Launcher.py`** (Recommended) or **`Launcher.bat`**.
- **Linux / macOS:** Run `./Launcher.sh` or `python3 Launcher.py`.

The GUI provides an intuitive interface with:
- **Game Folder Picker & Auto-detection:** Easily select where Zuma Deluxe is installed.
- **Destination Folder Picker:** Choose where to save the ready-to-flash ZIP.
- **Real-time Progress Bar:** Watch the build steps in real-time.
- **One-click Output Folder Access:** Open the folder containing your finished `Zuma_Deluxe_MiyooMini.zip`.

---

### Option B: Using the Command Line Builder
If you prefer using the terminal:

```bash
# Modo 1 (Rápido): Usar el binario precompilado (sin Docker, sólo requiere Python):
python tools/build_release.py "C:\Program Files (x86)\Steam\steamapps\common\Zuma Deluxe"

# Modo 2 (Desarrollador): Recompilar desde el código fuente C++ con Docker:
python tools/build_release.py "C:\Program Files (x86)\Steam\steamapps\common\Zuma Deluxe" --build
```

### What the Script Does Automatically:
1. Validates all loose PC asset folders (`fonts`, `images`, `levels`, `properties`, `sounds`, `music`).
2. Converts the soundtrack `music/zuma.mo3` to Impulse Tracker `music/zuma.it` using the included UNMO3 decoder.
3. Packs the loose asset folders into a high-performance PopCap `main.pak` container.
4. Generates both the **App** structure (`App/ZumaDeluxe/`) and the **Ports** structure (`Roms/PORTS/`).
5. Bundles the precompiled `Zuma` ARM binary and required hardware libraries (`libSDL2-2.0.so.0`, `libneonarmmiyoo.so`).
6. Creates a flash-ready release archive: `dist/Zuma_Deluxe_MiyooMini.zip`.

Simply extract `dist/Zuma_Deluxe_MiyooMini.zip` to the **ROOT** of your microSD card!

---

## 3. Manual Method (Step-by-Step)

If you prefer to set up the files manually, follow these steps:

### Step 3.1: Unpack `main.pak` (If necessary)
If your game install only has a `main.pak` file, unpack it using the included tool:

```bash
python tools/unpack_pak.py "path/to/main.pak" my_unpacked_assets
```

This extracts the following directories:
- `fonts/`
- `images/`
- `levels/`
- `properties/`
- `sounds/`
- `music/`

### Step 3.2: Convert the Soundtrack (`zuma.mo3` -> `zuma.it`)
The original PC game uses a compressed module format (`zuma.mo3`) encoded with proprietary codecs. The Miyoo Mini port uses `libxmp-lite`, which natively supports standard Impulse Tracker (`.it`) modules.

1. Locate `zuma.mo3` in the `music/` folder.
2. Use the free `unmo3` utility (provided in `tools/unmo3.exe` on Windows, or downloadable from [un4seen.com](https://www.un4seen.com/mo3.html) for Linux/macOS):

```bash
# Windows
tools\unmo3.exe -y "music/zuma.mo3" "music/zuma.it"

# Linux / macOS
unmo3 -y "music/zuma.mo3" "music/zuma.it"
```

3. Ensure the resulting `music/zuma.it` file is approximately 1.8 MB to 1.9 MB in size.

---

## 4. MicroSD Card Directory Layout
 
OnionOS standard Ports format installs under `Roms/PORTS/`:
 
```
SDCARD/
└── Roms/
    └── PORTS/
        ├── Imgs/
        │   └── Zuma Deluxe.png          <- Menu boxart cover
        ├── Shortcuts/
        │   └── Puzzle games/
        │       └── Zuma Deluxe.port     <- Port launcher shortcut
        └── Games/
            └── Zuma Deluxe/
                ├── Zuma                 <- Native ARM binary
                ├── launch.sh            <- Launcher script (0755)
                ├── launcher.sh          <- Launcher script (0755)
                ├── main.pak             <- Game asset container
                ├── libs/
                │   ├── libSDL2-2.0.so.0
                │   ├── libSDL2.so
                │   └── libneonarmmiyoo.so
                ├── music/
                │   └── zuma.it          <- Converted soundtrack
                ├── userdata/            <- Folder for game saves
                ├── fonts/
                ├── images/
                ├── levels/
                ├── properties/
                └── sounds/
```

---

## 5. Troubleshooting

### Game exits immediately to menu
- Ensure the `launch.sh` and `Zuma` files have Linux execution permissions (`chmod +x launch.sh Zuma`).
- Verify that `libs/libSDL2-2.0.so.0` and `libs/libneonarmmiyoo.so` are present inside the `libs/` subfolder.

### No music playing during gameplay
- Ensure `music/zuma.it` exists in the game folder. If only `zuma.mo3` is present, it must be decoded with `unmo3`.
- Sound effects work from `sounds/`, while background music requires `zuma.it`.

### Save games not saving
- Ensure an empty directory named `userdata` exists inside the game folder with write permissions.
