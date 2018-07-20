#include <unordered_map>
#include "index_map.h"

struct Data {
  float f1;
  float f2;
  float f3;
  Data(): Data(1.0f, 2.0f, 3.0f) {}
  Data(float _f1, float _f2, float _f3) {
    this->f1 = _f1;
    this->f2 = _f2;
    this->f3 = _f3;
  }
  bool operator==(const Data &d) {
    return f1 == d.f1 && f2 == d.f2 && f3 == d.f3;
  }
};

void test_insert() {
  index_map<uint64_t, Data> m;

  // insert
  auto ret = m.insert(std::make_pair(123ll, Data(3, 5, 7)));
  assert(ret.first->first == 123ll);
  assert(ret.first->second.f1 == 3);
  assert(ret.first->second.f2 == 5);
  assert(ret.first->second.f3 == 7);
  assert(ret.second == true);

  // find existing value
  auto it = m.find(123ll);
  assert(it->second.f1 = 3);
  assert(it->second.f2 = 5);
  assert(it->second.f3 = 7);

  // insert again
  ret = m.insert(std::make_pair(123ll, Data(1, 5, 7)));
  assert(ret.first->first == 123ll);
  assert(ret.first->second.f1 == 3);
  assert(ret.first->second.f2 == 5);
  assert(ret.first->second.f3 == 7);
  assert(ret.second == false);

  // find existing value
  assert(!(m.find(125ll) != m.end()));
}

void test_enumerate() {
  index_map<uint64_t, Data> m;
  uint64_t keys[] = {(uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand()};
  Data values[] = {Data(rand(),rand(),rand()), Data(rand(),rand(),rand()), Data()};

  uint64_t keys_sum = 0;
  float values_sum[3] = { 0 };
  for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    auto ret = m.insert(std::make_pair(keys[i], values[i]));
    if (ret.second) {
      keys_sum += keys[i];
      values_sum[0] += values[i].f1;
      values_sum[1] += values[i].f2;
      values_sum[2] += values[i].f3;
    }
  }

  uint64_t sum = 0;
  float sum_f[3] = { 0 };
  for (auto it : m) {
    sum += it.first;
    sum_f[0] += it.second.f1;
    sum_f[1] += it.second.f2;
    sum_f[2] += it.second.f3;
  }

  assert(sum == keys_sum);
  assert(sum_f[0] == values_sum[0]);
  assert(sum_f[1] == values_sum[1]);
  assert(sum_f[2] == values_sum[2]);
}

void test_index() {
  index_map<uint64_t, Data> m;
  uint64_t keys[] = {(uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand()};
  Data values[] = {Data(rand(),rand(),rand()), Data(rand(),rand(),rand()), Data()};

  for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    m.insert(std::make_pair(keys[i], values[i]));
  }

  for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    assert(m[keys[i]] == values[i]);
    m[keys[i]].f1 += 1;
    m[keys[i]].f2 += 2;
    m[keys[i]].f3 += 3;
  }

  for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    assert(m[keys[i]].f1 == values[i].f1 + 1);
    assert(m[keys[i]].f2 == values[i].f2 + 2);
    assert(m[keys[i]].f3 == values[i].f3 + 3);
  }
}

void test_size() {
  index_map<uint64_t, Data> m;
  int size = 0;
  uint64_t key = (uint64_t)rand() << 32 | rand();
  Data value = Data(rand(),rand(),rand());
  for (int i = 0; i < 10000000; ++i) {
    auto ret = m.insert(std::make_pair(key, value));
    if (ret.second) {
      size += 1;
    }
  }
  assert(m.size() == size);

  size = 0;
  for (auto it : m) {
    size += 1;
  }
  assert(m.size() == size);
}

void test_clear() {
  index_map<uint64_t, Data> m;
  Data value = Data(rand(),rand(),rand());
  m[0] = value;
  m[1] = value;
  m.clear();
  assert(m.size() == 0);
  assert(m.find(0) == m.end());
  assert(m.find(1) == m.end());
}

void compare_unordered_map() {
  index_map<uint64_t, Data> m;
  unordered_map<uint64_t, Data> u;

  for (int i = 0; i < 100000; ++i) {
    uint64_t key = (uint64_t)rand() << 32 | rand();
    Data value = Data(rand(),rand(),rand());
    auto ret1 = m.insert(std::make_pair(key, value));
    auto ret2 = u.insert(std::make_pair(key, value));
    assert(ret1.second == ret2.second);

    ret1 = m.insert(std::make_pair(key, value));
    ret2 = u.insert(std::make_pair(key, value));
    assert(ret1.second == ret2.second);
  }

  for (auto it : m) {
    assert(u[it.first] == it.second);
  }

  for (auto it : u) {
    assert(m[it.first] == it.second);
  }
}

int main() {
  test_insert();
  test_enumerate();
  test_index();
  test_size();

  compare_unordered_map();

  return 0;
}
