from subprocess import Popen, PIPE
import sys
import re

data_dir = "data/"
schema_file = "../schema_example.json"
file_size = ( #file_size in terms of # of tuples
    100,
    200,
    400,
    800,
    1500,
    5000,
    10000,
    50000,
    100000,
    500000,
    1000000,
    5000000,
    10000000,
    50000000,
    100000000
)

large_file = data_dir + "150000000"

for sz in file_size:
    data_file = data_dir + str(sz)
    with open(large_file, 'r') as lg_fd:
        with open(data_file, 'w') as write_file:
            for i in xrange(sz):
                write_file.write(lg_fd.readline())