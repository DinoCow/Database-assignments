#include <cstdio>
#include <string>
#include <vector>

#include <cstring>

using namespace std;

class RecordComparator;
enum TYPE {
  INT,
  FLOAT,
  STRING
};
/**
 * An attribute schema. You should probably modify
 * this to add your own fields.
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
  vector<int> data_offset;
  RecordComparator* comparator;
} Schema;

/**
 * A record can defined as a struct with a pointer
 * to the schema and some data. 
 */
typedef struct {
  Schema* schema;
  char* data;

  char* operator[] (const int index) const {
    int offset = schema->data_offset[index];
    return data + offset;
  }
} Record;

/**
 * Creates sorted runs of length `run_length` in
 * the `out_fp`.
 */
int mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema *schema);

/**
 * The iterator helps you scan through a run.
 * you can add additional members as your wish
 */
class RunIterator {
public:
  /**
   * Creates an interator using the `buf_size` to
   * scan through a run that starts at `start_pos`
   * with length `run_length`.
   */
  RunIterator(FILE *fp, long start_pos, long run_length, long buf_size,
              Schema *schema);

  /**
   * free memory
   */
  ~RunIterator();

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
  vector<int> sort_attrs;
  vector<Comparator*> cmp_fns;
public:
  RecordComparator(const vector<Attribute> &attrs, const char *sorting_attributes) {

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
    for (vector<int>::iterator it = sort_attrs.begin(); it != sort_attrs.end(); ++it){
      //compare lhs[sort_attr] < rhs[sort_attr] using cmp_fns
      bool cmp = (*cmp_fns[*it])(lhs[*it], rhs[*it]);

      if (cmp != 0){
        return cmp;
      }
    return false; //lhs == rhs
    }
  }
};
