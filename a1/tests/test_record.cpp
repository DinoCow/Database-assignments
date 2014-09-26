#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstring>

#include "../record.h"

using namespace std;

int main(int argc, char** argv) {

	
	Record rec(100, "1234567891234");
	assert( fixed_len_sizeof(&rec) == 1000) ;

	char * buf = new char [fixed_len_sizeof(&rec)];

	fixed_len_write(&rec, buf);

	for (int i=0; i< 100; i++){
		char* str = buf+10*i;
		//printf("%s!\n", str);
		//cout << str << endl;
		assert(strcmp(str, "123456789")==0);
	}

	Record rec2(100);
	fixed_len_read(buf, 1000, &rec2);
	for (unsigned int i=0; i<rec2.size(); i++) {

		assert(strcmp(rec2[i], "123456789")==0);
		//cout << rec2[i] << endl;
	}

	assert(rec2[5] != rec2[3]);

	//Now memleak free!
	delete[] buf;
	for (unsigned int i=0; i<rec2.size(); i++) {
		delete[] rec2[i];
	}
	cout << sizeof(size_t) << endl;
	cout << sizeof(unsigned int) << endl;
	cout << sizeof(rec2.size()) << endl;
	return 0;
}
