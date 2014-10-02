#include <iostream>
#include <cassert>

#include "../page.h"

using namespace std;

int main(int argc, char** argv) {
	
	Page page;
	init_fixed_len_page(&page, 20000, 1000);

	cout << "Size: " << fixed_len_page_capacity(&page) << endl;
	assert(fixed_len_page_capacity(&page) == 19);

	// Completely new page - all slots should be free
	assert(fixed_len_page_freeslots(&page) == 19);

	// Add 01000110 to the last byte in the page ~ 3 slots taken
	((char*)page.data)[page.page_size - 1] = 'b';
	assert(fixed_len_page_freeslots(&page) == 16);

	// Add 01000100 to the 3rd last byte 
	// Only the last 3 bits of the 3rd last byte should be counted
	((char*)page.data)[page.page_size - 3] = 'D';
	assert(fixed_len_page_freeslots(&page) == 15);

	// Add (replace) 01001000 to the 3rd last byte
	((char*)page.data)[page.page_size - 3] = 'H';
	assert(fixed_len_page_freeslots(&page) == 16);

	cout << "Num: " << fixed_len_page_freeslots(&page) << endl;

	delete[] (char*)page.data;
	page.data = NULL;


	return 0;
}
