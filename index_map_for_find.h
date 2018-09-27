#include <utility>
#include <cstring>
#include <cassert>
#include <malloc.h>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

template<typename K_T, typename V_T>
class index_bucket {
public:
  index_bucket() {
    record_num = 0;
    record_capacity = 0;
    records = NULL;
  }

  virtual ~index_bucket() {
    delete[] records;
  }

  // Returns a pair consisting of value index (inside records) and 
  // a bool denoting whether could do the insertion
  std::pair<int, bool> insert(const K_T &key, const V_T &val) {
    const int k_capacity = sizeof(k) / sizeof(k[0]);

    if (likely(record_num <= k_capacity)) {
      for (int idx = 0; idx < record_num; ++idx) {
        if (unlikely(k[idx] == key)) {
          return std::make_pair(idx, false);
        }
      }


      int idx = add_record(key, val);
      return std::make_pair(idx, true);
    }

    // The case that 'k' cannot cache all the keys
    else {
      for (int idx = 0; idx < k_capacity; ++idx) {
        if (unlikely(k[idx] == key)) {
          return std::make_pair(idx, false);
        }
      }

      // Try to find the key in records
      for (int idx = k_capacity; idx < record_num; ++idx) {
        if (unlikely(records[idx].first == key)) {
          return std::make_pair(idx, false);
        }
      }

      int idx = add_record(key, val);
      return std::make_pair(idx, true);
    }
  }


  // Return the index of the found key&value, -1 means not found
  int find(const K_T &key) const {
    const int k_capacity = sizeof(k) / sizeof(k[0]);

    if (likely(record_num <= k_capacity)) {
      for (int idx = 0; idx < record_num; ++idx) {
        if (k[idx] == key) {
          return idx;
        }
      }
      return -1;
    }

    // The case that 'k' cannot cache all the keys
    else {
      for (int idx = 0; idx < k_capacity; ++idx) {
        if (k[idx] == key) {
          return idx;
        }
      }

      // Try to find the key in records
      for (int idx = k_capacity; idx < record_num; ++idx) {
        if (records[idx].first == key) {
          return idx;
        }
      }

      return -1;
    }
  }

  // Erase the specified record by key
  // Return the index of erased record inside values, -1 means key not found
  int erase(const K_T &key) {
    int idx = find(key);
    return erase_by_index(idx);
  }

  // Erase the value by the value index, and return the erased index
  int erase_by_index(int idx) {
    if (idx != -1) {
      const int k_capacity = sizeof(k) / sizeof(k[0]);

      // Revise the value inside 'k' (repalced with the key in last record)
      if (idx < k_capacity) {
        if (record_num > 1) {
          k[idx] = records[record_num - 1].first;
        }
      }

      // Move last element in records to the removed place
      if (record_num > 1) {
        records[idx] = records[record_num - 1];
      }

      record_num -= 1;
    }

    return idx;
  }

  int get_record_num() const {
    return record_num;
  }

  std::pair<K_T, V_T> *get_records() {
    return records;
  }

  const std::pair<K_T, V_T> *get_records() const {
    return records;
  }

private:
  // Return the index of the new record
  int add_record(const K_T &key, const V_T &val) {

    // Enlarge the capacity
    if (record_num >= record_capacity) {
      std::pair<K_T, V_T> *old_records = records;
      record_capacity = (record_capacity == 0 ? 2 : record_capacity * 2);
      records = new std::pair<K_T, V_T>[record_capacity];
      for (int i = 0; i < record_num; ++i) {
        records[i] = old_records[i];
      }
      delete[] old_records;
    }

    int idx = record_num;
    records[idx].first = key;
    records[idx].second = val;
    record_num += 1;

    // There is still room in k
    const int k_capacity = sizeof(k) / sizeof(k[0]);
    if (idx < k_capacity) {
      k[idx] = key;
    }

    return idx;
  }

private:
  // Total records inside the bucket
  int record_num;
  // The capacity of 'records'
  int record_capacity;
  // First 6 keys
  K_T k[6];
  // All key and values
  std::pair<K_T, V_T> *records;
};

#define INDEX_MAP_INIT_BUCKETS 8096

template<typename K_T, typename V_T>
class index_map {
public:
      class _Iterator;
      class _ConstIterator;
      typedef          K_T                       key_type;
      typedef          V_T                       value_type;
      typedef typename std::pair<const K_T, V_T> mapped_type;
      typedef typename std::size_t               size_type;
      typedef typename std::ptrdiff_t            difference_type;
      typedef          value_type&               reference;
      typedef          const value_type&         const_reference;
      typedef          _Iterator                 iterator;
      typedef          _ConstIterator            const_iterator;

public:
  index_map(): index_map(INDEX_MAP_INIT_BUCKETS) {}

  index_map(size_type bucket_size):
      total_values_(0),
      bucket_size_(bucket_size) {
      allocate_buckets(bucket_size_);
  }

  index_map(std::initializer_list<mapped_type> init,
            size_type bucket_count = INDEX_MAP_INIT_BUCKETS):
            total_values_(0),
            bucket_size_(bucket_count) {
      allocate_buckets(bucket_size_);
      for (auto it = init.begin(); it != init.end(); ++it) {
          insert(*it);
      }
  }

  index_map(const index_map<K_T, V_T> &other) {
      total_values_ = other.total_values_;
      bucket_size_ = other.bucket_size_;
      allocate_buckets(bucket_size_);

      for (size_type i = 0; i < bucket_size_; ++i) {
          int rec_num = other.buckets_[i].get_record_num();
          if (rec_num > 0) {
              for (int r = 0; r < rec_num; ++r) {
                  auto records = other.buckets_[i].get_records();
                  buckets_[i].insert(records[r].first, records[r].second);
              }
          }
      }
  }

  // Move constructor
  index_map(index_map<K_T, V_T>&& other) {
      total_values_ = other.total_values_;
      bucket_size_ = other.bucket_size_;
      buckets_ = other.buckets_;

      other.total_values_ = 0;
      other.bucket_size_ = 0;
      other.buckets_ = NULL;
  }


  index_map<K_T, V_T> &operator=(const index_map<K_T, V_T> &other) {
      rehash(other.bucket_size_, other.buckets_, other.bucket_size_);
      return *this;
  }

  index_map<K_T, V_T> &operator=(index_map<K_T, V_T>&& other) {
      free_buckets(buckets_);

      total_values_ = other.total_values_;
      bucket_size_ = other.bucket_size_;
      buckets_ = other.buckets_;

      other.total_values_ = 0;
      other.bucket_size_ = 0;
      other.buckets_ = NULL;

      return *this;
  }

  virtual ~index_map() {
      free_buckets(buckets_);
  }

  class _IteratorBase {
      protected:
          _IteratorBase(index_map *_pmap, unsigned int _bucket_idx, int _value_idx) :
              pmap(_pmap), bucket_idx(_bucket_idx), value_idx(_value_idx) {
          }
          _IteratorBase(const _IteratorBase &it) :
              pmap(it.pmap), bucket_idx(it.bucket_idx), value_idx(it.value_idx) {
          }
          std::pair<K_T, V_T> &operator*() const {
              return pmap->buckets_[bucket_idx].get_records()[value_idx];
          }
          std::pair<K_T, V_T> *operator->() const {
              return &(pmap->buckets_[bucket_idx].get_records()[value_idx]);
          }
          bool operator!=(const _IteratorBase &it) const {
              return !operator==(it);
          }
          bool operator==(const _IteratorBase &it) const {
              return bucket_idx == it.bucket_idx && value_idx == it.value_idx && pmap == it.pmap;
          }
          void incr() {
              if (value_idx + 1 < pmap->buckets_[bucket_idx].get_record_num()) {
                  value_idx += 1;
                  return;
              }

              size_type bucket_count = pmap->bucket_count();
              while (++bucket_idx < bucket_count) {
                  if (pmap->buckets_[bucket_idx].get_record_num() > 0) {
                      break;
                  }
              }

              // Point to the first value in the bucket
              value_idx = 0;
          }

      private:
          index_map *pmap;
          unsigned int bucket_idx;
          int value_idx;

          friend class index_map;
  };

  class _Iterator : public _IteratorBase {
      public:
          _Iterator(index_map *_pmap, unsigned int _bucket_idx, int _value_idx) :
              _IteratorBase(_pmap, _bucket_idx, _value_idx) {
          }
          _Iterator(const _Iterator &it) : _IteratorBase(it) {
          }
          std::pair<K_T, V_T> &operator*() const {
              return _IteratorBase::operator*();
          }
          std::pair<K_T, V_T> *operator->() const {
              return _IteratorBase::operator->();
          }
          bool operator!=(const iterator &it) const {
              return _IteratorBase::operator!=(it);
          }
          bool operator==(const iterator &it) const {
              return _IteratorBase::operator==(it);
          }
          iterator& operator++() {
              _IteratorBase::incr();
              return *this;
          }
          iterator operator++(int) {
              iterator __tmp(*this);
              _IteratorBase::incr();
              return __tmp;
          }
  };

  class _ConstIterator : public _IteratorBase {
      public:
          _ConstIterator(const index_map *_pmap, unsigned int _bucket_idx, int _value_idx) :
              _IteratorBase(const_cast<index_map *>(_pmap), _bucket_idx, _value_idx) {
          }
          _ConstIterator(const _ConstIterator &it) : _IteratorBase(it) {
          }
          _ConstIterator(const _Iterator &it) : _IteratorBase(it) {
          }
          const std::pair<K_T, V_T> &operator*() const {
              return _IteratorBase::operator*();
          }
          const std::pair<K_T, V_T> *operator->() const {
              return _IteratorBase::operator->();
          }
          bool operator!=(const const_iterator &it) const {
              return _IteratorBase::operator!=(it);
          }
          bool operator==(const const_iterator &it) const {
              return _IteratorBase::operator==(it);
          }
          const_iterator& operator++() {
              _IteratorBase::incr();
              return *this;
          }
          const_iterator operator++(int) {
              const_iterator __tmp(*this);
              _IteratorBase::incr();
              return __tmp;
          }
  };

  iterator begin() {
      if (unlikely(size()== 0)) {
          return end();
      }

      for (unsigned int i = 0; i < bucket_size_; ++i) {
          if (buckets_[i].get_record_num() > 0) {
              return iterator(this, i, 0);
          }
      }

      assert(size() == 0);
      return end();
  }

  const_iterator begin() const {
      return cbegin();
  }

  const_iterator cbegin() const {
      if (unlikely(size()== 0)) {
          return cend();
      }

      for (unsigned int i = 0; i < bucket_size_; ++i) {
          if (buckets_[i].get_record_num() > 0) {
              return const_iterator(this, i, 0);
          }
      }

      assert(size() == 0);
      return cend();
  }

  iterator end() {
      return iterator(this, bucket_size_, 0);
  }

  const_iterator end() const {
      return cend();
  }

  const_iterator cend() const {
      return const_iterator(this, bucket_size_, 0);
  }

  bool empty() const {
      return size() == 0;
  }

  size_type size() const {
      return total_values_;
  }

  size_type max_size() const {
      // 2^32 - 1
      return 4294967295ll;
  }

  // Remove all the elements
  void clear() {
      total_values_ = 0;
      free_buckets(buckets_);
      allocate_buckets(INDEX_MAP_INIT_BUCKETS);
  }

  // Return iterator, and a bool value indicating whether the element was successfully inserted or not 
  std::pair<iterator, bool> insert(const std::pair<K_T, V_T> &value) {
      return insert_key_value(value.first, value.second);
  }

  std::pair<iterator, bool> insert(std::pair<K_T, V_T>&& value) {
      return insert_key_value(value.first, value.second);
  }

  // In our implementation, hint is ignored
  iterator insert(const_iterator hint, const mapped_type &value) {
      auto ret = insert(value);
      return ret.first;
  }

  template <typename InputIt>
  void insert(InputIt first, InputIt last) {
      for (auto it = first; it != last; ++it) {
          insert(*it);
      }
  }

  void insert(std::initializer_list<mapped_type> ilist) {
      for (auto it = ilist.begin(); it != ilist.end(); ++it) {
          insert(*it);
      }
  }

  // Inserts a new element into the container constructed in-place with the given args
  // This impl. does not work as in-place, but should be OK
  template<class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
      std::allocator<mapped_type> allocator;
      mapped_type *n = allocator.allocate(1);
      allocator.construct(n, std::forward<Args>(args)...);
      auto ret = insert(*n);
      allocator.deallocate(n, 1);
      return ret;
  }

  template<class... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args) {
      auto ret = emplace(std::forward<Args>(args)...);
      return ret.first;
  }

  // Removes the element at pos
  iterator erase(const_iterator pos) {
      K_T key = pos->first;
      unsigned int bucket_idx = get_hash_value(key);
      int value_idx = pos.value_idx;

      const_iterator ret = ++pos;

      if (buckets_[bucket_idx].erase_by_index(value_idx) != -1) {
          total_values_ -= 1;
      }

      return iterator(ret.pmap, ret.bucket_idx, ret.value_idx);
  }

  // Removes the elements in the range [first; last)
  iterator erase(const_iterator first, const_iterator last) {
      iterator cur = first;
      while (cur != last) {
          cur = erase(cur);
      }
      return cur;
  }

  // Removes the element with the key equivalent to key
  size_type erase(const K_T &key) {
      unsigned int bucket_idx = get_hash_value(key);
      if (buckets_[bucket_idx].erase(key) != -1) {
          total_values_ -= 1;
          return 1;
      }
      return 0;
  }

  void swap(index_map &other) {
      std::swap(buckets_, other.buckets_);
      std::swap(bucket_size_, other.bucket_size_);
      std::swap(total_values_, other.total_values_);
  }

  V_T &at(const K_T &key) {
      unsigned int bucket_idx = get_hash_value(key);
      int value_idx = buckets_[bucket_idx].find(key);
      if (value_idx != -1) {
          return buckets_[bucket_idx].get_records()[value_idx].second;
      } else {
          throw std::out_of_range("Cannot find the key");
      }
  }

  const V_T &at(const K_T &key) const {
      return const_cast<index_map *>(this)->at(key);
  }

  V_T &operator[](const K_T &key) {
      V_T def_val;
      std::pair<iterator, bool> ret = insert(std::make_pair(key, def_val));
      return ret.first->second;
  }

  size_type count(const K_T &key) const {
      unsigned int bucket_idx = get_hash_value(key);
      int value_idx = buckets_[bucket_idx].find(key);
      return (value_idx != -1) ? 1 : 0;
  }

  // Find the element by key
  iterator find(const K_T &key) {
      unsigned int bucket_idx = get_hash_value(key);
      int value_idx = buckets_[bucket_idx].find(key);
      if (value_idx != -1) {
          return iterator(this, bucket_idx, value_idx);
      } else {
          return end();
      }
  }

  const_iterator find(const K_T &key) const {
      unsigned int bucket_idx = get_hash_value(key);
      int value_idx = buckets_[bucket_idx].find(key);
      if (value_idx != -1) {
          return const_iterator(this, bucket_idx, value_idx);
      } else {
          return cend();
      }
  }

  // Returns a range containing all elements with the key
  std::pair<iterator, iterator> equal_range(const K_T &key) {
      iterator it = find(key);
      if (it != end()) {
          iterator next(it);
          ++next;
          return std::make_pair(it, next);
      } else {
          return std::make_pair(end(), end());
      }
  }

  std::pair<const_iterator, const_iterator> equal_range(const K_T &key) const {
      const_iterator it = find(key);
      if (it != cend()) {
          const_iterator next(it);
          ++next;
          return std::make_pair(it, next);
      } else {
          return std::make_pair(cend(), cend());
      }
  }

  size_type bucket_count() const {
      return bucket_size_;
  }

  size_type max_bucket_count() const {
      // 2^32 - 1
      return 4294967295ll;
  }

  // Returns the number of elements in the bucket with index n
  size_type bucket_size(size_type n) const {
      if (n < bucket_size_) {
          return buckets_[n].get_record_num();
      } else {
          return -1;
      }
  }

  // Return bucket index for the key
  size_type bucket(const K_T &key) const {
      unsigned int bucket_idx = get_hash_value(key);
      return bucket_idx;
  }

private:
  void allocate_buckets(unsigned int bucket_size) {
      buckets_ = new index_bucket<K_T, V_T>[bucket_size];
      bucket_size_ = bucket_size;
  }

  void free_buckets(index_bucket<K_T, V_T> *buckets) {
      delete[] buckets;
  }

  std::pair<iterator, bool> insert_key_value(const K_T key, const V_T &val) {
      // TODO: adjust the value?
      if (size() * 2 > bucket_size_) {
          rehash(2 * bucket_size_ + 1, buckets_, bucket_size_);
      }

      unsigned int bucket_idx = get_hash_value(key);

      std::pair<int, bool> ret = buckets_[bucket_idx].insert(key, val);
      if (ret.second) {
          total_values_ += 1;
      }

      int value_idx = ret.first;

      return std::make_pair(iterator(this, bucket_idx, value_idx), ret.second);
  }

  void rehash(unsigned int new_bktsize, index_bucket<K_T, V_T> *src_buckets, unsigned int src_bktsize) {
      index_bucket<K_T, V_T> *origin_buckets = buckets_;

      allocate_buckets(new_bktsize);

      // Copy values to the new buckets
      unsigned int values = 0;
      for (unsigned int idx = 0; idx < src_bktsize; ++idx) {
          int record_num = src_buckets[idx].get_record_num();
          for (int i = 0; i < record_num; ++i) {
              std::pair<K_T, V_T> &rec = src_buckets[idx].get_records()[i];
              unsigned int bucket_idx = get_hash_value(rec.first);
              auto ret = buckets_[bucket_idx].insert(rec.first, rec.second);
              if (ret.second) {
                  values += 1;
              }
          }
      }

      total_values_ = values;

      free_buckets(origin_buckets);
  }

  unsigned int get_hash_value(const K_T key) const {
      return (unsigned int)(key % bucket_size_);
  }

private:
  unsigned int total_values_;
  unsigned int bucket_size_;
  index_bucket<K_T, V_T> *buckets_;
};

template<typename K_T, typename V_T>
bool operator==(const index_map<K_T, V_T>& lhs, 
                const index_map<K_T, V_T>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (auto it : lhs) {
        if (rhs.find(it.first) == rhs.end()) {
            return false;
        }
    }
    return true;
}
template<typename K_T, typename V_T>
bool operator!=(const index_map<K_T, V_T>& lhs, 
                const index_map<K_T, V_T>& rhs) {
    return !operator==(lhs, rhs);
}
