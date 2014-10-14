#ifndef HEAPFILE_H
#define HEAPFILE_H

#include <stdio.h>
#include <vector>

#include "record.h"
#include "page.h"
#include "directory.h"

const int SLOTSIZE = 1000;

typedef int PageID;

typedef struct {
    int page_id;
    int slot;
} RecordID;

typedef struct {
    FILE *file_ptr;
    int page_size;
    std::vector<Directory*> directory_buffer;
    // The following variables are used for page cache
    Page *cached_page;
    PageID cached_pid;
    bool page_cache_dirty;
} Heapfile;

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size);

/**
 * Create the initial structure of a heapfile
 * This only needs to be used when creating new heapfile
 */
void create_heapfile(Heapfile* heapfile, char *filename);

void open_heapfile(Heapfile* heapfile, char *filename);

void close_heapfile(Heapfile *heapfile);

void put_record(Heapfile* heapfile, Record *rec);
    
void get_record(Heapfile* heapfile, RecordID *rid, Record *rec);

void update_record(Heapfile* heapfile, RecordID *rid, Record *rec);

void delete_record(Heapfile *heapfile, RecordID *rid);
/* Search existing pages with free space.
 * May allocate new page
 */
PageID get_free_pid(Heapfile *heapfile);

Page* get_page(Heapfile *heapfile, PageID pid);

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile);

/**
 * Read a page size block into memory
 */
void read_block(Heapfile *heapfile, char *buffer, int offset);

/**
 * Write a page size block from memory to disk
 */
void write_block(Heapfile *heapfile, char *buffer, int offset);

void commit_page(Heapfile *heapfile, PageID pid);


PageID append_entry(Heapfile *heapfile, Entry *entry);

// get entry from pid
Entry *get_entry(Heapfile *heapfile, PageID pid);

PageID vacant_page_id(Heapfile *heapfile);

class RecordIterator {
public:
    RecordIterator(Heapfile *heapfile);
    Record next();
    bool hasNext();
    RecordID nextRID();
private:
    void increment_iterator();
    Heapfile *heapfile;
    PageID pid;
    int slot;
    bool has_next;
};

#endif