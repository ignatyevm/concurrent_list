#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <random>

using iterator = typename polyndrom::acid_list<int>::iterator;

TEST(ConcurrentListTest, SimpleDataRace_ConcurrentInsert) {
    polyndrom::acid_list<int> list;
    auto inserter = [&list](int value) {
        list.insert(list.end(), value);
    };
    std::thread t1(inserter, 1);
    std::thread t2(inserter, 2);
    t1.join();
    t2.join();
    std::map<int, int> expected = {{1, 1}, {2, 1}};
    std::map<int, int> actual;
    std::for_each(list.begin(), list.end(), [&actual](int v) {
        ++actual[v];
    });
    EXPECT_EQ(expected, actual);
}

TEST(ConcurrentListTest, SimpleDataRace_ConcurrentErase_SameIterator) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    auto eraser = [&list](iterator it) {
        list.erase(it);
    };
    auto it = list.begin();
    std::thread t1(eraser, it);
    std::thread t2(eraser, it);
    t1.join();
    t2.join();
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(list.size(), 0);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(ConcurrentListTest, SimpleDataRace_ConcurrentErase_NearIterators) {
    polyndrom::acid_list<int> list;
    list.push_back(1);
    list.push_back(2);
    auto eraser = [&list](iterator it) {
        list.erase(it);
    };
    auto it1 = list.begin();
    auto it2 = std::next(it1);
    std::thread t1(eraser, it1);
    std::thread t2(eraser, it2);
    t1.join();
    t2.join();
    EXPECT_EQ(*it1, 1);
    EXPECT_EQ(*it2, 2);
    EXPECT_EQ(list.size(), 0);
    ++it1;
    ++it2;
    EXPECT_EQ(it1, list.end());
    EXPECT_EQ(it2, list.end());
}

TEST(ConcurrentListTest, ConcurrentInsert_SamePos) {
    const int data_size = 10000;
    std::vector<int> inserted_data = RandomIntVector(data_size);

    polyndrom::acid_list<int> list;
    auto inserter = [&list] (auto value) {
        list.insert(list.end(), value);
    };

    const int workers_count = 16;
    WorkerPool pool(workers_count);
    for (auto& worker : SplitTaskToWorkers(inserted_data, workers_count, inserter)) {
        pool.SubmitWorker(worker);
    }
    pool.Run();
    pool.Join();

    EXPECT_EQ(CountUniqueValues(inserted_data), CountUniqueValues(list));
}

TEST(ConcurrentListTest, ConcurrentRandomInsert) {
    const int data_size = 10000;
    std::vector<int> inserted_data = RandomIntVector(data_size);

    polyndrom::acid_list<int> list;
    auto inserter = [&list] (auto value) {
        list.insert(RandomIterator(list), value);
    };

    const int workers_count = 16;
    WorkerPool pool(workers_count);
    for (auto& worker : SplitTaskToWorkers(inserted_data, workers_count, inserter)) {
        pool.SubmitWorker(worker);
    }
    pool.Run();
    pool.Join();

    EXPECT_EQ(CountUniqueValues(inserted_data), CountUniqueValues(list));
}

TEST(ConcurrentListTest, ConcurrentRandomErase) {
    const int data_size = 10000;
    const int erased_data_size = 8000;
    std::vector<int> default_data = RandomUniqueIntVector(data_size);

    polyndrom::acid_list<int> list;
    auto eraser = [&list](auto it) {
        list.erase(it);
    };
    std::copy(default_data.begin(), default_data.end(), std::back_inserter(list));

    auto its_for_erase = RandomIteratorsVector(list, erased_data_size);

    const int workers_count = 16;
    WorkerPool pool(workers_count);
    for (auto& worker : SplitTaskToWorkers(its_for_erase, workers_count, eraser)) {
        pool.SubmitWorker(worker);
    }
    pool.Run();
    pool.Join();

    std::map<int, int> expected = CountUniqueValues(default_data);
    for (auto it : its_for_erase) {
        auto other_it = expected.find(*it);
        if (--other_it->second == 0) {
            expected.erase(other_it);
        }
    }
    auto actual = CountUniqueValues(list);
    EXPECT_EQ(actual, expected);
}
