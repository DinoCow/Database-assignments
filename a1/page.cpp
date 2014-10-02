//#include <cstring>
//#include <cassert>
//#include <cstdio>
#include <iostream>
#include "page.h"
#include <math.h> // ceil

using namespace std; // Get rid of this later...
/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size) {
	
	page->page_size = page_size;
	page->slot_size = slot_size;
	page->data = new char[page_size](); // () initializes array
}

/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page) {

	double directorySize = page->page_size/page->slot_size/8;
	return (int)(page->page_size - directorySize)/page->slot_size;
}

/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page) {

	int numRecs = fixed_len_page_capacity(page);
	int result = 0, counter = 0;

	// (ceil) numRecs/8.0 - getting the number of bytes
	// Iterating over byte first to make it easier to understand 
	for (int i = 1; i <= ((ceil)(numRecs / 8.0)); i++)
	{
		int temp = page->page_size - i;
		char charVal = ((char *)page->data)[temp];
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
	//TODO
	return 0;
}

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r) {
	//TODO
}

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r) {
	//TODO
}