#!/bin/sh
progdir="$(dirname "$0")"
cd "$progdir"

# Kill audioserver to prevent conflict with SDL2 MI_AO audio backend
killall -9 audioserver 2>/dev/null
killall -9 audioserver.mod 2>/dev/null

# Set CPU clock to maximum performance (1700 MHz for Miyoo Mini Plus on OnionOS)
if [ -f /usr/trimui/bin/cpuclock ]; then
    /usr/trimui/bin/cpuclock 1700 2>/dev/null || /usr/trimui/bin/cpuclock 1500 2>/dev/null
elif [ -f /mnt/SDCARD/.tmp_update/bin/cpuclock ]; then
    /mnt/SDCARD/.tmp_update/bin/cpuclock 1700 2>/dev/null || /mnt/SDCARD/.tmp_update/bin/cpuclock 1500 2>/dev/null
fi
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
echo 1700000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 2>/dev/null

# Miyoo SDL2 driver optimizations
export SDL_MMIYOO_TEXTURE_POOL=1
export SDL_MMIYOO_TEXTURE_POOL_MAX_BYTES=16777216
export SDL_MMIYOO_VSYNC_MODE=off
export SDL_HINT_RENDER_SCALE_QUALITY=0

# Set library path prioritizing local libs then Miyoo Mini hardware libraries
export LD_LIBRARY_PATH="$progdir/libs:/config/lib:/customer/lib:/lib:/usr/lib:$LD_LIBRARY_PATH"

# Prevent MainUI preloads from interfering with SDL2
unset LD_PRELOAD

# Run Zuma Deluxe without logging
./Zuma > /dev/null 2>&1

# Restore audioserver after Zuma exits
if [ -f /customer/app/audioserver ]; then
    /customer/app/audioserver &
fi

# Sync filesystem before returning to OnionOS launcher
sync
