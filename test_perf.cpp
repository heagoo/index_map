#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include "index_map.h"
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

int test_index_map(index_map<uint64_t, Data> &m) {
  {
    Timer t("index_map::insert");
    for (int i = 0; i < element_size; ++i) {
      uint64_t key = ((uint64_t)rand() << 32) | rand();
      m.insert(std::make_pair(key, Data(1.0f, 2.0f, 3.0f)));
    }
  }

  {
    Timer t("index_map::iteration");
    int total = 0;
    for (int i = 0; i < 10; ++i)
    for (auto it : m) {
      total += it.first + it.second.f1 + it.second.f2 + it.second.f3;
    }
  }

  {
    Timer t("index_map::find&erase");
    for (int i = 0; i < 5000; ++i) {
      uint64_t key = ((uint64_t)rand() << 32) | rand();
      auto it = m.find(key);
      if (it != m.end()) {
        m.erase(it);
      }
    }
  }

  {
    Timer t("index_map::[]");
    for (int i = 0; i < 5000; ++i) {
      uint64_t key = ((uint64_t)rand() << 32) | rand();
      m[key] = Data();
    }
  }
}

int test_unordered_map(unordered_map<uint64_t, Data> &m) {
  {
    Timer t("unordered_map::insert");
    for (int i = 0; i < element_size; ++i) {
      uint64_t key = ((uint64_t)rand() << 32) | rand();
      m.insert(std::make_pair(key, Data(1.0f, 2.0f, 3.0f)));
    }
  }

  {
    Timer t("unordered_map::iteration");
    int total = 0;
    for (int i = 0; i < 10; ++i)
    for (auto it : m) {
      total += it.first + it.second.f1 + it.second.f2 + it.second.f3;
    }
  }

  {
    Timer t("unordered_map::find&erase");
    for (int i = 0; i < 5000; ++i) {
      uint64_t key = ((uint64_t)rand() << 32) | rand();
      auto it = m.find(key);
      if (it != m.end()) {
        m.erase(it);
      }
    }
  }

  {
    Timer t("unordered_map::[]");
    for (int i = 0; i < 5000; ++i) {
      uint64_t key = ((uint64_t)rand() << 32) | rand();
      m[key] = Data();
    }
  }
}

int main() {
  index_map<uint64_t, Data> m1;
  unordered_map<uint64_t, Data> m2;

  srand(time(NULL));

  test_index_map(m1);

  cout << "--------------------------------" << endl;
  
  test_unordered_map(m2);
}
