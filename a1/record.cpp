#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>
#include "record.h"


/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record) {
	return record->size() * ATTRIBUTE_SIZE; 
}

/**
 * Serialize the record to a byte array to be stored in buf.
 * Assumes `buf` is large enough to fit record.
 */
void fixed_len_write(Record *record, void *buf) {

	char* dest = (char *)buf;
	for (Record::iterator it = record->begin(); it!=record->end(); ++it) {
		V attr = *it;
		// Copy at most ATTRIBUTE_SIZE characters.
		strncpy(dest, attr, ATTRIBUTE_SIZE);
		// Move to next position in the buffer.
		dest += ATTRIBUTE_SIZE;
	}
}

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`
 * This function does not resize `record` so make sure
 * `record` size is >= number of records to read.
 * Allocates memory for each attribute. Remember to free it later.
 */
void fixed_len_read(void *buf, int size, Record *record) {
	assert(record->size() >= (size_t)(size / ATTRIBUTE_SIZE));

	char* src = (char *)buf;
	for (Record::iterator it = record->begin(); it!=record->end(); ++it) {
		
		// Make room for null terminator
		char* value = new char[ATTRIBUTE_SIZE+1];
		
		strncpy(value, src, ATTRIBUTE_SIZE);
		// Let's not introduce a buffer overflow vulnerability
		// in our database :)
		value[ATTRIBUTE_SIZE] = '\0';
		*it = value;

		src += ATTRIBUTE_SIZE;
	}
}

/*
 * Print a record in csv format
 */
void print_record_as_csv(Record *record) {
	size_t num_rec = record->size();
	for (int i=0; i<num_rec-1; i++){
		std::cout << (*record)[i] <<",";
	}
	std::cout << (*record)[num_rec-1] << std::endl;
}

/*
 * Split line on ',' and insert tokens into record
 */
void load_csv_record(Record * record, char line[])
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