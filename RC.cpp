/*
Type: Release Consistency
*/

#include "Simulator.h"
#include <stack>
#include <math.h>

pair<int, int> RC::simulate(){
  int counter = 0;
  Word cacheWord;
  Word buf;

  int numofCS = 0;
  int cyclesCS = 0;

  stack<pair<int, int>> stkCS; // (Start End)

  int boundaryCS = -1;

  for (Ins& ins : code_vec){

    if (ins.code == LCK) {stkCS.emplace(INT32_MAX, INT32_MIN);}

    bool cacheHit = isCacheHit(ins.blk, cacheWord);

    buf.fetch = counter;
    buf.issue = max(boundaryCS, buf.fetch);

    // Unlock should be invoked only after everything before it is completed
    if (ins.code == UNLCK) {buf.issue = max(latestRetireTime(), buf.issue);}

    if (cacheHit){ // Cache Hit
        buf.issue = max(buf.issue, cacheWord.retire);
        buf.retire = buf.issue + HIT_LAT;
    }
    else{
        buf.retire = buf.issue + MISS_LAT;
    }

    if (ins.code == LCK){
        boundaryCS = buf.retire;
    }

    setCacheWord(ins.blk, buf);

    // Update Retire Queue
    if (rQueue.find(buf.retire) == rQueue.end()){
       rQueue[buf.retire] = list<string> {};
    }
    rQueue[buf.retire].push_back(ins.blk);

    // Update
    if (!stkCS.empty()){
        stkCS.top().first = min(stkCS.top().first, buf.issue);
        stkCS.top().second = max(stkCS.top().second, buf.retire);
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

  // printf("The avg RC critical section latency is : %d which is derived from %d / %d\n", (int)round(avgCycles), cyclesCS, numofCS);

  return {latestRetireTime(), (int)round(avgCycles)};
}
