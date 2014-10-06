#include <iostream>
#include <cassert>

#include "../page.h"

using namespace std;

int main(int argc, char** argv) {
	
	Page page1;
	//--- Tests for init ---//
	// Make sure the page size and slot size are proper inputs
	//init_fixed_len_page(&page1, 0, 0);
	//init_fixed_len_page(&page1, 10, 20);

	//--------------------------------------
	//--- Tests for len capacity ---//
	// Normal case
	init_fixed_len_page(&page1, 1000, 10);
	assert(fixed_len_page_capacity(&page1) == 98); 
	delete[] (char*)page1.data;
	page1.data = NULL;

	// Slot directory take up exactly 2 records
	init_fixed_len_page(&page1, 1620, 10);
	assert(fixed_len_page_capacity(&page1) == 160); 
	delete[] (char*)page1.data;
	page1.data = NULL;

	// One-off case for slot directory taking up exactly 2 records
	init_fixed_len_page(&page1, 1619, 10);
	assert(fixed_len_page_capacity(&page1) == 159); 
	delete[] (char*)page1.data;
	page1.data = NULL;

	// One-off case for slot directory taking up exactly 2 records
	init_fixed_len_page(&page1, 1621, 10);
	assert(fixed_len_page_capacity(&page1) == 160); 
	delete[] (char*)page1.data;
	page1.data = NULL;

	// Directory takes up more than 2 records
	init_fixed_len_page(&page1, 500, 5);
	assert(fixed_len_page_capacity(&page1) == 97); 
	delete[] (char*)page1.data;
	page1.data = NULL;


	//--------------------------------------
	//--- Tests for free slots ---//
	Page page;
	// Page setup...
	init_fixed_len_page(&page, 20000, 1000);
	assert(fixed_len_page_capacity(&page) == 19); 

	// Completely new page - all slots should be free
	assert(fixed_len_page_freeslots(&page) == 19);

	// Add 01100010 to the last byte in the page ~ 3 slots taken
	// Normal case
	((char*)page.data)[page.page_size - 1] = 'b';
	assert(fixed_len_page_freeslots(&page) == 16);

	// Add 01000100 to the 3rd last byte 
	// Only the last 3 bits of the 3rd last byte should be counted
	// Case - Set last bit of the directory 
	((char*)page.data)[page.page_size - 3] = 'D';
	assert(fixed_len_page_freeslots(&page) == 15);

	// Add (replace) 01001000 to the 3rd last byte
	// Case - Set bit one past the directory
	((char*)page.data)[page.page_size - 3] = 'H';
	assert(fixed_len_page_freeslots(&page) == 16); 
	

	//--------------------------------------
	//--- Tests for adding record ---//
	Record rec(100, "1234567891234");

	// Writing the the first slot
	// First byte of directory: 01100010 - first free = 0
	assert(add_fixed_len_page(&page, &rec) == 0);
	// Check if slot was set (01100011)
	assert(fixed_len_page_freeslots(&page) == 15);
	assert(((char*)page.data)[page.page_size - 1] == 'c');
	// Check if record written in
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (10*i));
		assert(strcmp(str, "123456789")==0);
	}

	Record rec2(100, "abcdefghijkl");
	// Writing to normal slot (3rd slot; offset = 2)
	assert(add_fixed_len_page(&page, &rec2) == 2);
	// Check if slot was set (01100111)
	assert(fixed_len_page_freeslots(&page) == 14);
	assert(((char*)page.data)[page.page_size - 1] == 'g');
	// Check if record written in
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (2*page.slot_size + 10*i));
		assert(strcmp(str, "abcdefghi")==0);
	}

	// Set so directory full but last slot
	((char*)page.data)[page.page_size - 1] = 'w'; // 2 slots empty
	((char*)page.data)[page.page_size - 2] = 'w'; // 2 slots empty
	((char*)page.data)[page.page_size - 3] = 'c'; // 01100011
	add_fixed_len_page(&page, &rec);
	add_fixed_len_page(&page, &rec);
	add_fixed_len_page(&page, &rec);
	add_fixed_len_page(&page, &rec);

	// Writing to last slot
	assert(add_fixed_len_page(&page, &rec) == 18);
	// Check if slot was set (01100111)
	assert(fixed_len_page_freeslots(&page) == 0);
	assert(((char*)page.data)[page.page_size - 3] == 'g');
	// Check if record written in
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (18*page.slot_size + 10*i));
		assert(strcmp(str, "123456789")==0);
	}

	// Writing when all slots are full
	assert(add_fixed_len_page(&page, &rec) == -1);
	

	//--------------------------------------
	//--- Tests for writing record ---//
	// Normal writing cases tested when adding reccord
	// Check for invlaid slot
	//write_fixed_len_page(&page, -1, &rec);

	// Check over-writing records
	// Initial first slot...
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (10*i));
		assert(strcmp(str, "123456789")==0);
	}
	assert(fixed_len_page_freeslots(&page) == 0);
	// Overwrite first slot
	write_fixed_len_page(&page, 0, &rec2);
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (10*i));
		assert(strcmp(str, "abcdefghi")==0);
	}
	assert(fixed_len_page_freeslots(&page) == 0);

	// Initial last slot...
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (18*page.slot_size + 10*i));
		assert(strcmp(str, "123456789")==0);
	}
	assert(fixed_len_page_freeslots(&page) == 0);
	// Overwrite first slot
	write_fixed_len_page(&page, 18, &rec2);
	for (int i=0; i< 100; i++){
		char* str = ((char*)page.data + (18*page.slot_size + 10*i));
		assert(strcmp(str, "abcdefghi")==0);
	}
	assert(fixed_len_page_freeslots(&page) == 0);


	//--------------------------------------
	//--- Tests for reading record ---//

	// If this is called, it should break the validity of read
	//((char*)page.data)[page.page_size - 1] = 'w';
	
	Record rec3(100);
	read_fixed_len_page(&page, 0, &rec3);
	for (unsigned int i=0; i<rec3.size(); i++) {
		assert(strcmp(rec3[i], "abcdefghi")==0);
	}
	assert(rec3[5] != rec3[3]);

	for (unsigned int i=0; i<rec3.size(); i++) {
		delete[] rec3[i];
	}
	delete[] (char*)page.data;
	page.data = NULL;

	return 0;
}
