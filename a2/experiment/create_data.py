from subprocess import Popen, PIPE
import sys
import re

data_dir = "data/"
schema_file = "../schema_example.json"

large_file = data_dir + "150000000"
p = Popen(
    ['../data_generator.py', schema_file, large_file, 150000000])