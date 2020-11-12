#include "Simualator.h"

void Model::print_codevec(){
  for (Ins& ins : code_vec){
      ins.print();
  }
}

void Model::extract(const char* filename){
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

bool Model::isCacheHit(const string& blk, Word& buf){
    if (cache.find(blk) != cache.end()) {
      buf = cache[blk];
      return true;
    }
    else return false;
}

void Model::setCacheWord(const string& blk, const Word& buf){
  if (cache.find(blk) != cache.end()) {
    cache[blk] = buf;
  }
  else {
    cache.emplace(blk, buf);
  }
}

Ins Model::decode(const vector<string>& vec){
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

vector<string> Model::split(const string& str){
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
