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
		cerr << "Update one attribute of a single record in the heap file given its record ID" << endl;
		cerr << "Usage: update <heapfile> <record_id> <attribute_id> <new_value> <page_size>" << endl;
		return (1);
	}

	char *heapfile_name = argv[1];
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

	RecordID rid;
	rid.page_id = pid;
	rid.slot = slot;

	// Get the new value
	char *new_value = new char[ATTRIBUTE_SIZE + 1];
	strncpy(new_value, argv[4], ATTRIBUTE_SIZE);
	new_value[ATTRIBUTE_SIZE] = '\0';

	if (!(pid >= 0) || !(slot >= 0) || !(attribute_id >= 0) || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size);
	open_heapfile(heapfile, heapfile_name);

	Record rec(NUM_ATTR);
	get_record(heapfile, &rid, &rec);
	// update the attribute
	rec[attribute_id] = new_value;
	update_record(heapfile, &rid, &rec);

	// FREE ALL THE SPACE
	for (unsigned int i = 0; i < rec.size(); i++) {
		delete[]rec[i];
	}

	close_heapfile(heapfile);
}
