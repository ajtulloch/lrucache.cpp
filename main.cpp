#include "lrucache.h"
#include <boost/assert.hpp>
int main(int, char**) {
  using lrucache::LRUCacheMap;
  
  {
    LRUCacheMap<int64_t, int64_t> cache(1, 1);

    cache.set(1, 1);
    int64_t result;
    BOOST_VERIFY(cache.get(1, &result));
    BOOST_VERIFY(!cache.get(2, &result));
    cache.set(2, 5);
    BOOST_VERIFY(!cache.get(1, &result));
    BOOST_VERIFY(cache.get(2, &result) && result == 5);
    BOOST_VERIFY(cache.begin() != cache.end());

  }

  {
    LRUCacheMap<int64_t, int64_t> cache(5, 1);
    for (const auto el: {1, 2, 3, 4, 5}) {
      cache.set(el, el);
    }
    {
      int64_t count = 0;
      for (auto& elem: cache) {
        BOOST_VERIFY(elem.first == 5 - count);
        BOOST_VERIFY(elem.second == 5 - count);
        count += 1;
      }
    }

    {
      int64_t count = 1;
      for (auto& elem: boost::adaptors::reverse(cache)) {
        BOOST_VERIFY(elem.first == count);
        BOOST_VERIFY(elem.second == count);
        count += 1;
      }
    }
  }
  return 0;
}
