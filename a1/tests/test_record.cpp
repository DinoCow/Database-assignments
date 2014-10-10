#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstring>

#include "../record.h"

using namespace std;

int main(int argc, char** argv) {

	char teststr[] = "12345678"; //less than 10 characters 
	Record rec(100, teststr);
	assert( fixed_len_sizeof(&rec) == 1000) ;

	char * buf = new char [fixed_len_sizeof(&rec)];

	fixed_len_write(&rec, buf);

	Record rec2(100);
	fixed_len_read(buf, 1000, &rec2);
	for (unsigned int i=0; i<rec2.size(); i++) {
		assert(strcmp(rec2[i], teststr)==0);
	}

	assert(rec2[5] != rec2[3]);
	for (unsigned int i=0; i<rec2.size(); i++) {
		delete[] rec2[i];
	}

	char teststr2[] = "123456789012345"; //more than 10 characters
	char expected[] = "1234567890";// expect it to truncate to 10 chars
	Record rec3(100, teststr2);

	fixed_len_write(&rec3, buf);
	fixed_len_read(buf, 1000, &rec2);
	for (unsigned int i=0; i<rec2.size(); i++) {
		assert(strcmp(rec2[i], expected)==0);
	}

	//Now memleak free!
	delete[] buf;
	for (unsigned int i=0; i<rec2.size(); i++) {
		delete[] rec2[i];
	}

	return 0;
}
