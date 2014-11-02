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
    1600,
    3200,
    6400,
    12800,
    25600,
    51200
)

perf_x = [ 1, 4, 10, 14]
perf_y = [100, 200, 400, 800]

#pseudo code for now:
for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../data_generator.py', schema_file, data_file, '%d' % sz])
