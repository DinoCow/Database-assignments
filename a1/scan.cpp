#include <cassert>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "record.h"
#include "page.h"
#include "heapfile.h"

#include <sys/timeb.h>
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

	FILE *heapfd = fopen(argv[1], "r");
	int page_size = atoi(argv[2]);

	if (!heapfd || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	Heapfile heapfile;
	init_heapfile(&heapfile, page_size, heapfd);

    RecordIterator rit(&heapfile);

    while(rit.hasNext()) {
    	Record rec = rit.next();
    	print_record_as_csv(&rec);
    }

	fclose(heapfd);
}