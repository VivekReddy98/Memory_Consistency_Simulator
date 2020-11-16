#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <iterator>
#include <list>
#include <string>
#include <stack>
#include <queue>

using namespace std;

// Constants
constexpr int HIT_LAT = 10;
constexpr int MISS_LAT = 100;
constexpr int STORE_BUF_LAT = 1;
constexpr size_t STORE_BUF_SIZE = 16;

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



class PC : public Model
{
  class STBuffer {
        unordered_map<string, Word> mp;
        unordered_map<string, list<string>::iterator> mpL;
        list<string> LL;
        size_t limit;

      public:
        STBuffer(const size_t& capacity) : limit(capacity) {}

        bool get(const string& key, Word& evicted_word){
          if (mp.find(key) != mp.end()){
            evicted_word = mp[key];
            return true;
          }
          else return false;
        }

        bool put(const string& key, const Word& buf, string& evicted_blk, Word& evicted_word) {

            // If the value is already in the Store Buffer, dont evict anything
            if (mp.find(key) != mp.end()){
               mp[key] = buf;
               return false;
            }

            mp[key] = buf;
            LL.push_front(key);
            mpL[key] = LL.begin();

            // Only Evict if the size has reached the limit
            if (mp.size() > limit){
                evicted_blk = LL.back();
                evicted_word = mp[evicted_blk];

                LL.pop_back();
                mp.erase(evicted_blk);
                mpL.erase(evicted_blk);
                return true;
            }

            return false;
        }

        vector<pair<string, Word>> evict_all(){
            vector<pair<string, Word>> ans;
            for (auto itr = mp.begin(); itr != mp.end(); itr++){
                ans.push_back(*itr);
            }
            mp.clear();
            mpL.clear();
            LL.clear();
            return ans;
        }
    };

private:
    map<int, list<string>> rlQueue; // Retire Load Queue (LCK also falls under this queue)
    map<int, list<string>> rsQueue; // Retire Store Queue
    map<int, list<string>> stBufQueue; // Queue for Store Buffer Evictions

    void updateStack(stack<pair<int, int>>& stk, int issue, int retire);

    STBuffer STBuf = {STORE_BUF_SIZE};
    int latestRetireTime(const map<int, list<string>>& Q);
    int latestRetireTime();

    void addtoQ(map<int, list<string>>& Q, const Word& temp_buf, const string& blk);

    Word evictFromSTbuf(const string& blk, const Word& ST_Word, bool L1Hit, int tempBoundary);

public:
    pair<int, int> simulate();
};
