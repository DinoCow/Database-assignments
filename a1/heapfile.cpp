#include "heapfile.h"

#include <stdio.h>
#include <cassert>

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file) {
	assert(page_size > 1001);

	heapfile->file_ptr = file;
	heapfile->page_size = page_size;

	//create the directory page
	char buf[page_size];

	Directory *directory = new Directory;
	init_directory_page(directory, page_size);

	buf = directory->data

	FILE *fp = fopen(file, "w");
	if (!fp) {
        printf("can not open heapfile\n");
        return;   // bail out ?
    }

    bzero(buf, page_size);
    fwrite(buf, 1, page_size, fp);
    fflush(fp);
    fclose(fp);

    heapfile->num_pages = 1;
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
	
	FILE *fp = fopen(heapfile->file_ptr, "a");


 	Page *page = new Page; 	
 	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);

 	PageID pid = heapfile->num_pages;

 	//should we calculate this ahead of time in the init?
	int free_slots = fixed_len_page_freeslots(page);
	
	PageEntry page_entry = new PageEntry;
	page_entry->offset = 
	page_entry->free_space =

	FILE *fp = fopen(heapfile->file_ptr, "a");
	if (!fp) {
        printf("can not open heapfile\n");
        return;   // bail out ?
    }

    fsee

    //write the page 

	//write page_offset, freespace at the end of the heapfile

	//close file

	//return the PageID
 	
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){}

