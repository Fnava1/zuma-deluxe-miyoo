# On-Console Autonomous Installer (zuma_setup) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

## Overview
Implement an independent, standalone on-console installer (`zuma_setup`) with an SDL2 graphical UI for the Miyoo Mini / OnionOS. This enables users to copy their raw, legal PC installation files of *Zuma Deluxe* (e.g. from Steam, GOG, or CD) directly to the microSD card. Upon first boot in OnionOS, `launch.sh` detects that the assets have not been converted, automatically launches `zuma_setup` to display progress, convert audio (`zuma.mo3` -> `zuma.it`), pack assets into `main.pak`, and launch the game cleanly.

---

## File Structure & Responsibilities

| File | Responsibility |
| :--- | :--- |
| `source/Setup/pack_pak_native.h` / `.c` | Portable, high-speed C implementation of PopCap PAK archive generator (Magic `0xBAC04AC0`, XOR `0xF7`). |
| `source/Setup/mo3_decode.h` / `.c` | Dynamic loader wrapper for `libunmo3.so` to decompress `zuma.mo3` into `zuma.it` on ARM Linux. |
| `source/Setup/zuma_setup.c` | Standalone SDL2 graphical installer binary (`zuma_setup`): renders 640x480 UI, embedded bitmap font, progress bar, error prompts, and coordinates conversion. |
| `packaging/Roms/PORTS/Games/Zuma Deluxe/libs/libunmo3.so` | Hardfp-patched ARMv7 shared library for decoding MO3 audio. |
| `packaging/Roms/PORTS/Games/Zuma Deluxe/launch.sh` | Updated bootstrapper script: detects installation status, handles missing assets, executes `zuma_setup` with error traps, and runs `Zuma`. |
| `CMakeLists.txt` | Adds build target for `zuma_setup` compiled against SDL2 and math libraries. |
| `tools/build_release.py` | Bundles `zuma_setup` and `libunmo3.so` into the distribution package. |

---

## Tasks

### Task 1: Native C PopCap PAK Packer Routine
- [x] Create `source/Setup/pack_pak_native.h` and `source/Setup/pack_pak_native.c` implementing the PopCap `.pak` container format.
- [x] Support progress callback (`void (*progress_cb)(int pct, const char *msg)`) for real-time UI updates.
- [x] Recursively index and sort asset folders (`fonts`, `images`, `levels`, `properties`, `sounds`, `music`).
- [x] Write directory header (`0xBAC04AC0`), record table, and payload with `0xF7` XOR masking.
- [x] Write unit test / verification script to confirm byte-for-byte structural equivalence with `pack_pak.py`.

### Task 2: ARM Native MO3 Decoder Wrapper
- [x] Copy and deploy `libunmo3.so` (with ARM hard-float ABI flags) into `packaging/.../libs/`.
- [x] Implement `source/Setup/mo3_decode.c` using `dlopen` to load `libunmo3.so` dynamically at runtime.
- [x] Provide fallback and clean error reporting if `zuma.mo3` is corrupted or already converted (`zuma.it`).

### Task 3: Graphical SDL2 Installer (`zuma_setup`)
- [x] Implement `source/Setup/zuma_setup.c`:
  - 640x480 resolution matching Miyoo Mini display.
  - Zero-dependency embedded 8x8 font rendering (no TTF file dependencies).
  - Smooth progress bar and status text.
  - Interactive error screen if required asset folders are missing, allowing clean exit via Menu / B button.
- [x] Add `zuma_setup` target to `CMakeLists.txt`.

### Task 4: Shell Wrapper Integration (`launch.sh`)
- [x] Update `launch.sh` in packaging templates:
  - Check if `main.pak` and `music/zuma.it` exist.
  - If missing, set up library search paths, ensure compatibility symlinks, and run `./zuma_setup`.
  - Check return status: if setup succeeded, continue to `./Zuma`; if canceled or missing files, unwind cleanly and exit to OnionOS without black screens.

### Task 5: Compilation, Packaging & End-to-End Verification
- [x] Cross-compile `Zuma` and `zuma_setup` using Docker toolchain (`aemiii91/miyoomini-toolchain`).
- [x] Update `tools/build_release.py` to ensure `zuma_setup` and `libunmo3.so` are included in generated release packages.
- [x] Validate end-to-end flow: test unpackaging loose files, running conversion, and launching game.

