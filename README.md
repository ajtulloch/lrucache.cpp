# LRUCache.cpp

Just a simple LRU cache map in C++.  Mimics most of the associative
container concept from STL.

## Design

Threads a doubly-linked intrusive list through a hash map for O(1)

- insert
- update
- erase
- size

operations.

## Dependencies

A relatively modern version of `boost`.

## Usage

```bash
git clone https://github.com/ajtulloch/lrucache.cpp
cd lrucache.cpp
mkdir build
cd build
cmake .. && make && make test
```

To use, copy `src/lrucache.h` into your include path.

## Test




