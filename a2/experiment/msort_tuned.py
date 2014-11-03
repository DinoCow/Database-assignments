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

msort <schema_file> <input_file> <out_file> <mem_capacity> <k> <sorting_attributes>
#pseudo code for now:
for sz in file_size:
    data_file = data_dir + str(sz)
    # p = Popen(
    #      ['../msort', schema_file, data_file, <out_file>, <mem_capacity>, <k>, <sorting_attributes>],
    #      stdout=PIPE)
    # s = p.stdout.read().strip()
    #     print '>%s' % s
    #     perf_y.append( float(re.findall(r'\d+', s)[0]))
    #     perf_x.append( sz)


import matplotlib
from pylab import *

line = plot(perf_x, perf_y, marker='.')
xlabel('file size')
ylabel('time')
show()