from subprocess import Popen, PIPE
import sys
import re

data_dir = "data/"
schema_file = "../schema_example.json"
out_file = "msort_output"
mem_capacity = "409600000"
k1 = 4
k2 = 8
k3 = 16
sorting_attributes="student_number"
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
         ['../msort', schema_file, data_file, out_file, mem_capacity, "%s" % k1, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y.append( float(re.findall(r'\d+', s)[0]))
    perf_x.append( sz)

for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../msort', schema_file, data_file, out_file, mem_capacity, "%s" % k2, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y2.append( float(re.findall(r'\d+', s)[0]))
    perf_x2.append( sz)

for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../msort', schema_file, data_file, out_file, mem_capacity, "%s" % k3, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y3.append( float(re.findall(r'\d+', s)[0]))
    perf_x3.append( sz)


import matplotlib
from pylab import *

fig, (ax0) = subplots(nrows=1)
plot(perf_x, perf_y, marker='.', color='r', label='k=4')
plot(perf_x2, perf_y2, marker='o', color='b', label="k=8") 
plot(perf_x3, perf_y3, marker='^', color='g', label="k=16") 
xlabel('file size (# tuples)')
ylabel('time(ms)')
ax0.set_title('msort k')
legend()
savefig('msort_kway.png')
show()