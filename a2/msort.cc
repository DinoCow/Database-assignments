#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>

#include "library.h"
#include "json/json.h"

using namespace std;

// Evil global
RecordComparator *rec_cmp;

// <algorithm> std::swap also works
static void swap_fp(FILE **a, FILE **b){
  FILE *tmp = *a;
  *a = *b;
  *b = tmp;
}

static int records_per_buffer(long mem_capacity, Schema &schema){

  int record_size = 0;
  for (size_t i = 0; i < schema.attrs.size(); ++i) {
    record_size += schema.attrs[i].length;
  }

  return mem_capacity / record_size;

}

int main(int argc, const char* argv[]) {
  if (argc < 7) {
    cout << "ERROR: invalid input parameters!" << endl;
    cout << "Please enter <schema_file> <input_file> <output_file> <mem_capacity> <k> <sorting_attributes>" << endl;
    exit(1);
  }
  string schema_file(argv[1]);
  const char *input_file = argv[2];
  const char* output_file = argv[3];
  const long mem_capacity = atol(argv[4]);
  const int k = atoi(argv[5]);
  const char* sorting_attributes = argv[6];

  assert(mem_capacity>0);
  assert(k>0);

  // Parse the schema JSON file
  Json::Value json_value;
  Json::Reader json_reader;
  // Support for std::string argument is added in C++11
  // so you don't have to use .c_str() if you are on that.
  ifstream schema_file_istream(schema_file.c_str(), ifstream::binary);
  bool successful = json_reader.parse(schema_file_istream, json_value, false);
  if (!successful) {
    cout << "ERROR: " << json_reader.getFormatedErrorMessages() << endl;
    exit(1);
  }

  Schema schema;
  // Print out the schema
  string attr_name, attr_type;
  int attr_len;
  int offset = 0;
  for (int i = 0; i < json_value.size(); ++i) {
    attr_name = json_value[i].get("name", "UTF-8").asString();
    attr_type = json_value[i].get("type", "UTF-8").asString();
    attr_len = json_value[i].get("length", "UTF-8").asInt();

    cout << "{name : " << attr_name 
    << ", length : " << attr_len 
    << ", type : " << attr_type 
    << "}" << endl;

    //set_schema(attr_name, attr_type, attr_len, schema);

    Attribute attr;
    attr.name = attr_name;
    attr.length = attr_len;
    if (attr_type == "integer"){
      attr.type = INT;
      schema.data_offset.push_back(offset);
      offset += sizeof(int);
    } else if (attr_type == "float") {
      attr.type = FLOAT;
      schema.data_offset.push_back(offset);
      offset += sizeof(float);
    } else if (attr_type == "string") {
      attr.type = STRING;
      schema.data_offset.push_back(offset);
      offset += attr_len;
    } else {
      //TODO error
    }
    
    schema.attrs.push_back(attr);
  }

  schema.record_size = 0;
  for (size_t i = 0; i < schema.attrs.size(); ++i) {
    cout << schema.data_offset[i] << endl;
    schema.record_size += schema.attrs[i].length;
  }



  rec_cmp = new RecordComparator(schema.attrs, sorting_attributes, &schema);
  
  // Do the sort

  FILE *in_fp = fopen(input_file, "r");
  if (!in_fp) {
    perror("Open input file");
    exit(1);
  }
  // Maybe use tmpfile() but for now it's good for debugging
  FILE *tmp_out_fp = fopen("/tmp/output", "w");
  if (!tmp_out_fp) {
    perror("Open temporary output file");
    exit(1);
  }
  FILE *tmp_in_fp = fopen("/tmp/input", "w");
  if (!tmp_in_fp) {
    perror("Open temporary input file");
    exit(1);
  }

  // length of sorted segments
  const long run_length = records_per_buffer(mem_capacity, schema);
  const int num_records = mk_runs(in_fp, tmp_out_fp, run_length, &schema);
  
  // swap the file pointers so the n-sorted segments become tmp_in_fp
  // this makes the part below easier to understand.
  swap_fp(&tmp_in_fp, &tmp_out_fp);

  // divide avail. memory into k + 1 chunks
  // 1 is for merge output buffer
  const int buf_size = mem_capacity / (k+1);
  char *merge_buf = new char[buf_size];
  

  //const int num_runs = num_records / run_length;
  //  long output_pos = 0;
  //  long start_pos = 0;


  //for debug
  int pass = 0;

  // n is the current length of sorted segments
  for (int n = run_length; n < num_records; n = n * k){

    pass++;
    cout << "Pass: " << pass << " n: " << n << " k: " << k 
      << " runlength: " << run_length << " numrec: " << num_records << endl;
    //merge(D,E,n)// E is kn sorted

    RunIterator *iterators[k];
    Record* buf[k];
 

    for(int offset=0; offset < num_records; offset += n*k) {
        // initialize streams and merge buffer
        for (int i = 0; i < k; i++){
          // TODO adjust n size of thingo
          iterators[i] = new RunIterator(tmp_in_fp, offset+i*n, 
            n, buf_size, &schema);

          buf[i] = iterators[i]->has_next() ? iterators[i]->next() : NULL;
        }

        // perform (up to) k-way merge
        //merge_runs(iterators, k, tmp_out_fp, output_pos, merge_buf, buf_size);
        Record *min_rec = NULL;
        int min_idx = -1;
        do {
          for (int i = 0; i < k; i++){
            if (buf[i]) {
              if (min_rec){
                if ((*rec_cmp)(*buf[i], *min_rec)) {
                  //buf[i] < minrec
                  min_rec = buf[i];
                  min_idx = i;
                  int *start_year = (int*)(buf[i])->get_attr(2, &schema);
                  cout << *start_year << endl;
                } 
              }else {
                min_rec = buf[i];
                min_idx = i;
                
              }
            }
          }
          assert(0 <= min_idx  && min_idx < k);
          //fwrite(buf, schema.record_size, num_records, tmp_out_fp);
          fwrite(min_rec->data, schema.record_size, 1, tmp_out_fp);
          buf[min_idx] = iterators[min_idx]->has_next() ? 
                         iterators[min_idx]->next() : NULL;
 
        } while(count(buf, buf+k, (Record*)NULL) == k);

        // Free Iterators
        for (int i=0; i<k; i++){
          delete iterators[i];
        }
    }

    // tmp_out_fp is kn sortedssss
    rewind(tmp_out_fp);rewind(tmp_in_fp);
    swap_fp(&tmp_in_fp, &tmp_out_fp);
  }


  RunIterator *full_iterator = new RunIterator(tmp_in_fp, 0, num_records, buf_size, &schema);
  
  FILE *out_fp = fopen(output_file, "w");
  //write_sorted_records(full_iterator, out_fp, &schema);
  
  fclose(tmp_in_fp);
  fclose(tmp_out_fp);
  fclose(out_fp);

  return 0;
}
