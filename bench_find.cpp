#include <iostream>
#include <sstream>
#include <unordered_map>
#include <cstdlib>
#include "index_map_for_find.h"
#include "timer.h"

struct Data {
  float f1;
  float f2;
  float f3;
  Data(): Data(0, 0, 0) {}
  Data(float _f1, float _f2, float _f3) {
    f1 = _f1;
    f2 = _f2;
    f3 = _f3;
  }
};

static const int element_size = 100000000;

void bench_index_map(index_map<uint64_t, Data> &m, uint64_t *keys) {
  const int loops = 10;
  int index = 0;
  for (int loop = 0; loop < loops; ++loop) {
    {
    std::ostringstream s;
    s << "    index_map::insert (" << (element_size / loops) << " elements)";
    Timer t(s.str().c_str());
    for (int i = 0; i < element_size / loops; ++i) {
      m.insert(std::make_pair(keys[index++], Data(1.0f, 2.0f, 3.0f)));
    }
    }

    {
    index_map<uint64_t, Data>::size_type size = m.size();
    std::ostringstream s;
    s << "    index_map::find   (" << size << " elements)";
    Timer t(s.str().c_str());
    for (unsigned int i = 0; i < size; ++i) {
      m.find(keys[i]);
    }
    }
  } // end for
}

void bench_unordered_map(unordered_map<uint64_t, Data> &m, uint64_t *keys) {
  const int loops = 10;
  int index = 0;
  for (int loop = 0; loop < loops; ++loop) {
    {
    std::ostringstream s;
    s << "unordered_map::insert (" << (element_size / loops) << " elements)";
    Timer t(s.str().c_str());
    for (unsigned int i = 0; i < element_size / loops; ++i) {
      m.insert(std::make_pair(keys[index++], Data(1.0f, 2.0f, 3.0f)));
    }
    }

    {
    unordered_map<uint64_t, Data>::size_type size = m.size();
    std::ostringstream s;
    s << "unordered_map::find   (" << size << " elements)";
    Timer t(s.str().c_str());
    for (unsigned int i = 0; i < size; ++i) {
      m.find(keys[i]);
    }
    }
  } // end for
}

int main() {
  // Prepare random keys
  srand(time(NULL));
  uint64_t *keys = new uint64_t[element_size];
  for (int i = 0; i < element_size; ++i) {
    keys[i] = ((uint64_t)rand() << 32) | rand();
  }

  unordered_map<uint64_t, Data> m1;
  index_map<uint64_t, Data> m2;

  bench_unordered_map(m1, keys);

  cout << "-----------------------------------------------------" << endl;
  
  bench_index_map(m2, keys);

  // Cleanup
  delete[] keys;
}
