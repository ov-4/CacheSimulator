// IGNORE THIS ARCH
// I WILL MODIFY THIS STUFF SOON

#pragma once
#include <string>
#include <vector>

namespace ov4 {

class cacheLayer;
class Cache;

typedef unsigned long long ull;
typedef signed long long ll;
typedef std::size_t x64;

inline x64 timeNow;
inline x64 byte; // normal CPU would have same cache line size across L1 L2 L3, so anyway it's glogal

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
