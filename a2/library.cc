#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "library.h"
const int MAX_LINE_LEN = 10000;

/* comparison function just for testing.
   need to write a proper comparator for each type
*/
int cstr_cmp(const void *a, const void *b){
	return strcmp((const char*)a,(const char*)b);
}

void sort_and_write(char *buf, int num_records, Schema *schema, FILE *out_fp){
	
	//refactor this.
	int record_size = 0;
  	for (size_t i = 0; i < schema->attrs.size(); ++i) {
		record_size += schema->attrs[i].length;
	}

	qsort(buf, num_records, record_size, cstr_cmp);
	//TODO error checkin'
	fwrite(buf, record_size, num_records, out_fp);
}


/*
 * Split line on ',' and insert tokens into record
 */
void load_csv_to_buffer(char *line, Schema *schema, char *buf)
{
	char *token;
	int column = 0;
	
	for (token = strtok(line, ","), column = 0;
				token; token=strtok(NULL, ","), column++) {
		
		// based on type in schema, put token into  buffer
		string type = schema->attrs[column].type;
		int length = schema->attrs[column].length;
		//cout << type << endl;
		if (type == "integer") {
			int value = atoi(token);
			int *ibuf = (int*)buf;
			*ibuf = value;
			//cout << *ibuf << endl;
			assert(value == *ibuf);
			buf += sizeof(int);
			
		} else if (type == "float") {
			float value = atof(token);
			float *fbuf = (float*)buf;
			*fbuf = value;
			//cout << *fbuf << endl;
			assert(value == *fbuf);
			buf += sizeof(float);

		} else if (type == "string") {
			strncpy(buf, token, length);
			buf += length;
		}
	}
}

void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema)
{
	int record_size = 0;
  	for (size_t i = 0; i < schema->attrs.size(); ++i) {
		record_size += schema->attrs[i].length;
	}
	char *buf = new char[run_length*record_size];
	
	char line[MAX_LINE_LEN];
	int i = 0;
	while(fgets(line, MAX_LINE_LEN, in_fp)) {
		
		load_csv_to_buffer(line, schema, buf+i*record_size);
		i++;
		if (i == run_length){
			//sort and write all records in buffer
			sort_and_write(buf, i, schema, out_fp);
			i=0; //reset counter
		}
	}

	//reach here at EOF
	if (i != 0) {
		// sort and write the first i records in buffer
		sort_and_write(buf, i, schema, out_fp);
 	}

 	//assert outfp index == (runlength+remainder)*sizeof record

	delete[] buf;

}

void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation
}
