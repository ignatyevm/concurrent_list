#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>
#include <memory>

using iterator = typename polyndrom::acid_list<int>::iterator;

TEST(ConsistentListTest, InvalidateAllDirect) {
    int n = 10000;
    polyndrom::acid_list<int> list;
    std::vector<iterator> its;
    its.reserve(n);
    for (int i = 0; i < n; i++) {
        list.push_back(i);
    }
    for (auto it = list.begin(); it != list.end(); ++it) {
        its.push_back(it);
    }
    list.clear();
    for (int i = 0; i < (int) its.size(); i++) {
        EXPECT_EQ(*its[i], i);
        ++its[i];
        EXPECT_TRUE(its[i] == list.end());
    }
}

TEST(ConsistentListTest, InvalidateAllReverse) {
    int n = 10000;
    polyndrom::acid_list<int> list;
    std::vector<iterator> its;
    its.reserve(n);
    for (int i = 0; i < n; i++) {
        list.push_back(i);
    }
    for (auto it = list.begin(); it != list.end(); ++it) {
        its.push_back(it);
    }
    list.clear();
    for (int i = 0; i < (int) its.size(); i++) {
        EXPECT_EQ(*its[i], i);
        --its[i];
        EXPECT_TRUE(++its[i] == list.end());
    }
}

TEST(ConsistentListTest, InvalidateRandomDirect) {
    int n = 10000;
    int m = 5000;
    polyndrom::acid_list<int> list;
    std::vector<std::pair<int, iterator>> its;
    its.reserve(n);
    auto values = random_unique_int_vector(n);
    for (int v : values) {
        list.push_back(v);
    }
    for (int i = 0; i < m; i++) {
        int k = int_generator::random_int(0, (int) values.size() - 1);
        its.emplace_back(values[k], std::next(list.begin(), k));
        list.erase(its.back().second);
        values.erase(values.begin() + k);
    }
    for (int i = 0; i < m; i++) {
        int value = its[i].first;
        auto it = its[i].second;
        EXPECT_EQ(*it, value);
        EXPECT_TRUE(std::find(values.begin(), values.end(), *it) == values.end());
        ++it;
        if (it != list.end()) {
            EXPECT_FALSE(std::find(values.begin(), values.end(), *it) == values.end());
        }
    }
}

TEST(ConsistentListTest, InvalidateRandomReverse) {
    int n = 10000;
    int m = 5000;
    polyndrom::acid_list<int> list;
    std::vector<std::pair<int, iterator>> its;
    its.reserve(n);
    auto values = random_unique_int_vector(n);
    for (int v : values) {
        list.push_back(v);
    }
    for (int i = 0; i < m; i++) {
        int k = int_generator::random_int(0, (int) values.size() - 1);
        its.emplace_back(values[k], std::next(list.begin(), k));
        list.erase(its.back().second);
        values.erase(values.begin() + k);
    }
    for (int i = 0; i < m; i++) {
        int value = its[i].first;
        auto it = its[i].second;
        EXPECT_EQ(*it, value);
        EXPECT_TRUE(std::find(values.begin(), values.end(), *it) == values.end());
        --it;
        if (it != --list.begin()) {
            EXPECT_FALSE(std::find(values.begin(), values.end(), *it) == values.end());
        }
    }
}
