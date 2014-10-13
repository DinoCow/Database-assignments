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

int main(int argc, char** argv) {
	if (argc != 6) {
		cerr << "Usage: select <colstore_name> <attribute_id> "
		     << "<start> <end> <page_size>"
		     << endl;
		return (1);
	}

	char *directory_name = argv[1];
	int attribute_id = atoi(argv[2]);
	int page_size = atoi(argv[5]);

	if (!(page_size > 0) || !(attribute_id >= 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	char filename[20];
	sprintf(filename, "%s/%d", directory_name, attribute_id);
	
	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size);
	open_heapfile(heapfile, filename);

	RecordIterator rit(heapfile);
	while(rit.hasNext()) {
    	Record rec = rit.next();
    	if (strcmp(rec[attribute_id], argv[3]) >= 0 && 
    		strcmp(rec[attribute_id], argv[4]) <= 0)
    	{
    		//TODO supposed to print SUBSTRING(A, 1, 5) . See assignment sheet.
    		cout << rec[attribute_id] << endl;
    	}
    }

	close_heapfile(heapfile);
}
