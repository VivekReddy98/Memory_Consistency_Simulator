/*
Type: Weak Ordering
*/

#include "Simulator.h"
#include <stack>
#include <math.h>



pair<int, int> WO::simulate(){
  int counter = 0;
  Word cacheWord;
  Word buf;

  Ins* prev = NULL;
  int numofCS = 0;
  int cyclesCS = 0;

  stack<pair<int, int>> stkCS; // (Start End)

  int boundaryCS = -1;

  for (Ins& ins : code_vec){

    if (ins.code == LCK){
        stkCS.emplace(INT32_MAX, INT32_MIN);
        boundaryCS = latestRetireTime();
    }
    else if(ins.code == UNLCK){
        boundaryCS = latestRetireTime();
    }

    bool cacheHit = isCacheHit(ins.blk, cacheWord);
    buf.fetch = counter;

    buf.issue = max(boundaryCS, buf.fetch);

    // // Set the Issue Cycle based on the precense inside a critical section
    // if (ins.code == LCK){
    //
    // }
    // else if (!stkCS.empty() || (prev != NULL && prev->code == UNLCK)){  //ins.code == LCK || ){
    //     buf.issue = max(boundaryCS, buf.fetch);
    // }
    // else {
    //     buf.issue = buf.fetch;
    // }

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


    if (rQueue.find(buf.retire) == rQueue.end()){
       rQueue[buf.retire] = list<string> {};
    }

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
        boundaryCS = buf.retire;
    }

    cout << "is Cache hit? : " <<  cacheHit << " for " << ins.blk << " ";
    buf.print();

    rQueue[buf.retire].push_back(ins.blk);
    ++counter;

    prev = &ins;
  }

  auto avgCycles = numofCS > 0 ? cyclesCS / numofCS : 0;

  return {latestRetireTime(), (int)round(avgCycles)};
}
