# Cache Simulator

## Features

- utilize modern C++ 23
- true & full implementation of academic LRU (Least Recently Used) algorithm <br>NB. it is not the version used by Intel, but the university/textbook version
- N-level cache hierarchy, with separate set `s` and associativity `E`
- strictly inclusive arch
- write-back and write-allocate policy
- detailed log per cycle

## TODO

- read/write length support (known issue: please edit variable `b` in the head file rather than `main.cc` to change block size)


## Usage

make sure your compiler support C++ 23

```bash
g++ -std=c++23 main.cc
```

and have fun

arguments:
- `-v` verbose
- `-f <cacheFile>`
- `-t <traceFile>`

## Note

`traces/` files are copied from CSAPP Cache Lab

DO NOT OPEN ANY ISSUES IF YOU COMPILE WITH VISUAL STUDIO OR ANYTHING EXCEPT MINGW-W64 (EVEN WITH MINGW-W64, WINDOWS IS LIKELY TO BE A PROBLEM MAKER)

ONLY TESTED ON DEBIAN G++ 14 WITH C++ 23 (AND A BIT MINGW-W64)

Copyright (C) _ov4

This is free software; see the source for copying conditions. There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

under GNU AFFERO GENERAL PUBLIC LICENSE (AGPL) Version 3
