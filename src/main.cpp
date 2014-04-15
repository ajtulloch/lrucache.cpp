// Copyright 2014 Andrew Tulloch <andrew@tullo.ch>
#include <boost/assert.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "lrucache.h"

int main(int, char**) {
  using lrucache::LRUCacheMap;

  {
    LRUCacheMap<int64_t, int64_t> cache(1, 1);

    cache.insert({1, 1});
    BOOST_VERIFY(cache.find(1) != cache.end());
    BOOST_VERIFY(cache.find(2) == cache.end());
    cache.insert({2, 5});
    BOOST_VERIFY(cache.find(1) == cache.end());
    BOOST_VERIFY(cache.find(2) != cache.end() && cache.find(2)->second == 5);
    BOOST_VERIFY(cache.begin() != cache.end());
    cache[2] = 4;
    BOOST_VERIFY(cache.find(2)->second == 4);
    BOOST_VERIFY(cache.erase(2) == 1);
    BOOST_VERIFY(cache.find(2) == cache.end());
    BOOST_VERIFY(cache.empty());
    BOOST_VERIFY(cache.size() == 0);
  }

  {
    LRUCacheMap<int64_t, int64_t> cache(5, 1);
    for (const auto el: {1, 2, 3, 4, 5}) {
      cache.insert({el, el});
    }

    {
      int64_t count = 0;
      for (const auto& elem: cache) {
        BOOST_VERIFY(elem.first == 5 - count);
        BOOST_VERIFY(elem.second == 5 - count);
        count += 1;
      }
    }

    {
      int64_t count = 1;
      for (const auto& elem: boost::adaptors::reverse(cache)) {
        BOOST_VERIFY(elem.first == count);
        BOOST_VERIFY(elem.second == count);
        count += 1;
      }
    }
  }
  return 0;
}
