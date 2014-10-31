#!/usr/bin/env python
'''
You should implement this script to generate test data for your
merge sort program.

The schema definition should be separate from the data generation
code. See example schema file `schema_example.json`.
'''

def generate_data(schema, out_file, nrecords):
  '''
  Generate data according to the `schema` given,
  and write to `out_file`.
  `schema` is an list of dictionaries in the form of:
    [ 
      {
        'name' : <attribute_name>, 
        'length' : <fixed_length>,
        type integer, float, string
        ...
      },
      ...
    ]
  `out_file` is the name of the output file.
  The output file must be in csv format, with a new line
  character at the end of every record.
  '''
  print "Generating %d records" % nrecords


  record_generators = create_record_generators(schema)
  
  with open(out_file, 'w') as csv_file:
    for i in xrange(nrecords):
      column = [str(gen()) for gen in record_generators]
      line = ",".join(column)
      csv_file.write(line)
      csv_file.write("\n")


def create_record_generators(schema):
  '''Return a list of functions which generates random values based 
  on the schema'''
  # TODO
  
  generators = []
  for attr in schema:
    if attr["type"] == "string":
      function = partial(random_str, attr["length"])
    
    elif attr["type"] == "integer":

      if "distribution" in attr:
        function = partial(rand_int_generator, attr["length"], attr["distribution"])
      else:
        function = partial(random.randint, 0, attr["length"])

    elif attr["type"] == "float":

      if "distribution" in attr:
        function = partial(rand_float_generator, attr["length"], attr["distribution"])
      else:
        function = partial(random.randint, 0, attr["length"])

    else:
      pass

    generators.append(function)

  return generators

def rand_int_generator(length, dist):
  if dist["name"] == "uniform":
    return str(random.randint(dist["min"], dist["max"]))
  
  elif dist["name"] == "normal":
    return str(round(random.gauss(dist["mu"], dist["sigma"])))

def random_str(length):
  return "".join([random.choice(string.ascii_letters) for n in xrange(length)])

def rand_float_generator(length, dist):
  if dist["name"] == "uniform":
    return str(random.uniform(dist["min"], dist["max"]))[:length] 

  elif dist["name"] == "normal":
    return str(random.gauss(dist["mu"], dist["sigma"]))[:length]


if __name__ == '__main__':
  import sys, json, random, string
  from functools import partial
  from pprint import pprint
  if not len(sys.argv) == 4:
    print "data_generator.py <schema json file> <output csv file> <# of records>"
    sys.exit(2)

  schema = json.load(open(sys.argv[1]))
  output = sys.argv[2]
  nrecords = int(sys.argv[3])
  pprint( schema )
  
  generate_data(schema, output, nrecords)

