#include <cstdio>
#include <string>
#include <vector>

#include <cstring>

using namespace std;
const int MAX_LINE_LEN = 10000;

// class RecordComparator;
enum TYPE {
  INT,
  FLOAT,
  STRING
};
/**
 * An attribute schema.
 */
typedef struct {
  string name;
  TYPE type;
  int length;
} Attribute;

/**
 * A record schema contains an array of attribute
 * schema `attrs`, as well as an array of sort-by 
 * attributes (represented as the indices of the 
 * `attrs` array).
 */
typedef struct {
  vector<Attribute> attrs;
  vector<int> sort_attrs;
  vector<int> data_offset;
  size_t record_size;
} Schema;

/**
 * A record is just a pointer to data. 
 */
typedef struct {
  char* data;

  char *get_attr(int index, Schema *schema) const {
    int offset = schema->data_offset[index];
    return data + offset;
  }
} Record;

long get_time_ms();

/**
 * Creates sorted runs of length `run_length` in
 * the `out_fp`.
 */
int mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema);

/**
 * Sets the schema information for each attribute
 * Returns the length of the attribute
 */
int set_schema(string name, string type, int len, Schema &schema);

// Set the sort_attr vector within the schema
void set_schema_sort_attr(Schema &schema, const char *sorting_attr);

/**
 * The iterator helps you scan through a run.
 * you can add additional members as your wish
 */
class RunIterator {
  FILE *fp;
  long start_pos;
  long run_length;
  long buf_size;
  Schema *schema;
  char *read_buf;
  long pos;
  long buf_no;
  long rec_per_buf;
public:
  /**
   * Creates an interator using the `buf_size` to
   * scan through a run that starts at `start_pos`
   * with length `run_length`.
   */
  RunIterator(FILE *fp, long start_pos, long run_length, long buf_size,
              Schema *schema): fp(fp), start_pos(start_pos),
               run_length(run_length), buf_size(buf_size), schema(schema),
               pos(0), buf_no(0){
    read_buf = new char[buf_size];
    rec_per_buf = buf_size / schema->record_size;
  }

  /**
   * free memory
   */
  ~RunIterator(){
    delete[] read_buf;
  }

  /**
   * reads the next record
   */
  Record* next();

  /**
   * return false if iterator reaches the end
   * of the run
   */
  bool has_next();
};

/**
 * Merge runs given by the `iterators`.
 * The number of `iterators` should be equal to the `num_runs`.
 * Write the merged runs to `out_fp` starting at position `start_pos`.
 * Cannot use more than `buf_size` of heap memory allocated to `buf`.
 */
void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size);


class Comparator {
public:
  virtual int operator() (const char *lhs, const char *rhs) = 0;
};

// For comparing int, float
template <typename T>
class NumericalComparator: public Comparator {
  int operator() (const char *lhs, const char *rhs) {
    //TODO c-style comparator
    return ( *(T*)lhs - *(T*)rhs );
  }
};
// For comparing cstrings
class StringComparator : public Comparator {
  int strlen;
public:
  StringComparator(int length): strlen(length){};
  int operator() (const char *lhs, const char *rhs) {
    return strncmp(lhs, rhs, strlen);
  }
};


class RecordComparator {
public:
  vector<int> sort_attrs;
  vector<Comparator*> cmp_fns;
  Schema *schema;
  RecordComparator(const vector<Attribute> &attrs, const char *sorting_attributes, Schema *schema) {
    this->schema = schema;
    //TODO fill sort attr
    sort_attrs.push_back(3);//cGPA

    for (vector<int>::iterator it = sort_attrs.begin(); it != sort_attrs.end(); ++it){
      switch(attrs[*it].type){
        case INT:
          cmp_fns.push_back(new NumericalComparator<int>());
          break;
        case FLOAT:
          cmp_fns.push_back(new NumericalComparator<float>());
          break;
        case STRING:
          cmp_fns.push_back(new StringComparator(attrs[*it].length));
          break;
      }
    }
  }

  // Return true iff lhs < rhs
  bool operator() (const Record &lhs, const Record &rhs) {
    for (size_t i=0; i<sort_attrs.size(); i++){
      //compare lhs[sort_attr] < rhs[sort_attr] using cmp_fns
      //cout << *it << endl;
      int sort_idx = sort_attrs[i];
      const char *lhs_attr = lhs.get_attr(sort_idx, schema);
      const char *rhs_attr = rhs.get_attr(sort_idx, schema);

      Comparator *cmp_fn = cmp_fns[i];
      bool cmp = (*cmp_fn)(lhs_attr, rhs_attr);

      if (cmp != 0){
        return cmp;
      }
    }
    return false; //lhs == rhs
  }
};


void write_sorted_records(RunIterator *it, FILE *out_fp, Schema *schema);
