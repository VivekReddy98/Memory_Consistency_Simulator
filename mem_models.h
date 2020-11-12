#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <string>

using namespace std;

// Hit & Miss Latencies
constexpr int HIT_LAT = 10;
constexpr int MISS_LAT = 100;

// Possible Code Words
typedef enum {LOAD, STORE, LCK, UNLCK} Code;
vector<string> Codes = {"Load ", "Store ", "Lock", "unlock"};

// Struct representing an Instruction
typedef struct _Ins{
  Code code;
  string blk;
  _Ins() {}
  _Ins(Code _code, string _blk) : code(_code), blk(_blk) {}
  void print() {
    cout << Codes[code] << blk << endl;
  }
}Ins;

// Base Class Representing a Memory Model
class Model{
public:
  unordered_set<string> cache;
  vector<Ins> code_vec;

  void print_codevec(){
    for (Ins& ins : code_vec){
        ins.print();
    }
  }
  void extract(const char* filename){
    ifstream inp;
    cache.clear();
    code_vec.clear();

    try{
      inp.open(filename);
      if (!inp.good()) throw std::invalid_argument("File not Found");
      string str;
      while (std::getline(inp, str)) {
          auto vec = split(str);
          code_vec.push_back(decode(vec));
      }
      inp.close();
    }
    catch(const std::exception& e){
      if (inp.is_open()) inp.close();
      cout << e.what() << endl;
      throw e;
    }
  }

private:
  vector<string> split(const string& str){
      stringstream ss(str);
      string temp;
      vector<string> ans;
      while(getline(ss, temp, ' ')){
          if (!temp.empty()){
             ans.push_back(temp);
          }
      }
      return ans;
  }
  Ins decode(const vector<string>& vec){
      Ins ins;
      if (vec.size() == 2){
        if (!vec[0].compare("Load")){
            ins.code = LOAD;
            ins.blk = vec[1];
        }else if (!vec[0].compare("Store")){
            ins.code = STORE;
            ins.blk = vec[1];
        }else{
            cout << "Invalid Code Word: " << vec[0] << endl;
            throw std::domain_error("Invalid Input File Formatting");
        }
      }
      else if (vec.size() == 1){
         if (vec[0].find("Lock") != string::npos){
            ins.code = LCK;
            ins.blk = vec[0].substr(4, vec[0].size()-4);
         }else if (vec[0].find("Unlock") != string::npos){
            ins.code = UNLCK;
            ins.blk = vec[0].substr(6, vec[0].size()-6);
         }
      }
      else {
          throw std::domain_error("Invalid Input File Formatting");
      }
      return ins;
  }
};

class SC : public Model
{

};

class PC : public Model
{

};

class WO : public Model
{

};

class RC : public Model
{

};
