#!/usr/bin/env python3
"""
Zuma Deluxe — Release Package Builder for Miyoo Mini / OnionOS
==============================================================
Builds the final distribution ZIP (ready to flash/copy to SD card)
from a legally owned PC installation of Zuma Deluxe.

BACKGROUND ON ASSETS & MAIN.PAK:
--------------------------------
The original retail PC version of Zuma Deluxe does NOT contain a 'main.pak'
file; instead, PopCap shipped it with loose asset folders:
  - fonts/
  - images/
  - levels/
  - music/      (contains 'zuma.mo3')
  - properties/ (contains 'resources.xml')
  - sounds/

On the Miyoo Mini, reading 400+ loose files sequentially from an SD card
causes significant I/O latency and slower loading screens. Therefore, this
tool:
  1. Converts 'music/zuma.mo3' -> 'music/zuma.it' using UNMO3 (required by libxmp-lite).
  2. Packs the loose asset folders into a single optimized 'main.pak' (Magic: 0xBAC04AC0, XOR 0xF7).
  3. Bundles the compiled 'Zuma' ARM binary, launcher scripts, hardware libraries, and configs.
  4. Generates both OnionOS installation layouts ('App/' and 'Roms/PORTS/').
  5. Packages everything into 'Zuma_Deluxe_MiyooMini.zip'.

Usage:
    python tools/build_release.py [path_to_pc_zuma_deluxe_folder] [--output <dir>]
"""

import os
import sys
import shutil
import subprocess
import argparse
import zipfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
sys.path.insert(0, SCRIPT_DIR)

from pack_pak import pack_pak

REQUIRED_ASSET_DIRS = ["fonts", "images", "levels", "properties", "sounds"]

def find_unmo3_binary():
    candidates = [
        os.path.join(SCRIPT_DIR, "unmo3.exe"),
        os.path.join(SCRIPT_DIR, "unmo3"),
        os.path.join(REPO_ROOT, "unmo3.exe"),
        os.path.join(REPO_ROOT, "unmo3"),
        shutil.which("unmo3"),
        shutil.which("unmo3.exe")
    ]
    for c in candidates:
        if c and os.path.isfile(c):
            return c
    return None

def convert_music(source_music_dir, dest_music_dir):
    os.makedirs(dest_music_dir, exist_ok=True)
    target_it = os.path.join(dest_music_dir, "zuma.it")
    source_it = os.path.join(source_music_dir, "zuma.it")
    source_mo3 = os.path.join(source_music_dir, "zuma.mo3")

    # If zuma.it already exists in source
    if os.path.isfile(source_it):
        print(f"Found existing converted music: {source_it}")
        shutil.copy2(source_it, target_it)
        return True

    # Otherwise convert from zuma.mo3
    if not os.path.isfile(source_mo3):
        print("Warning: Neither 'zuma.it' nor 'zuma.mo3' was found in music folder!")
        return False

    unmo3_bin = find_unmo3_binary()
    if not unmo3_bin:
        print("\n" + "="*70)
        print("ERROR: 'unmo3' utility not found!")
        print("The original game uses 'zuma.mo3' which must be converted to 'zuma.it'.")
        print("Please download UNMO3 from: https://www.un4seen.com/mo3.html")
        print(f"Place 'unmo3.exe' (or 'unmo3' on Linux) inside: {SCRIPT_DIR}")
        print("="*70 + "\n")
        return False

    print(f"Converting music: {source_mo3} -> {target_it}...")
    try:
        cmd = [unmo3_bin, "-y", source_mo3, target_it]
        subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        if os.path.isfile(target_it) and os.path.getsize(target_it) > 500000:
            print(f"  Converted successfully ({os.path.getsize(target_it)/(1024*1024):.2f} MB).")
            return True
        else:
            print("  UNMO3 failed to produce valid 'zuma.it'.")
            return False
    except Exception as e:
        print(f"  Error running UNMO3: {e}")
        return False

def copy_tree_clean(src, dst):
    if os.path.exists(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)

def main():
    parser = argparse.ArgumentParser(description="Build complete release package for Zuma Deluxe Miyoo Mini port")
    parser.add_argument("source", nargs="?", default="", help="Path to PC Zuma Deluxe installation folder")
    parser.add_argument("--output", "-o", default=os.path.join(REPO_ROOT, "dist"), help="Output directory (default: dist/)")
    parser.add_argument("--build", "-b", action="store_true", help="Compile binary from C++ source using Docker before packaging")
    args = parser.parse_args()

    source = args.source
    if not source:
        # Search common locations
        candidates = [
            os.path.join(REPO_ROOT, "Zuma Deluxe"),
            os.path.join(REPO_ROOT, "..", "Zuma Deluxe"),
            r"C:\Program Files (x86)\Steam\steamapps\common\Zuma Deluxe",
            r"C:\Program Files\Steam\steamapps\common\Zuma Deluxe",
            r"C:\PopCap Games\Zuma Deluxe",
        ]
        for c in candidates:
            if os.path.isdir(c):
                source = c
                print(f"Automatically detected Zuma Deluxe PC folder: {source}")
                break

    if not source or not os.path.isdir(source):
        print("Error: Could not find Zuma Deluxe PC folder.")
        print("Usage: python tools/build_release.py <path_to_pc_zuma_deluxe_folder>")
        sys.exit(1)

    print(f"\n=======================================================")
    print(f" Zuma Deluxe — Release Builder for Miyoo Mini (OnionOS)")
    print(f" Source assets: {source}")
    print(f"=======================================================\n")

    # 1. Validate required asset directories
    for req in REQUIRED_ASSET_DIRS:
        req_path = os.path.join(source, req)
        if not os.path.isdir(req_path):
            print(f"Error: Missing required folder '{req}' in source directory.")
            sys.exit(1)

    # 2. Check for or compile Zuma ARM executable
    def compile_with_docker():
        print("\n" + "="*70)
        print(" Compiling Zuma from C++ source using Docker toolchain...")
        print(" Toolchain container: aemiii91/miyoomini-toolchain:latest")
        print("="*70 + "\n")
        if not shutil.which("docker"):
            print("Error: Docker is not installed or not in PATH.")
            return None
        try:
            cmd = [
                "docker", "run", "--rm",
                "-v", f"{REPO_ROOT}:/root/workspace",
                "aemiii91/miyoomini-toolchain:latest",
                "/bin/bash", "-c",
                "cd /root/workspace && rm -rf build-miyoo && mkdir -p build-miyoo && cd build-miyoo && cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/miyoomini.toolchain.cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)"
            ]
            ret = subprocess.run(cmd)
            if ret.returncode == 0:
                built = os.path.join(REPO_ROOT, "build-miyoo", "source", "CircleShoot", "Zuma")
                if os.path.isfile(built):
                    target = os.path.join(REPO_ROOT, "bin", "Zuma")
                    os.makedirs(os.path.dirname(target), exist_ok=True)
                    shutil.copy2(built, target)
                    print(f"\nCompilation successful! Binary saved to: {target}\n")
                    return target
        except Exception as e:
            print(f"Error during Docker compilation: {e}")
        return None

    bin_candidates = [
        os.path.join(REPO_ROOT, "bin", "Zuma"),
        os.path.join(REPO_ROOT, "packaging", "App", "ZumaDeluxe", "Zuma"),
        os.path.join(REPO_ROOT, "build-miyoo", "source", "CircleShoot", "Zuma"),
        os.path.join(REPO_ROOT, "..", "zuma-portable", "build-miyoo", "source", "CircleShoot", "Zuma")
    ]
    zuma_bin = None

    if args.build:
        print("Flag --build specified: Compiling binary from source...")
        zuma_bin = compile_with_docker()
        if not zuma_bin:
            sys.exit(1)
    else:
        for b in bin_candidates:
            if os.path.isfile(b):
                zuma_bin = b
                break

        if not zuma_bin:
            print("\nPrecompiled 'Zuma' binary not found.")
            try:
                ans = input("Would you like to compile it now from source using Docker? [y/N]: ").strip().lower()
            except (EOFError, KeyboardInterrupt):
                ans = "n"
            if ans in ["y", "yes", "s", "si"]:
                zuma_bin = compile_with_docker()

            if not zuma_bin:
                print("\n" + "="*70)
                print("ERROR: No Zuma binary available!")
                print("Options to resolve:")
                print("  1. Compile from source: python tools/build_release.py --build")
                print("     or run: tools/build_docker.bat (Windows) / tools/build_docker.sh (Linux)")
                print("  2. Download precompiled 'Zuma' from GitHub Releases and place it in bin/Zuma")
                print("="*70 + "\n")
                sys.exit(1)
        else:
            print(f"Using precompiled Zuma binary: {zuma_bin}")
            print("  (Tip: Use 'python tools/build_release.py --build' to recompile from source with Docker)")

    # 3. Setup output directory structure
    dist_root = os.path.abspath(args.output)
    app_dir = os.path.join(dist_root, "App", "ZumaDeluxe")
    ports_dir = os.path.join(dist_root, "Roms", "PORTS")
    games_zuma_dir = os.path.join(ports_dir, "Games", "Zuma Deluxe")

    if os.path.exists(dist_root):
        shutil.rmtree(dist_root)
    os.makedirs(app_dir, exist_ok=True)
    os.makedirs(games_zuma_dir, exist_ok=True)

    # 4. Copy packaging templates (scripts, libraries, configs, icons)
    pkg_app = os.path.join(REPO_ROOT, "packaging", "App", "ZumaDeluxe")
    pkg_ports = os.path.join(REPO_ROOT, "packaging", "Roms", "PORTS")

    copy_tree_clean(pkg_app, app_dir)
    copy_tree_clean(pkg_ports, ports_dir)

    # Place executable in both layouts
    shutil.copy2(zuma_bin, os.path.join(app_dir, "Zuma"))
    shutil.copy2(zuma_bin, os.path.join(games_zuma_dir, "Zuma"))

    # 5. Convert & Copy music (music/zuma.it)
    print("\n[Step 1/3] Preparing soundtrack (zuma.mo3 -> zuma.it)...")
    source_music = os.path.join(source, "music")
    convert_music(source_music, os.path.join(app_dir, "music"))
    convert_music(source_music, os.path.join(games_zuma_dir, "music"))

    # 6. Generate optimized main.pak from loose folders
    print("\n[Step 2/3] Generating optimized main.pak from loose folders...")
    app_pak = os.path.join(app_dir, "main.pak")
    pack_pak(source, app_pak, target_dirs=["fonts", "images", "levels", "properties", "sounds", "music"])
    shutil.copy2(app_pak, os.path.join(games_zuma_dir, "main.pak"))

    # 7. Copy loose asset folders as guaranteed fallback
    print("Deploying asset folders...")
    def deploy_loose(dest_target):
        os.makedirs(os.path.join(dest_target, "userdata"), exist_ok=True)
        for fld in REQUIRED_ASSET_DIRS:
            copy_tree_clean(os.path.join(source, fld), os.path.join(dest_target, fld))

    deploy_loose(app_dir)
    deploy_loose(games_zuma_dir)

    # 8. Create distribution ZIP archive
    print("\n[Step 3/3] Creating ready-to-flash release ZIP archive...")
    zip_path = os.path.join(dist_root, "Zuma_Deluxe_MiyooMini.zip")
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zipf:
        for root, _, files in os.walk(dist_root):
            for file in files:
                full_path = os.path.join(root, file)
                if full_path == zip_path:
                    continue
                rel_path = os.path.relpath(full_path, dist_root)
                zipinfo = zipfile.ZipInfo(rel_path)
                # Ensure Linux executable permissions for binaries and shell scripts
                if file in ["Zuma", "launch.sh", "Zuma Deluxe.port"] or file.endswith(".sh") or file.endswith(".so") or ".so." in file:
                    zipinfo.external_attr = 0o755 << 16
                else:
                    zipinfo.external_attr = 0o644 << 16
                with open(full_path, "rb") as f:
                    zipf.writestr(zipinfo, f.read())

    zip_size_mb = os.path.getsize(zip_path) / (1024 * 1024)
    print(f"\n" + "="*70)
    print(" RELEASE PACKAGE BUILT SUCCESSFULLY!")
    print("="*70)
    print(f" Archive: {zip_path} ({zip_size_mb:.2f} MB)")
    print(f" Directory: {dist_root}")
    print("\nINSTALLATION ON MIYOO MINI (OnionOS):")
    print("  1. Extract 'Zuma_Deluxe_MiyooMini.zip' to the ROOT of your microSD card.")
    print("     (Or drag and drop the 'App' and 'Roms' folders to your microSD card).")
    print("  2. Insert microSD into Miyoo Mini.")
    print("  3. Launch 'Zuma Deluxe' from:")
    print("     - Main Menu -> Apps -> Zuma Deluxe, OR")
    print("     - Main Menu -> Games -> Expert / Ports -> Zuma Deluxe.")
    print("="*70 + "\n")

if __name__ == "__main__":
    main()
