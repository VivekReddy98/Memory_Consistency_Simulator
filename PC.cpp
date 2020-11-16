/*
Type: Processor Consistency
*/

#include "Simulator.h"
#include <stack>
#include <math.h>
#include <algorithm>

int PC::latestRetireTime(const map<int, list<string>>& Q){
  auto itr = Q.rbegin();
  if (itr != Q.rend()){
     return itr->first;
  }
  else{
     return 0;
  }
}

int PC::latestRetireTime(){
  return max(latestRetireTime(rlQueue),latestRetireTime(rsQueue));
}

// Keep Track of the issue and retire time (if in critical Section )
void PC::updateStack(stack<pair<int, int>>& stkCS, int issue, int retire){
  if (!stkCS.empty()){
      stkCS.top().first = min(stkCS.top().first, issue);
      stkCS.top().second = max(stkCS.top().second, retire);
  }
}

Word PC::evictFromSTbuf(const string& blk, const Word& ST_Word, bool L1Hit, int temp_boundary){
  Word buf;
  buf.fetch = ST_Word.fetch;
  buf.issue = temp_boundary;

  if (!L1Hit){ // Cache Miss in L1 Cache
    buf.retire = temp_boundary + MISS_LAT;
  }
  else{
    buf.retire = temp_boundary + HIT_LAT;
  }

  setCacheWord(blk, buf);
  return buf;
}

void PC::addtoQ(map<int, list<string>>& Q, const Word& temp_buf, const string& blk){
  if (Q.find(temp_buf.retire) == Q.end()){
     Q[temp_buf.retire] = list<string> {};
  }
  Q[temp_buf.retire].push_back(blk);
}

/*
Simulate Processor Consistency
*/
pair<int, int> PC::simulate(){
  int counter = 0;
  Word cacheWord;
  Word buf;

  int numofCS = 0;
  int cyclesCS = 0;
  stack<pair<int, int>> stkCS; // (Start End)

  for (Ins& ins : code_vec){

    if(ins.code == LCK) {
      stkCS.emplace(INT32_MAX, INT32_MIN);
    }

    // Completely Evict Store Buffer
    if (ins.code == UNLCK) {
        auto list_blks = STBuf.evict_all();
        Word temp_buf;
        int temp_boundary = latestRetireTime();
        for (pair<string, Word> c_blk : list_blks){
            bool cacheHit = isCacheHit(c_blk.first, cacheWord);
            auto temp_buf = evictFromSTbuf(c_blk.first, c_blk.second, cacheHit, temp_boundary);
            updateStack(stkCS, temp_buf.issue, temp_buf.retire);
            addtoQ(stBufQueue, temp_buf, c_blk.first);
        }
    }

    // Find if the current blk is a L1 cache hit
    bool cacheHit = isCacheHit(ins.blk, cacheWord);

    // if Not a hit in L1 Cache, search in Store Buffer
    if (!cacheHit && ins.code == STORE) {
       cacheHit = STBuf.get(ins.blk, cacheWord);
    }

    // Fetch the Instruction at time = counter
    buf.fetch = counter;

    // Calculate the Issue Time
    if (ins.code == LOAD){ // Since Store -> Load is Relaxed in Processor consistency
        buf.issue = latestRetireTime(rlQueue);
    }
    else if(ins.code == UNLCK){
        buf.issue = max(latestRetireTime(), latestRetireTime(stBufQueue));
    }
    else {
        buf.issue = latestRetireTime();
    }

    // Calculate the Retire Time
    if (cacheHit){ // Cache Hit
        buf.issue = max(buf.issue, cacheWord.retire);
        if (ins.code != STORE){
          buf.retire = buf.issue + HIT_LAT;
        }
        else buf.retire = buf.issue + STORE_BUF_LAT;
    }
    else {
        buf.issue = buf.issue >= 0 ? buf.issue : buf.fetch;
        if (ins.code != STORE){
            buf.retire = buf.issue + MISS_LAT;
        }
        else buf.retire = buf.issue + STORE_BUF_LAT;
    }

    updateStack(stkCS, buf.issue, buf.retire);

    // Update Cache (L1 & Store buf)
    if (ins.code == STORE) { // Store's dont go into L1 Cache directly, instead go into STORE Buffer
        Word temp_word;
        Word temp_buf;
        string evicted_blk;
        bool didEvict = STBuf.put(ins.blk, buf, evicted_blk, temp_word);
        if (didEvict){
           temp_buf = evictFromSTbuf(evicted_blk, temp_word, isCacheHit(evicted_blk, temp_buf), latestRetireTime());
           updateStack(stkCS, temp_buf.issue, temp_buf.retire);
           addtoQ(stBufQueue, temp_buf, evicted_blk);
        }
    }
    else {
        setCacheWord(ins.blk, buf);
    }

    // Update Retire Queues
    if (ins.code != STORE && ins.code != UNLCK) { // Maintaining different Queues for Store and Load
      addtoQ(rlQueue, buf, ins.blk);
    }
    else {
      addtoQ(rsQueue, buf, ins.blk);
    }


    // Record Data at UnLock
    if(ins.code == UNLCK){
        numofCS++;
        cyclesCS += stkCS.top().second - stkCS.top().first;
        if (stkCS.size() > 1){
            auto temp = stkCS.top();
            stkCS.pop();
            stkCS.top().first = min(stkCS.top().first, temp.first);
            stkCS.top().second = max(stkCS.top().second, temp.second);
        }
        else{
            stkCS.pop();
        }
    }

    // cout << "is Cache hit? : " <<  cacheHit << " for " << ins.blk << " ";
    // buf.print();

    ++counter;


  }

  auto avgCycles = numofCS > 0 ? cyclesCS / numofCS : 0;
  // printf("The avg PC critical section latency is : %d which is derived from %d / %d\n", (int)round(avgCycles), cyclesCS, numofCS);
  return {latestRetireTime(), (int)round(avgCycles)};
}
