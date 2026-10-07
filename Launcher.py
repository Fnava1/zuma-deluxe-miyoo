#!/usr/bin/env python3
"""
Zuma Deluxe — Port Builder Launcher (Python entrypoint)
Runs the graphical builder interface without triggering Windows SmartScreen batch script warnings.
"""

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TOOLS_DIR = os.path.join(SCRIPT_DIR, "tools")
sys.path.insert(0, TOOLS_DIR)

from gui_builder import main

if __name__ == "__main__":
    main()
