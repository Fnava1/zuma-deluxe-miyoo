@echo off
setlocal
echo ==========================================================
echo  Building Zuma Deluxe for Miyoo Mini (Cortex-A7 NEON)
echo  Toolchain container: aemiii91/miyoomini-toolchain:latest
echo ==========================================================

for %%i in ("%~dp0..") do set "REPO_ROOT=%%~fi"

docker run --rm -v "%REPO_ROOT%:/root/workspace" aemiii91/miyoomini-toolchain:latest /bin/bash -c "cd /root/workspace && rm -rf build-miyoo && mkdir -p build-miyoo && cd build-miyoo && cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/miyoomini.toolchain.cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)"

if exist "%REPO_ROOT%\build-miyoo\source\CircleShoot\Zuma" (
    echo.
    echo ==========================================================
    echo  BUILD SUCCESSFUL!
    echo  Binary: build-miyoo\source\CircleShoot\Zuma
    echo ==========================================================
    if not exist "%REPO_ROOT%\bin" mkdir "%REPO_ROOT%\bin"
    copy /y "%REPO_ROOT%\build-miyoo\source\CircleShoot\Zuma" "%REPO_ROOT%\bin\Zuma"
    echo  Copied to: bin\Zuma
) else (
    echo Error: Binary not found after compilation.
)

pause
