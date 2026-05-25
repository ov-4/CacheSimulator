// IGNORE THIS ARCH
// I WILL MODIFY THIS STUFF SOON

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <utility>
#include <memory>
#include <iterator>

#define LOG !(ov4::GLOBAL_DEBUG && ov4::DEBUG) ? (void)0 : ov4::LogVoidify() & std::clog

namespace ov4 {

class cacheLayer;
class Cache;

typedef unsigned long long ull;
typedef signed long long ll;
typedef std::size_t x64;
typedef std::pair<x64,x64> Pair;

inline x64 timeNow = 0;
inline x64 b = 6;   // normal CPU would have same cache line size across L1 L2 L3, so anyway it's global
                    // 2^6 = 64, intel default value

inline const x64 ARCH = 64;

inline bool DEBUG = false;
inline constexpr bool GLOBAL_DEBUG = true; // change this to false for performance 

struct LogVoidify { void operator&(std::ostream&) const {} };

class Cache
{
friend class cacheLayer;

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

    inline void write(bool updateTime = true)
    {
        written = true;
        if (updateTime) setTime();
    }
    
    inline void read()
    {
        setTime();
    }
};


class cacheLayer
{
friend class Cache;

public:
    ll s = 0, E = 0;
    ll hitCnt = 0;
    ll missCnt = 0;
    ll eviCnt = 0;
    ll partialCnt = 0; // paritial hit+miss
    ll writeCnt = 0;

    // feel free to dereference this
    cacheLayer *upper = nullptr;
    cacheLayer *lower = nullptr;

    std::vector<std::vector<Cache>> cache;

    inline void setSize(x64 x, x64 y) { s = x; E = y; cache.resize(((x64)1<<s), std::vector<Cache>(E)); }

    inline void setUpper(cacheLayer *p) { upper = p; }

    inline void setLower(cacheLayer *p) { lower = p; }

    inline cacheLayer* getUpper() { return upper; }

    inline cacheLayer* getLower() { return lower; }

    cacheLayer(x64 s, x64 E, cacheLayer *up, cacheLayer *lo) { setSize(s, E); setUpper(up); setLower(lo); }
    cacheLayer() = default;

    inline void addr2TagSet(x64 addr, x64 &Tag, x64 &Set)
    {
        Tag = addr >> (s+b);
        Set = ((addr << (ARCH-s-b)) >> (ARCH-s-b))>>b;
    }

    inline x64 tagSet2Addr(x64 Tag, x64 Set)
    {
        return ((Tag << s) | Set) << b;
    }

    inline Cache* findTag(x64 Tag, x64 Set)
    {
        auto ret = std::find_if(cache.at(Set).begin(), cache.at(Set).end(),
                                [&](const Cache &c)
                                {
                                    return c.valid && c.tag == Tag;
                                });
        return (ret != cache.at(Set).end()) ? (std::to_address(ret)) : (nullptr);
    }

    // simulate writing
    inline void write(x64 addr, bool increment = 1)
    {
        timeNow++;
        LOG << "time: " << timeNow << " addr 0x" << std::hex << addr << std::endl;

        x64 Tag, Set;
        addr2TagSet(addr, Tag, Set);
        auto findTarget = findTag(Tag, Set);

        // if there is no corresponding cache
        if (findTarget == nullptr)
        {
            // when write-back, `i+1` level is always a subset of `i`, so never miss
            missCnt++;
            LOG << "write: miss: " << missCnt << " addr 0x" << std::hex << addr << std::endl;
            allocate(Tag, Set);
            findTarget = findTag(Tag, Set);
        } else {
            hitCnt += increment;
            LOG << "write: hit: " << hitCnt << " increment: " << increment << " addr 0x" << std::hex << addr << std::endl;
        }

        writeCnt += increment;
        LOG << "write: write: " << writeCnt << " increment: " << increment << " addr 0x" << std::hex << addr << std::endl;
        findTarget -> write(increment);

        if (lower != nullptr)
        {
            x64 lowerTag, lowerSet;
            lower -> addr2TagSet(addr, lowerTag, lowerSet);
            auto found = lower -> findTag(lowerTag, lowerSet);
            if (found != nullptr) // we've loaded the cache block to this level, so normally it's ok
                                                            // just in case
            {
                if (increment) 
                { 
                    LOG << "write: setTime() for lower level" << " addr 0x" << std::hex << addr << std::endl;
                    found -> setTime();
                }
            }
        }
    }

    // simulate reading
    inline void read(x64 addr)
    {
        timeNow++;
        LOG << "time: " << timeNow << " addr 0x" << std::hex << addr << std::endl;

        x64 Tag, Set;
        addr2TagSet(addr, Tag, Set);
        auto findTarget = findTag(Tag, Set);

        // if there is no corresponding cache
        if (findTarget == nullptr)
        {
            missCnt++;
            LOG << "read: miss: " << missCnt << " addr 0x" << std::hex << addr << std::endl;
            allocate(Tag, Set);
            findTarget = findTag(Tag, Set);
        } else {
            hitCnt++;
            LOG << "read: hit: " << hitCnt << " addr 0x" << std::hex << addr << std::endl;
        }

        findTarget -> read();

        if (lower != nullptr)
        {
            x64 lowerTag, lowerSet;
            lower -> addr2TagSet(addr, lowerTag, lowerSet);
            auto found = lower -> findTag(lowerTag, lowerSet);
            if (found != nullptr) // we've loaded the cache block to this level, so normally it's ok
                                                            // just in case
            {
                LOG << "read: setTime() for lower level" << " addr 0x" << std::hex << addr << std::endl;
                found -> setTime();
            }
        }
    }

    // evict and write back 
    inline void evi(x64 addr)
    {
        x64 Tag, Set;
        addr2TagSet(addr, Tag, Set);

        // find the corresponding block to remove
        // same level
        auto findTarget = findTag(Tag, Set);
        if (findTarget == nullptr) return;

        eviCnt++;
        LOG << "evi: evication: " << eviCnt << " addr 0x" << std::hex << addr << std::endl;
        
        if (upper != nullptr)
        {
            LOG << "evi: upper level evict addr 0x" << std::hex << addr << std::endl;
            upper -> evi(addr);
        }

        findTarget -> setValid(false);

        if (lower != nullptr && findTarget -> written)
        {
            // write-back, so doesn't count into write
            LOG << "evi: lower level write back addr 0x" << std::hex << addr << std::endl;
            lower -> write(addr, 0);
        }
    }

    // allocate new block
    void allocate(x64 Tag, x64 Set)
    {
        // when we call allocate(), it's always a miss
        // get data from lower level
        if (lower != nullptr)
        {
            LOG << "allocate: reading lower level tag: 0x" << std::hex << Tag << " set: 0x" << std::hex << Set << std::endl;
            lower -> read(this -> tagSet2Addr(Tag, Set));
        }

        // if we can find actual free slot, in current level
        auto findFree = std::find_if(cache.at(Set).begin(), cache.at(Set).end(),
                                [&](const Cache &c)
                                {
                                    return !c.valid;
                                });
        if (findFree != cache.at(Set).end())
        {
            LOG << "allocate: found free block tag: 0x" << std::hex << Tag << " set: 0x" << std::hex << Set << std::endl;
            findFree->valid = true;
            findFree->time = timeNow;
            findFree->tag = Tag;
            findFree->written = false;
        } else {
            // no slot is actually free, we have to find the oldest
            auto findOld = min_element(cache.at(Set).begin(), cache.at(Set).end(),
                                       [](const Cache &x, const Cache &y)
                                       {
                                           return x.time < y.time;
                                       });
            
            LOG << "allocate: evicting block, tag: 0x" << std::hex << Tag << " set: 0x" << std::hex << Set << std::endl;

            // remove old one first
            evi(tagSet2Addr(findOld->tag, Set));

            // add new one
            findOld->valid = true;
            findOld->time = timeNow;
            findOld->tag = Tag;
            findOld->written = false;
        }
    }
};


    
}
// namespace ov4
