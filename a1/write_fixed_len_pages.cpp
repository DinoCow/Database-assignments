#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "record.h"
#include "page.h"

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

/*
 * Split line on ',' and insert tokens into record
 */
void load_record(Record * record, char line[])
{
	char *token;
	char *attribute;
	token = strtok(line, ",");
	while (token != NULL) {
		//printf ("%s\n",token);
		attribute = new char[ATTRIBUTE_SIZE + 1];

		strncpy(attribute, token, ATTRIBUTE_SIZE);
		attribute[ATTRIBUTE_SIZE] = '\0';
		record->push_back(attribute);

		token = strtok(NULL, ",");
	}
	assert(record->size() == 100);	// I think...
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		cerr <<
		    "Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>"
		    << endl;
		return (1);
	}

	FILE *csv_file = fopen(argv[1], "r");
	FILE *page_file = fopen(argv[2], "w");
	int page_size = atoi(argv[3]);

	if (!csv_file || !page_file || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	int num_records = 0;
	int num_pages = 0;
	long start = get_time_ms();

	Page *page = NULL;

	char line[MAX_LINE_LEN];

	while (fgets(line, MAX_LINE_LEN, csv_file)) {
		Record rec;
		// split csv and push into record
		load_record(&rec, line);
		num_records++;

		// Allocate a new page if necessary
		if (!page || fixed_len_page_freeslots(page) == 0) {
			if (page) {	// page is full
				fwrite(page->data, sizeof(char), page_size,
				       page_file);
				//Free page here
        delete[] (char*)page->data;
        delete page;
			}
			page = new Page;
			init_fixed_len_page(page, page_size, SLOT_SIZE);
			num_pages++;
		}
		// add record to page
		if (add_fixed_len_page(page, &rec) == -1) {
			// Something went wrong.
		}
		//Free char arrays in record
		for (int i = 0; i < rec.size(); i++) {
			//printf("attr:%s\n", rec[i]);
			delete[] rec[i];
		}
	}

	// Write the last page to file.
	fwrite(page->data, sizeof(char), page_size, page_file);
	fclose(csv_file);
	fclose(page_file);
	long end = get_time_ms();

	cout << "NUMBER OF RECORDS: " << num_records << endl;
	cout << "NUMBER OF PAGES: " << num_pages << endl;
	cout << "TIME: " << end - start << " milliseconds" << endl;

}
