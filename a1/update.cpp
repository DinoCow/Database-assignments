#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "record.h"
#include "page.h"
#include "heapfile.h"

#include <vector>

using namespace std;
const int NUM_ATTR = 100; // Number of attributes

int main(int argc, char *argv[])
{
	if (argc != 6) {
		cerr <<
		    "Usage: update <heapfile> <record_id> <attribute_id> "
		    << "<new_value> <page_size>" << endl;
		return (1);
	}

	FILE *heapfd = fopen(argv[1], "w");
	int attribute_id = atoi(argv[3]);
	int page_size = atoi(argv[5]);

	// Get pid and slot from record_id (<page_id,offset>)
	PageID pid;
	int slot;
	char *record_id;
	record_id = strtok(argv[2], ",");
	if (record_id != NULL)
	{
		pid = atoi(record_id);
		record_id = strtok(NULL, ",");
	}
	if (record_id != NULL)
		slot = atoi(record_id);

	// Get the new value
	char *new_value = new char[ATTRIBUTE_SIZE + 1];
	strncpy(new_value, argv[4], ATTRIBUTE_SIZE);
	new_value[ATTRIBUTE_SIZE] = '\0';

	if (!heapfd || !(pid > 0) || !(slot > 0) || !(attribute_id > 0) || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	Heapfile heapfile;
	init_heapfile(&heapfile, page_size, heapfd);

	Page *page = new Page;
	Record rec(NUM_ATTR);

	// Read record from disk and modify value
	read_page(&heapfile, pid, page);
	read_fixed_len_page(page, slot, &rec);
	rec[attribute_id] = new_value;

	// Write record back out to disk
	write_fixed_len_page(page, slot, &rec);
	write_page(page, &heapfile, pid);

	// FREE ALL THE SPACE
	for (int i = 0; i < rec.size(); i++) {
		delete[]rec[i];
	}

	delete[](char *)page->data;
	delete page;

	fclose(heapfd);
}
