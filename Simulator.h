#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <string>

using namespace std;

// Hit & Miss Latencies
constexpr int HIT_LAT = 10;
constexpr int MISS_LAT = 100;

// Possible Code Words
typedef enum {LOAD, STORE, LCK, UNLCK} Code;
vector<string> Codes = {"Load ", "Store ", "Lock", "Unlock"};

// Struct representing an Instruction
typedef struct _Ins{
  Code code;
  string blk;
  _Ins() {}
  _Ins(Code _code, string _blk) : code(_code), blk(_blk) {}
  void print() {cout << Codes[code] << blk << endl;}
}Ins;

typedef struct _Word{
    int fetch = -1;
    int issue = -1;
    int retire = -1;
    _Word() {}
}Word;

// Base Class Representing a Memory Model
class Model{

public:
  unordered_map<string, Word> cache;
  vector<Ins> code_vec;

  void print_codevec();
  void extract(const char* filename);
  bool isCacheHit(const string& blk, Word& buf);
  void setCacheWord(const string& blk, const Word& buf);
  virtual pair<int, int> simulate();

private:
  vector<string> split(const string& str);
  Ins decode(const vector<string>& vec);
};

class SC : public Model
{
    pair<int, int> simulate();
};

class PC : public Model
{
    pair<int, int> simulate();
};

class WO : public Model
{
    pair<int, int> simulate();
};

class RC : public Model
{
    pair<int, int> simulate();
};
