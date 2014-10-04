# define the C compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -Wall -g

SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:.cpp=.o)

LIBRARY = record.o page.o heapfile.o
TARGET = csv2heapfile scan insert update delete select csv2colstore select2 select3

.PHONY: clean
all: $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Libraries
record.o: record.cpp record.h
	$(CC) $(CFLAGS) -c $< -o $@

page.o: page.cpp page.h
	$(CC) $(CFLAGS) -c $< -o $@

heapfile.o: heapfile.cpp heapfile.h
	$(CC) $(CFLAGS) -c $< -o $@

# Applications
csv2heapfile: csv2heapfile.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

csv2colstore: csv2colstore.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

scan: scan.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

insert: insert.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

update: update.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

delete: delete.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

select: select.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

select2: select2.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

select3: select3.o $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@

tests/test_record: tests/test_record.o record.o
	$(CC) $(CFLAGS) $^ -o $@

tests/test_page: tests/test_page.o page.o record.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) *.o *~ $(TARGET) pl1.tar

tar:
	tar cfv pl1.tar $(SOURCES) $(HEADERS)


# Not sure if I want to keep the tests.
test: tests/test_record 
	tests/test_record 

test2: tests/test_page
	tests/test_page
