#include <unordered_map>
#include <iostream>
#include "index_map_for_find.h"

using namespace std;

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

void test_constructor() {
    // default constructor: empty map
    index_map<int, std::string> m1;
 
    // list constructor
    index_map<int, std::string> m2 =
    {
        {1, "foo"},
        {3, "bar"},
        {2, "baz"},
    };
 
    // copy constructor
    index_map<int, std::string> m3 = m2;
 
    // move constructor
    index_map<int, std::string> m4 = std::move(m2);
 
    assert(m1.size() == 0);
    assert(m2.size() == 0);
    assert(m3.size() == 3);
    assert(m4.size() == 3);
}

void test_assign() {
    index_map<int, int> nums1 {{3, 1}, {4, 1}, {5, 9}, 
                               {6, 1}, {7, 1}, {8, 9}};
    index_map<int, int> nums2; 
    index_map<int, int> nums3;

    assert(nums1.size() == 6);
    assert(nums2.size() == 0);
    assert(nums3.size() == 0);
 
    nums2 = nums1;
 
    assert(nums1.size() == 6);
    assert(nums2.size() == 6);
    assert(nums3.size() == 0);
 
    // move assignment moves data from nums1 to nums3,
    // modifying both nums1 and nums3
    nums3 = std::move(nums1);
 
    assert(nums1.size() == 0);
    assert(nums2.size() == 6);
    assert(nums3.size() == 6);
}

void test_begin_end() {
    index_map<int, Data> mag = {
        { 1, {1, 2, 3} },
        { 2, {4, 5, 6} },
        { 3, {7, 8, 9} }
    };
 
    int iter_num = 0;
    for(auto iter = mag.begin(); iter != mag.end(); ++iter){
        auto cur = iter->first;
        assert(iter->second.f1 == mag[cur].f1);
        assert(iter->second.f2 == mag[cur].f2);
        assert(iter->second.f3 == mag[cur].f3);
        iter_num += 1;
    }

    assert(iter_num == 3);

    const index_map<int, Data> mag2 = mag;

    auto it1 = mag.begin();
    auto it2 = mag2.begin();
    for(; it1 != mag.end() && it2 != mag2.end(); ++it1, ++it2){
        assert(it2->second.f1 == it1->second.f1);
        assert(it2->second.f2 == it1->second.f2);
        assert(it2->second.f3 == it1->second.f3);
    }

    assert(it1 == mag.end());
    assert(it2 == mag2.end());
}

void test_empty() {
    index_map<int, int> numbers;
    assert(numbers.empty());
 
    numbers.insert(std::make_pair(13317, 123));  
    assert(!numbers.empty());
}

void test_size() {
    index_map<int,char> nums {{1, 'a'}, {3, 'b'}, {5, 'c'}, {7, 'd'}};
    assert(nums.size() == 4);

    nums.erase(1);
    assert(nums.size() == 3);

  index_map<uint64_t, Data> m;
  unsigned int size = 0;
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
  for (auto it = m.begin(); it != m.end(); ++it) {
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
  assert(m.begin() == m.end());
}

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

  index_map<int, std::string> dict = {{1, "one"}, {2, "two"}};
  dict.insert({3, "three"});
  dict.insert(std::make_pair(4, "four"));
  dict.insert({{4, "another four"}, {5, "five"}});
  assert(dict[1] == "one");
  assert(dict[2] == "two");
  assert(dict[3] == "three");
  assert(dict[4] == "four");
  assert(dict[5] == "five");
}

void test_emplace() {
    index_map<int, std::string> m;
 
    m.emplace(std::make_pair(1, std::string("a")));
 
    m.emplace(std::make_pair(2, "b"));
 
    m.emplace(3, "c");

    assert(m.size() == 3);
    assert(m[1] == "a");
    assert(m[2] == "b");
    assert(m[3] == "c");
}

void test_erase() {
    index_map<int, std::string> c = {{1, "one"}, {2, "two"}, {3, "three"},
                                     {4, "four"}, {5, "five"}, {6, "six"}};
    // erase all odd numbers from c
    for(auto it = c.begin(); it != c.end(); ++it)
        if(it->first % 2 == 1)
            it = c.erase(it);
        else
            ++it;
    assert(c.size() == 3);
    for(auto& p : c) {
        assert(p.first % 2 == 0);
    }
}

void test_swap() {
    index_map<uint64_t, Data> m1 = {{1, {1, 2, 3}}, {2, {4, 5, 6}}};
    index_map<uint64_t, Data> m2 = {{1, {1, 2, 3}}};
    m1.swap(m2);
    assert(m1.size() == 1);
    assert(m2.size() == 2);
    assert(m1[1].f1 == 1);
    assert(m1[1].f2 == 2);
    assert(m1[1].f3 == 3);
    assert(m2[1].f1 == 1);
    assert(m2[1].f2 == 2);
    assert(m2[1].f3 == 3);
    assert(m2[2].f1 == 4);
    assert(m2[2].f2 == 5);
    assert(m2[2].f3 == 6);
}

void test_at() {
    index_map<uint64_t, Data> m = {{1, {1, 2, 3}}, {2, {4, 5, 6}}};
    assert(m.at(1) == Data(1, 2, 3));
    assert(m.at(2) == Data(4, 5, 6));
    try {
        m.at(3);
    } catch (std::out_of_range &e) {
    }
}

void test_index() {
  index_map<uint64_t, Data> m;
  uint64_t keys[] = {(uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand()};
  Data values[] = {Data(rand(),rand(),rand()), Data(rand(),rand(),rand()), Data()};

  for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    m.insert(std::make_pair(keys[i], values[i]));
  }

  for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    assert(m[keys[i]] == values[i]);
    m[keys[i]].f1 += 1;
    m[keys[i]].f2 += 2;
    m[keys[i]].f3 += 3;
  }

  for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    assert(m[keys[i]].f1 == values[i].f1 + 1);
    assert(m[keys[i]].f2 == values[i].f2 + 2);
    assert(m[keys[i]].f3 == values[i].f3 + 3);
  }
}

void test_count() {
    index_map<uint64_t, Data> m = {{1, {1, 2, 3}}, {2, {4, 5, 6}}};
    assert(m.count(1) == 1);
    assert(m.count(2) == 1);
    assert(m.count(3) == 0);
}

void test_find() {
    index_map<int,char> example = {{1,'a'},{2,'b'}};
 
    auto search = example.find(2);
    assert(search->first == 2);
    assert(search->second == 'b');

    auto notfound = example.find(100);
    assert(notfound == example.end());
}

void test_bucket_count() {
    index_map<uint64_t, Data> m = {{1, {1, 2, 3}}, {2, {4, 5, 6}}};
    assert(m.bucket_count() > 0);
    assert(m.max_bucket_count() > m.bucket_count());
    assert(m.bucket_size(0) <= 1);
}

void test_equal_range() {
    index_map<uint64_t, char> map = {{1,'a'},{1,'b'},{1,'d'},{2,'b'}};
    auto range = map.equal_range(1);
    for (auto it = range.first; it != range.second; ++it) {
        assert(it->first == 1);
        assert(it->second == 'a');
    }
}

void test_enumerate() {
  index_map<uint64_t, Data> m;
  uint64_t keys[] = {(uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand(), (uint64_t)rand() << 32 | rand()};
  Data values[] = {Data(rand(),rand(),rand()), Data(rand(),rand(),rand()), Data()};

  uint64_t keys_sum = 0;
  float values_sum[3] = { 0 };
  for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
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

void test_equal() {
    index_map<uint64_t, Data> m1 = {{1, {1, 2, 3}}, {2, {4, 5, 6}}};
    index_map<uint64_t, Data> m2 = {{1, {1, 2, 3}}};
    index_map<uint64_t, Data> m3(m1);
    assert(m1 == m3);
    assert(m1 != m2);
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
  test_constructor();
  test_assign();
  test_begin_end();
  test_empty();
  test_size();
  test_clear();
  test_insert();
  test_emplace();
  test_erase();
  test_swap();
  test_at();
  test_index();
  test_count();
  test_find();
  test_bucket_count();
  test_equal_range();
  test_enumerate();
  test_equal();

  compare_unordered_map();

  return 0;
}
