#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <list>
#include <string>

using namespace std;

// Constants
constexpr int HIT_LAT = 10;
constexpr int MISS_LAT = 100;
constexpr int STORE_BUF_LAT = 1;
constexpr int STORE_BUF_SIZE = 16;

// Possible Code Words
typedef enum {LOAD, STORE, LCK, UNLCK} Code;
extern vector<string> Codes;

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
  void print() {cout << "{ " << fetch << ", " << issue << ", " << retire << "}" << endl;}
}Word;

// Base Class Representing a Memory Model
class Model{

protected:
    unordered_map<string, Word> cache;
    map<int, list<string>> rQueue; // Retire Queue
    vector<Ins> code_vec;
    int latestRetireTime();
    bool isCacheHit(const string& blk, Word& buf); // If Cache Hit, it updates the buffer
    void setCacheWord(const string& blk, const Word& buf);

public:
    virtual pair<int, int> simulate() = 0;
    void print_codevec();
    void extract(const char* filename);

private:
    vector<string> split(const string& str);
    Ins decode(const vector<string>& vec);
};

class SC : public Model
{
public:
    pair<int, int> simulate();
};


class PC : public Model
{
public:
    pair<int, int> simulate();
};

class WO : public Model
{
public:
    pair<int, int> simulate();
};

class RC : public Model
{
public:
    pair<int, int> simulate();
};
