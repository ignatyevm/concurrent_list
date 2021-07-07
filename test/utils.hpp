#pragma once

#include "gtest/gtest.h"

#include <vector>
#include <random>
#include <climits>

class int_generator {
public:
    int_generator(int start = INT_MIN, int end = INT_MAX) : distribution(start, end) {}
    int next_value() {
        return distribution(device);
    }
    static inline int random_int(int start = INT_MIN, int end = INT_MAX) {
        int_generator generator(start, end);
        return generator.next_value();
    }
private:
    std::random_device device;
    std::uniform_int_distribution<int> distribution;
};

inline std::vector<int> random_int_vector(int n) {
    std::vector<int> values;
    values.reserve(n);
    for (int i = 0; i < n; i++) {
        values.push_back(int_generator::random_int());
    }
    return values;
}

inline std::vector<int> random_unique_int_vector(int n, int s = 0) {
    std::vector<int> values;
    values.reserve(n);
    for (int i = s; i < s + n; i++) {
        values.push_back(i);
    }
    std::random_shuffle(values.begin(), values.end(), [](int n) {
        return int_generator::random_int(0, n - 1);
    });
    return values;
}

template <class C1, class C2>
inline void check_content(const C1& c1, const C2& c2) {
    auto it1 = c1.begin();
    auto it2 = c2.begin();
    while (it1 != c1.end()) {
        EXPECT_EQ(*it1, *it2);
        ++it1;
        ++it2;
    }
}

template <class C>
typename C::iterator random_element(C& c) {
    int_generator generator(0, (int) c.size() - 1);
    auto it = c.begin();
    std::advance(it, generator.next_value());
    return it;
}

template <class C>
std::vector<typename C::iterator> random_iterators_vector(C& c, int n) {
    std::vector<typename C::iterator> its;
    its.reserve(n);
    auto it = c.begin();
    for (int i = 0; i < n; i++) {
        int step = int_generator::random_int(0, 10);
        for (int j = 0; j < step; j++) {
            ++it;
            if (it == c.end()) {
                it = c.begin();
            }
        }
        its.push_back(it);
    }
    return its;
}