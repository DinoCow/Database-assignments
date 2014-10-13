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
const int MAX_LINE_LEN = 10000;

int main(int argc, char** argv) {
	if (argc != 4) {
		cerr <<
		    "Usage: csv2heapfile <csv_file> <colstore_name> <page_size>"
		    << endl;
		return (1);
	}

	
	FILE *csv_file = fopen(argv[1], "r");
	char *directory_name = argv[2];
	int page_size = atoi(argv[3]);
	

	if (!csv_file || !(page_size > 0)) {
		cerr << "FAIL" << endl;
		return (1);
	}

	char line[MAX_LINE_LEN];
	int tuple_id = 0;
	char filename[20];
	char *token;
	int attribute_id = 0;
	Heapfile *heapfile;
	Heapfile *heapfiles[100];

	//create heapfiles
	fgets(line, MAX_LINE_LEN, csv_file);
	//split line on ','
	token = strtok(line, ",");
	while (token != NULL) {
		//printf ("%s\n",token);
		sprintf(filename, "%s/%d", directory_name, attribute_id);
		//printf("%s\n", filename);
		heapfile = new Heapfile;
		init_heapfile(heapfile, page_size);
		create_heapfile(heapfile, filename);

		heapfiles[attribute_id] = heapfile;

		Record rec;
		load_col_record(&rec, token, tuple_id);
		put_record(heapfile, &rec);
		
		//Free char arrays in record
		for (int i = 0; i < rec.size(); i++) {
			//printf("attr:%s\n", rec[i]);
			delete[]rec[i];
		}

		token = strtok(NULL, ",");
		attribute_id++;
	}
	tuple_id++;

	//add the rest 
	while (fgets(line, MAX_LINE_LEN, csv_file)) {
		attribute_id = 0;
		token = strtok(line, ",");
		while (token != NULL) {
			heapfile = heapfiles[attribute_id];
			Record rec;
			load_col_record(&rec, token, tuple_id);
			put_record(heapfile, &rec);
		
			//Free char arrays in record
			for (int i = 0; i < rec.size(); i++) {
				//printf("attr:%s\n", rec[i]);
				delete[]rec[i];
			}

			token = strtok(NULL, ",");
			attribute_id++;
		}	
		tuple_id++;
	}
	
	//close all the heapfiles
	for(int i= 0; i < 100; i++){
		if (heapfiles[i]){
			close_heapfile(heapfiles[i]);
		}
	}

	fclose(csv_file);

}
