#!/bin/sh
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

if command -v python3 >/dev/null 2>&1; then
    python3 tools/gui_builder.py "$@"
elif command -v python >/dev/null 2>&1; then
    python tools/gui_builder.py "$@"
else
    echo "Error: Python 3 is required to run the Port Builder launcher."
    exit 1
fi
