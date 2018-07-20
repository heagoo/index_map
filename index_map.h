#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

using namespace std;

template<typename K_T, typename V_T>
class index_bucket {
public:
  index_bucket() {
    indice[0] = -1;
    indice[1] = -1;
    indice[2] = -1;
    indice[3] = -1;
    pindice = NULL;
  }

  virtual ~index_bucket() {
    delete pindice;
  }

  // Returns a pair consisting of an address and a bool denoting whether could do the insertion
  // The address records the index of a value of std::pair<K_T, V_T>
  std::pair<int *, bool> insert(std::pair<K_T, V_T> *values, const K_T &key, const V_T &val) {
    int i;
    for (i = 0; i < sizeof(indice) / sizeof(indice[0]); ++i) {
      int idx = indice[i];
      if (idx >= 0) {
        if (values[idx].first == key) {
          return std::make_pair(&indice[i], false);
        }
      } else {
        break;
      }
    }

    // The key does not exist, and there is empty place
    if (i < sizeof(indice) / sizeof(indice[0])) {
      return std::make_pair(&indice[i], true);
    }

    // Check in the extended records
    if (pindice != NULL) {
      for (i = 0; i < pindice->size(); ++i) {
        int idx = (*pindice)[i];
        if (values[idx].first == key) {
          return std::make_pair(pindice->data() + i, false);
        }
      }

      // Still cannot find the key in the extended records
      pindice->push_back(-1);
      return std::make_pair(pindice->data() + i, true);

    } else {
      pindice = new std::vector<int>();
      pindice->push_back(-1);
      return std::make_pair(pindice->data(), true);
    }
  }

  // This function ONLY record the value index
  // Used when need to rehash the map
  void record_value_index(int val_idx) {
    int i;
    for (i = 0; i < sizeof(indice) / sizeof(indice[0]); ++i) {
      if (indice[i] < 0) {
        indice[i] = val_idx;
        break;
      }
    }

    if (i < sizeof(indice) / sizeof(indice[0])) {
      return;
    }

    // Record the index to the list
    if (pindice != NULL) {
      pindice->push_back(val_idx);
    } else {
      pindice = new std::vector<int>();
      pindice->push_back(val_idx);
    }
  }

  // Return the index of the found key&value, -1 means not found
  int find(std::pair<K_T, V_T> *values, const K_T &key) {
    int i;
    for (i = 0; i < sizeof(indice) / sizeof(indice[0]); ++i) {
      int idx = indice[i];
      if (idx >= 0) {
        if (values[idx].first == key) {
          return idx;
        }
      } else {
        break;
      }
    }

    // The key does not exist, and there is empty place
    if (i < sizeof(indice) / sizeof(indice[0])) {
      return -1;
    }

    // Check in the extended records
    if (pindice != NULL) {
      for (i = 0; i < pindice->size(); ++i) {
        int idx = (*pindice)[i];
        if (values[idx].first == key) {
          return idx;
        }
      }
    }

    return -1;
  }

  // Erase the specified record by key
  // Return the index of erased record inside values, -1 means key not found
  int erase(std::pair<K_T, V_T> *values, const K_T &key) {
    int i;
    for (i = 0; i < sizeof(indice) / sizeof(indice[0]); ++i) {
      int idx = indice[i];
      if (idx >= 0) {
        if (values[idx].first == key) {
          shrink_slot(i);
          return idx;
        }
      } else {
        break;
      }
    }

    // Check in the extended records
    if (pindice != NULL) {
      for (i = 0; i < pindice->size(); ++i) {
        int idx = (*pindice)[i];
        if (values[idx].first == key) {
          pindice->erase(pindice->begin() + i);
          if (pindice->empty()) {
            delete pindice;
            pindice = NULL;
          }
          return idx;
        }
      }
    }

    return -1;
  }

  // Erase the specified record by value index
  // Return how many records was erased
  int erase(int value_idx) {
    for (int i = 0; i < sizeof(indice) / sizeof(indice[0]); ++i) {
      int idx = indice[i];
      if (idx == value_idx) {
        shrink_slot(i);
        return idx;
      }
    }

    // Check in the extended records
    if (pindice != NULL) {
      for (int i = 0; i < pindice->size(); ++i) {
        int idx = (*pindice)[i];
        if (idx == value_idx) {
          pindice->erase(pindice->begin() + i);
          if (pindice->empty()) {
            delete pindice;
            pindice = NULL;
          }
          return 1;
        }
      }
    }

    return 0;
  }

private:
  void shrink_slot(int idx) {
    // Move one index value from the vector
    if (pindice != NULL) {
      indice[idx] = pindice->back();
      pindice->pop_back();
      if (pindice->empty()) {
        delete pindice;
        pindice = NULL;
      }
    }
    // Shrink the array
    else {
      while (idx + 1 < 4) {
        indice[idx] = indice[idx + 1];
        idx += 1;
      }
    }
  }

private:
  // when pindice = NULL, only use the index in indice
  int indice[4];
  std::vector<int> *pindice;
};

template<typename K_T, typename V_T>
class value_container {
public:
  value_container(int _capacity) {
    init(_capacity);
  }

  virtual ~value_container() {
    delete[] key_values;
    key_values = NULL;
  }

  // Clear all the values
  // _capacity: the size of container to keep after clear
  void clear(int _capacity) {
    delete[] key_values;
    key_values = NULL;

    available_slots.clear();

    init(_capacity);
  }

  int get_size() {
    return size;
  }

  // Get a value by index
  std::pair<K_T, V_T> &operator[](int index) {
    assert(index < capacity);
    return key_values[index];
  }

  int get_first_nonempty_slot() {
    int i;
    for (i = 0; i < next_empty_slot; ++i) {
      // Is valid value
      if (key_values[i].first >= 0) { 
        break;
      }
    }
    return i;
  }

  int get_next_empty_slot() {
    return next_empty_slot;
  }

  // Return the index inside key_values
  int insert(const K_T &key, const V_T &val) {
    int idx = -1;

    if (available_slots.empty()) {
      // key_values is full, enlarge the buffer
      if (unlikely(next_empty_slot >= capacity)) {
        capacity *= 2;
        std::pair<K_T, V_T> *new_values = new std::pair<K_T, V_T>[capacity];

        // Copy values, consider to use memcpy if value can be directly copied
        //memcpy(new_values, key_values, next_empty_slot * sizeof(std::pair<K_T, V_T>));
        for (int i = 0; i < next_empty_slot; ++i) {
          new_values[i] = key_values[i];
        }

        delete[] key_values;
        key_values = new_values;
      }

      idx = next_empty_slot;
      next_empty_slot += 1;
    }
    // Fill in holes inside key_values
    else {
      idx = available_slots.back();
      available_slots.pop_back();
    }

    key_values[idx].first = key;
    key_values[idx].second = val;
    size += 1;

    return idx;
  }

  void erase(int idx) {
    key_values[idx].first = -1;
    size -= 1;
    available_slots.push_back(idx);
  }

private:
  void init(int _capacity) {
    capacity = _capacity;
    next_empty_slot = 0;
    size = 0;

    key_values = new std::pair<K_T, V_T>[capacity];
  }

private:
  // Capacity of key_values
  int capacity;
  // Next available slot, all after that are also available
  int next_empty_slot;
  // Total size with values
  int size;
  // Erased slots, that are holes inside key_values
  std::vector<int> available_slots;

  std::pair<K_T, V_T> *key_values;
};

#define INDEX_MAP_INIT_BUCKETS 8096

template<typename K_T, typename V_T>
class index_map {
public:
  index_map(): index_map(INDEX_MAP_INIT_BUCKETS) {}

  index_map(int _bucket_size): 
    bucket_size(_bucket_size),
    values(_bucket_size) {
    buckets = new index_bucket<K_T, V_T>[bucket_size];
  }

  index_map(const index_map<K_T, V_T> &m);

  index_map<K_T, V_T> &operator=(const index_map<K_T, V_T> &m);

  virtual ~index_map() {
    delete[] buckets;
  }

  class iterator {
  public:
    iterator(index_map *_pmap) : iterator(_pmap, 0) {
    }
    iterator(index_map *_pmap, int idx) : pmap(_pmap), cur_index(idx) {
    }
    std::pair<K_T, V_T> &operator*() {
      return pmap->values[cur_index];
    }
    std::pair<K_T, V_T> *operator->() {
      return &(pmap->values[cur_index]);
    }
    iterator& operator++() {
      this->incr();
      return *this;
    }
    iterator operator++(int) {
      iterator __tmp(cur_index);
      this->incr();
      return __tmp;
    }
    bool operator!=(const iterator &it) const {
      return cur_index != it.cur_index || pmap != it.pmap;
    }
    bool operator==(const iterator &it) const {
      return cur_index == it.cur_index && pmap == it.pmap;
    }
  
  private:
    void incr() {
      cur_index += 1;
      int end_idx = pmap->get_end_index();
      if (likely(cur_index < end_idx && pmap->values[cur_index].first >= 0)) {
        return;
      }
      while (cur_index < end_idx) {
        // Valid value (not hole)
        if (pmap->values[cur_index].first >= 0) {
          break;
        } else {
          cur_index += 1;
        }
      } // end while
    }

  private:
    index_map *pmap;
    int cur_index;

    friend class index_map;
  };

  // Return iterator, and a bool value indicating whether the element was successfully inserted or not 
  std::pair<iterator, bool> insert(const std::pair<K_T, V_T> &value) {
    // TODO: adjust the value?
    if (size() * 2 > bucket_size) {
      rehash();
    }

    const K_T key = value.first;
    
    int bucket_idx = (int)(key % bucket_size);

    std::pair<int *, bool> ret = buckets[bucket_idx].insert(
                                 &values[0], value.first, value.second);

    int value_idx;

    // Could do the insert, means the key does not exist
    if (ret.second) {
      value_idx = values.insert(key, value.second);
      *ret.first = value_idx;
    } else {
      value_idx = *ret.first;
    }

    return std::make_pair(iterator(this, value_idx), ret.second);
  }

  int size() {
    return values.get_size();
  }

  V_T &operator[](const K_T &key) {
    V_T def_val;
    std::pair<iterator, bool> ret = insert(std::make_pair(key, def_val));
    return ret.first->second;
  }

  iterator begin() {
    return iterator(this, get_begin_index());
  }

  iterator end() {
    return iterator(this, get_end_index());
  }

  // Removes the element at pos
  iterator erase(iterator pos) {
    K_T key = pos->first;
    int bucket_idx = (int)(key % bucket_size);
    int value_idx = pos.cur_index;
    
    iterator ret = ++pos;

    buckets[bucket_idx].erase(value_idx);
    values.erase(value_idx);

    return ret;
  }

  // Removes the elements in the range [first; last)
  iterator erase(iterator first, iterator last) {
    iterator cur = first;
    while (cur != last) {
      cur = erase(cur);
    }
    return cur;
  }

  // Removes the element with the key equivalent to key
  int erase(const K_T &key) {
    int bucket_idx = (int)(key % bucket_size);
    int value_idx = buckets[bucket_idx].erase(&values[0], key);
    if (value_idx != -1) {
      values.erase(value_idx);
      return 1;
    } else {
      return 0;
    }
  }

  // Find the element by key
  iterator find(const K_T &key) {
    int bucket_idx = (int)(key % bucket_size);
    int value_idx = buckets[bucket_idx].find(&values[0], key);
    if (value_idx != -1) {
      return iterator(this, value_idx);
    } else {
      return end();
    }
  }

  // Remove all the elements
  void clear() {
    bucket_size = INDEX_MAP_INIT_BUCKETS;
    
    delete[] buckets;
    buckets = new index_bucket<K_T, V_T>[bucket_size];

    values.clear(bucket_size);
  }

private:
  int get_begin_index() {
    return values.get_first_nonempty_slot();
  }

  int get_end_index() {
    return values.get_next_empty_slot();
  }

  void rehash() {
    bucket_size = bucket_size * 3 + 1;

    delete[] buckets;
    buckets = new index_bucket<K_T, V_T>[bucket_size];

    // Rehash to buckets
    int end = get_end_index();
    for (int i = get_begin_index(); i < end; ++i) {
      if (values[i].first >= 0) {
        int bucket_idx = (int)(values[i].first % bucket_size);
        buckets[bucket_idx].record_value_index(i);
      }
    }
  }

private:
  int bucket_size;

  index_bucket<K_T, V_T> *buckets;

  value_container<K_T, V_T> values;
};
