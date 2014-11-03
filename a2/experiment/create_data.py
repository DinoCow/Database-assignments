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
    100000000,
    150000000

)

perf_x = [ 1, 4, 10, 14]
perf_y = [100, 200, 400, 800]

#pseudo code for now:
for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../data_generator.py', schema_file, data_file, '%d' % sz])
