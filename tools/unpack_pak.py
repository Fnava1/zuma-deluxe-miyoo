#!/usr/bin/env python3
"""
PopCap PAK Unpacker for Zuma Deluxe
Extracts all game assets from official PopCap main.pak archive (Magic: 0xBAC04AC0)
with XOR 0xF7 decryption.
"""

import os
import sys
import struct

POP_PAK_MAGIC = 0xBAC04AC0

def unpack_pak(pak_path, output_dir):
    if not os.path.isfile(pak_path):
        print(f"Error: PAK file '{pak_path}' not found.")
        return False
        
    print(f"Reading PAK archive: {pak_path}")
    with open(pak_path, "rb") as f:
        magic, ver = struct.unpack("<II", f.read(8))
        if magic != POP_PAK_MAGIC:
            print(f"Error: Invalid PAK magic (0x{magic:08X}), expected 0x{POP_PAK_MAGIC:08X}")
            return False
            
        file_entries = []
        while True:
            flag_byte = f.read(1)
            if not flag_byte:
                break
            flags = flag_byte[0]
            if flags & 0x80:
                break
            name_len = f.read(1)[0]
            rel_name = f.read(name_len).decode("latin-1").replace("\\", os.sep).replace("/", os.sep)
            size, low_time, high_time = struct.unpack("<iII", f.read(12))
            file_entries.append((rel_name, size))
            
        print(f"Found {len(file_entries)} file records in archive.")
        
        # Extract files sequentially
        for rel_name, size in file_entries:
            dest_file = os.path.join(output_dir, rel_name)
            os.makedirs(os.path.dirname(os.path.abspath(dest_file)), exist_ok=True)
            
            raw_data = f.read(size)
            if len(raw_data) != size:
                print(f"Warning: Unexpected EOF reading '{rel_name}' (expected {size}, got {len(raw_data)})")
                
            # XOR 0xF7 decryption
            decrypted = bytes(b ^ 0xF7 for b in raw_data)
            with open(dest_file, "wb") as out_f:
                out_f.write(decrypted)
                
    print(f"Successfully unpacked {len(file_entries)} assets to: {output_dir}")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python unpack_pak.py <path_to_main.pak> [output_directory]")
        sys.exit(1)
    pak_file = sys.argv[1]
    out_dir = sys.argv[2] if len(sys.argv) > 2 else "unpacked_assets"
    unpack_pak(pak_file, out_dir)
