#ifndef MO3_DECODE_H
#define MO3_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts a .mo3 music file to standard Impulse Tracker (.it) format.
 * Dynamically loads libunmo3.so from candidate locations.
 *
 * @param input_mo3_path   Path to zuma.mo3
 * @param output_it_path   Path to destination zuma.it
 * @param libunmo3_path    Optional explicit path to libunmo3.so (or NULL to search standard paths)
 * @return                 0 on success, negative error code on failure:
 *                         -1: Invalid arguments
 *                         -2: Input file not found or cannot be read
 *                         -3: Failed to load libunmo3.so via dlopen
 *                         -4: UNMO3_Decode returned error (corrupted or invalid format)
 *                         -5: Failed to write output file
 */
int convert_mo3_to_it(const char *input_mo3_path, const char *output_it_path, const char *libunmo3_path);

#ifdef __cplusplus
}
#endif

#endif // MO3_DECODE_H
