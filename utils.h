#pragma once
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>


struct FirstLess
{
   template<typename T1, typename T2>
   bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs)
   {
      return lhs.first < rhs.first;
   }

   template<typename T1, typename T2>
   bool operator()(const std::pair<T1, T2>& lhs, const T1& rhs)
   {
      return lhs.first < rhs;
   }

   template<typename T1, typename T2>
   bool operator()(const T1& lhs, const std::pair<T1, T2>& rhs)
   {
      return lhs < rhs.first;
   }
};

template<class InputIt1, class InputIt2>
bool set_intersects(InputIt1 first1, InputIt1 last1,
                    InputIt2 first2, InputIt2 last2)
{
    while (first1 != last1 && first2 != last2)
    {
        if (*first1 < *first2)
        {
            ++first1;
        }
        else
        {
            if (!(*first2 < *first1))
                return true;

            ++first2;
        }
    }
    return false;
}

template<class SortedRange1, class SortedRange2>
bool set_intersects(const SortedRange1& r1, const SortedRange2& r2)
{
    return set_intersects(std::begin(r1), std::end(r1), std::begin(r2), std::end(r2));
}


template<typename T, typename A = std::allocator<T>>
class SortedSet
{
public:
    SortedSet() = default;
    explicit SortedSet(const A& a) : elems_(a) {}
    SortedSet(std::initializer_list<T> lst) : SortedSet(std::begin(lst), std::end(lst)) {}

    template<class Container, typename = typename std::enable_if_t<!std::is_same_v<std::decay_t<Container>, SortedSet>>>
    SortedSet(const Container& cont) : SortedSet(std::begin(cont), std::end(cont)) {}

    template<class Iter>
    explicit SortedSet(Iter first, Iter last)
        : elems_(first, last)
    {
        std::sort(elems_.begin(), elems_.end());
        elems_.erase(std::unique(elems_.begin(), elems_.end()), elems_.end());
    }

    bool contains(const T& value) const
    {
        auto it = lower_bound(value);
        return (it != elems_.end() && *it == value);
    }
    auto insert(const T& value)
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), value);
        if(it != elems_.end() && *it == value)
            return it;

        return elems_.emplace(it, value);
    }

    bool erase(const T& value)
    {
        auto it = lower_bound(value);
        if(it == elems_.end() || *it != value)
            return false;

        elems_.erase(it);
        return true;
    }

    bool empty() const { return elems_.empty(); }
    std::size_t size() const { return elems_.size(); }
    auto begin() const { return elems_.begin(); }
    auto end() const { return elems_.end(); }

    auto lower_bound(const T& value) const
    {
        return std::lower_bound(elems_.begin(), elems_.end(), value);
    }

    auto find(const T& value) const
    {
       auto it = lower_bound(value);
       if(it == elems_.end() || value < *it)
          return elems_.end();

        return it;
    }

private:
    std::vector<T> elems_;
};


template<typename K, typename T, typename A = std::allocator<std::pair<K, T>>>
class SortedMap
{
public:
   SortedMap() = default;
   explicit SortedMap(const A& a)
      : elems_(a) 
   { }

   const std::vector<std::pair<K, T>, A>& elems() const { return elems_; }
   std::vector<std::pair<K, T>, A>& elems() { return elems_; }

   T& operator[](const K& key)
   {
      auto it = std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
      if(it == elems_.end() || FirstLess()(key, *it))
         it = elems_.emplace(it, key, T{});

      return it->second;
   }

   auto lower_bound(const K& key)
   {
      return std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
   }

   auto emplace_hint(typename std::vector<std::pair<K, T>, A>::iterator hint,
                     const K& key,
                     const T& value)
   {
      FirstLess comp;
      if((hint == begin() || (hint != end() && comp(*std::prev(hint), key))) && (hint == end() || !comp(*hint, key)))
         hint = lower_bound(key);

      return elems_.emplace(hint, key, value);
   }

   void clear() { elems_.clear(); }
   auto begin() const { return elems_.begin(); }
   auto end() const { return elems_.end(); }

private:
   std::vector<std::pair<K, T>, A> elems_;
};
