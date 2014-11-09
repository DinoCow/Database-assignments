#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <sys/timeb.h>

#include "library.h"

long get_time_ms()
{
	struct timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}

/* comparison function just for testing.
   need to write a proper comparator for each type
*/
int cstr_cmp(const void *a, const void *b){
	return strcmp((const char*)a,(const char*)b);
}

RecordComparator *rec_cmp;
int qsort_cmp(const void *lhs, const void *rhs){
	//assert(rec_cmp);
    /*
    for (size_t i=0; i<rec_cmp->sort_attrs.size(); i++){
      //compare lhs[sort_attr] < rhs[sort_attr] using cmp_fns
      int sort_idx = rec_cmp->schema->sort_attrs[i];
      Comparator *cmp_fn = rec_cmp->cmp_fns[i];
      
      const char *lhs_attr = (lhs, sort_idx, rec_cmp->schema);
      const char *rhs_attr = r_rec.get_attr(sort_idx, rec_cmp->schema);

      bool cmp = (*cmp_fn)( lhs_attr, rhs_attr);

      if (cmp != 0){
        return cmp;
      }
    }
    */
    return 0; //lhs == rhs

}

void sort_and_write(char *buf, int num_records, Schema *schema, FILE *out_fp){
	
	size_t record_size = schema->record_size;
	
	qsort(buf, num_records, record_size, qsort_cmp);

	//TODO error checkin'
	assert(fwrite(buf, record_size, num_records, out_fp) == (size_t)num_records);
}


void write_sorted_records(RunIterator *it, FILE *out_fp, Schema *schema){
	while (it->has_next()){
		Record *rec = it->next();
		fwrite(rec->data, schema->record_size, 1, out_fp);
	}
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
		TYPE type = schema->attrs[column].type;
		int length = schema->attrs[column].length;
		// TODO switch
		if (type == INT) {
			int value = atoi(token);
			int *ibuf = (int*)buf;
			*ibuf = value;
			//cout << *ibuf << endl;
			assert(value == *ibuf);
			buf += sizeof(int);
			
		} else if (type == FLOAT) {
			float value = atof(token);
			float *fbuf = (float*)buf;
			*fbuf = value;
			//cout << *fbuf << endl;
			assert(value == *fbuf);
			buf += sizeof(float);

		} else if (type == STRING) {
			strncpy(buf, token, length);
			buf += length;
		}
	}
}

int mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema)
{
	int num_records = 0;
	size_t record_size = schema->record_size, rd;
	char *buf = new char[run_length*record_size];
	
	while((rd = fread(buf, record_size, run_length, in_fp)) != 0){
		qsort(buf, rd, record_size, cstr_cmp);
		fwrite(buf, record_size, rd, out_fp);
		num_records += rd;
	}

	delete[] buf;
	return num_records;
}

void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size)
{
  // Your implementation
}

// Sets the schema information for each attribute
// Returns the length of the attribute
int set_schema(string name, string type, int len, Schema &schema)
{
	//todo ctor
	Attribute attr;
    attr.name = name;
    attr.length = len;
    if (type == "integer"){
      attr.type = INT;
    } else if (type == "float") {
      attr.type = FLOAT;
    } else if (type == "string") {
      attr.type = STRING;
    } else {
      //TODO error
    }
    
    schema.attrs.push_back(attr);

    return attr.length;
}

// Set the sort_attr vector within the schema
void set_schema_sort_attr(Schema &schema, const char *sorting_attr)
{
	char *token;
	char *attrs = const_cast<char *>(sorting_attr);
	token = strtok(attrs, ",");

	while (token != NULL) {
		string sort_attr(token);
		bool attr_found = false;

		for (size_t i = 0; i < schema.attrs.size(); ++i) {
			if (sort_attr.compare(schema.attrs[i].name) == 0)
			{
				schema.sort_attrs.push_back(i);
				attr_found = true;
				break;
			}
		}

		if (!attr_found){
			cout << "ERROR: invalid sorting attributes!" << endl;
			exit(1);
		}

		token = strtok(NULL, ",");
	}
}

/****************** RunIterator *********************/

/**
* reads the next record
*/
Record* RunIterator::next(){

	size_t record_size = schema->record_size;
	if (  pos % rec_per_buf == 0 ){	
		long offset = (start_pos + buf_no * rec_per_buf) * record_size;
		//cout << "Offset: "<< offset << endl;
		if (fseek(fp, offset, SEEK_SET) != 0){
			perror("fseek");
			exit(1);
		}
		fread(read_buf, record_size, rec_per_buf, fp);
		buf_no++;
	}
	Record *rec = new Record();
	
	rec->data = &read_buf[(pos % rec_per_buf) * record_size];
	
	/*
	char debug[100];
	strncpy(debug, rec->data, record_size);
	debug[record_size] = '\0';
	printf(debug);
	*/
	pos++;
	return rec;
}

/**
* return false if iterator reaches the end
* of the run
*/
bool RunIterator::has_next(){
	return pos < run_length;
}



char *get_attr(int i, char *data, Schema *schema){
  char *start = data;
  for (int j=0; j < i; j++){
    start += schema->attrs[j].length+1; // +1 for comma
  }
  return start;
}