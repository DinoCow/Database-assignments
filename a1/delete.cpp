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
	if (argc != 4) {
		cerr <<
		    "Usage: delete <heapfile> <record_id> <page_size>" << endl;
		return (1);
	}

	char *heapfile_name = argv[1];
	int page_size = atoi(argv[3]);

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
	cerr << "PID:" << pid << " SLOT:" << slot << endl;

	if (!(pid >= 0) || !(slot >= 0) || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size);
	open_heapfile(heapfile, heapfile_name);

	// so easy!
	delete_record(heapfile, &rid);

	close_heapfile(heapfile);
}
