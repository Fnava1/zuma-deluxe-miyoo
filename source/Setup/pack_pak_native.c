#include "pack_pak_native.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEP '\\'
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEP '/'
#endif

#define POP_PAK_MAGIC   0xBAC04AC0
#define POP_PAK_VERSION 0x00000000
#define FILEFLAGS_END   0x80
#define CHUNK_SIZE      65536

static const char *DEFAULT_TARGET_DIRS[] = {
    "fonts", "images", "levels", "music", "properties", "sounds"
};
#define NUM_DEFAULT_DIRS (sizeof(DEFAULT_TARGET_DIRS) / sizeof(DEFAULT_TARGET_DIRS[0]))

typedef struct {
    char rel_path[256];
    char full_path[1024];
    uint32_t size;
} FileEntry;

typedef struct {
    FileEntry *entries;
    size_t count;
    size_t capacity;
} FileList;

static void file_list_init(FileList *fl) {
    fl->count = 0;
    fl->capacity = 256;
    fl->entries = (FileEntry *)malloc(fl->capacity * sizeof(FileEntry));
}

static void file_list_add(FileList *fl, const char *rel_path, const char *full_path, uint32_t size) {
    if (fl->count >= fl->capacity) {
        fl->capacity *= 2;
        fl->entries = (FileEntry *)realloc(fl->entries, fl->capacity * sizeof(FileEntry));
    }
    FileEntry *entry = &fl->entries[fl->count++];
    strncpy(entry->rel_path, rel_path, sizeof(entry->rel_path) - 1);
    entry->rel_path[sizeof(entry->rel_path) - 1] = '\0';
    strncpy(entry->full_path, full_path, sizeof(entry->full_path) - 1);
    entry->full_path[sizeof(entry->full_path) - 1] = '\0';
    entry->size = size;
}

static void file_list_free(FileList *fl) {
    if (fl->entries) {
        free(fl->entries);
        fl->entries = NULL;
    }
    fl->count = 0;
    fl->capacity = 0;
}

static int compare_entries(const void *a, const void *b) {
    const FileEntry *fa = (const FileEntry *)a;
    const FileEntry *fb = (const FileEntry *)b;
    return strcmp(fa->rel_path, fb->rel_path);
}

static void scan_directory_recursive(FileList *fl, const char *base_dir, const char *sub_path) {
    char current_full[1024];
    if (sub_path && sub_path[0]) {
        snprintf(current_full, sizeof(current_full), "%s/%s", base_dir, sub_path);
    } else {
        snprintf(current_full, sizeof(current_full), "%s", base_dir);
    }

#ifdef _WIN32
    char search_pattern[1024];
    snprintf(search_pattern, sizeof(search_pattern), "%s/*", current_full);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        char child_rel[512];
        if (sub_path && sub_path[0]) {
            snprintf(child_rel, sizeof(child_rel), "%s/%s", sub_path, fd.cFileName);
        } else {
            snprintf(child_rel, sizeof(child_rel), "%s", fd.cFileName);
        }

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_directory_recursive(fl, base_dir, child_rel);
        } else {
            char file_full[1024];
            snprintf(file_full, sizeof(file_full), "%s/%s", base_dir, child_rel);
            
            // Format relative path with backslashes for PopCap engine
            char pak_rel[256];
            strncpy(pak_rel, child_rel, sizeof(pak_rel) - 1);
            pak_rel[sizeof(pak_rel) - 1] = '\0';
            for (size_t i = 0; pak_rel[i]; i++) {
                if (pak_rel[i] == '/') pak_rel[i] = '\\';
            }

            file_list_add(fl, pak_rel, file_full, (uint32_t)fd.nFileSizeLow);
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
#else
    DIR *dir = opendir(current_full);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char child_rel[512];
        if (sub_path && sub_path[0]) {
            snprintf(child_rel, sizeof(child_rel), "%s/%s", sub_path, entry->d_name);
        } else {
            snprintf(child_rel, sizeof(child_rel), "%s", entry->d_name);
        }

        char child_full[1024];
        snprintf(child_full, sizeof(child_full), "%s/%s", base_dir, child_rel);

        struct stat st;
        if (stat(child_full, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_directory_recursive(fl, base_dir, child_rel);
            } else if (S_ISREG(st.st_mode)) {
                char pak_rel[256];
                strncpy(pak_rel, child_rel, sizeof(pak_rel) - 1);
                pak_rel[sizeof(pak_rel) - 1] = '\0';
                for (size_t i = 0; pak_rel[i]; i++) {
                    if (pak_rel[i] == '/') pak_rel[i] = '\\';
                }
                file_list_add(fl, pak_rel, child_full, (uint32_t)st.st_size);
            }
        }
    }
    closedir(dir);
#endif
}

int pack_pak_native(const char *base_dir, const char *output_pak,
                    const char **target_dirs, size_t num_dirs,
                    pack_progress_cb cb) {
    if (!base_dir || !output_pak) return -1;

    if (!target_dirs || num_dirs == 0) {
        target_dirs = DEFAULT_TARGET_DIRS;
        num_dirs = NUM_DEFAULT_DIRS;
    }

    if (cb) cb(5, "Escaneando directorios de assets...");

    FileList fl;
    file_list_init(&fl);

    for (size_t i = 0; i < num_dirs; i++) {
        char dir_check[1024];
        snprintf(dir_check, sizeof(dir_check), "%s/%s", base_dir, target_dirs[i]);
        struct stat st;
        if (stat(dir_check, &st) == 0 && (st.st_mode & S_IFDIR)) {
            scan_directory_recursive(&fl, base_dir, target_dirs[i]);
        }
    }

    if (fl.count == 0) {
        file_list_free(&fl);
        return -2; // No assets found
    }

    // Sort entries alphabetically
    qsort(fl.entries, fl.count, sizeof(FileEntry), compare_entries);

    FILE *out = fopen(output_pak, "wb");
    if (!out) {
        file_list_free(&fl);
        return -3; // Output file cannot be opened
    }

    if (cb) cb(15, "Escribiendo encabezados del paquete...");

    // 1. Magic and Version
    uint32_t magic = POP_PAK_MAGIC;
    uint32_t version = POP_PAK_VERSION;
    fwrite(&magic, sizeof(uint32_t), 1, out);
    fwrite(&version, sizeof(uint32_t), 1, out);

    // 2. Records table
    for (size_t i = 0; i < fl.count; i++) {
        FileEntry *fe = &fl.entries[i];
        uint8_t flags = 0x00;
        uint8_t name_len = (uint8_t)strlen(fe->rel_path);
        uint32_t file_size = fe->size;
        uint32_t file_time[2] = {0, 0};

        fwrite(&flags, sizeof(uint8_t), 1, out);
        fwrite(&name_len, sizeof(uint8_t), 1, out);
        fwrite(fe->rel_path, sizeof(char), name_len, out);
        fwrite(&file_size, sizeof(uint32_t), 1, out);
        fwrite(file_time, sizeof(uint32_t), 2, out);
    }

    // 3. End of records marker
    uint8_t end_marker = FILEFLAGS_END;
    fwrite(&end_marker, sizeof(uint8_t), 1, out);

    // 4. Data payload with 0xF7 XOR mask
    uint8_t buffer[CHUNK_SIZE];
    uint64_t total_bytes_written = 0;
    uint64_t total_bytes = 0;
    for (size_t i = 0; i < fl.count; i++) {
        total_bytes += fl.entries[i].size;
    }

    for (size_t i = 0; i < fl.count; i++) {
        FileEntry *fe = &fl.entries[i];
        FILE *in = fopen(fe->full_path, "rb");
        if (!in) {
            continue;
        }

        size_t nread;
        while ((nread = fread(buffer, 1, sizeof(buffer), in)) > 0) {
            for (size_t b = 0; b < nread; b++) {
                buffer[b] ^= 0xF7;
            }
            fwrite(buffer, 1, nread, out);
            total_bytes_written += nread;
        }
        fclose(in);

        if (cb && total_bytes > 0) {
            int pct = 20 + (int)((total_bytes_written * 75) / total_bytes);
            char status_msg[128];
            snprintf(status_msg, sizeof(status_msg), "Empaquetando: %s", fe->rel_path);
            cb(pct, status_msg);
        }
    }

    fclose(out);
    file_list_free(&fl);

    if (cb) cb(98, "main.pak creado exitosamente.");
    return 0;
}
