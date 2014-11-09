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

    set_schema(attr_name, attr_type, attr_len, schema);
  }

  schema.record_size = 0;
  for (size_t i = 0; i < schema.attrs.size(); ++i) {
    schema.record_size += schema.attrs[i].length + 1;
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
  const long run_length = mem_capacity / schema.record_size;  
  const int num_records = mk_runs(in_fp, tmp_out_fp, run_length, &schema);
  
  cout << run_length << ", " << num_records << endl;

  // swap the file pointers so the n-sorted segments become tmp_in_fp
  // this makes the part below easier to understand.
  swap_fp(&tmp_in_fp, &tmp_out_fp);

  // divide avail. memory into k + 1 chunks
  // TODO 1 is for merge output buffer
  const int buf_size = mem_capacity / (k+1);
  char *merge_buf = new char[buf_size];
  

  //for debug
  int pass = 0;

  // n is the current length of sorted segments
  for (int n = run_length; n < num_records; n = n * k){
    cout << "Pass: " << pass++ << " n: " << n << " k: " << k 
      << " runlength: " << run_length << " numrec: " << num_records << endl;
   
    RunIterator *iterators[k];
    Record* buf[k];

    long unprocessed = num_records;
    // n * k lines processed at a time
    for(int offset=0; offset < num_records; offset += n*k) {
        // initialize streams and merge buffer
        
        for (int i = 0; i < k; i++){
          long length;
          if (unprocessed < n){
            length = unprocessed % n;
            unprocessed = n;
          } else {
             length = n;
          }
          iterators[i] = new RunIterator(tmp_in_fp, offset+i*n, 
            length, buf_size, &schema);

          buf[i] = iterators[i]->has_next() ? iterators[i]->next() : NULL;
          unprocessed -= n;
        }
        /*
        // perform k-way merge
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
                } 
              }else {
                min_rec = buf[i];
                min_idx = i;
                
              }
            }
          }
          assert(0 <= min_idx  && min_idx < k);
          fwrite(min_rec->data, schema.record_size, 1, tmp_out_fp);
          buf[min_idx] = iterators[min_idx]->has_next() ? 
                         iterators[min_idx]->next() : NULL;
 
        } while(count(buf, buf+k, (Record*)NULL) == k);
        */
        // Free Iterators
        for (int i=0; i<k; i++){
          delete iterators[i];
        }
    }

    // tmp_out_fp is kn sorted
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
