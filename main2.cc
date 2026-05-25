#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>
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
    size_t num = 0, i = 0;
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
    // ifstream trace(traceFile);
    // if (!trace.is_open()) {
    //     cerr << "LOL" << endl;
    //     return -1;
    // }
    ifstream cache(cacheFile);
    if (!cache.is_open()) {
        cerr << "LOL" << endl;
        return -1;
    }

    while (getline(cache, line))
    {
        if (line.empty() || line.at(0) == ' ' || line.at(0) == '#') continue;
        if (all_of(line.begin(), line.end(), [](unsigned char c) { return std::isdigit(c); }))
        {
            sscanf(line.c_str(), "%llu", num);
            break;
        }
    }
    
    ca.resize(num);

    while (getline(cache, line))
    {
        if (line.empty() || line.at(0) == ' ' || line.at(0) == '#') continue;
        size_t s, E;
        sscanf(line.c_str(), "%llu %llu", &s, &E);

        ca.at(i).setSize(s, E);
        i++;
        if (i >= num)
        {
            cerr << "LOL number of cache levels err" << endl;
            return -1;
        }
    }
}
