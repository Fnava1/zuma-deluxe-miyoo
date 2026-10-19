---
title: On-console autonomous installer and native asset converter architecture
author: Fnava1
date: 2026-10-19
branch: master
touches:
  - CMakeLists.txt
  - packaging/Roms/PORTS/Games/Zuma Deluxe/launch.sh
  - packaging/Roms/PORTS/Games/Zuma Deluxe/launcher.sh
  - tools/build_release.py
  - ".gemini/settings.json"
  - AGENTS.md
  - GEMINI.md
  - bin/zuma_setup
  - docs/superpowers/plans/2026-10-19-on-console-installer.md
  - packaging/Roms/PORTS/Games/Zuma Deluxe/libs/libunmo3.so
---

## Decided
- Created standalone zuma_setup ARM binary with zero-dependency embedded 8x8 font and SDL2 UI at 640x480.
- Implemented pack_pak_native in portable C (streaming XOR 0xF7 and 0xBAC04AC0 magic) for fast on-device container packing under 2 seconds.
- Patched libunmo3.so to hard-float ABI and created mo3_decode.c with runtime dlopen for on-console soundtrack decoding (zuma.mo3 -> zuma.it).
- Updated launch.sh to intercept missing main.pak or zuma.it on first boot, invoke zuma_setup, and exit cleanly without black screens if files are missing or user cancels.

## Tried and ruled out
- Requiring users to run Python on PC: Inconvenient for mobile-only or drag-and-drop users.
- Running Python on OnionOS: Minimal embedded Linux lacks python3 runtime.
- Silent background conversion: Users would perceive a 10s black screen as a system freeze and pull battery.

## Watch out
- Always preserve 0755 executable permissions on zuma_setup and shared libraries in ZIP packaging.
- Maintain libc.so symlink fallback for libraries targeting Android/generic libc sonames.
