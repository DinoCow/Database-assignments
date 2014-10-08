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
	init_directory_page(directory, page_size, 0);
	write_directory(heapfile, 0, directory);

	heapfile->directory = directory;
    heapfile->num_pages = 1;
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
	Entry *entry = new Entry;
	Page *page = new Page; 	
	
	get_next_entry(heapfile, entry);

	//Create the new page
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);

	//assign entry pointers
	fseek(heapfile->file_ptr, 0, SEEK_END);
	entry->offset = ftell(heapfile->file_ptr);
    entry->free_space = fixed_len_page_freeslots(page);
    //make pid the offset to the entry from the start of the file
    PageID pid = heapfile->directory->offset + sizeof(int) * 2 + (*heapfile->directory->n_entries - 1) *  sizeof(Entry);

    //write the directory and page out
    write_page(page, heapfile, pid);

    heapfile->num_pages++;

	return pid;
}

/**
 * Get the next entry that can be allocated from directory
 */
void get_next_entry(Heapfile *heapfile, Entry* entry){
	entry = next_entry(heapfile->directory);

	if (entry == NULL){		
		fseek(heapfile->file_ptr, 0, SEEK_END);
		int new_directory_offset = ftell(heapfile->file_ptr);
		*heapfile->directory->next_directory = new_directory_offset;

		//write out full directory
		write_directory(heapfile, heapfile->directory->offset, heapfile->directory);
		
		//add a new directory and point heapfile to new empty directory
		Directory *new_directory = new Directory;
		init_directory_page(new_directory, heapfile->page_size, new_directory_offset);
		
		//write out for the first time
		write_directory(heapfile, new_directory_offset, new_directory);
		heapfile->directory = new_directory;
		//get entry from the new dir
		entry = next_entry(new_directory);
	}
}

/**
 * Write a directory into disk from memory
 */
void write_directory(Heapfile *heapfile, int offset, Directory *directory){
	fseek(heapfile->file_ptr, offset, SEEK_SET);
	int result = fwrite(directory->data, 1, heapfile->page_size, heapfile->file_ptr);

	if (result != heapfile->page_size){
		printf("panic panic, can't write page");
	}

	fflush(heapfile->file_ptr);
}

/*
 * Read a directory into memory 
 */
void read_directory(Heapfile *heapfile, int offset, Directory *directory){
	fseek(heapfile->file_ptr, offset, SEEK_SET);
	char buf[heapfile->page_size];

	int result = fread(buf, 1, heapfile->page_size, heapfile->file_ptr);
	if(result != heapfile->page_size) { 
		printf("panic panic, can't read directory \n");
	}

	init_directory_page(directory, heapfile->page_size, offset, buf);
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

/**
 * get the page offset from the directory
 */
int get_page_offset(Heapfile *heapfile, PageID pid){
	fseek(heapfile->file_ptr, pid, SEEK_SET);
	int page_offset;

	int result = fread(&page_offset, 1, sizeof(int), heapfile->file_ptr);
	if(result != sizeof(int)){
		printf("panic panic, can't read Entry \n");
	}

	return page_offset;
}

