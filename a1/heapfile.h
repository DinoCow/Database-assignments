#ifndef HEAPFILE_H
#define HEAPFILE_H

#include <stdio.h>

#include "record.h"
#include "page.h"
#include "directory.h"

const int SLOTSIZE = 1000;

typedef struct {
    FILE *file_ptr;
    int page_size;
    int num_pages;
} Heapfile;

typedef int PageID;

typedef struct {
    int page_id;
    int slot;
} RecordID;

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);
/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile);

Entry* get_next_entry(Heapfile *heapfile, Directory *directory, int *offset);

Directory* read_directory(Heapfile *heapfile, int offset);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);
/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid); 

class RecordIterator {
    public:
    RecordIterator(Heapfile *heapfile);
    Record next();
    bool hasNext();
};

#endif