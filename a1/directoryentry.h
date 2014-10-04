#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

#include <vector>

typedef const char* DirectoryEntry;
typedef std::vector<V> DirectoryEntry;
const int ENTRY_SIZE = 4; //bytes

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(DirectoryEntry *entry, void *buf);

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, DirectoryEntry *entry);

#endif