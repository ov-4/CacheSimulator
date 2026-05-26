#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <memory>
#include <iterator>
#include "cache.h"

extern "C" {
    #include <getopt.h>
    #include <stdlib.h>
    #include <unistd.h>
}

using namespace std;

int main(int argc, char *argv[])
{
    string traceFile = "", cacheFile = "", line = "";
    size_t num = 0, i = 0, b = 6;
    vector<ov4::cacheLayer> ca;

    // get arguments
    int opt = '?';
    while ((opt = getopt(argc, argv, "f:t:v")) != -1) 
    {
        switch (opt) 
        {
            case 'f':
                cacheFile = optarg;
                break;
            case 't':
                traceFile = optarg;
                break;
            case 'v':
                ov4::DEBUG = true;
                break;
            case '?':
                cerr << "Unknow arg" << endl;
                break;
            default:
                break;
        }
    }
    ifstream trace(traceFile);
    if (!trace.is_open()) {
        cerr << "LOL" << endl;
        return -1;
    }
    ifstream cache(cacheFile);
    if (!cache.is_open()) {
        cerr << "LOL" << endl;
        return -1;
    }

    // get numberOfLevels and b
    while (getline(cache, line))
    {
        if (line.empty() || line.at(0) == ' ' || line.at(0) == '#') continue;
        
        sscanf(line.c_str(), "%llu %llu", &num, &b);
        break;
    }
    
    ca.resize(num);

    while (getline(cache, line))
    {
        if (line.empty() || line.at(0) == ' ' || line.at(0) == '#') continue;
        if (i >= num)
        {
            cerr << "LOL number of cache levels err" << endl;
            return -1;
        }
        size_t s, E;
        sscanf(line.c_str(), "%llu %llu", &s, &E);

        ca.at(i).setSize(s, E);
        i++;
    }

    for (auto it = ca.begin(); it != ca.end(); it++)
    {
        if (num == 1) break;
        if (it == ca.begin())
        {
            it->setLower(to_address(next(it, +1)));
            continue;
        }
        if (next(it, +1) == ca.end())
        {
            it->setUpper(to_address(next(it, -1)));
            continue;
        }
        it->setUpper(to_address(next(it, -1)));
        it->setLower(to_address(next(it, +1)));
    }

    while (getline(trace, line))
    {
        if (line.empty() || line.at(0) == '#' || line.at(0) == 'I') continue;
        char op;
        size_t addr;
        int useless;
        sscanf(line.c_str(), " %c %zx,%d", &op, &addr, &useless);

        cout << op << endl;
        switch (op)
        {
            case 'L':
                ca.at(0).read(addr);
                break;
            case 'S':
                ca.at(0).write(addr);
                break;
            case 'M':
                ca.at(0).read(addr);
                ca.at(0).write(addr);
                break;
            default:
                cerr << "LOL trace err" << endl;
                break;
        }
    }

    for (auto &c : ca)
    {
        cout << "hit: " << c.hitCnt << " miss: " << c.missCnt << " write: " << c.writeCnt << endl;
    }
    return 0;
}
