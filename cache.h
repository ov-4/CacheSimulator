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

x64 timeNow;

class cacheLayer
{
public:
    ll hitCnt;
    ll missCnt;
    ll eviCnt;
    ll partialCnt; // paritial hit+miss
    ll writeCnt;

    std::vector<std::vector<Cache>> cache;

    inline void setSize(x64 s, x64 E) { cache.resize((1<<s), std::vector<Cache>(E)); }

    cacheLayer(x64 s, x64 E) { setSize(s, E); }
};

class Cache
{
public:
    bool valid = false;
    bool written = false;
    x64 tag = 0;
    x64 time = 0;


    inline void setTime(x64 x) { time = x; }

    inline x64 getTime() { return time; }

    inline void setTag(x64 x) { tag = x; }

    inline x64 getTag() { return tag; }

    inline void setValid(bool x) { valid = x; }

    inline bool getValid() { return valid; }

    inline void setWritten(bool x) { written = x; }

    inline bool getWritten() { return written; }
};


    
}
