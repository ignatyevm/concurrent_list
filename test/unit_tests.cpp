#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>
#include <random>

using iterator = typename polyndrom::acid_list<int>::iterator;

TEST(ListTest, SimplePushBack) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    EXPECT_EQ(*list.begin(), 1);
    EXPECT_EQ(*std::prev(list.end()), 3);
}

TEST(ListTest, SimplePushFront) {
    polyndrom::acid_list<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    EXPECT_EQ(*list.begin(), 3);
    EXPECT_EQ(*std::prev(list.end()), 1);
}

TEST(ListTest, SimpleInsert) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(3);
    list.insert(std::find(list.begin(), list.end(), 3), 2);
    std::initializer_list<int> values {1, 2, 3};
    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin()));
    EXPECT_EQ(list.size(), 3);
}

TEST(ListTest, SimpleErase) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(5);
    list.push_back(3);
    list.erase(std::find(list.begin(), list.end(), 5));
    std::initializer_list<int> values {1, 2, 3};
    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin()));
    EXPECT_EQ(list.size(), 3);
}

TEST(ListTest, PushBack) {
    int n = 10000;
    polyndrom::acid_list<int> list;
    auto values = RandomIntVector(n);
    for (int v : values) {
        list.push_back(v);
    }
    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin()));
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, PushFront) {
    int n = 10000;
    polyndrom::acid_list<int> list;
    auto values = RandomIntVector(n);
    for (int v : values) {
        list.push_front(v);
    }
    EXPECT_TRUE(std::equal(values.rbegin(), values.rend(), list.begin()));
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, Insert) {
    int n = 5000;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> value_distribution(std::numeric_limits<int>::min(),
                                                          std::numeric_limits<int>::max());
    std::uniform_int_distribution<int> step_distribution(0, n);

    std::vector<int> values;
    polyndrom::acid_list<int> list;

    values.push_back(1);
    list.push_back(1);

    for (int i = 0; i < n - 1; i++) {
        int step = step_distribution(mt) % static_cast<int>(list.size());
        int value = value_distribution(mt);
        list.insert(std::next(list.begin(), step), value);
        values.insert(std::next(values.begin(), step), value);
    }

    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin()));
    EXPECT_EQ(list.size(), n);
}

TEST(ListTest, Erase) {
    int n = 5000;
    int m = 3000;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> step_distribution(0, n);

    auto values = RandomIntVector(n);
    polyndrom::acid_list<int> list;
    std::copy(values.begin(), values.end(), std::back_inserter(list));

    for (int i = 0; i < m; i++) {
        int step = step_distribution(mt) % static_cast<int>(list.size());
        list.erase(std::next(list.begin(), step));
        values.erase(std::next(values.begin(), step));
    }

    EXPECT_TRUE(std::equal(values.begin(), values.end(), list.begin()));
    EXPECT_EQ(values.size(), list.size());
}

TEST(ListTest, IteratorDirectAdvance) {
    int n = 10000;
    polyndrom::acid_list<int> list;
    auto values = RandomIntVector(n);
    std::copy(values.begin(), values.end(), std::back_inserter(list));
    auto it1 = list.begin();
    for (auto it2 = values.begin(); it2 != values.end(); ++it2) {
        EXPECT_EQ(*it1, *it2);
        ++it1;
    }
}

TEST(ListTest, IteratorReverseAdvance) {
    int n = 10000;
    polyndrom::acid_list<int> list;
    auto values = RandomIntVector(n);
    std::copy(values.begin(), values.end(), std::back_inserter(list));
    auto it1 = std::prev(list.end());
    for (auto it2 = values.rbegin(); it2 != values.rend(); ++it2) {
        EXPECT_EQ(*it1, *it2);
        --it1;
    }
}

TEST(ConsistentListTest, SimpleInvalidate1) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    list.erase(it);
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(++it, list.end());
}

TEST(ConsistentListTest, SimpleInvalidate2) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    list.erase(it);
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(ConsistentListTest, SimpleInvalidate3) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    auto it1 = list.begin();
    auto it2 = std::next(it1);
    EXPECT_EQ(*it1, 1);
    EXPECT_EQ(*it2, 2);
    list.erase(it1);
    list.erase(it2);
    EXPECT_EQ(*it1, 1);
    EXPECT_EQ(*it2, 2);
    ++it1;
    EXPECT_EQ(*it1, 3);
    EXPECT_EQ(*it2, 2);
    ++it2;
    EXPECT_EQ(*it1, 3);
    EXPECT_EQ(*it2, 3);
}

TEST(ConsistentListTest, StackOverflowWhenRelease) {
    using value_type = std::tuple<int64_t, int64_t, int64_t, int64_t,
                                  int64_t, int64_t, int64_t, int64_t>;
    value_type value = {0, 0, 0, 0, 0, 0, 0, 0};
    int n = 200000;
    polyndrom::acid_list<value_type> list;
    for (int i = 0; i < n; i++) {
        list.push_back(value);
    }
    auto it = std::next(list.begin(), n / 2);
    {
        auto it = list.begin();
        auto end = list.end();
        while (it != end) {
            list.erase(it);
            ++it;
        }
    }
    std::destroy_at(&it);
}

TEST(ConsistentListTest, InvalidateAllDirect) {
    int n = 5000;
    polyndrom::acid_list<int> list;
    std::fill_n(std::back_inserter(list), n, 0);
    std::iota(list.begin(), list.end(), 0);
    std::vector<iterator> its;
    its.reserve(n);
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
    std::fill_n(std::back_inserter(list), n, 0);
    std::iota(list.begin(), list.end(), 0);
    std::vector<iterator> its;
    its.reserve(n);
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
    int n = 5000;
    int m = 3000;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> index_distribution(0, n - 1);

    auto values = RandomIntVector(n);
    std::vector<std::pair<int, bool>> values_history(n);
    std::transform(values.begin(), values.end(), values_history.begin(), [](int v) {
        return std::make_pair(v, false);
    });
    polyndrom::acid_list<int> list;
    std::copy(values.begin(), values.end(), std::back_inserter(list));
    std::vector<std::pair<iterator, decltype(values_history.begin())>> its;
    its.reserve(n);

    for (int i = 0; i < m; i++) {
        int k = index_distribution(mt);
        its.emplace_back(std::next(list.begin(), k), std::next(values_history.begin(), k));
    }

    for (auto [it1, it2] : its) {
        list.erase(it1);
        it2->second = true;
    }

    for (auto [it1, it2] : its) {
        EXPECT_EQ(*it1, it2->first);
        ++it1;
        auto first_not_removed = std::find_if(it2, values_history.end(), [](auto p) {
            return !p.second;
        });
        if (first_not_removed == values_history.end()) {
            EXPECT_EQ(it1, list.end());
        } else {
            EXPECT_EQ(*it1, first_not_removed->first);
        }
    }
}

TEST(ConsistentListTest, InvalidateRandomReverse) {
    int n = 5000;
    int m = 3000;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> index_distribution(0, n - 1);

    auto values = RandomIntVector(n);
    std::vector<std::pair<int, bool>> values_history(n);
    std::transform(values.begin(), values.end(), values_history.begin(), [](int v) {
        return std::make_pair(v, false);
    });
    polyndrom::acid_list<int> list;
    std::copy(values.begin(), values.end(), std::back_inserter(list));
    std::vector<std::pair<iterator, decltype(values_history.begin())>> its;
    its.reserve(n);

    for (int i = 0; i < m; i++) {
        int k = index_distribution(mt);
        its.emplace_back(std::next(list.begin(), k), std::next(values_history.begin(), k));
    }

    its.emplace_back(list.begin(), values_history.begin());

    for (auto [it1, it2] : its) {
        list.erase(it1);
        it2->second = true;
    }

    for (auto [it1, it2] : its) {
        EXPECT_EQ(*it1, it2->first);
        --it1;
        auto first_not_removed = std::find_if(std::make_reverse_iterator(it2), values_history.rend(), [](auto p) {
            return !p.second;
        });
        if (first_not_removed == values_history.rend()) {
            EXPECT_EQ(it1, std::prev(list.begin()));
        } else {
            EXPECT_EQ(*it1, first_not_removed->first);
        }
    }
}
