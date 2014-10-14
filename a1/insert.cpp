#include <cassert>
#include <iostream>
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
const int MAX_LINE_LEN = 10000;


int main(int argc, char *argv[])
{
	if (argc != 4) {
		cerr << "Insert all records in the CSV file to a heap file" << endl;
		cerr << "Usage: insert <heapfile> <csv_file> <page_size>" << endl;
		return (1);
	}


	char *heapfile_name = argv[1];
	FILE *csv_file = fopen(argv[2], "r");
	int page_size = atoi(argv[3]);

	if (!csv_file || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	Heapfile *heapfile = new Heapfile;
	init_heapfile(heapfile, page_size, SLOT_SIZE);
	open_heapfile(heapfile, heapfile_name);

	char line[MAX_LINE_LEN];

	while (fgets(line, MAX_LINE_LEN, csv_file)) {
		Record rec;
		// split csv and push into record
		load_csv_record(&rec, line);
		put_record(heapfile, &rec);
		
		//Free char arrays in record
		for (unsigned int i = 0; i < rec.size(); i++) {
			delete[]rec[i];
		}
	}

	close_heapfile(heapfile);
	fclose(csv_file);
}
