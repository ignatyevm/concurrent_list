#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>

TEST(ListTest, PushBack) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = random_int_vector(n);
    for (int v : values) {
        list.push_back(v);
    }
    check_content(values, list);
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, PushFront) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = random_int_vector(n);
    for (int v : values) {
        list.push_front(v);
    }
    std::reverse(values.begin(), values.end());
    check_content(values, list);
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, Insert) {
    int n = 10000;
    std::vector<int> values;
    values.reserve(n);
    polyndrom::acid_list<int> list;
    list.push_back(1);
    values.push_back(1);
    for (int i = 0; i < n - 1; i++) {
        int v = int_generator::random_int();
        int index = int_generator::random_int(0, (int) list.size() - 1);
        auto it1 = std::next(list.begin(), index);
        auto it2 = std::next(values.begin(), index);
        list.insert(it1, v);
        values.insert(it2, v);
    }
    check_content(values, list);
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, Erase) {
    int n = 10000;
    int m = 10000;
    polyndrom::acid_list<int> list;
    auto values = random_int_vector(n);
    for (int v : values) {
        list.push_back(v);
    }
    for (int i = 0; i < m; i++) {
        int k = int_generator::random_int(0, (int) values.size() - 1);
        auto it1 = std::next(values.begin(), k);
        auto it2 = std::next(list.begin(), k);
        EXPECT_EQ(*it1, *it2);
        it1 = values.erase(it1);
        it2 = list.erase(it2);
        if (it1 != values.end()) {
            EXPECT_EQ(*it1, *it2);
        }
        check_content(values, list);
    }
    EXPECT_EQ(values.size(), list.size());
}

TEST(ListTest, IteratorDirectAdvance) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = random_int_vector(n);
    for (int v : values) {
        list.insert(list.end(), v);
    }
    auto it1 = list.begin();
    for (auto it2 = values.begin(); it2 != values.end(); ++it2) {
        EXPECT_EQ(*it1, *it2);
        ++it1;
    }
}

TEST(ListTest, IteratorReverseAdvance) {
    int n = 100000;
    polyndrom::acid_list<int> list;
    auto values = random_int_vector(n);
    for (int v : values) {
        list.insert(list.end(), v);
    }
    auto it1 = std::next(list.begin(), n - 1);
    for (auto it2 = values.rbegin(); it2 != values.rend(); ++it2) {
        EXPECT_EQ(*it1, *it2);
        --it1;
    }
}