#include "mo3_decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

typedef unsigned int (*fn_UNMO3_GetVersion)(void);
typedef int (*fn_UNMO3_Decode)(void **data, unsigned int *len, unsigned int flags);
typedef void (*fn_UNMO3_Free)(void *data);

static const char *DEFAULT_LIB_CANDIDATES[] = {
    "./libs/libunmo3.so",
    "libs/libunmo3.so",
    "libunmo3.so",
    "/mnt/SDCARD/Roms/PORTS/Games/Zuma Deluxe/libs/libunmo3.so"
};
#define NUM_LIB_CANDIDATES (sizeof(DEFAULT_LIB_CANDIDATES) / sizeof(DEFAULT_LIB_CANDIDATES[0]))

int convert_mo3_to_it(const char *input_mo3_path, const char *output_it_path, const char *libunmo3_path) {
    if (!input_mo3_path || !output_it_path) return -1;

    // Check if input file exists and read it
    FILE *in = fopen(input_mo3_path, "rb");
    if (!in) {
        return -2;
    }

    fseek(in, 0, SEEK_END);
    long in_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (in_size <= 0) {
        fclose(in);
        return -2;
    }

    void *mo3_buf = malloc((size_t)in_size);
    if (!mo3_buf) {
        fclose(in);
        return -2;
    }

    if (fread(mo3_buf, 1, (size_t)in_size, in) != (size_t)in_size) {
        free(mo3_buf);
        fclose(in);
        return -2;
    }
    fclose(in);

    // Resolve libunmo3
#ifndef _WIN32
    void *handle = NULL;
    if (libunmo3_path && libunmo3_path[0]) {
        handle = dlopen(libunmo3_path, RTLD_NOW);
    }
    if (!handle) {
        for (size_t i = 0; i < NUM_LIB_CANDIDATES; i++) {
            handle = dlopen(DEFAULT_LIB_CANDIDATES[i], RTLD_NOW);
            if (handle) break;
        }
    }
    if (!handle) {
        free(mo3_buf);
        return -3; // Failed to load libunmo3.so
    }

    fn_UNMO3_Decode decode_fn = (fn_UNMO3_Decode)dlsym(handle, "UNMO3_Decode");
    fn_UNMO3_Free free_fn = (fn_UNMO3_Free)dlsym(handle, "UNMO3_Free");

    if (!decode_fn || !free_fn) {
        dlclose(handle);
        free(mo3_buf);
        return -3;
    }

    unsigned int out_len = 0;
    int res = decode_fn(&mo3_buf, &out_len, 0);
    if (res != 0 || out_len == 0) {
        dlclose(handle);
        free(mo3_buf);
        return -4; // Decoding error
    }

    FILE *out = fopen(output_it_path, "wb");
    if (!out) {
        free_fn(mo3_buf);
        dlclose(handle);
        return -5;
    }

    size_t written = fwrite(mo3_buf, 1, out_len, out);
    fclose(out);
    free_fn(mo3_buf);
    dlclose(handle);

    if (written != out_len) {
        return -5;
    }
#else
    // Windows fallback: not directly used on device, but for compilation testing
    free(mo3_buf);
    return -3;
#endif

    return 0;
}
