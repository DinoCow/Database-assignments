#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "directoryentry.h"

typedef struct{
	int page_size;
	void *data;
} Directory;

/**
 * Initializes a directory using the entry size
 */
void init_fixed_len_page(Directory *directory, int page_size);

/**
 * Calculates the maximal number of records that fit in a directory
 */
int fixed_len_page_capacity(Directory *directory);
 
/**
 * Calculate the free space (number of free slots) in the directory
 */
int fixed_len_page_freeslots(Directory *directory);
 
/**
 * Add a entry to the directory
 * Returns:
 *   entry slot offset if successful,
 *   -1 if unsuccessful (directory full)
 */
int add_fixed_len_page(Directory *directory, DirectoryEntry *entry);
 
/**
 * Write a entry into a given slot.
 */
void write_fixed_len_page(Directory *directory, int slot, DirectoryEntry *entry);
 
/**
 * Read a entry from the directory from a given slot.
 */
void read_fixed_len_page(Directory *directory, int slot, DirectoryEntry *entry);


#endif