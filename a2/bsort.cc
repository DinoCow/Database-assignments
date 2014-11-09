#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>

#include "library.h"
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "json/json.h"

using namespace std;

Schema schema; //make this global, so that it can be accessed by comparator

// The comparator function for the B+ Tree
// Three-way comparsison function:
// 	if a < b: negative result
//	if a > b: positive result
// 	else: zero result
class SortComparator : public leveldb::Comparator {
	public: 
		int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
			string token;
			vector<string> a_vec;
			vector<string> b_vec;
			const char *a_buf = a.ToString().c_str();
			const char *b_buf = b.ToString().c_str();

			int size = schema.sort_attrs.size();
			
			// Increment by two because after each value is followed by its type
			for (int i = 0; i < size; i++)
			{
				int attrs_idx = schema.sort_attrs[i];
				int attr_len = schema.attrs[attrs_idx].length;
				TYPE type = schema.attrs[attrs_idx].type; 

				// If a == b, interate to next value to compare
				if (type == INT)
				{
					int a_int = atoi(a_buf);
					int b_int = atoi(b_buf);

					if (a_int < b_int) return -1;
					if (a_int > b_int) return 1;
				}
				else if (type == FLOAT)
				{
					float a_float = atof(a_buf);
					float b_float = atof(b_buf);

					if (a_float < b_float) return -1;
					if (a_float > b_float) return 1;
				}
				else if (type == STRING)
				{
					int cmp = strcmp(a_buf, b_buf);
					if (cmp < 0) return -1;
					if (cmp > 0) return 1;
				}

				a_buf = a_buf + attr_len + 1;
			 	b_buf = b_buf + attr_len + 1;
			}

			// If the same values - base on the counter or will ignore duplicate
			int last_a = atoi(a_buf);
			int last_b = atoi(b_buf);

			if (last_a < last_b) return -1;
			if (last_a > last_b) return 1;

			return 0;
		}

		// Leave the other functions...
		const char* Name() const { return "SortComparator"; }
		void FindShortestSeparator(string*, const leveldb::Slice&) const { }
		void FindShortSuccessor(string*) const { }
};

// Get the key for the record (line)
// The key is the list of attributes in order of sorted_attributes
string get_key(char *line, int unique_counter) 
{
	string key;

	// For each attribute in the list of attributes we wish to sort by
	for (size_t i = 0; i < schema.sort_attrs.size(); ++i) 
	{
		// Get the length to the start of the current attribute
		int attrs_idx = schema.sort_attrs[i];
		int offset = schema.data_offset[attrs_idx];
		int attr_len = schema.attrs[attrs_idx].length;
		
		// Get the value of the current attribute
		char *attribute = new char[attr_len];
		strncpy(attribute, line+offset, attr_len);
		attribute[attr_len] = '\0';

		// Compile the key
		if (i > 0) 
		{
			key = key + '\0' + attribute;
		} else 
		{
			key = attribute;
		}
	}

	std::stringstream ss;
	ss << unique_counter;
	key = key + '\0' + ss.str();
	return key;
}

// Insert the file (in_fp) into records into the b+ tree
void insert_tree(FILE *in_fp, leveldb::DB *db)
{
	char line[MAX_LINE_LEN];
    long unique_counter = 0;

	while(fgets(line, MAX_LINE_LEN, in_fp)) 
	{
		// Put in a unique key (use a counter)
		string key = get_key(line, unique_counter);
		leveldb::Status s = db->Put(leveldb::WriteOptions(), key, line);
		if (!s.ok())
		{
			cerr << "Cannot insert: " << key << " " << s.ToString() << endl;
		}
		unique_counter++;
	}
}

// Retrieve the records from the b+ tree to a file (out_fp)
void retrieve_tree(FILE *out_fp, leveldb::DB *db)
{
	// Get all the records and put into the outfile
 	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
 	for (it->SeekToFirst(); it->Valid(); it->Next()) {
 		string key_str = it->key().ToString();
 		string val_str = it->value().ToString();

 		// +1 for the null terminator
 		fwrite(val_str.c_str(), sizeof(char), val_str.length()+1, out_fp);
 	}
 	assert (it->status().ok());
 	delete it;
}

int main(int argc, const char* argv[]) {

	if (argc != 5) { 
		cout << "ERROR: invalid input parameters!" << endl;
		cout << "Please enter <schema_file> <input_file> <out_index> <sorting_attributes>" << endl;
		exit(1);
	}
	string schema_file(argv[1]);
  	const char* input_file = argv[2];
  	const char* output_file = argv[3];
  	const char* sorting_attributes = argv[4]; 

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

  	long start = get_time_ms();
  	int total_length = 0;

  	// Create schema
  	string attr_name, attr_type;
  	int attr_len;
  	for (int i = 0; i < json_value.size(); ++i) {
    	attr_name = json_value[i].get("name", "UTF-8").asString();
	    attr_type = json_value[i].get("type", "UTF-8").asString();
	    attr_len = json_value[i].get("length", "UTF-8").asInt();

	    // Set the schema information
	    int len = set_schema(attr_name, attr_type, attr_len, schema);
	    schema.data_offset.push_back(total_length);
	    total_length += len + 1;
  	}
  	set_schema_sort_attr(schema, sorting_attributes);

    // Open a database connection to "./leveldb_dir"
    SortComparator cmp;
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = true;
    options.comparator = &cmp;
    leveldb::Status status = leveldb::DB::Open(options, "./leveldb_dir", &db);

    // Check database opened properly
    if (!status.ok())
    {
    	cerr << "Datatbase Status: " << status.ToString() << endl;
    	exit(1);
    }

  	// Open the data file and put data into btree
  	FILE *in_fp = fopen(input_file, "r");
  	if (!in_fp) {
    	perror("Open input file");
    	exit(1);
  	}
    insert_tree(in_fp, db);

    // Open output file and output sorted records
	FILE *out_fp = fopen(output_file, "w");
  	if (!out_fp) {
    	perror("Open output file");
    	exit(1);
  	}
  	retrieve_tree(out_fp, db);

 	// Clean up!
 	fclose(in_fp);
 	fclose(out_fp);
	delete db;
	long end = get_time_ms();
	cout << "TIME: " << end - start << " milliseconds" << endl;

	return 0;
}