#!/usr/bin/env python3
"""
Zuma Deluxe Asset Preparation & Packaging Tool for Miyoo Mini / OnionOS
(Wrapper for tools/build_release.py)
"""
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)

from build_release import main

if __name__ == "__main__":
    main()
