
#ifndef MJUMPER_DEBUG_DUMP_H
#define MJUMPER_DEBUG_DUMP_H

/**
 * Appends the given buffer to the given file as a hex dump in the rough style
 * of xxd. A small one-line hader is written before each dump to separate one
 * dump from the next. This is intended as a quick-and-dirty way to get
 * reliable dumps of binary data. Both dump.h and dump.c have no dependencies
 * (other than stdio) and can just be dropped wherever needed.
 *
 * @param file
 *     The name of the file to append the hex dump to.
 *
 * @param buffer
 *     The buffer to print.
 *
 * @param length
 *     The length of the buffer in bytes.
 */
void dump(const char* file, const void* buffer, int length);

#endif

