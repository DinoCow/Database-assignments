CC = g++
CCFLAGS = -O3 -Wall -g
LEVELDB_DIR = ./leveldb-1.15.0
LEVELDB_LIB = $(LEVELDB_DIR)/libleveldb.a
LEVELDB_OPTS = -I $(LEVELDB_DIR)/include -pthread $(LEVELDB_LIB)
JSONCPP_OPTS = -I .

all: library.o jsoncpp.o msort bsort

$(LEVELDB_LIB):
	cd $(LEVELDB_DIR) && $(MAKE)

library.o: library.cc library.h
	$(CC) -o $@ -c $< $(CCFLAGS)

jsoncpp.o: jsoncpp.cpp json/json.h
	$(CC) -o $@ -c $< $(CCFLAGS) $(JSONCPP_OPTS)

msort: msort.cc library.o jsoncpp.o
	$(CC) -o $@ $^ $(CCFLAGS)

bsort: bsort.cc jsoncpp.o $(LEVELDB_LIB)
	$(CC) -o $@ $^ $(CCFLAGS) $(LEVELDB_OPTS)
	
clean:
	rm -rf *.o msort bsort msort.dSYM bsort.dSYM
	cd $(LEVELDB_DIR) && $(MAKE) clean
