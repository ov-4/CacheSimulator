// IGNORE THIS ARCH
// I WILL MODIFY THIS STUFF SOON

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <utility>

namespace ov4 {

class cacheLayer;
class Cache;

typedef unsigned long long ull;
typedef signed long long ll;
typedef std::size_t x64;

inline x64 timeNow = 0;
inline x64 byte = 8; // normal CPU would have same cache line size across L1 L2 L3, so anyway it's glogal

inline bool DEBUG = false;

class cacheLayer
{
public:
    ll s = 0, E = 0;
    ll hitCnt = 0;
    ll missCnt = 0;
    ll eviCnt = 0;
    ll partialCnt = 0; // paritial hit+miss
    ll writeCnt = 0;

    // feel free to dereference this
    cacheLayer *upper = NULL;
    cacheLayer *lower = NULL;

    std::vector<std::vector<Cache>> cache;

    inline void setSize(x64 x, x64 y) { s = x; E = y; cache.resize((1<<s), std::vector<Cache>(E)); }

    inline void setPrev(cacheLayer *p, cacheLayer *q) { upper = p; lower = q; }

    inline cacheLayer* getUpper() { return upper; }

    inline cacheLayer* getLower() { return lower; }

    cacheLayer(x64 s, x64 E, cacheLayer *p, cacheLayer *q) { setSize(s, E); setPrev(p, q); }

    inline std::pair<x64,x64> addr2TagSet(x64 addr)
    {
        x64 Tag, Set;
        Tag = addr >> (s+byte);
        Set = ((addr << (64-s-byte)) >> (64-s-byte))>>byte;
        return std::make_pair(Tag, Set);
    }

    inline x64 tagSet2Addr(x64 Tag, x64 Set)
    {
        return ((Tag << s) | Set) << byte;
    }

    inline void evi(x64 addr)
    {
        std::pair<x64,x64> tmp = addr2TagSet(addr);
        x64 Tag = tmp.first, Set = tmp.second;

        // find the corresponding block to remove
        // same level
        auto findTarget = std::find_if(cache[Set].begin(), cache[Set].end(),
                                       [&](const Cache &c)
                                       {
                                           return c.valid && c.tag == Tag;
                                       });
        if (findTarget == cache[Set].end()) return;

        eviCnt++;
        
        if (upper != NULL)
            upper -> evi(addr);

        findTarget -> setValid(false);

        if (lower != NULL && findTarget -> written)
        {
            lower -> writeCnt ++;
        }
    }

    void findFreeSlot(x64 Tag, x64 Set)
    {
        // if we can find actual free slot
        auto findFree = std::find_if(cache[Set].begin(), cache[Set].end(),
                                [&](const Cache &c)
                                {
                                    return !c.valid;
                                });
        if (findFree != cache[Set].end())
        {
            findFree->valid = true;
            findFree->time = timeNow;
            findFree->tag = Tag;
            return;
        }

        // no slot is actually free, we have to find the oldest
        auto findOld = min_element(cache[Set].begin(), cache[Set].end(),
                                   [](const Cache &x, const Cache &y)
                                   {
                                       return x.time < y.time;
                                   });

        // remove old one first, and add new one
        evi(tagSet2Addr(findOld -> tag, Set));
        
        findOld->valid = true;
        findOld->time = timeNow;
        findOld->tag = Tag;
        
    }
};

class Cache
{
public:
    bool valid = false;
    bool written = false;
    x64 tag = 0;
    x64 time = 0;


    inline void setTime() { time = timeNow; }

    inline x64 getTime() { return time; }

    inline void setTag(x64 x) { tag = x; }

    inline x64 getTag() { return tag; }

    inline void setValid(bool x) { valid = x; }

    inline bool getValid() { return valid; }

    inline void setWritten(bool x) { written = x; }

    inline bool getWritten() { return written; }
};


    
}
