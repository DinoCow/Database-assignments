#include <cassert>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/timeb.h>

#include "record.h"
#include "page.h"
#include "heapfile.h"

#include <vector>

using namespace std;
const int SLOT_SIZE = 1000;	// Size of a record

long get_time_ms()
{
	struct timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}

int main(int argc, char *argv[])
{
	if (argc != 6) {
		cerr << "Usage: select <heapfile> <attribute_id> "
		     << "<start> <end> <page_size>"
		     << endl;
		return (1);
	}

	char *heapfile_name = argv[1];
	int attribute_id = atoi(argv[2]);
	int page_size = atoi(argv[5]);

	if (!(page_size > 0) || !(attribute_id >= 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	long start = get_time_ms();
	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size, SLOT_SIZE);
	open_heapfile(heapfile, heapfile_name);
	char buf[6];
    RecordIterator rit(heapfile);

    while(rit.hasNext()) {
    	Record rec = rit.next();
    	if (strcmp(rec[attribute_id], argv[3]) >= 0 && 
    		strcmp(rec[attribute_id], argv[4]) <= 0)
    	{
    		//print SUBSTRING(A, 1, 5)
    		strncpy(buf, rec[attribute_id], 5);
    		buf[5] = '\0';
    		cout << buf << endl;
    	}
    }

	close_heapfile(heapfile);
	long end = get_time_ms();
	cout << "TIME: " << end - start << " milliseconds" << endl;
}
