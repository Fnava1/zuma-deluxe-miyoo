#!/bin/sh
progdir="$(dirname "$0")"
cd "$progdir"

# Differentiate hardware model: Miyoo Mini Plus vs Miyoo Mini (v1/v2/v3/v4)
IS_PLUS=0
if [ -d /sys/class/net/wlan0 ] || grep -qi "plus" /proc/cmdline 2>/dev/null; then
    IS_PLUS=1
fi

# Set safe target CPU frequency:
# Miyoo Mini Plus: 1500 MHz (stable and power efficient)
# Miyoo Mini (non-Plus): 1300 MHz (stable, avoids voltage lockups on SSD202D)
if [ "$IS_PLUS" -eq 1 ]; then
    CPU_FREQ=1500
else
    CPU_FREQ=1300
fi

# Apply safe CPU clock using official OnionOS / system tool (adjusts voltage properly)
if [ -f /usr/trimui/bin/cpuclock ]; then
    /usr/trimui/bin/cpuclock $CPU_FREQ 2>/dev/null
elif [ -f /mnt/SDCARD/.tmp_update/bin/cpuclock ]; then
    /mnt/SDCARD/.tmp_update/bin/cpuclock $CPU_FREQ 2>/dev/null
fi
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null

# Clean up audioserver to prevent conflict with SDL2 MI_AO audio backend
killall -9 audioserver 2>/dev/null
killall -9 audioserver.mod 2>/dev/null

# Cleanup handler executed on normal exit or termination signal
cleanup() {
    # Restore CPU governor and clock back to stock (1200 MHz ondemand)
    echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
    if [ -f /usr/trimui/bin/cpuclock ]; then
        /usr/trimui/bin/cpuclock 1200 2>/dev/null
    elif [ -f /mnt/SDCARD/.tmp_update/bin/cpuclock ]; then
        /mnt/SDCARD/.tmp_update/bin/cpuclock 1200 2>/dev/null
    fi

    # Restore audioserver
    if [ -f /customer/app/audioserver ]; then
        /customer/app/audioserver &
    fi

    # Flush all dirty buffers to protect filesystem integrity and OnionOS configs
    sync
}
trap cleanup EXIT INT TERM HUP

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

exit 0
