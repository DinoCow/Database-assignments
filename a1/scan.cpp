#include <cassert>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "record.h"
#include "page.h"
#include "heapfile.h"

#include <vector>

using namespace std;
const int SLOT_SIZE = 1000;	// Size of a record

int main(int argc, char *argv[])
{
	if (argc != 3) {
		cerr <<
		    "Usage: scan <heapfile> <page_size>"
		    << endl;
		return (1);
	}


	char *heapfile_name = argv[1];
	int page_size = atoi(argv[2]);

	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size);
	open_heapfile(heapfile, heapfile_name);

	if (!(page_size > 0)) {
		cerr << "Page size must be a positive integer" << endl;
		return (1);
	}

    RecordIterator rit(heapfile);

    while(rit.hasNext()) {
    	RecordID rid = rit.nextRID();
    	Record rec = rit.next();
    	cout << "RID("<< rid.page_id << "," << rid.slot << ") ";
    	print_record_as_csv(&rec);


    			//Free char arrays in record
		for (int i = 0; i < rec.size(); i++) 
		{
			delete[]rec[i];
		}
    }

	close_heapfile(heapfile);
}