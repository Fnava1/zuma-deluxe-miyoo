#!/usr/bin/env python3
"""
PopCap PAK Packer for Zuma Deluxe (Miyoo Mini Port)
Packs asset folders into standard PopCap main.pak format (Magic: 0xBAC04AC0).
"""

import os
import sys
import struct

POP_PAK_MAGIC = 0xBAC04AC0
POP_PAK_VERSION = 0
FILEFLAGS_END = 0x80

def pack_pak(input_dir, output_pak_path, target_dirs=None):
    if target_dirs is None:
        target_dirs = ["fonts", "images", "levels", "music", "properties", "sounds"]
    
    file_entries = [] # (relative_path_with_backslashes, file_size, full_path)
    
    print(f"Scanning directories in {input_dir}: {target_dirs}")
    for d in target_dirs:
        dir_path = os.path.join(input_dir, d)
        if not os.path.isdir(dir_path):
            print(f"Warning: Directory '{dir_path}' not found, skipping.")
            continue
        for root, _, files in os.walk(dir_path):
            for file in sorted(files):
                full_path = os.path.join(root, file)
                # Compute path relative to input_dir, using backslashes as PopCap engine expects
                rel_path = os.path.relpath(full_path, input_dir).replace('/', '\\')
                size = os.path.getsize(full_path)
                file_entries.append((rel_path, size, full_path))

    print(f"Found {len(file_entries)} asset files to pack.")
    
    total_unpacked_size = sum(f[1] for f in file_entries)
    print(f"Total uncompressed size: {total_unpacked_size / (1024*1024):.2f} MB")

    os.makedirs(os.path.dirname(os.path.abspath(output_pak_path)), exist_ok=True)
    with open(output_pak_path, "wb") as out_fp:
        # 1. Header: Magic and Version
        out_fp.write(struct.pack("<II", POP_PAK_MAGIC, POP_PAK_VERSION))
        
        # 2. File records table
        for rel_path, size, _ in file_entries:
            name_bytes = rel_path.encode("latin-1")
            name_len = len(name_bytes)
            if name_len > 255:
                raise ValueError(f"Path too long ({name_len} bytes): {rel_path}")
            
            # flags = 0, name_len
            out_fp.write(struct.pack("<BB", 0x00, name_len))
            out_fp.write(name_bytes)
            # file size
            out_fp.write(struct.pack("<i", size))
            # FILETIME (dwLowDateTime, dwHighDateTime)
            out_fp.write(struct.pack("<II", 0, 0))
            
        # 3. Terminate records table with 0x80 (FILEFLAGS_END)
        out_fp.write(struct.pack("<B", FILEFLAGS_END))
        
        # 4. Write data block (concatenated file bytes XORed with 0xF7)
        for rel_path, size, full_path in file_entries:
            with open(full_path, "rb") as in_fp:
                data = in_fp.read()
            # Fast XOR in python
            xored = bytes(b ^ 0xF7 for b in data)
            out_fp.write(xored)

    pak_size = os.path.getsize(output_pak_path)
    print(f"Successfully created {output_pak_path} ({pak_size / (1024*1024):.2f} MB).")

if __name__ == "__main__":
    src = sys.argv[1] if len(sys.argv) > 1 else r"c:\DEV\Zuma\Zuma Deluxe"
    dest = sys.argv[2] if len(sys.argv) > 2 else r"c:\DEV\Zuma\main.pak"
    pack_pak(src, dest)
