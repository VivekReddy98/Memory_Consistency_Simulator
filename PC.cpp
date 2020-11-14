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
     return -1;
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
        }
    }

    // Find if the current blk is a L1 cache hit
    bool cacheHit = isCacheHit(ins.blk, cacheWord);

    // if Not a hit in L1 Cache, search in Store Buffer
    if (!cacheHit) {
       cacheHit = STBuf.get(ins.blk, cacheWord);
    }

    // Fetch the Instruction at time = counter
    buf.fetch = counter;

    // Calculate the Issue Time
    if (ins.code == LOAD){ // Since Store -> Load is Relaxed in Processor consistency
        buf.issue = latestRetireTime(rlQueue);
    }
    else {
        buf.issue = latestRetireTime();
    }

    // Calculate the Retire Time
    if (cacheHit){ // Cache Hit
        buf.issue = max(buf.issue, cacheWord.retire);
        buf.retire = buf.issue + HIT_LAT;
    }
    else{
        buf.issue = buf.issue > 0 ? buf.issue : buf.fetch;
        buf.retire = buf.issue + MISS_LAT;
    }

    updateStack(stkCS, buf.issue, buf.retire);

    // Update Cache (L1 & Store buf)
    if (ins.code == STORE) { // Store's dont go into L1 Cache directly, instead go into STORE Buffer
        Word temp_word;
        string evicted_blk;
        bool didEvict = STBuf.put(ins.blk, buf, evicted_blk, temp_word);
        if (didEvict){
           auto temp_buf = evictFromSTbuf(evicted_blk, temp_word, cacheHit, latestRetireTime());
           updateStack(stkCS, temp_buf.issue, temp_buf.retire);
        }
    }
    else {
        setCacheWord(ins.blk, buf);
    }

    // Update Retire Queues
    if (ins.code != STORE) { // Maintaining different Queues for Store and Load.
      if (rlQueue.find(buf.retire) == rlQueue.end()){
         rlQueue[buf.retire] = list<string> {};
      }
      rlQueue[buf.retire].push_back(ins.blk);
    }
    else {
      if (rsQueue.find(buf.retire) == rsQueue.end()){
         rsQueue[buf.retire] = list<string> {};
      }
      rsQueue[buf.retire].push_back(ins.blk);
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

    ++counter;
  }

  auto avgCycles = numofCS > 0 ? cyclesCS / numofCS : 0;
  printf("The avg RC critical section latency is : %d which is derived from %d / %d\n", (int)round(avgCycles), cyclesCS, numofCS);
  return {latestRetireTime(), (int)round(avgCycles)};
}
