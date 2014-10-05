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

	buf = directory->data;

	FILE *fp = fopen(file, "w");
	if (!fp) {
        printf("can not open heapfile\n");
        return;   // bail out ?
    }

    fwrite(buf, 1, page_size, fp);
    fflush(fp);
    fclose(fp);

    heapfile->num_pages = 1;
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
	
	FILE *fp = fopen(heapfile->file_ptr, "r+");
	if (!fp) {
        printf("can not open heapfile\n");
        return;   // bail out ?
    }

	char buf[heapfile->page_size];

	result = fread(buf,1,heapfile->page_size,fp);
	if(result != heapfile->page_size) { 
		printf("panic panic, can't read directory \n");
		return;
	}

	Directory *directory = new Directory;
	init_directory_page(directory, heapfile->page_size, buf);

	Page *page = new Page; 	
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);
	
	//allocate an entry
	Entry* entry = next_entry(directory);
	fseek(file, 0, SEEK_END);

	if(entry == NULL){
		//need to check if this directory has called another
		directory->next_directory = ftell(fp);
		//allocate new heap page
		Directory *new_directory = new Directory;
		init_directory_page(new_directory, heapfile->page_size);

		buf = new_directory->data;
    	entry = next_entry(new_directory);
    }

    fwrite(buf, 1, heapfile->page_size, fp);

    int entry->offset = ftell(fp);
    int entry->free_space = fixed_len_page_freeslots(page);

    buf = page->data;
    fwrite(buf, 1, heapfile->page_size, fp);

	fclose(fp);
 	
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){}

