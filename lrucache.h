#pragma once

#include <memory>
#include <unordered_map>
#include <limits>

#include <boost/intrusive/list.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/noncopyable.hpp>

namespace lrucache {

namespace detail {

template<class Key, class T>
class Entry : public boost::intrusive::list_base_hook<> {
 public:
  Entry(const Key& key, const T& value) : p_(key, value) {}
  std::pair<const Key, T> p_;
};

}

template<class Key,
         class T,
         class Hash = std::hash<Key>,
         class KeyEqual = std::equal_to<Key>,
         class Allocator = std::allocator<detail::Entry<Key, T>>>
class LRUCacheMap : public boost::noncopyable {
 private:
  template<typename VT, typename UT>
  class Iterator;

  using Entry = detail::Entry<Key, T>;
  using LinkedList = boost::intrusive::list<Entry>;

 public:
  // Member Types
  typedef Key key_type;
  typedef std::pair<const Key, T> value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef Hash hasher;
  typedef KeyEqual key_equal;
  typedef Allocator allocator_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  typedef typename std::allocator_traits<Allocator>::pointer
      pointer;
  typedef typename std::allocator_traits<Allocator>::const_pointer
      const_pointer;

  typedef Iterator<value_type, typename LinkedList::iterator>
      iterator;
  typedef Iterator<const value_type, typename LinkedList::const_iterator>
      const_iterator;
  typedef Iterator<value_type, typename LinkedList::reverse_iterator>
      reverse_iterator;
  typedef Iterator<const value_type, typename LinkedList::const_reverse_iterator>
      const_reverse_iterator;

  // Constructor
  LRUCacheMap(size_t maxSize = std::numeric_limits<size_t>::max(),
              size_t reclaimSize = 1)
      : maxSize_(maxSize)
      , reclaimSize_(reclaimSize) {
  }

  // Iterators
  iterator begin() { return iterator(entries_.begin()); }
  iterator end() { return iterator(entries_.end()); }
  const_iterator begin() const { return const_iterator(entries_.begin()); }
  const_iterator end() const { return const_iterator(entries_.end()); }
  reverse_iterator rbegin() { return reverse_iterator(entries_.rbegin()); }
  reverse_iterator rend() { return reverse_iterator(entries_.rend()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(entries_.rbegin());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(entries_.rend());
  }
  // Capacity
  bool empty() const { return map_.empty(); }
  size_t size() const { return map_.size(); }
  size_t max_size() const { return maxSize_; }

  // Modifiers
  void clear() { entries_.clear();  map_.clear(); }

  void set(const Key& key, const T& value) {
    Entry candidate(key, value);
    const auto p = map_.insert({key, candidate});
    auto& e = p.first->second;
    if (!p.second) {
      e.p_.second = std::move(candidate.p_.second);
      moveToFront(e);
      return;
    }
    entries_.push_front(e);

    // Possible removal
    if (map_.size() > maxSize_) {
      shrinkToFit();
    }
  }

  // Lookup
  bool get(const Key& key, T* value) {
    assert(value != nullptr);
    const auto it = map_.find(key);
    if (it == map_.end()) {
      return false;
    }
    *value = it->second.p_.second;
    moveToFront(it->second);
    return true;
  }

 private:
  template<class VT, class UT>
  class Iterator : public boost::iterator_adaptor<Iterator<VT, UT>, UT, VT> {
   public:
    Iterator(UT iter)
        : Iterator::iterator_adaptor_(iter) {}
   private:
    friend class boost::iterator_core_access;
    VT& dereference() const {
      return this->base_reference()->p_;
    }
  };

  void moveToFront(Entry& entry) {
    auto it = entries_.iterator_to(entry);
    assert(it != entries_.end());
    entries_.erase(it);
    entries_.push_front(*it);
  }

  void shrinkToFit() {
    for (size_t numRemoved = 0;
         map_.size() > 0 && map_.size() > maxSize_ && numRemoved < reclaimSize_;
         ++numRemoved) {
      const auto& key = entries_.back().p_.first;
      entries_.pop_back();
      map_.erase(key);
    }
  }

  size_t maxSize_{0};
  size_t reclaimSize_{0};
  std::unordered_map<Key, Entry, Hash, KeyEqual> map_;
  LinkedList entries_;
};

}
