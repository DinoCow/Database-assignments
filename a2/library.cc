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


RecordComparator *rec_cmp;
int qsort_cmp(const void *lhs, const void *rhs){
	//assert(rec_cmp);
    Schema *schema = rec_cmp->schema;
    for (size_t i=0; i<schema->sort_attrs.size(); i++){
		//compare lhs[sort_attr] < rhs[sort_attr] using cmp_fns
		int sort_idx = schema->sort_attrs[i];
		char lhs_attr[40], rhs_attr[40];

		memcpy(lhs_attr, get_attr(sort_idx, (char*)lhs, schema), schema->attrs[sort_idx].length );
		lhs_attr[schema->attrs[i].length] = '\0';
		memcpy(rhs_attr, get_attr(sort_idx, (char*)rhs, schema), schema->attrs[sort_idx].length );
		rhs_attr[schema->attrs[i].length] = '\0';

		cmp_fn_t cmp_fn = rec_cmp->cmp_fns[i];
		int cmp = (*cmp_fn)(lhs_attr, rhs_attr);
   		if (cmp != 0){
			return cmp;
		}
    }
    return 0; //lhs == rhs

}


void write_records(RunIterator *it, FILE *out_fp, Schema *schema){
	while (it->has_next()){
		Record *rec = it->next();
		fwrite(rec->data, schema->record_size, 1, out_fp);
		delete rec;
	}
}

int mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema)
{
	int num_records = 0;
	size_t record_size = schema->record_size, rd;
	char *buf = new char[run_length*record_size];
	
	while((rd = fread(buf, record_size, run_length, in_fp)) != 0){
		qsort(buf, rd, record_size, qsort_cmp);
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
  //TODO this could be optimized with schema->data_offset
  for (int j=0; j < i; j++){
    start += schema->attrs[j].length+1; // +1 for comma
  }
  return start;
}

int int_cmp(const void *lhs, const void *rhs){
	return (atoi((char*)lhs) - atoi((char*)rhs));
}
int float_cmp(const void *lhs, const void *rhs){
  	float fl = atof((char*)lhs);
  	float fr = atof((char*)rhs);
  	return (fl > fr) - (fl < fr);
}

int string_cmp(const void *lhs, const void *rhs){
	return strcmp((char*)lhs, (char*)rhs);
}
