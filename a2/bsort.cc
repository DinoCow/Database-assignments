#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>

#include "library.h"
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "json/json.h"
const int MAX_LINE_LEN = 10000;

using namespace std;

void parseKey(vector<string>& v, string s)
{
	string token;
	size_t pos = 0;

	while ((pos = s.find(",")) != string::npos)
	{
		token = s.substr(0, pos);
		v.push_back(token);
		// +1 to erase comma
		s.erase(0, pos+1);
	}
	v.push_back(s);
}

class TwoPartComparator : public leveldb::Comparator {
	public: 
		// Three-way comparsison function:
		// 	if a < b: negative result
		//	if a > b: positive result
		// 	else: zero result
		int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
			string token;
			vector<string> a_vec;
			vector<string> b_vec;

			parseKey(a_vec, a.ToString());
			parseKey(b_vec, b.ToString());

			// a_vec and b_vec should have the same size
			// Increment by two because after each value is followed by its type
			for (int i = 0; i < a_vec.size() - 1; i = i + 2)
			{
				string type = a_vec[i+1];
				
				// If a == b, interate to next value to compare
				if (type == "integer")
				{
					int a_int = atoi(a_vec[i].c_str());
					int b_int = atoi(b_vec[i].c_str());

					if (a_int < b_int) return -1;
					if (a_int > b_int) return 1;
				}
				else if (type == "float")
				{
					float a_float = atof(a_vec[i].c_str());
					float b_float = atof(b_vec[i].c_str());

					if (a_float < b_float) return -1;
					if (a_float > b_float) return 1;
				}
				else if (type == "string")
				{
					int cmp = a_vec[i].compare(b_vec[i]);
					if (cmp < 0) return -1;
					if (cmp > 0) return 1;
				}
			}

			return 0;
		}

		// Ignore other functions for now
		const char* Name() const { return "TwoPartComparator"; }
		void FindShortestSeparator(string*, const leveldb::Slice&) const { }
		void FindShortSuccessor(string*) const { }
};

// Set the sort_attr vector within the schema
void set_schema_sort_attr(Schema &schema, const char *sorting_attr)
{
	char *token;
	char *attrs = const_cast<char *>(sorting_attr);
	token = strtok(attrs, ",");

	while (token != NULL) {
		string sort_attr(token);
		for (size_t i = 0; i < schema.attrs.size(); ++i) {
			if (sort_attr.compare(schema.attrs[i].name) == 0)
			{
				schema.sort_attrs.push_back(i);
				break;
			}
		}

		token = strtok(NULL, ",");
	}
}

// Returns the length from the start of the record to the indexed attribute
int lenth_to_index(int index, Schema *schema)
{
	int total_length = 0;
	for (int i = 0; i < index; ++i) {
		// Plus 1 for the comma
		total_length += schema->attrs[i].length + 1;
	}
	return total_length;
}

// Get the key for the record (line)
// The key is the list of attributes in order of sorted_attributes
string get_key(char *line, Schema *schema) 
{
	string key;

	// For each attribute in the list of attributes we wish to sort by
	for (size_t i = 0; i < schema->sort_attrs.size(); ++i) 
	{
		// Get the length to the start of the current attribute
		int length = lenth_to_index(schema->sort_attrs[i], schema);
		int attrs_idx = schema->sort_attrs[i];
		int attr_len = schema->attrs[attrs_idx].length;
		
		// Get the value of the current attribute
		char *attribute = new char[attr_len];
		strncpy(attribute, line+length, attr_len);
		attribute[attr_len] = '\0';

		// Compile the key
		if (i > 0)
			key = key + ",";

		// Use comma for type again because 
		// no guarantee other delimiters won't be in the data itself
		TYPE type = schema->attrs[attrs_idx].type;
		if (type == INT)
			key = key + attribute + "," + "integer";
		else if (type == FLOAT)
			key = key + attribute + "," + "float";
		else if (type == STRING)
			key = key + attribute + "," + "string";
	}

	return key;
}

int main(int argc, const char* argv[]) {

	// Account for people putting spaces in sorting_attributes later (<5)
	if (argc != 5) { 
		cout << "ERROR: invalid input parameters!" << endl;
		cout << "Please enter <schema_file> <input_file> <out_index> <sorting_attributes>" << endl;
		exit(1);
	}
	string schema_file(argv[1]);
  	const char* input_file = argv[2];
  	const char* output_file = argv[3];
  	// This has to change if taking into account spaces in sorting_attributes
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

  	Schema schema;

  	// Create schema
  	string attr_name, attr_type;
  	int attr_len;
  	for (int i = 0; i < json_value.size(); ++i) {
    	attr_name = json_value[i].get("name", "UTF-8").asString();
	    attr_type = json_value[i].get("type", "UTF-8").asString();
	    attr_len = json_value[i].get("length", "UTF-8").asInt();

	    Attribute attr;
	    attr.name = attr_name;
	    attr.length = attr_len;
	    if (attr_type == "interger"){
	      attr.type = INT;
	    } else if (attr_type == "float") {
	      attr.type = FLOAT;
	    } else if (attr_type == "string") {
	      attr.type = STRING;
	    } else {
	      //TODO error
	    }
	    schema.attrs.push_back(attr);
  	}
  	set_schema_sort_attr(schema, sorting_attributes);

  	// Open the data file
  	FILE *in_fp = fopen(input_file, "r");
  	if (!in_fp) {
    	perror("Open input file");
    	exit(1);
  	}

    // Open a database connection to "./leveldb_dir"
    TwoPartComparator cmp;
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    options.comparator = &cmp;
    leveldb::Status status = leveldb::DB::Open(options, "./leveldb_dir", &db);

    // Check database opened properly
    if (!status.ok())
    {
    	cerr << "Datatbase Status: " << status.ToString() << endl;
    	exit(1);
    }

    // Insert all the records into the tree
    // Using an index based on <sorting_attributes> => COMPARATOR!
    char line[MAX_LINE_LEN];
	while(fgets(line, MAX_LINE_LEN, in_fp)) 
	{
		// TODO: Deal with unique keys!!
		string key = get_key(line, &schema);
		leveldb::Status s = db->Put(leveldb::WriteOptions(), key, line);
		if (!s.ok())
		{
			cerr << "Cannot insert: " << key << " " << s.ToString() << endl;
		}
	}

	FILE *out_fp = fopen(output_file, "w");
  	if (!out_fp) {
    	perror("Open output file");
    	exit(1);
  	}

    // Get all the records and put into the outfile
 	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
 	for (it->SeekToFirst(); it->Valid(); it->Next()) {
 		string key_str = it->key().ToString();
 		string val_str = it->value().ToString();
 		cout << val_str;
 		// +1 for the null terminator
 		fwrite(val_str.c_str(), sizeof(char), val_str.length()+1, out_fp);
 	}
 	assert (it->status().ok());

 	// Clean up!
 	fclose(in_fp);
 	fclose(out_fp);
 	delete it;
	delete db;

	return 0;
}