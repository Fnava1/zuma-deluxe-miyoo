#!/usr/bin/env python3
"""
Zuma Deluxe — Release Package Builder for Miyoo Mini / OnionOS
==============================================================
Builds the final distribution ZIP (ready to flash/copy to SD card)
from a legally owned PC installation of Zuma Deluxe.
Can be run via command line or called programmatically by gui_builder.py.

SAFETY GUARANTEES:
- NEVER deletes or purges the user's selected output directory.
- Builds entirely in an isolated temporary staging directory.
- Safely deploys files without Windows file lock crashes.
"""

import os
import sys
import shutil
import subprocess
import argparse
import tempfile
import time
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

def convert_music(source_music_dir, dest_music_dir, log_fn=print):
    os.makedirs(dest_music_dir, exist_ok=True)
    target_it = os.path.join(dest_music_dir, "zuma.it")
    source_it = os.path.join(source_music_dir, "zuma.it")
    source_mo3 = os.path.join(source_music_dir, "zuma.mo3")

    # If zuma.it already exists in source
    if os.path.isfile(source_it):
        log_fn(f"Found existing converted music: {source_it}")
        shutil.copy2(source_it, target_it)
        return True

    # Otherwise convert from zuma.mo3
    if not os.path.isfile(source_mo3):
        log_fn("Warning: Neither 'zuma.it' nor 'zuma.mo3' was found in music folder!")
        return False

    unmo3_bin = find_unmo3_binary()
    if not unmo3_bin:
        log_fn("\n" + "="*70)
        log_fn("ERROR: 'unmo3' utility not found!")
        log_fn("The original game uses 'zuma.mo3' which must be converted to 'zuma.it'.")
        log_fn("Please download UNMO3 from: https://www.un4seen.com/mo3.html")
        log_fn(f"Place 'unmo3.exe' (or 'unmo3' on Linux) inside: {SCRIPT_DIR}")
        log_fn("="*70 + "\n")
        return False

    log_fn(f"Converting music: {source_mo3} -> {target_it}...")
    try:
        cmd = [unmo3_bin, "-y", source_mo3, target_it]
        subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        if os.path.isfile(target_it) and os.path.getsize(target_it) > 500000:
            log_fn(f"  Converted successfully ({os.path.getsize(target_it)/(1024*1024):.2f} MB).")
            return True
        else:
            log_fn("  UNMO3 failed to produce valid 'zuma.it'.")
            return False
    except Exception as e:
        log_fn(f"  Error running UNMO3: {e}")
        return False

def copy_tree_merge(src, dst, log_fn=print):
    """
    Safely copy/merge files from src into dst WITHOUT deleting dst.
    Handles existing files and retries on transient file locks.
    """
    os.makedirs(dst, exist_ok=True)
    for root, dirs, files in os.walk(src):
        rel = os.path.relpath(root, src)
        target_dir = os.path.join(dst, rel) if rel != "." else dst
        os.makedirs(target_dir, exist_ok=True)
        for f in files:
            src_file = os.path.join(root, f)
            dst_file = os.path.join(target_dir, f)
            for attempt in range(3):
                try:
                    shutil.copy2(src_file, dst_file)
                    break
                except PermissionError:
                    if attempt < 2:
                        time.sleep(0.2)
                    else:
                        log_fn(f"Notice: Could not overwrite {f} (file may be in use).")
                except Exception as e:
                    log_fn(f"Notice copying {f}: {e}")
                    break

def compile_with_docker(log_fn=print):
    log_fn("\n" + "="*70)
    log_fn(" Compiling Zuma from C++ source using Docker toolchain...")
    log_fn(" Toolchain container: aemiii91/miyoomini-toolchain:latest")
    log_fn("="*70 + "\n")
    if not shutil.which("docker"):
        log_fn("Error: Docker is not installed or not in PATH.")
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
                log_fn(f"\nCompilation successful! Binary saved to: {target}\n")
                return target
    except Exception as e:
        log_fn(f"Error during Docker compilation: {e}")
    return None

def build_package(source, output_dir=None, build_from_source=False, progress_cb=None, log_cb=None):
    """
    Safe core build routine.
    progress_cb(percentage: int, status_message: str)
    log_cb(message: str)
    """
    def log(msg):
        if log_cb:
            log_cb(str(msg))
        else:
            print(msg)

    def set_progress(pct, msg):
        if progress_cb:
            progress_cb(pct, msg)
        log(f"[{pct}%] {msg}")

    # Canonical paths
    source = os.path.abspath(source)
    if not output_dir:
        output_dir = os.path.join(REPO_ROOT, "dist")
    output_dir = os.path.abspath(output_dir)

    # SAFETY CHECK 1: Source directory validation
    set_progress(5, "Validating source directory...")
    if not os.path.isdir(source):
        raise ValueError(f"Source directory does not exist: {source}")

    for req in REQUIRED_ASSET_DIRS:
        req_path = os.path.join(source, req)
        if not os.path.isdir(req_path):
            raise FileNotFoundError(f"Missing required folder '{req}' in source directory: {source}")

    # SAFETY CHECK 2: Source and Output must NOT be identical
    if os.path.normcase(source) == os.path.normcase(output_dir):
        raise ValueError("Source and Output folders cannot be the same! Please select a different output folder.")

    # SAFETY CHECK 3: Output cannot be a drive root or system directory
    drive, rest = os.path.splitdrive(output_dir)
    if rest.rstrip("\\/") == "":
        raise ValueError("Cannot write directly to drive root. Please select or create a subfolder (e.g., E:\\ZumaPort).")

    # 1. Resolve executable
    set_progress(15, "Resolving Zuma ARM binary...")
    bin_candidates = [
        os.path.join(REPO_ROOT, "bin", "Zuma"),
        os.path.join(REPO_ROOT, "packaging", "App", "ZumaDeluxe", "Zuma"),
        os.path.join(REPO_ROOT, "build-miyoo", "source", "CircleShoot", "Zuma"),
        os.path.join(REPO_ROOT, "..", "zuma-portable", "build-miyoo", "source", "CircleShoot", "Zuma")
    ]
    zuma_bin = None

    if build_from_source:
        set_progress(20, "Compiling binary from source with Docker...")
        zuma_bin = compile_with_docker(log_fn=log)
        if not zuma_bin:
            raise RuntimeError("Compilation with Docker failed.")
    else:
        for b in bin_candidates:
            if os.path.isfile(b):
                zuma_bin = b
                break
        if not zuma_bin:
            log("No precompiled binary found. Attempting Docker compilation...")
            set_progress(20, "Compiling binary with Docker...")
            zuma_bin = compile_with_docker(log_fn=log)
            if not zuma_bin:
                raise FileNotFoundError("Could not find or build 'Zuma' binary. Please build it or place it in bin/Zuma.")

    log(f"Using Zuma binary: {zuma_bin}")

    # Check if target ZIP is locked before starting work
    os.makedirs(output_dir, exist_ok=True)
    target_zip = os.path.join(output_dir, "Zuma_Deluxe_MiyooMini.zip")
    if os.path.isfile(target_zip):
        try:
            with open(target_zip, "a+b"):
                pass
        except PermissionError:
            raise PermissionError(
                f"The target file '{target_zip}' is currently in use or open in another program.\n"
                "Please close Windows Explorer, 7-Zip, or any other app viewing it and try again."
            )

    # 2. Build completely in an isolated temporary staging directory
    staging_dir = tempfile.mkdtemp(prefix="zuma_build_staging_")
    try:
        set_progress(30, "Setting up OnionOS package structure...")
        app_dir = os.path.join(staging_dir, "App", "ZumaDeluxe")
        ports_dir = os.path.join(staging_dir, "Roms", "PORTS")
        games_zuma_dir = os.path.join(ports_dir, "Games", "Zuma Deluxe")

        os.makedirs(app_dir, exist_ok=True)
        os.makedirs(games_zuma_dir, exist_ok=True)

        # Copy packaging templates
        pkg_app = os.path.join(REPO_ROOT, "packaging", "App", "ZumaDeluxe")
        pkg_ports = os.path.join(REPO_ROOT, "packaging", "Roms", "PORTS")

        copy_tree_merge(pkg_app, app_dir, log_fn=log)
        copy_tree_merge(pkg_ports, ports_dir, log_fn=log)

        # Place executable in both layouts
        shutil.copy2(zuma_bin, os.path.join(app_dir, "Zuma"))
        shutil.copy2(zuma_bin, os.path.join(games_zuma_dir, "Zuma"))

        # 3. Convert & Copy music
        set_progress(45, "Preparing soundtrack (zuma.mo3 -> zuma.it)...")
        source_music = os.path.join(source, "music")
        convert_music(source_music, os.path.join(app_dir, "music"), log_fn=log)
        convert_music(source_music, os.path.join(games_zuma_dir, "music"), log_fn=log)

        # 4. Generate optimized main.pak
        set_progress(60, "Packing asset folders into main.pak container...")
        app_pak = os.path.join(app_dir, "main.pak")
        pack_pak(source, app_pak, target_dirs=["fonts", "images", "levels", "properties", "sounds", "music"])
        shutil.copy2(app_pak, os.path.join(games_zuma_dir, "main.pak"))

        # 5. Deploy asset folders
        set_progress(75, "Deploying asset folders...")
        os.makedirs(os.path.join(app_dir, "userdata"), exist_ok=True)
        os.makedirs(os.path.join(games_zuma_dir, "userdata"), exist_ok=True)
        for fld in REQUIRED_ASSET_DIRS:
            copy_tree_merge(os.path.join(source, fld), os.path.join(app_dir, fld), log_fn=log)
            copy_tree_merge(os.path.join(source, fld), os.path.join(games_zuma_dir, fld), log_fn=log)

        # 6. Create ZIP archive inside isolated staging directory
        set_progress(90, "Creating ready-to-flash ZIP archive...")
        staging_zip = os.path.join(staging_dir, "Zuma_Deluxe_MiyooMini.zip")
        with zipfile.ZipFile(staging_zip, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zipf:
            for root, _, files in os.walk(staging_dir):
                for file in files:
                    full_path = os.path.join(root, file)
                    if full_path == staging_zip:
                        continue
                    rel_path = os.path.relpath(full_path, staging_dir)
                    zipinfo = zipfile.ZipInfo(rel_path)
                    if file in ["Zuma", "launch.sh", "Zuma Deluxe.port"] or file.endswith(".sh") or file.endswith(".so") or ".so." in file:
                        zipinfo.external_attr = 0o755 << 16
                    else:
                        zipinfo.external_attr = 0o644 << 16
                    with open(full_path, "rb") as f:
                        zipf.writestr(zipinfo, f.read())

        # 7. Safely copy to final destination (WITHOUT DELETING ANYTHING)
        set_progress(96, "Deploying files to destination...")
        shutil.copy2(staging_zip, target_zip)
        copy_tree_merge(os.path.join(staging_dir, "App"), os.path.join(output_dir, "App"), log_fn=log)
        copy_tree_merge(os.path.join(staging_dir, "Roms"), os.path.join(output_dir, "Roms"), log_fn=log)

        set_progress(100, "Release package built successfully!")
        size_mb = os.path.getsize(target_zip) / (1024 * 1024)
        log(f"Archive created: {target_zip} ({size_mb:.2f} MB)")
        return target_zip, output_dir

    finally:
        # Clean up temporary staging directory
        try:
            shutil.rmtree(staging_dir, ignore_errors=True)
        except Exception:
            pass

def main():
    parser = argparse.ArgumentParser(description="Build complete release package for Zuma Deluxe Miyoo Mini port")
    parser.add_argument("source", nargs="?", default="", help="Path to PC Zuma Deluxe installation folder")
    parser.add_argument("--output", "-o", default=os.path.join(REPO_ROOT, "dist"), help="Output directory (default: dist/)")
    parser.add_argument("--build", "-b", action="store_true", help="Compile binary from C++ source using Docker before packaging")
    args = parser.parse_args()

    source = args.source
    if not source:
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

    try:
        zip_path, dist_dir = build_package(source, args.output, build_from_source=args.build)
        print("\n" + "="*70)
        print(" RELEASE PACKAGE BUILT SUCCESSFULLY!")
        print("="*70)
        print(f" Archive: {zip_path}")
        print(f" Directory: {dist_dir}")
        print("\nINSTALLATION ON MIYOO MINI (OnionOS):")
        print("  1. Extract 'Zuma_Deluxe_MiyooMini.zip' to the ROOT of your microSD card.")
        print("  2. Insert microSD into Miyoo Mini.")
        print("  3. Launch 'Zuma Deluxe' from Apps or Ports.")
        print("="*70 + "\n")
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
