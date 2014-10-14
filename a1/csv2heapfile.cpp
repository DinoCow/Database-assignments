#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "record.h"
#include "heapfile.h"

#include <sys/timeb.h>
#include <vector>

using namespace std;
const int SLOT_SIZE = 1000;	// Size of a record
const int MAX_LINE_LEN = 10000;

long get_time_ms()
{
	struct timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		cerr <<
		    "Usage: csv2heapfile <csv_file> <heap_file> <page_size>"
		    << endl;
		return (1);
	}

	FILE *csv_file = fopen(argv[1], "r");
	char *heapfile_name = argv[2];
	int page_size = atoi(argv[3]);

	if (!csv_file || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	long start = get_time_ms();
	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size);
	create_heapfile(heapfile, heapfile_name);

	char line[MAX_LINE_LEN];

	while (fgets(line, MAX_LINE_LEN, csv_file)) {
		Record rec;
		// split csv and push into record
		load_csv_record(&rec, line);
		put_record(heapfile, &rec);
		
		//Free char arrays in record
		for (int i = 0; i < rec.size(); i++) {
			//printf("attr:%s\n", rec[i]);
			delete[]rec[i];
		}
	}

	close_heapfile(heapfile);
	fclose(csv_file);

	long end = get_time_ms();
	cout << "TIME: " << end - start << " milliseconds" << endl;
}
