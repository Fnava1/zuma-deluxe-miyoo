---
title: OnionOS import ports compatibility and GameDataFile wildcard
author: Fnava1
date: 2026-10-19
branch: master
touches:
  - CMakeLists.txt
  - packaging/Roms/PORTS/Games/Zuma Deluxe/launch.sh
  - packaging/Roms/PORTS/Games/Zuma Deluxe/launcher.sh
  - packaging/Roms/PORTS/Shortcuts/Puzzle games/Zuma Deluxe.port
  - tools/build_release.py
  - ".gemini/settings.json"
  - AGENTS.md
  - GEMINI.md
  - bin/zuma_setup
  - docs/superpowers/plans/2026-10-19-on-console-installer.md
---

## Decided
- Set GameDataFile='main*.pak' as a single static line in Zuma Deluxe.port matching the Plants vs. Zombies reference fix.
- Cleaned up dynamic directory resolution to 4 standard branches without redundant /mnt/SDCARD/ROMS fallbacks.
- Removed any header comments preceding GameName to prevent static parsing issues with OnionOS import.sh.

## Tried and ruled out
- GameDataFile='main.pak': If a user copies loose PC files or runs ~Import ports before launching, import.sh cannot find the exact match and renames the shortcut to Zuma Deluxe.notfound.
- Conditional if/else for GameDataFile: import.sh uses grep GameDataFile= which extracts multiple lines and breaks find.

## Watch out
- Exactly one assignment of GameDataFile= must exist in the .port file.
- Rebuild dist ZIPs whenever packaging/ .port or launch scripts are modified.
