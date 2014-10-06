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
	Directory *directory = new Directory;
	init_directory_page(directory, page_size);

	FILE *fp = fopen(*file, "w");
	if (!fp) {
        printf("can not open heapfile\n");
        // bail out ?
    }

    fwrite(directory->data, 1, page_size, fp);
    fflush(fp);
    fclose(fp);

    heapfile->num_pages = 1;
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
	int directory_offset = 0;

	FILE *fp = fopen(heapfile->file_ptr, "r+");
	if (!fp) {
        printf("can not open heapfile\n");
		// bail out ?
    }

	Directory *directory = read_directory(heapfile, 0, fp);

	//allocate an entry
	Entry *entry = new Entry;
	entry = get_next_entry(heapfile, directory, fp, &directory_offset);

	Page *page = new Page; 	
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);
	
	fseek(fp, 0, SEEK_END);
	fwrite(page, 1, heapfile->page_size, fp);

	fseek(fp, *offset, SEEK_SET)
	entry->offset = ftell(fp);
    entry->free_space = fixed_len_page_freeslots(page);
    fwrite(directory->data, 1, heapfile->page_size, fp);
    fflush(fp);
	fclose(fp);

	PageID pid = 0;
	return pid;
}

Entry* get_next_entry(Heapfile *heapfile, Directory *directory, FILE *fp, int *offset){
	Entry *entry = new Entry; 
	entry = next_entry(directory);
	if (entry == NULL && *directory->next_directory != 0) {		
		//directory is full and points to another
		directory = read_directory( heapfile, directory->next_directory, fp);
		*offset = directory->next_directory;
		return get_next_entry( heapfile, directory, fp, directory->next_directory);

	} else if (entry == NULL){		
		//add a new directory
		fseek(fp, 0, SEEK_END);
		directory->next_directory = ftell(fp);

		Directory *new_directory = new Directory;
		init_directory_page(new_directory, heapfile->page_size);

    	entry = next_entry(new_directory);
		//go back to write to the directory
		fseek(fp, offset, SEEK_SET);
		fwrite(directory->data, 1, heapfile->page_size, fp);

		offset = directory->next_directory;
		directory = new_directory;
		return next_entry(directory);
	} else {
		return next_entry(directory);
	}

}

Directory* read_directory(Heapfile *heapfile, int offset, FILE *fp){
	fseek(fp, offset, SEEK_SET);
	char buf[heapfile->page_size];

	int result = fread(buf, 1, heapfile->page_size,fp);
	if(result != heapfile->page_size) { 
		printf("panic panic, can't read directory \n");
		return NULL;
	}

	Directory *directory = new Directory;
	init_directory_page(directory, heapfile->page_size, buf);
	return directory;
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){}

