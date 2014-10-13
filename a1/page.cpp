//#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include "page.h"
#include <math.h> // ceil

/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size) {
	
	assert (0 < slot_size && slot_size < page_size);

	page->page_size = page_size;
	page->slot_size = slot_size;
	page->data = new char[page_size](); // () initializes array
}

void free_page(Page *page) {
	delete[](char *)page->data;
	delete page;
}
/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page) {

	int directorySize = ceil(page->page_size/page->slot_size/8.0);
	return (page->page_size - directorySize)/page->slot_size;
}

/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page) {

	int numRecs = fixed_len_page_capacity(page);
	int result = 0, counter = 0, numBytes = (ceil)(numRecs / 8.0);;

	// Iterating over byte first to make it easier to understand 
	for (int i = 1; i <= numBytes; i++)
	{
		int index = page->page_size - i;
		char charVal = ((char *)page->data)[index];
		for (int bit = 0; bit < 8; bit++)
		{
			// Because iterating over bytes, make sure we don't go
			// past the record directory
			if (counter < numRecs) 
			{
				result += charVal & 0X01;
				charVal >>= 1;
				counter++;
			}
			else { break; }
		}
	}
	return numRecs - result;
}

/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r) {
	
	int numRecs = fixed_len_page_capacity(page);
	int index = 0, counter = 0, slot = -1, numBytes = (ceil)(numRecs / 8.0);


	// Get the first empty slot
	for (int i = 1; i <= numBytes; i++)
	{
		index = page->page_size - i;
		char charVal = ((char *)page->data)[index];

		fprintf(stderr,"Add fix len Bitset "BYTETOBINARYPATTERN"\n", BYTETOBINARY(charVal));
		for (int bit = 0; bit < 8; bit++)
		{
			// Because iterating over bytes, make sure we don't go
			// past the record directory
			if (counter < numRecs)
			{
				// Stop searching once an empty slot has been found
				if (!(charVal & 0x01))
				{
					slot = counter;
					break;
				}
				charVal >>= 1;
				counter++;
			}
			else { break; }
		}

		// Stop searching once an empty slot has been found
		if (slot >= 0)
			break;
	}

	// Write the record onto the page
	if (slot >= 0)
	{
		write_fixed_len_page(page, slot, r);
	}

	return slot;
}

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r) {
	
	assert (slot >= 0);
	void *record = (char *)page->data + slot * page->slot_size;
	fixed_len_write(r, record);

	// Make sure the slot flag is 1
	int index = page->page_size - (ceil)((slot+1) / 8.0);

	char byte = ((char *)page->data)[index];
	fprintf(stderr,"before write Bitset "BYTETOBINARYPATTERN"\n", BYTETOBINARY(byte));

	((char *)page->data)[index] |= 1 << (slot % 8);
	byte = ((char *)page->data)[index];
	fprintf(stderr,"after write Bitset to slot (%d) "BYTETOBINARYPATTERN"\n", slot, BYTETOBINARY(byte));	
}

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r) {
	
	assert (slot >= 0);

	// Check record is valid before being read
	int index = page->page_size - (ceil)((slot+1) / 8.0);

	char byte = ((char *)page->data)[index];
	fprintf(stderr,"read Bitset "BYTETOBINARYPATTERN"\n", BYTETOBINARY(byte));

	int valid = ((char *)page->data)[index] & (1 << (slot % 8));
	assert (valid > 0);

	void *record = (char *)page->data + slot * page->slot_size;
	fixed_len_read(record, page->slot_size, r);
}

/**
 * Remove a record from a given slot.
 */
void delete_fixed_len_page(Page *page, int slot){

	assert (slot >= 0);

	// Set the slot flag to 0
	int index = page->page_size - (ceil)((slot+1) / 8.0);
	((char *)page->data)[index] &= ~(1 << (slot % 8));

}