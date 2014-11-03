from subprocess import Popen, PIPE
import sys
import re

data_dir = "data/"
schema_file = "../schema_example.json"
out_file = "msort_output"
mem_capacity = "100"
mem_capacity2 = "500"
mem_capacity3 = "1000"
k = 40
sorting_attributes="student_number"
file_size = ( #file_size in terms of # of tuples
    100,
    200,
    400,
    800
    # 1500,
    # 5000,
    # 10000,
    # 50000,
    # 100000,
    # 500000,
    # 1000000,
    # 5000000,
    # 10000000
    # 50000000,
    # 100000000,
    # 150000000
)

perf_x = []
perf_y = []
perf_x2 = []
perf_y2 = []
perf_x3 = []
perf_y3 = []

#pseudo code for now:
for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../msort', schema_file, data_file, outfile, mem_capacity, "%s" % k, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y.append( float(re.findall(r'\d+', s)[0]))
    perf_x.append( sz)

for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../msort', schema_file, data_file, outfile, mem_capacity2, "%s" % k, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y2.append( float(re.findall(r'\d+', s)[0]))
    perf_x2.append( sz)

for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../msort', schema_file, data_file, outfile, mem_capacity3, "%s" % k, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y3.append( float(re.findall(r'\d+', s)[0]))
    perf_x3.append( sz)


import matplotlib
from pylab import *

fig, (ax0) = subplots(nrows=1)
plot(perf_x, perf_y, marker='.', color='r', label='mem=100')
plot(perf_x2, perf_y2, marker='o', color='b', label="mem=500") 
plot(perf_x3, perf_y3, marker='^', color='g', label="mem=1000") 
xlabel('file size')
ylabel('time(ms)')
ax0.set_title('msort memeory')
legend()
show()