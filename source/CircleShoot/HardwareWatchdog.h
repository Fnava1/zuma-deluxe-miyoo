#pragma once

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class HardwareWatchdog
{
private:
    static pthread_t sThread;
    static volatile bool sRunning;
    static int sTargetCpuMhz;

    static void RestoreCpuClockFast(int targetMhz)
    {
        // 1. Re-enable CPU core 1 if OnionOS/kernel put it offline during sleep
        int fd_cpu1 = open("/sys/devices/system/cpu/cpu1/online", O_WRONLY);
        if (fd_cpu1 >= 0)
        {
            write(fd_cpu1, "1\n", 2);
            close(fd_cpu1);
        }

        // 2. Restore scaling governor to performance
        int fd_gov = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", O_WRONLY);
        if (fd_gov >= 0)
        {
            write(fd_gov, "performance\n", 12);
            close(fd_gov);
        }

        // 3. Write target frequency in kHz to scaling_setspeed
        if (targetMhz > 0)
        {
            char freqStr[32];
            int len = snprintf(freqStr, sizeof(freqStr), "%d000\n", targetMhz);
            if (len > 0)
            {
                int fd_speed = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", O_WRONLY);
                if (fd_speed >= 0)
                {
                    write(fd_speed, freqStr, len);
                    close(fd_speed);
                }
            }
        }
    }

    static void* WatchdogThreadFunc(void* arg)
    {
        (void)arg;
        struct timespec lastTs;
        clock_gettime(CLOCK_MONOTONIC, &lastTs);

        while (sRunning)
        {
            // Sleep 250 ms
            struct timespec req;
            req.tv_sec = 0;
            req.tv_nsec = 250000000L; // 250ms
            nanosleep(&req, NULL);

            if (!sRunning) break;

            struct timespec curTs;
            clock_gettime(CLOCK_MONOTONIC, &curTs);

            uint64_t elapsedMs = ((uint64_t)curTs.tv_sec - (uint64_t)lastTs.tv_sec) * 1000ULL +
                                 ((int64_t)curTs.tv_nsec - (int64_t)lastTs.tv_nsec) / 1000000LL;
            lastTs = curTs;

            // If more than 1000 ms elapsed between 250 ms iterations,
            // the console was suspended (keymon sent SIGSTOP).
            if (elapsedMs > 1000)
            {
                RestoreCpuClockFast(sTargetCpuMhz);
            }
        }
        return NULL;
    }

public:
    static void Start()
    {
        if (sRunning) return;

        // Determine target CPU frequency from environment or hardware model
        const char* targetEnv = getenv("TARGET_CPU_MHZ");
        if (targetEnv && targetEnv[0] != '\0')
        {
            sTargetCpuMhz = atoi(targetEnv);
        }
        if (sTargetCpuMhz <= 0)
        {
            // Fallback safe detection: check /tmp/deviceModel or /customer/app/axp_test
            int fd_model = open("/tmp/deviceModel", O_RDONLY);
            bool isPlus = false;
            if (fd_model >= 0)
            {
                char buf[16] = {0};
                read(fd_model, buf, sizeof(buf) - 1);
                close(fd_model);
                if (strstr(buf, "354") != NULL) isPlus = true;
            }
            if (!isPlus && access("/customer/app/axp_test", F_OK) == 0)
            {
                isPlus = true;
            }
            sTargetCpuMhz = isPlus ? 1500 : 1300;
        }

        sRunning = true;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 64 * 1024); // Minimal 64KB stack
        pthread_create(&sThread, &attr, WatchdogThreadFunc, NULL);
        pthread_attr_destroy(&attr);
    }

    static void Stop()
    {
        if (!sRunning) return;
        sRunning = false;
        pthread_join(sThread, NULL);
    }
};

#ifdef DEFINE_HARDWARE_WATCHDOG_STATICS
pthread_t HardwareWatchdog::sThread = 0;
volatile bool HardwareWatchdog::sRunning = false;
int HardwareWatchdog::sTargetCpuMhz = 1500;
#endif

#else // _WIN32
class HardwareWatchdog {
public:
    static void Start() {}
    static void Stop() {}
};
#endif
