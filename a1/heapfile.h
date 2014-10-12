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
    Directory *directory;
    Page *page_buffer;
} Heapfile;

typedef int PageID;

typedef struct {
    int page_id;
    int slot;
} RecordID;

/**
 * Initalize a heapfile to use the page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size);

void close_heapfile(Heapfile *heapfile);

/**
 * Create the initial structure of a heapfile
 * This only needs to be used when creating new heapfile
 */
void create_heapfile(Heapfile *heapfile, char* filename);

void put_record(Heapfile* heapfile, Record *rec);

void get_record(Heapfile* heapfile, RecordID rid, Record *rec);

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile);

/**
 * Get the next entry that can be allocated from directory
 */
void get_next_entry(Heapfile *heapfile, Entry* entry);

/**
 * Write a directory into disk from memory
 */
void write_directory(Heapfile *heapfile, int offset, Directory *directory);

/*
 * read a directory into memory 
 */
void read_directory(Heapfile *heapfile, int offset, Directory *directory);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);
/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid); 

/**
 * get the page offset from the direcotry 
 */
int get_page_offset(Heapfile *heapfile, PageID pid);

class RecordIterator {
    public:
    RecordIterator(Heapfile *heapfile);
    Record next();
    bool hasNext();
};

#endif