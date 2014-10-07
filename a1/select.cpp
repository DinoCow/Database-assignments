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

int main(int argc, char *argv[])
{
	if (argc != 6) {
		cerr << "Usage: select <heapfile> <attribute_id> "
		     << "<start> <end> <page_size>"
		     << endl;
		return (1);
	}

	FILE *heapfd = fopen(argv[1], "r");
	int attribute_id = atoi(argv[2]);
	int page_size = atoi(argv[5]);

	if (!heapfd || !(page_size > 0) || !(attribute_id > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	Heapfile heapfile;
	init_heapfile(&heapfile, page_size, heapfd);

    RecordIterator rit(&heapfile);

    while(rit.hasNext()) {
    	Record rec = rit.next();
    	if (strcmp(rec[attribute_id], argv[3]) >= 0 && 
    		strcmp(rec[attribute_id], argv[4]) <= 0)
    	{
    		print_record_as_csv(&rec);
    	}
    }

	fclose(heapfd);
}