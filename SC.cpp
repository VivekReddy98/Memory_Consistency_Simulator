
/*
Type: Sequential Consistency
*/

#include "Simulator.h"
#include <stack>
#include <math.h>

pair<int, int> SC::simulate(){
    int counter = 0;
    Word cacheWord;
    Word buf;

    stack<pair<string, int>> stkCS;

    int cyclesCS = 0;
    int numofCS = 0;

    for (Ins& ins : code_vec){

        if (ins.code == LCK){
            stkCS.emplace(ins.blk, 0);
        }

        bool cacheHit = isCacheHit(ins.blk, cacheWord);
        buf.fetch = counter;
        int retirelast = latestRetireTime();

        if (cacheHit){ // Cache Hit
            buf.issue = max(retirelast, cacheWord.retire);
            buf.retire = buf.issue + HIT_LAT;
        }
        else{
            buf.issue = retirelast > 0 ? retirelast : buf.fetch;
            buf.retire = buf.issue + MISS_LAT;
        }

        setCacheWord(ins.blk, buf);

        if (rQueue.find(buf.retire) == rQueue.end()){
           rQueue[buf.retire] = list<string> {};
        }

        rQueue[buf.retire].push_back(ins.blk);
        ++counter;

        if (!stkCS.empty()){
            stkCS.top().second += buf.retire - buf.issue;
        }

        // Record Data at UnLock
        if(ins.code == UNLCK){
            numofCS++;
            cyclesCS += stkCS.top().second;
            if (stkCS.size() > 1){
                int temp = stkCS.top().second;
                stkCS.pop();
                stkCS.top().second += temp;
            }
            else{
                stkCS.pop();
            }
        }

    }

    auto avgCycles = numofCS > 0 ? cyclesCS / numofCS : 0;

    return {latestRetireTime(), (int)round(avgCycles)};
}
