from subprocess import Popen, PIPE
import sys
import re
import os
import shutil

data_dir = "data/"
schema_file = "../schema_example.json"
out_index = "bsort_index"
sorting_attributes= "student_number"
multiple_sorting_attr = "student_number,cgpa"
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

multi_perf_x =[]
multi_perf_y =[]

#pseudo code for now:
for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../bsort', schema_file, data_file, out_index, sorting_attributes],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    perf_y.append(float(re.findall(r'\d+', s)[0]))
    perf_x.append(sz)
    os.remove(out_index)
    shutil.rmtree("./leveldb_dir")

#pseudo code for now:
for sz in file_size:
    data_file = data_dir + str(sz)
    p = Popen(
         ['../bsort', schema_file, data_file, out_index, multiple_sorting_attr],
         stdout=PIPE)
    s = p.stdout.read().strip()
    print '>%s' % s
    multi_perf_y.append(float(re.findall(r'\d+', s)[0]))
    multi_perf_x.append(sz)
    os.remove(out_index)
    shutil.rmtree("./leveldb_dir")

import matplotlib
from pylab import *

fig, (ax0) = subplots(nrows=1)
plot(perf_x, perf_y, marker='.', color='r', label='single attribute')
plot(multi_perf_x, multi_perf_y, marker='o', color='b', label="multi attributes") 
xlabel('file size(# tuples)')
ylabel('time(ms)')
ax0.set_title('bsort')
legend()
savefig('bsort.png')
show()