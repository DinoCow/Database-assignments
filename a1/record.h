#ifndef RECORD_H
#define RECORD_H

#include <vector>

typedef const char* V;
typedef std::vector<V> Record;
const int ATTRIBUTE_SIZE = 10; //bytes

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record);

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf);

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record);

/*
 * Print a record in csv format
 */
void print_record_as_csv(Record *record);

/*
 * Split line on ',' and insert tokens into record
 */
void load_csv_record(Record * record, char line[])
#endif