// Copyright 2014 Andrew Tulloch <andrew@tullo.ch>
#pragma once

#include <limits>
#include <memory>
#include <unordered_map>

#include <boost/intrusive/list.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
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
  typedef Iterator<const value_type,
                   typename LinkedList::const_reverse_iterator>
      const_reverse_iterator;

  // Constructor
  explicit LRUCacheMap(size_t maxSize = std::numeric_limits<size_t>::max(),
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
  size_type size() const { return map_.size(); }
  size_type max_size() const { return maxSize_; }

  // Modifiers
  void clear() {
    entries_.clear();
    map_.clear();
  }

  iterator find(const Key& key) {
    const auto it = findEntry(key);
    if (it == entries_.end()) {
      return end();
    }
    moveToFront(it);
    return iterator(findEntry(key));
  }

  size_type count(const Key& key) const {
    return map_.find(key) == map_.end() ? 0 : 1;
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    //  Path where value exists
    if (find(value.first) != end()) {
      return {find(value.first), false};
    }

    // Path where value does not exist
    const auto it = map_.insert(
         {value.first, Entry(value.first, value.second)}).first;
    entries_.push_front(it->second);

    // Possible removal
    if (map_.size() > maxSize_) {
      shrinkToFit();
    }
    return {find(value.first), true};
  }

  T& operator[](const Key& key) {
    const auto it = find(key);
    if (it != end()) {
      return it->second;
    }
    return insert({key, T()}).first->second;
  }

  size_type erase(const Key& key) {
    const auto it = find(key);
    if (it == end()) {
      return 0;
    }
    erase(it);
    return 1;
  }

  iterator erase(const_iterator pos) {
    const auto it = find(pos->first);
    assert(it != end());
    const auto ret = iterator(entries_.erase(it.base()));
    map_.erase(pos->first);
    return ret;
  }

  // Observers
  hasher hash_function() const { return hasher(); }
  key_equal key_eq() const { return key_equal(); }

  void reserve(size_type count) { map_.reserve(count); }
  void rehash(size_type count) { map_.rehash(count); }

 private:
  template<class VT, class UT>
  class Iterator : public boost::iterator_adaptor<Iterator<VT, UT>, UT, VT> {
   public:
    explicit Iterator(UT iter): Iterator::iterator_adaptor_(iter) {}

    template <class OtherVT, class OtherUT>
    Iterator(
        Iterator<OtherVT, OtherUT> const& other,
        typename std::enable_if<
          std::is_convertible<OtherUT, UT>::value
        >::type* = nullptr): Iterator::iterator_adaptor_(other.base()) {}

   private:
    friend class boost::iterator_core_access;

    VT& dereference() const { return this->base_reference()->p_; }
  };

  void moveToFront(typename LinkedList::iterator it) {
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

  typename LinkedList::iterator findEntry(const Key& key) {
    const auto it = map_.find(key);
    if (it == map_.end()) {
      return entries_.end();
    }
    return entries_.iterator_to(it->second);
  }

  size_t maxSize_{0};
  size_t reclaimSize_{0};
  std::unordered_map<Key, Entry, Hash, KeyEqual> map_;
  LinkedList entries_;
};

}
