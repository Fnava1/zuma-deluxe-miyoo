---
title: "Miyoo Mini porting architecture, teardown and audio learnings"
author: Fnava1
date: 2026-10-19
branch: master
touches:
  - ".gemini/settings.json"
  - AGENTS.md
  - GEMINI.md
---

## Decided
- Main loop teardown: Never call exit(0) or terminate abruptly. Let SexyAppBase::DoMainLoop() unwind cleanly, destroy the SDL window/surface, and restore the framebuffer to prevent black screens on Miyoo Mini v4.
- Audio playback: Zuma's original MO3 requires decoding to IT or pre-cached WAV to prevent high CPU load on Ingenic / Sigmastar chips.
- PAK assets: The engine expects standard PopCap PAK (magic 0xBAC04AC0). The Steam release distributes loose folders, which build_release.py packages natively into main.pak.
- Device detection: Inspect /tmp/miyoo_version or device nodes to dynamically tune CPU frequencies between Miyoo Mini v4 (1200MHz) and Miyoo Mini Plus (1300MHz).

## Tried and ruled out
- Immediate exit(0) on Home/Menu: Leaves the screen in an un-cleared direct FB state requiring a battery pull.
- Pure software audio decompression on the fly without caching: Caused stuttering and audio starvation during rapid ball combos.

## Watch out
- Do not commit copyrighted PopCap assets or PAKs to git.
- Keep build scripts clean of packed binaries to avoid antivirus false positives.
