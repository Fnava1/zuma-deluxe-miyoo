@echo off
setlocal
title Zuma Deluxe — Miyoo Mini Port Builder
echo Starting Zuma Deluxe Port Builder Launcher...
cd /d "%~dp0"

where python >nul 2>nul
if %errorlevel% neq 0 (
    where py >nul 2>nul
    if %errorlevel% neq 0 (
        echo.
        echo ==============================================================
        echo ERROR: Python is not found on your system!
        echo Please install Python 3.8 or newer from https://www.python.org/
        echo (Make sure to check "Add Python to PATH" during installation)
        echo ==============================================================
        echo.
        pause
        exit /b 1
    ) else (
        py tools\gui_builder.py
    )
) else (
    python tools\gui_builder.py
)
