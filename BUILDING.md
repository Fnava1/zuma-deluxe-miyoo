# Building Zuma Deluxe for Miyoo Mini / OnionOS

This document covers the steps required to compile the native ARM binary for the **Miyoo Mini** and **Miyoo Mini Plus** handhelds.

---

## 1. Prerequisites

The recommended method to build the port is using the official community Docker toolchain container: `aemiii91/miyoomini-toolchain:latest`.

- **Docker Desktop** (Windows / macOS) or **Docker Engine** (Linux) must be installed and running.
- Alternatively, you can use any custom sysroot containing `arm-linux-gnueabihf-gcc` (glibc 2.28 or compatible).

---

## 2. Quick Build with Docker (Recommended)

### Linux / macOS
From the root of the repository:

```bash
chmod +x tools/build_docker.sh
./tools/build_docker.sh
```

### Windows (Command Prompt or PowerShell)
From the root of the repository:

```cmd
tools\build_docker.bat
```

### What Happens:
1. Docker pulls `aemiii91/miyoomini-toolchain:latest` (if not already cached).
2. It sets up the build directory `build-miyoo/`.
3. Invokes CMake using the cross-toolchain configuration [`cmake/miyoomini.toolchain.cmake`](cmake/miyoomini.toolchain.cmake).
4. Compiles the engine sources and static libraries in parallel (`make -j$(nproc)`).
5. Copies the output executable to `bin/Zuma`.

---

## 3. Manual Cross-Compilation

If you have a local cross-toolchain installed without Docker:

```bash
mkdir -p build-miyoo && cd build-miyoo

cmake \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/miyoomini.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  ..

make -j$(nproc)
```

The compiled binary will be generated at:
`build-miyoo/source/CircleShoot/Zuma`

---

## 4. Architecture & Compiler Flags

The build system applies performance-critical flags tuned specifically for the SigmaStar SSD202D dual-core ARM Cortex-A7 processor:

- **Target CPU**: `-mcpu=cortex-a7`
- **Floating Point / SIMD**: `-mfpu=neon-vfpv4 -mfloat-abi=hard`
- **Optimization Level**: `-O3 -fno-strict-aliasing -ftree-vectorize -fomit-frame-pointer`
- **Runtime Search Path**: Embedded RPATH configured for `$ORIGIN/libs` to load optimized SDL2 and NEON libraries locally without modifying system firmware libraries.

---

## 5. Output Verification

You can verify the compiled binary with `readelf` or `file`:

```bash
file bin/Zuma
# Output:
# bin/Zuma: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, ...
```
