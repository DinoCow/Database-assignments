#include <vector>
#include <cstring>
#include <cassert>
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

		strncpy(dest, attr, ATTRIBUTE_SIZE);
		// Make sure all strings are NULL terminated.
		dest[ATTRIBUTE_SIZE-1] = '\0';
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
	
		char* value = new char[ATTRIBUTE_SIZE];
		
		strncpy(value, src, ATTRIBUTE_SIZE);
		// Let's not introduce a buffer overflow vulnerability
		// in our database :)
		value[ATTRIBUTE_SIZE-1] = '\0';
		*it = value;

		src += ATTRIBUTE_SIZE;
	}
}
