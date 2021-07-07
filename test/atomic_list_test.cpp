#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <atomic>

using iterator = typename polyndrom::acid_list<int>::iterator;

TEST(AtomicListTest, SimpleDataRace_Move_SameIterator) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);
    auto it = list.begin();
    auto it_move_func = [](iterator& it) {
        ++it;
    };
    std::thread move_thread1(it_move_func, std::ref(it));
    std::thread move_thread2(it_move_func, std::ref(it));
    std::thread move_thread3(it_move_func, std::ref(it));
    move_thread1.join();
    move_thread2.join();
    move_thread3.join();
    EXPECT_EQ(*it, 4);
}

TEST(AtomicListTest, SimpleDataRace_Insert_SameIterator) {
    polyndrom::acid_list<int> list;
    auto it = list.end();
    auto insert_func = [](polyndrom::acid_list<int>& list, iterator it, iterator& res_it, int v) {
        res_it = list.insert(it, v);
    };
    iterator insert_res_it1, insert_res_it2, insert_res_it3;
    std::thread insert_thread1(insert_func, std::ref(list), it, std::ref(insert_res_it1), 1);
    std::thread insert_thread2(insert_func, std::ref(list), it, std::ref(insert_res_it2), 2);
    std::thread insert_thread3(insert_func, std::ref(list), it, std::ref(insert_res_it3), 3);
    insert_thread1.join();
    insert_thread2.join();
    insert_thread3.join();
    EXPECT_EQ(*insert_res_it1, 1);
    EXPECT_EQ(*insert_res_it2, 2);
    EXPECT_EQ(*insert_res_it3, 3);
}

TEST(AtomicListTest, SimpleDataRace_Erase_SameIterator) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    auto it = list.begin();
    auto erase_func = [](polyndrom::acid_list<int>& list, iterator it) {
        list.erase(it);
    };
    std::thread erase_thread1(erase_func, std::ref(list), it);
    std::thread erase_thread2(erase_func, std::ref(list), it);
    std::thread erase_thread3(erase_func, std::ref(list), it);
    erase_thread1.join();
    erase_thread2.join();
    erase_thread3.join();
    EXPECT_TRUE(++it == list.end());
}

TEST(AtomicListTest, SimpleDataRace_Erase_DifferentIterator) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    auto it1 = list.begin();
    auto it2 = std::next(it1);
    auto it3 = std::next(it2);
    auto erase_func = [](polyndrom::acid_list<int>& list, iterator it) {
        list.erase(it);
    };
    std::thread erase_thread1(erase_func, std::ref(list), it1);
    std::thread erase_thread2(erase_func, std::ref(list), it2);
    std::thread erase_thread3(erase_func, std::ref(list), it3);
    erase_thread1.join();
    erase_thread2.join();
    erase_thread3.join();
    EXPECT_TRUE(++it1 == list.end());
    EXPECT_TRUE(++it2 == list.end());
    EXPECT_TRUE(++it3 == list.end());
}

TEST(AtomicListTest, SimpleDataRace_MoveInsertErase_SameIterator) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    auto it = list.begin();

    auto it_move_func = [](polyndrom::acid_list<int>& list, iterator it, iterator& res_it) {
        res_it = ++it;
    };
    auto insert_func = [](polyndrom::acid_list<int>& list, iterator it, iterator& res_it) {
        res_it = list.insert(it, 2);
    };
    auto erase_func = [](polyndrom::acid_list<int>& list, iterator it, iterator& res_it) {
        res_it = list.erase(it);
    };

    iterator move_res_it;
    iterator insert_res_it;
    iterator erase_res_it;

    std::thread it_move_thread(it_move_func, std::ref(list), it, std::ref(move_res_it));
    std::thread insert_thread(insert_func, std::ref(list), it, std::ref(insert_res_it));
    std::thread erase_thread(erase_func, std::ref(list), it, std::ref(erase_res_it));

    it_move_thread.join();
    insert_thread.join();
    erase_thread.join();

    EXPECT_TRUE(move_res_it == list.end() || *move_res_it == 2);
    EXPECT_EQ(*insert_res_it, 2);
    EXPECT_TRUE(erase_res_it == list.end() || *erase_res_it == 2);
}

TEST(AtomicListTest, ConcurrentPushBack) {
    int n = 100000;
    int m = 4;
    std::vector<int> values = random_unique_int_vector(n);
    polyndrom::acid_list<int> list;
    auto f = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.push_back(*it);
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(m);
    int p = n / m;
    for (int i = 0; i < m; i++) {
        threads.emplace_back(std::thread(f, std::ref(list), values.begin() + i * p, values.begin() + (i + 1) * p));
    }
    for (auto& t : threads) {
        t.join();
    }
    std::set<int> checker(list.begin(), list.end());
    for (int v : values) {
        EXPECT_TRUE(checker.count(v) == 1);
    }
    EXPECT_EQ(list.size(), n);
}

TEST(AtomicListTest, ConcurrentPushFront) {
    int n = 100000;
    int t = 4;
    std::vector<int> values = random_unique_int_vector(n);
    polyndrom::acid_list<int> list;
    auto f = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.push_front(*it);
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(t);
    int p = n / t;
    for (int i = 0; i < t; i++) {
        threads.emplace_back(std::thread(f, std::ref(list), values.begin() + i * p, values.begin() + (i + 1) * p));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    std::set<int> checker(list.begin(), list.end());
    for (int v : values) {
        EXPECT_TRUE(checker.count(v) == 1);
    }
    EXPECT_EQ(list.size(), n);
}

TEST(AtomicListTest, ConcurrentInsert) {
    int n = 100000;
    int m = 200000;
    int t = 4;
    std::vector<int> default_values = random_unique_int_vector(n);
    std::vector<int> inserted_values = random_unique_int_vector(m, n);
    std::set<int> values;
    values.insert(default_values.begin(), default_values.end());
    values.insert(inserted_values.begin(), inserted_values.end());
    polyndrom::acid_list<int> list;
    for (int v : default_values) {
        list.push_back(v);
    }
    std::vector<iterator> random_its = random_iterators_vector(list, m);
    std::vector<std::pair<iterator, int>> its;
    its.reserve(m);
    for (int i = 0; i < m; i++) {
        its.emplace_back(random_its[i], inserted_values[i]);
    }
    auto f = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.insert(it->first, it->second);
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(t);
    int p = m / t;
    for (int i = 0; i < t; i++) {
        if (i == t - 1) {
            threads.emplace_back(std::thread(f, std::ref(list), its.begin() + i * p, its.end()));
        } else {
            threads.emplace_back(std::thread(f, std::ref(list), its.begin() + i * p, its.begin() + (i + 1) * p));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    std::set<int> checker(list.begin(), list.end());
    EXPECT_EQ(values, checker);
    EXPECT_EQ(list.size(), m + n);
}

TEST(AtomicListTest, ConcurrentEraseAll) {
    int n = 100000;
    int t = 4;
    std::vector<int> values = random_unique_int_vector(n);
    std::set<int> checker(values.begin(), values.end());
    polyndrom::acid_list<int> list;
    for (int v : values) {
        list.push_back(v);
    }
    std::vector<iterator> its;
    its.reserve(3 * n);
    for (auto it = list.begin(); it != list.end(); ++it) {
        for (int i = 0; i < 3; i++) {
            its.push_back(it);
        }
    }
    std::random_shuffle(its.begin(), its.end(), [](int n) {
        return int_generator::random_int(0, n - 1);
    });
    auto f = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.erase(*it);
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(t);
    int p = n / t;
    for (int i = 0; i < t; i++) {
        if (i == t - 1) {
            threads.emplace_back(std::thread(f, std::ref(list), its.begin() + i * p, its.end()));
        } else {
            threads.emplace_back(std::thread(f, std::ref(list), its.begin() + i * p, its.begin() + (i + 1) * p));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    for (auto it : its) {
        EXPECT_TRUE(checker.count(*it) == 1);
    }
    EXPECT_EQ(list.size(), 0);
}

TEST(AtomicListTest, ConcurrentEraseRandom) {
    int n = 100000;
    int t = 4;
    std::vector<int> values = random_unique_int_vector(n);
    std::set<int> checker(values.begin(), values.end());
    polyndrom::acid_list<int> list;
    for (int v : values) {
        list.push_back(v);
    }
    std::vector<iterator> its;
    its.reserve(n - 1000);
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it >= 1000) {
            its.push_back(it);
        }
    }
    std::random_shuffle(its.begin(), its.end(), [](int n) {
        return int_generator::random_int(0, n - 1);
    });
    auto f = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.erase(*it);
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(t);
    int p = n / t;
    for (int i = 0; i < t; i++) {
        if (i == t - 1) {
            threads.emplace_back(std::thread(f, std::ref(list), its.begin() + i * p, its.end()));
        } else {
            threads.emplace_back(std::thread(f, std::ref(list), its.begin() + i * p, its.begin() + (i + 1) * p));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    for (auto it : its) {
        EXPECT_TRUE(checker.count(*it) == 1);
    }
    for (int v : list) {
        EXPECT_LT(v, 1000);
    }
    EXPECT_EQ(list.size(), 1000);
}

TEST(AtomicListTest, ConcurrentClear) {
    int n = 100000;
    std::vector<int> values = random_unique_int_vector(n);
    polyndrom::acid_list<int> list;
    for (int v : values) {
        list.push_back(v);
    }
    auto f = [](polyndrom::acid_list<int>& list) {
        list.clear();
    };
    std::thread t1(f, std::ref(list));
    std::thread t2(f, std::ref(list));
    std::thread t3(f, std::ref(list));
    t1.join();
    t2.join();
    t3.join();
    EXPECT_EQ(list.size(), 0);
}

TEST(AtomicListTest, ConcurrentInsertEraseFind) {
    int n = 20000;
    int m = 19500;
    int insert_threads = 3;
    int erase_threads = 3;
    int find_threads = 2;
    std::vector<int> values = random_unique_int_vector(n);
    polyndrom::acid_list<int> list;
    for (int v : values) {
        list.push_back(v);
    }
    std::vector<int> items_for_find(n - m);
    std::iota(items_for_find.begin(), items_for_find.end(), m);
    std::vector<std::pair<iterator, int>> its_for_insert;
    std::vector<iterator> its_for_erase;
    its_for_insert.reserve(m);
    its_for_erase.reserve(m);
    int k = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it < m) {
            its_for_insert.emplace_back(it, n + k);
            its_for_erase.push_back(it);
            ++k;
        }
    }
    std::random_shuffle(its_for_insert.begin(), its_for_insert.end(), [](int n) {
        return int_generator::random_int(0, n - 1);
    });
    std::random_shuffle(its_for_erase.begin(), its_for_erase.end(), [](int n) {
        return int_generator::random_int(0, n - 1);
    });
    auto find_func = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            EXPECT_TRUE(std::find(list.begin(), list.end(), *it) != list.end());
        }
    };
    auto insert_func = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.insert(it->first, it->second);
        }
    };
    auto erase_func = [](polyndrom::acid_list<int>& list, auto first_it, auto last_it) {
        for (auto it = first_it; it != last_it; ++it) {
            list.erase(*it);
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(insert_threads + erase_threads + find_threads);
    int p1 = m / insert_threads;
    int p2 = m / erase_threads;
    for (int i = 0; i < insert_threads; i++) {
        if (i == insert_threads - 1) {
            threads.emplace_back(std::thread(insert_func, std::ref(list),
                                             its_for_insert.begin() + i * p1,
                                             its_for_insert.end()));
        } else {
            threads.emplace_back(std::thread(insert_func, std::ref(list),
                                             its_for_insert.begin() + i * p1,
                                             its_for_insert.begin() + (i + 1) * p1));
        }
    }
    for (int i = 0; i < erase_threads; i++) {
        if (i == erase_threads - 1) {
            threads.emplace_back(std::thread(erase_func, std::ref(list),
                                             its_for_erase.begin() + i * p2,
                                             its_for_erase.end()));
        } else {
            threads.emplace_back(std::thread(erase_func, std::ref(list),
                                             its_for_erase.begin() + i * p2,
                                             its_for_erase.begin() + (i + 1) * p2));
        }
    }
    for (int i = 0; i < find_threads; i++) {
        if (i < find_threads / 2) {
            threads.emplace_back(std::thread(find_func, std::ref(list), items_for_find.begin(), items_for_find.end()));
        } else {
            threads.emplace_back(std::thread(find_func, std::ref(list), items_for_find.rbegin(), items_for_find.rend()));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    for (int v : list) {
        EXPECT_LT(v,  n + m);
        EXPECT_GE(v, m);
    }
    EXPECT_EQ(list.size(), n);
}
