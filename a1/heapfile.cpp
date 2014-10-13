#include "heapfile.h"

#include <stdio.h>
#include <cassert>
#include <cstdlib>

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size) {
	assert(page_size > 1001);

	heapfile->page_size = page_size;
}

/**
 * Create the initial structure of a heapfile
 * This only needs to be used when creating new heapfile
 */
void create_heapfile(Heapfile* heapfile, char *filename) {
	heapfile->file_ptr = fopen(filename, "w+");

	//create the directory page
	Directory *directory = new Directory;
	// First page of heapfile is a directory.
	init_directory_page(directory, heapfile->page_size, 0);
	write_block(heapfile, (char *)directory->data, 0);

	// set it as the curent directory in the buffer
	heapfile->directory = directory;
	//heapfile->directory.push_back(directory);
}


void close_heapfile(Heapfile *heapfile) {

	//write out directory buffer
	Directory *directory = heapfile->directory;
	write_block(heapfile, (char *)directory->data, directory->offset);
	delete[] (char *)directory->data;
	delete directory;

	fclose(heapfile->file_ptr);
	delete heapfile;
}

void put_record(Heapfile* heapfile, Record *rec) {

	printf("Put record:\n");
	PageID pid = get_free_pid(heapfile);

	printf("   get_free_pid=%d\n", pid);
	Page *page = new Page;
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);
	
	printf("   get_page\n");
	get_page(heapfile, pid, page);
	
	printf("   addfix\n");
	add_fixed_len_page(page, rec);
	
	printf("   commit\n");
	commit_page(heapfile, page, pid);

	free_page(page);
}

void get_record(Heapfile* heapfile, RecordID *rid, Record *rec) {
	Page *page = new Page;
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);
	get_page(heapfile, rid->page_id, page);
	read_fixed_len_page(page, rid->slot, rec);

	printf("getrecord:%d\n", rid->page_id);
	free_page(page);
}

/* Search existing pages with free space.
 * May allocate new page
 */
PageID get_free_pid(Heapfile *heapfile) {

	printf("get free pid:\n");

	for (PageID pid=0; pid<heapfile->entry_list.size(); pid++) {

	printf("pid:%d freespace:%d\n", pid,heapfile->entry_list[pid].free_space);
		if ( heapfile->entry_list[pid].free_space > 0 ) {
			return pid;
		}
	}
	//no free pages available so allocate new page.
	return alloc_page(heapfile);
}


void get_page(Heapfile *heapfile, PageID pid, Page *page) {
	//if pid in cache: return page
	//else:

	printf("getpage: size of entry_list:%d [%d]\n", heapfile->entry_list.size(), pid);

	Entry entry = heapfile->entry_list.at(pid);
	//write out cache in buffer
	//free old page
	//load new page

	printf("get page: pid=%d  offset=%d\n", pid, entry.offset);
	read_block(heapfile, (char*)page->data, entry.offset);
	//push into cache
}
/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){

	printf("alloc page:\n");
	Entry entry;
	Page *page = new Page; 	

	//Create the new page
	init_fixed_len_page(page, heapfile->page_size, SLOTSIZE);

	//assign entry pointers
	fseek(heapfile->file_ptr, 0, SEEK_END);
	entry.offset = ftell(heapfile->file_ptr);
    entry.free_space = fixed_len_page_freeslots(page);

	// add the new entry into entry list   
   	heapfile->entry_list.push_back(entry);
   	PageID pid = heapfile->entry_list.size()-1;

    //write the page out
    write_block(heapfile, (char*)page->data, entry.offset);

	printf("alloc page: pid=%d  offset=%d\n", pid, entry.offset);


    heapfile->num_pages++;
    // push page to cache
    free_page(page);
	return pid;
}

/**
 * Read a page size block into memory
 */
void read_block(Heapfile *heapfile, char *buffer, int offset){
	fseek(heapfile->file_ptr, offset, SEEK_SET);

	int result = fread(buffer, 1, heapfile->page_size, heapfile->file_ptr);
	if(result != heapfile->page_size){
		perror("read_block");
		printf("panic panic, can't read block(%d)\n", result);
		exit(1);
	}

	printf("-----------------------------------read block(%d - %d)\n", offset, offset+result);

}

/**
 * Write a page size block from memory to disk
 */
void write_block(Heapfile *heapfile, char *buffer, int offset){

	fseek(heapfile->file_ptr, offset, SEEK_SET);

	int result = fwrite(buffer, 1, heapfile->page_size, heapfile->file_ptr);

	if (result != heapfile->page_size){
		perror("write block");
		printf("panic panic, can't write block(%d)\n", result);
		exit(1);
	}

	printf("----------------------------------wrote block(%d - %d)\n", offset, offset+result);
	fflush(heapfile->file_ptr);
}

void commit_page(Heapfile *heapfile, Page *page, PageID pid) {

	Entry *entry = &heapfile->entry_list[pid];
	entry->free_space = fixed_len_page_freeslots(page);

	printf("Commit page(%d)\n", entry->offset);
	write_block(heapfile, (char*)page->data, entry->offset);
}
