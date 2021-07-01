#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>

std::vector<int> prepare_data_for_test(int n) {
    std::vector<int> values;
    values.reserve(n);
    for (int i = 0; i < n; i++) {
        values.push_back(random_int());
    }
    return values;
}

template <class C1, class C2>
void check_content(const C1& c1, const C2& c2) {
    auto it1 = c1.begin();
    auto it2 = c2.begin();
    while (it1 != c1.end()) {
        EXPECT_EQ(*it1, *it2);
        ++it1;
        ++it2;
    }
}

TEST(ListTest, PushBack) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = prepare_data_for_test(n);
    for (int v : values) {
        list.push_back(v);
    }
    check_content(values, list);
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, PushFront) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = prepare_data_for_test(n);
    for (int v : values) {
        list.push_front(v);
    }
    std::reverse(values.begin(), values.end());
    check_content(values, list);
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, Insert) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = prepare_data_for_test(n);
    for (int v : values) {
        list.insert(list.end(), v);
    }
    check_content(values, list);
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, Erase) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = prepare_data_for_test(n);
    for (int v : values) {
        list.push_back(v);
    }
    int k = 0;
    for (auto it = list.begin(); it != list.end(); it++) {
        auto next_it = list.erase(it);
        if (next_it == list.end()) {
            break;
        }
        EXPECT_EQ(*next_it, values[k + 1]);
        ++k;
    }
    EXPECT_EQ(list.size(), 0);
}

TEST(ListTest, IteratorReverseAdvance) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = prepare_data_for_test(n);
    for (int v : values) {
        list.insert(list.end(), v);
    }

}