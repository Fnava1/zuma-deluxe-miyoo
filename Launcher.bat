@echo off
setlocal
chcp 65001 >nul
title Zuma Deluxe - Miyoo Mini Port Builder
echo Starting Zuma Deluxe Port Builder Launcher...
cd /d "%~dp0"

:: Try python directly first
python --version >nul 2>nul
if %errorlevel% equ 0 (
    python tools\gui_builder.py
    exit /b %errorlevel%
)

:: Try py launcher
py --version >nul 2>nul
if %errorlevel% equ 0 (
    py tools\gui_builder.py
    exit /b %errorlevel%
)

:: Try standard Python installation paths on Windows
if exist "%LOCALAPPDATA%\Programs\Python\Python313\python.exe" (
    "%LOCALAPPDATA%\Programs\Python\Python313\python.exe" tools\gui_builder.py
    exit /b %errorlevel%
)
if exist "%LOCALAPPDATA%\Programs\Python\Python312\python.exe" (
    "%LOCALAPPDATA%\Programs\Python\Python312\python.exe" tools\gui_builder.py
    exit /b %errorlevel%
)
if exist "%LOCALAPPDATA%\Programs\Python\Python311\python.exe" (
    "%LOCALAPPDATA%\Programs\Python\Python311\python.exe" tools\gui_builder.py
    exit /b %errorlevel%
)

echo.
echo ==============================================================
echo ERROR: Python 3 was not found on your system!
echo Please install Python 3.8+ from https://www.python.org/
echo (Make sure to check "Add Python to PATH" during installation)
echo ==============================================================
echo.
pause
exit /b 1
