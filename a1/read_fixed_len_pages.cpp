#include <iostream>
#include <fstream>
#include <cstdlib>
#include "record.h"
#include "page.h"

#include <sys/timeb.h>
#include <vector>

using namespace std;
const int NUM_ATTR = 100; // Number of attributes
const int SLOT_SIZE = 1000;	// Size of a record

long get_time_ms()
{
	struct timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}

int main(int argc, char *argv[])
{
	if (argc != 3) 
	{
		cerr <<
		    "Usage: read_fixed_len_pages <page_file> <page_size>"
		    << endl;
		return (1);
	}
	FILE *page_file = fopen(argv[1], "r");
	int page_size = atoi(argv[2]);

	if (!page_file || !(page_size > 0)) 
	{
		cerr << "FAIL" << endl;
		return (1);
	}

	long start = get_time_ms();
	Page *page = NULL;
	char line[page_size];

	// Iterate over each page in the page file
	while (fread(line, sizeof(char), page_size, page_file)) 
	{	
		page = new Page;
		init_fixed_len_page(page, page_size, SLOT_SIZE);
		page->data = line;

		Record rec(NUM_ATTR);
		int numRecs = fixed_len_page_capacity(page) - fixed_len_page_freeslots(page);
		
		// Iterate over the records
		for (int i = 0; i < numRecs; i++)
		{
			read_fixed_len_page(page, i, &rec);
			// Output the record as a csv
			print_record_as_csv(&rec);
		}

		//Free char arrays in record
		for (int i = 0; i < rec.size(); i++) 
		{
			delete[]rec[i];
		}
        delete page;
	}

	fclose(page_file);
	long end = get_time_ms();
	cout << "TIME: " << end - start << " milliseconds" << endl;
}
