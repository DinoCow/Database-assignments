from subprocess import Popen, PIPE
import sys
import re

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
# for sz in page_size:
#     p = Popen(
#         ['../csv2colstore', filename, colstore, '%d' % sz],
#         stdout=PIPE)
#     s = p.stdout.read().strip()
#     print '>%s' % s
#     perf_y.append( num_of_rec/ (float(re.findall(r'\d+', s)[0]) * 1000))
#     perf_x.append( sz)

import matplotlib
from pylab import *

line = plot(perf_x, perf_y, marker='.')
xlabel('file size')
ylabel('time')
show()