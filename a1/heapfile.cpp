#include "heapfile.h"

#include <stdio.h>
#include <cassert>

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file) {
	assert(page_size > 1001);

	heapfile->file_ptr= file;
	heapfile->page_size = page_size;

	//create the directory page
	Directory *directory = new Directory;
	init_directory_page(directory, page_size);

 	fwrite(directory->data, 1, page_size, file);

    fflush(file);

    heapfile->num_pages = 1;
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
	int directory_offset = 0;

	fseek(heapfile->file_ptr, 0, SEEK_SET);
	Directory *directory = new Directory;
	read_directory(heapfile, 0, directory);

	//allocate an entry
	Entry *entry = new Entry;
	get_next_entry(heapfile, directory, &directory_offset, entry);

	Page *page = new Page; 	
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);
	
	fseek(heapfile->file_ptr, 0, SEEK_END);
	fwrite(page->data, 1, heapfile->page_size, heapfile->file_ptr);

	fseek(heapfile->file_ptr, directory_offset, SEEK_SET);
	entry->offset = ftell(heapfile->file_ptr);
    entry->free_space = fixed_len_page_freeslots(page);
    fwrite(directory->data, 1, heapfile->page_size, heapfile->file_ptr);
    fflush(heapfile->file_ptr);

    //just return the offset, that's the index
	PageID pid = directory_offset + sizeof(int) * 2 + (*directory->n_entries - 1) *  sizeof(Entry);
	return pid;
}

void get_next_entry(Heapfile *heapfile, Directory *directory, int *directory_offset, Entry* entry){
	entry = next_entry(directory);
	if (entry == NULL && *directory->next_directory != 0) {		
		//directory is full and points to another
		*directory_offset = *directory->next_directory;
		read_directory( heapfile, *directory_offset, directory);
		get_next_entry( heapfile, directory, directory_offset, entry);

	} else if (entry == NULL){		
		//add a new directory
		fseek(heapfile->file_ptr, 0, SEEK_END);
		*directory->next_directory = ftell(heapfile->file_ptr);

		Directory *new_directory = new Directory;
		init_directory_page(new_directory, heapfile->page_size);
    	
		//go back to write to the directory
		fseek(heapfile->file_ptr, *directory_offset, SEEK_SET);
		fwrite(directory->data, 1, heapfile->page_size, heapfile->file_ptr);

		//assign all the return values
		*directory_offset = *directory->next_directory;
		directory = new_directory;
		entry = next_entry(new_directory);
	} else {
		entry = next_entry(directory);
	}

}

void read_directory(Heapfile *heapfile, int offset, Directory *directory){
	fseek(heapfile->file_ptr, offset, SEEK_SET);
	char buf[heapfile->page_size];

	int result = fread(buf, 1, heapfile->page_size, heapfile->file_ptr);
	if(result != heapfile->page_size) { 
		printf("panic panic, can't read directory \n");
	}

	init_directory_page(directory, heapfile->page_size, buf);
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){
	int page_offset = get_page_offset(heapfile, pid);

	fseek(heapfile->file_ptr, page_offset, SEEK_SET);

	char page_data[heapfile->page_size];

	int result = fread(page_data, 1, heapfile->page_size, heapfile->file_ptr);
	if(result != heapfile->page_size){
		printf("panic panic, can't read Page \n");
	}

	page->data = page_data;
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){
	int page_offset = get_page_offset(heapfile, pid);

	fseek(heapfile->file_ptr, page_offset, SEEK_SET);

	int result = fwrite(page->data, 1, heapfile->page_size, heapfile->file_ptr);

	if (result != heapfile->page_size){
		printf("panic panic, can't read page");
	}

	fflush(heapfile->file_ptr);
}

int get_page_offset(Heapfile *heapfile, PageID pid){
	fseek(heapfile->file_ptr, pid, SEEK_SET);
	char buf[sizeof(int)];

	int result = fread(buf, 1, sizeof(int), heapfile->file_ptr);
	if(result != sizeof(int)){
		printf("panic panic, can't read Entry \n");
	}

	return *buf;
}

