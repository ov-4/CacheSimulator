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
typedef std::pair<x64,x64> Pair;

inline x64 timeNow = 0;
inline x64 b = 6;   // normal CPU would have same cache line size across L1 L2 L3, so anyway it's glogal
                    // 2^6 = 64, intel default value

inline bool DEBUG = false;

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

    inline bool getWritten() { return written; }

    inline void write()
    {
        written = true;
        setTime();
    }
    
    inline void read()
    {
        setTime();
    }
};

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

    inline void addr2TagSet(x64 addr, x64 &Tag, x64 &Set)
    {
        Tag = addr >> (s+b);
        Set = ((addr << (64-s-b)) >> (64-s-b))>>b;
    }

    inline x64 tagSet2Addr(x64 Tag, x64 Set)
    {
        return ((Tag << s) | Set) << b;
    }

    inline std::vector<Cache>::iterator findTag(x64 Tag, x64 Set)
    {
        return std::find_if(cache[Set].begin(), cache[Set].end(),
                            [&](const Cache &c)
                            {
                                return c.valid && c.tag == Tag;
                            });
    }

    inline void write(x64 addr, bool increment = 1)
    {
        timeNow++;

        x64 Tag, Set;
        addr2TagSet(addr, Tag, Set);
        auto findTarget = findTag(Tag, Set);

        // if there is no corresponding cache
        if (findTarget == cache[Set].end())
        {
            // when write-back, `i+1` level is always a subset of `i`, so never miss
            missCnt++;
            allocate(Tag, Set);
            findTarget = findTag(Tag, Set);
        } else {
            hitCnt += increment;
        }

        findTarget -> write();
        writeCnt += increment;

        if (lower != NULL)
        {
            x64 lowerTag, lowerSet;
            lower -> addr2TagSet(addr, lowerTag, lowerSet);
            auto found = lower -> findTag(lowerTag, lowerSet);
            if (found != lower -> cache.at(lowerSet).end()) // we've loaded the cache block to this level, so normally it's ok
                                                            // just in case
            {
                found -> setTime();
            }
        }
    }

    inline void read(x64 addr)
    {
        timeNow++;

        x64 Tag, Set;
        addr2TagSet(addr, Tag, Set);
        auto findTarget = findTag(Tag, Set);

        // if there is no corresponding cache
        if (findTarget == cache[Set].end())
        {
            missCnt++;
            allocate(Tag, Set);
            findTarget = findTag(Tag, Set);
        } else {
            hitCnt++;
        }

        findTarget -> read();

        if (lower != NULL)
        {
            x64 lowerTag, lowerSet;
            lower -> addr2TagSet(addr, lowerTag, lowerSet);
            auto found = lower -> findTag(lowerTag, lowerSet);
            if (found != lower -> cache.at(lowerSet).end()) // we've loaded the cache block to this level, so normally it's ok
                                                            // just in case
            {
                found -> setTime();
            }
        }
    }

    inline void evi(x64 addr)
    {
        x64 Tag, Set;
        addr2TagSet(addr, Tag, Set);

        // find the corresponding block to remove
        // same level
        auto findTarget = findTag(Tag, Set);
        if (findTarget == cache[Set].end()) return;

        eviCnt++;
        
        if (upper != NULL)
            upper -> evi(addr);

        findTarget -> setValid(false);

        if (lower != NULL && findTarget -> written)
        {
            // write-back, so doesn't count into write
            lower -> write(addr, 0);
        }
    }

    void allocate(x64 Tag, x64 Set)
    {
        // when we call allocate(), it's always a miss
        // get data from lower level
        if (lower != NULL)
        {
            lower -> read(this -> tagSet2Addr(Tag, Set));
        }

        // if we can find actual free slot, in current level
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
            findFree->written = false;
        } else {
            // no slot is actually free, we have to find the oldest
            auto findOld = min_element(cache[Set].begin(), cache[Set].end(),
                                       [](const Cache &x, const Cache &y)
                                       {
                                           return x.time < y.time;
                                       });

            // remove old one first, and add new one
            evi(tagSet2Addr(findOld->tag, Set));

            findOld->valid = true;
            findOld->time = timeNow;
            findOld->tag = Tag;
            findOld->written = false;
        }
    }
};


    
}
// namespace ov4
