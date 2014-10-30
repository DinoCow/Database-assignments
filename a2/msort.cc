#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cassert>

#include "library.h"
#include "json/json.h"

using namespace std;

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
  for (int i = 0; i < json_value.size(); ++i) {
    attr_name = json_value[i].get("name", "UTF-8").asString();
    attr_type = json_value[i].get("type", "UTF-8").asString();
    attr_len = json_value[i].get("length", "UTF-8").asInt();

    Attribute attr = {attr_name, attr_type, attr_len};
    schema.attrs.push_back(attr);

    cout << "{name : " << attr_name 
    << ", length : " << attr_len 
    << ", type : " << attr_type 
    << "}" << endl;
  }

  // Do the sort

  FILE *in_fp = fopen(input_file, "r");
  if (!in_fp) {
    perror("Open input file");
    exit(1);
  }
  // Maybe use tmpfile() but for now it's good for debugging
  FILE *tmp_out_fp = fopen("/tmp/partially_sorted", "w");
  if (!tmp_out_fp) {
    perror("Open temporary output file");
    exit(1);
  }

  long run_length = records_per_buffer(mem_capacity, schema);

  mk_runs(in_fp, tmp_out_fp, run_length, &schema);
  


  /*** This is pretty much pseudo code 
  
  char *buf = new char[buf_size];
  long start_pos = 0;

  for (long n = run_length; n < num_records; n *= k)
  {
    //Create k iterators that read in n-sorted input file
    RunIterator[k] iterators;

    merge_runs(iterators, k, temp_out_fp, start_pos, buf, buf_size);

    start_pos += buf_size;
    swap(n_sorted_fp, tmp_out_fp);
  }

  
  FILE *out_fp = fopen(output_file, "w");
  write_sorted_records(tmp_out_fp, output_file);
  
  */

  return 0;
}
