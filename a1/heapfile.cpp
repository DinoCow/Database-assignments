#include "heapfile.h"

#include <stdio.h>
#include <cassert>
#include <cstdlib>
#include <cmath>

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, int slot_size) {
	assert(page_size > 1001);

	heapfile->page_size = page_size;
	heapfile->cached_page = NULL;
	heapfile->cached_pid = -1;
	heapfile->page_cache_dirty = false;
	heapfile->slot_size = slot_size;
}

/**
 * Create the initial structure of a heapfile
 * This only needs to be used when creating new heapfile
 */
void create_heapfile(Heapfile* heapfile, char *filename) {
	//(w+) Create an empty file and open it for input and output.
	// If a file with the same name already exists its contents
	// are discarded and the file is treated as a new empty file.
	heapfile->file_ptr = fopen(filename, "w+");
	assert(heapfile->file_ptr);
	//create the directory page
	Directory *directory = new Directory;
	// First page of heapfile is a directory.
	init_directory_page(directory, heapfile->page_size, 0);
	write_block(heapfile, (char *)directory->data, 0);

	//  directories are buffered
	heapfile->directory_buffer.push_back(directory);
	//heapfile->directory.push_back(directory);
}

void open_heapfile(Heapfile *heapfile, char *filename) {
	//(r+) Open a file for input and output. The file must exist.
	heapfile->file_ptr = fopen(filename, "r+");
	assert(heapfile->file_ptr);

	int offset = 0;
	do {
	//create the directory page
	Directory *directory = new Directory;
	// First page of heapfile is a directory.
	init_directory_page(directory, heapfile->page_size, offset);
	read_block(heapfile, (char *)directory->data, offset);

	//  directories are buffered
	heapfile->directory_buffer.push_back(directory);
	offset = *directory->next_directory;
	} while (offset != 0);
}

void close_heapfile(Heapfile *heapfile) {

	//write out cache in buffer if dirty
	if (heapfile->page_cache_dirty) {
		Entry *entry = get_entry(heapfile, heapfile->cached_pid);
		write_block(heapfile, (char*)heapfile->cached_page->data, entry->offset);
	}
	//free old page
	if (heapfile->cached_pid != -1) {
		free_page(heapfile->cached_page);
	}
	//write out directory buffer
	int num_directories = heapfile->directory_buffer.size();
	for (int dir_no=0; dir_no < num_directories; dir_no++){
		Directory *dir = heapfile->directory_buffer[dir_no];
		write_block(heapfile, (char *)dir->data, dir->offset);

		//DEBUG PURPOSE
		int num_records = 0;
		int directorySize = ceil(heapfile->page_size/heapfile->slot_size/8.0);
		int page_capacity = (heapfile->page_size - directorySize)/heapfile->slot_size;

		for(int i=0; i<*dir->n_entries; i++){
			num_records += page_capacity - dir->entries[i].free_space;
		}
	


		delete[] (char *)dir->data;
		delete dir;
	}

	fclose(heapfile->file_ptr);
	delete heapfile;
}

void put_record(Heapfile* heapfile, Record *rec) {

	PageID pid = get_free_pid(heapfile);


	Page *page = get_page(heapfile, pid);
	add_fixed_len_page(page, rec);


	int index = page->page_size - 1;
	char byte = ((char *)page->data)[index];

	commit_page(heapfile, pid);

	//---SANITY CHECK

	Page *page2 = get_page(heapfile, pid);

	char byte2 = ((char *)page2->data)[index];
	assert(byte==byte2);
	assert(page==page2);
	//---
}

void get_record(Heapfile* heapfile, RecordID *rid, Record *rec) {

	Page *page = get_page(heapfile, rid->page_id);
	read_fixed_len_page(page, rid->slot, rec);

}

void update_record(Heapfile* heapfile, RecordID *rid, Record *rec) {

	Page *page = get_page(heapfile, rid->page_id);
	write_fixed_len_page(page, rid->slot, rec);
	commit_page(heapfile, rid->page_id);
}

void delete_record(Heapfile *heapfile, RecordID *rid){

	Page *page = get_page(heapfile, rid->page_id);

	delete_fixed_len_page(page, rid->slot);
	
	commit_page(heapfile, rid->page_id);
}

/* Search existing pages with free space.
 * May allocate new page
 */
PageID get_free_pid(Heapfile *heapfile) {


	// return pid with a vacant slot or -1 if none available
	PageID pid = vacant_page_id(heapfile);
	if (pid == -1) {
		//no free pages available so allocate new page.
		pid = alloc_page(heapfile);
	}

	return pid;
}

Page *get_page(Heapfile *heapfile, PageID pid) {
	if (heapfile->cached_pid == pid) {

		return heapfile->cached_page;
	}

	//write out cache in buffer if dirty
	if (heapfile->page_cache_dirty) {
		Entry *entry = get_entry(heapfile, heapfile->cached_pid);
		write_block(heapfile, (char*)heapfile->cached_page->data, entry->offset);
	}

	//free old page
	if (heapfile->cached_pid != -1) {
		free_page(heapfile->cached_page);
	}

	//load new page
	Page *page = new Page;
	init_fixed_len_page(page, heapfile->page_size, heapfile->slot_size);

	Entry *entry = get_entry(heapfile, pid);

	read_block(heapfile, (char*)page->data, entry->offset);
	
	//push into cache
	heapfile->cached_page = page;
	heapfile->cached_pid = pid;
	heapfile->page_cache_dirty = false;
	return page;
}
/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){

	Entry entry;
	Page *page = new Page; 	

	//Create the new page
	init_fixed_len_page(page, heapfile->page_size, heapfile->slot_size);

	//assign entry pointers
	fseek(heapfile->file_ptr, 0, SEEK_END);
	entry.offset = ftell(heapfile->file_ptr);
    entry.free_space = fixed_len_page_freeslots(page);

    //write the page out
    write_block(heapfile, (char*)page->data, entry.offset);

	// add the new entry into entry list
    PageID pid = append_entry(heapfile, &entry);

	//write out cache in buffer if dirty
	if (heapfile->page_cache_dirty) {
		Entry *entry = get_entry(heapfile, heapfile->cached_pid);
		write_block(heapfile, (char*)heapfile->cached_page->data, entry->offset);
	}
	//free old page
	if (heapfile->cached_pid != -1) {
		free_page(heapfile->cached_page);
	}

    // push page to cache
	heapfile->cached_page = page;
	heapfile->cached_pid = pid;
	heapfile->page_cache_dirty = false;

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
		exit(1);
	}


}

/**
 * Write a page size block from memory to disk
 */
void write_block(Heapfile *heapfile, char *buffer, int offset){

	fseek(heapfile->file_ptr, offset, SEEK_SET);

	int result = fwrite(buffer, 1, heapfile->page_size, heapfile->file_ptr);

	if (result != heapfile->page_size){
		perror("write block");
		exit(1);
	}

	fflush(heapfile->file_ptr);
}

void commit_page(Heapfile *heapfile, PageID pid) {

	Entry *entry = get_entry(heapfile, heapfile->cached_pid);
	entry->free_space = fixed_len_page_freeslots(heapfile->cached_page);
	assert(heapfile->cached_pid == pid);
	heapfile->page_cache_dirty = true;
}


PageID append_entry(Heapfile *heapfile, Entry *entry) {

	int num_directories = heapfile->directory_buffer.size();
	int dir_no;
	for (dir_no=0; dir_no < num_directories; dir_no++){
		Directory *dir = heapfile->directory_buffer[dir_no];
		if (!is_full(dir)) {
			int slot_no = next_vacant_slot(dir);
			dir->entries[slot_no] = *entry;

			return dir_no * dir->capacity + slot_no;
		}
	}

	// no capacity. must alloc new direcotry
	Directory *last_dir = heapfile->directory_buffer[num_directories-1];

	fseek(heapfile->file_ptr, 0, SEEK_END);
	int offset =  ftell(heapfile->file_ptr);
	*last_dir->next_directory = offset;
	Directory *new_dir = new Directory;
	init_directory_page(new_dir, heapfile->page_size, offset);
    write_block(heapfile, (char*)new_dir->data, offset);

    heapfile->directory_buffer.push_back(new_dir);
	int slot_no = next_vacant_slot(new_dir);
	new_dir->entries[slot_no] = *entry;

	return (num_directories) * new_dir->capacity + slot_no;
//
}


	// get entry from pid
Entry *get_entry(Heapfile *heapfile, PageID pid) {

	int capacity = heapfile->directory_buffer[0]->capacity;



	int dir_no = pid / capacity;
	int slot_no = pid % capacity;

	Directory *dir = heapfile->directory_buffer[dir_no];
	assert(slot_no < *dir->n_entries);
	return & dir->entries[slot_no];
}

PageID vacant_page_id(Heapfile *heapfile){

	int num_directories = heapfile->directory_buffer.size();

	//check every directory page
	for (int dir_no=0; dir_no < num_directories; dir_no++){
		Directory *dir = heapfile->directory_buffer[dir_no];
		// check every slot
		int slot_no = find_free_page(dir);
		if (slot_no != -1) {
			return dir_no * dir->capacity + slot_no;
		}
	}
	// No vacant pages available.
	return -1;
}


RecordIterator::RecordIterator(Heapfile *heap){

	heapfile = heap;
	pid = 0;
	slot = -1;
	increment_iterator();
}

Record RecordIterator::next(){

	Page *page = get_page(heapfile, pid);

	Record record(100);
	read_fixed_len_page(page, slot, &record);
	
	// Increment iterator
	increment_iterator();
	return record;
}


void RecordIterator::increment_iterator(){

	int dir_capacity = heapfile->directory_buffer[0]->capacity;
	int dir_no = pid / dir_capacity;
	int slot_no = pid % dir_capacity;

	Directory *dir = heapfile->directory_buffer[dir_no];
	bool page_is_allocated = (slot_no < *dir->n_entries);
	// (slot_no < *dir->n_entries) // page exists according to directory
	
	//while (page_is_allocated(pid)) {
	while(page_is_allocated) {
		//get page (pid)
		Page *page = get_page(heapfile, pid);
		
		//get next slot after `slot` that contains a record
		slot = get_next_filled_slot(page, slot);

		if (slot >= 0 ) {// next slot exists
			//all good.
			return;
		} else { // bitset exhausted
			//try next page
			pid++;
			dir_no = pid / dir_capacity;
			slot_no = pid % dir_capacity;
			dir = heapfile->directory_buffer[dir_no];
			page_is_allocated = (slot_no < *dir->n_entries);
		}
	}
	// exhausted all pages. No more records in heapfile.
	has_next = false;
}

bool RecordIterator::hasNext() {
	return has_next;
}

RecordID RecordIterator::nextRID(){
	RecordID rid;
	rid.page_id = pid;
	rid.slot = slot;
	return rid;
}