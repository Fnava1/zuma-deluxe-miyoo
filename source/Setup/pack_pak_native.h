#ifndef PACK_PAK_NATIVE_H
#define PACK_PAK_NATIVE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pack_progress_cb)(int percentage, const char *status_msg);

/**
 * Packs asset directories from base_dir into a standard PopCap PAK archive (Magic 0xBAC04AC0).
 *
 * @param base_dir       Directory containing asset folders (fonts, images, levels, properties, sounds, music)
 * @param output_pak     Full path to destination main.pak
 * @param target_dirs    Array of folder names to include (or NULL for default list)
 * @param num_dirs       Number of elements in target_dirs (0 if target_dirs is NULL)
 * @param cb             Optional progress callback
 * @return               0 on success, negative error code on failure
 */
int pack_pak_native(const char *base_dir, const char *output_pak,
                    const char **target_dirs, size_t num_dirs,
                    pack_progress_cb cb);

#ifdef __cplusplus
}
#endif

#endif // PACK_PAK_NATIVE_H
