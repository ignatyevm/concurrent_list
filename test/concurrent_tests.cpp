#include "acid_list.hpp"
#include "utils.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <random>
#include <barrier>

using iterator = typename polyndrom::acid_list<int64_t>::iterator;

TEST(ConcurrentListTest, ConcurrentInsert_SamePos) {
    const size_t threads_count = 4;
    const size_t data_per_thread = 100000;
    polyndrom::acid_list<int64_t> list;
    WorkerPool pool(threads_count);
    for (int i = 0; i < threads_count; i++) {
        const int64_t left_bound = data_per_thread * i;
        const int64_t right_bound = data_per_thread * (i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound]() {
           for (int64_t i = left_bound; i < right_bound; i++) {
               list.insert(list.end(), i);
           }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_TRUE(IsContainsUnique(list));
    EXPECT_EQ(list.size(), threads_count * data_per_thread);
}

TEST(ConcurrentListTest, ConcurrentErase_SameIterator) {
    const size_t data_size = 1000;
    const size_t threads_count = 4;
    polyndrom::acid_list<int64_t> list;
    for (size_t i = 0; i < data_size; i++) {
        list.push_back(i);
    }
    WorkerPool pool(threads_count);
    std::barrier barrier(threads_count);
    for (int i = 0; i < threads_count; i++) {
        pool.SubmitWorker([&list, it = list.begin(), &barrier]() mutable {
            while (it != list.end()) {
                list.erase(it++);
                barrier.arrive_and_wait();
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_EQ(list.size(), 0);
}

TEST(ConcurrentListTest, ConcurrentRandomInsert) {
    const size_t threads_count = 4;
    const size_t data_per_thread = 10000;
    polyndrom::acid_list<int64_t> list;
    WorkerPool pool(threads_count);
    for (size_t i = 0; i < threads_count; i++) {
        const int64_t left_bound = data_per_thread * i;
        const int64_t right_bound = data_per_thread * (i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound]() {
            auto pos = list.begin();
            for (int64_t value = left_bound; value < right_bound; value++) {
                list.insert(pos, value);
                pos = RandomIterator(list, pos);
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_TRUE(IsContainsUnique(list));
    EXPECT_EQ(list.size(), threads_count * data_per_thread);
}

TEST(ConcurrentListTest, ConcurrentRandomErase) {
    const size_t threads_count = 4;
    const size_t data_size = 500000;
    const size_t erased_data_size = 80 * data_size / 100;
    const size_t data_per_thread = erased_data_size / threads_count;
    polyndrom::acid_list<int64_t> list;
    for (size_t i = 0; i < data_size; i++) {
        list.push_back(i);
    }
    std::vector<iterator> positions = MakeRandomIteratorsVector(list, data_size);
    std::vector<int64_t> not_erased;
    std::transform(positions.begin() + erased_data_size, positions.end(), std::back_inserter(not_erased), [] (auto it) {
        return *it;
    });
    positions.resize(erased_data_size);
    WorkerPool pool(threads_count);
    for (size_t i = 0; i < threads_count; i++) {
        const size_t left_bound = data_per_thread * i;
        const size_t right_bound = data_per_thread * (i + 1);
        pool.SubmitWorker([&list, &positions, left_bound, right_bound]() {
            for (size_t i = left_bound; i != right_bound; i++) {
                list.erase(positions[i]);
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_TRUE(IsContainsUnique(list));
    EXPECT_EQ(list.size(), data_size - erased_data_size);
    std::sort(not_erased.begin(), not_erased.end());
    EXPECT_TRUE(std::equal(list.begin(), list.end(), not_erased.begin()));
}

TEST(ConcurrentListTest, SequentialInsert) {
    const size_t data_size = 400000;
    const size_t left_inserters_count = 2;
    const size_t right_inserters_count = 2;
    const size_t data_per_thread = data_size / (left_inserters_count + right_inserters_count);
    polyndrom::acid_list<int64_t> list;
    WorkerPool pool(left_inserters_count + right_inserters_count);
    for (int i = 0; i < left_inserters_count; i++) {
        const int64_t left_bound = data_per_thread * i;
        const int64_t right_bound = data_per_thread * (i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound]() {
            auto it = list.begin();
            for (int64_t value = left_bound; value != right_bound; value++) {
                list.insert(it, value);
                if (it == list.end()) {
                    it = list.begin();
                }
                it++;
            }
        });
    }
    for (int i = 0; i < right_inserters_count; i++) {
        const int64_t left_bound =  data_per_thread * (left_inserters_count + i);
        const int64_t right_bound = data_per_thread * (left_inserters_count + i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound]() {
            while (list.size() == 0);
            auto it = list.end();
            for (int64_t value = left_bound; value != right_bound; value++) {
                auto prev_it = std::prev(it);
                if (prev_it == std::prev(list.begin())) {
                    prev_it = list.end();
                }
                list.insert(it, value);
                it = prev_it;
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_EQ(list.size(), data_size);
    EXPECT_TRUE(IsContainsUnique(list));
    for (int64_t value : list) {
        EXPECT_TRUE(0 <= value && value < data_size);
    }
}

TEST(ConcurrentListTest, SequentialErase) {
    const size_t init_data_size = 100000;
    const size_t left_erasers_count = 2;
    const size_t right_erasers_count = 2;
    polyndrom::acid_list<int64_t> list;
    for (size_t i = 0; i < init_data_size; i++) {
        list.push_back(i);
    }
    WorkerPool pool(left_erasers_count + right_erasers_count);
    for (int i = 0; i < left_erasers_count; i++) {
        pool.SubmitWorker([&list]() {
            for (auto it = list.begin(); it != list.end(); it++) {
                list.erase(it);
            }
        });
    }
    for (int i = 0; i < right_erasers_count; i++) {
        pool.SubmitWorker([&list]() {
            for (auto it = std::prev(list.end()); it != std::prev(list.begin()); it--) {
                list.erase(it);
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_EQ(list.size(), 0);
}

TEST(ConcurrentListTest, SequentialInsertErase) {
    // 0 - left inserters count
    // 1 - right inserters count
    // 2 - left erasers count
    // 3 - right erasers count
    std::vector<size_t> threads_counts = {2, 2, 2, 2};
    const size_t init_data_size = 100000;
    const size_t inserted_data_size = 150000;
    const size_t inserted_data_per_thread = inserted_data_size / (threads_counts[0] + threads_counts[1]);
    polyndrom::acid_list<int64_t> list;
    for (size_t i = 0; i < init_data_size; i++) {
        list.push_back(i);
    }
    std::vector<iterator> positions = MakeIteratorsVector(list, init_data_size);
    WorkerPool pool(threads_counts[0] + threads_counts[1] + threads_counts[2] + threads_counts[3]);
    for (int i = 0; i < threads_counts[0]; i++) {
        const int64_t left_bound = init_data_size + inserted_data_per_thread * i;
        const int64_t right_bound = init_data_size + inserted_data_per_thread * (i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound]() {
            auto pos = list.begin();
            for (int64_t value = left_bound; value < right_bound; value++) {
                list.insert(pos, value);
                pos++;
                if (pos == list.end()) {
                    pos = list.begin();
                }
            }
        });
    }
    for (int i = 0; i < threads_counts[1]; i++) {
        const int64_t left_bound = init_data_size + inserted_data_per_thread * (threads_counts[0] + i);
        const int64_t right_bound = init_data_size + inserted_data_per_thread * (threads_counts[0] + i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound]() {
            auto pos = list.end();
            for (int64_t value = left_bound; value < right_bound; value++) {
                list.insert(pos, value);
                pos--;
                if (pos == std::prev(list.begin())) {
                    pos = list.end();
                }
            }
        });
    }
    for (int i = 0; i < threads_counts[2]; i++) {
        pool.SubmitWorker([&list, positions]() {
            for (auto pos = positions.begin(); pos != positions.end(); pos++) {
                list.erase(*pos);
            }
        });
    }
    for (int i = 0; i < threads_counts[3]; i++) {
        pool.SubmitWorker([&list, positions]() {
            for (auto pos = positions.rbegin(); pos != positions.rend(); pos++) {
                list.erase(*pos);
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_EQ(list.size(), inserted_data_size);
    EXPECT_TRUE(IsContainsUnique(list));
    for (int64_t value : list) {
        EXPECT_TRUE(init_data_size <= value && value < init_data_size + inserted_data_size);
    }
}

TEST(ConcurrentListTest, ParallelInsert) {
    const size_t threads_count = 4;
    const size_t data_per_thread = 100000;
    polyndrom::acid_list<int64_t> list;
    WorkerPool pool(threads_count);
    for (size_t i = 0; i < 2 * threads_count; i++) {
        list.push_back(-(static_cast<int>(i) + 1));
    }
    std::vector<iterator> positions;
    positions.reserve(threads_count);
    for (auto it = list.begin(); it != list.end(); it = std::next(it, 2)) {
        positions.push_back(it);
    }
    for (size_t i = 0; i < threads_count; i++) {
        const int64_t left_bound = data_per_thread * i;
        const int64_t right_bound = data_per_thread * (i + 1);
        pool.SubmitWorker([&list, left_bound, right_bound, pos = positions[i]]() {
            for (int64_t value = left_bound; value < right_bound; value++) {
                list.insert(pos, value);
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_TRUE(IsContainsUnique(list));
    EXPECT_EQ(list.size(), threads_count * data_per_thread + 2 * threads_count);
}

TEST(ConcurrentListTest, ParallelErase) {
    const size_t threads_count = 4;
    const size_t data_size = 400000;
    const size_t data_per_thread = data_size / threads_count;
    polyndrom::acid_list<int64_t> list;
    WorkerPool pool(threads_count);
    for (size_t i = 0; i < data_size + threads_count - 1; i++) {
        list.push_back(i);
    }
    std::vector<int64_t> borders;
    for (size_t i = 0; i < threads_count - 1; i++) {
        borders.push_back(data_per_thread * (i + 1) + i);
    }
    for (size_t i = 0; i < threads_count; i++) {
        auto first = std::next(list.begin(), data_per_thread * i + i);
        auto last = std::next(first, data_per_thread);
        pool.SubmitWorker([&list, first, last]() {
            for (auto it = first; it != last; it++) {
                list.erase(it);
            }
        });
    }
    pool.Run();
    pool.Join();
    EXPECT_EQ(list.size(), threads_count - 1);
    EXPECT_TRUE(std::equal(list.begin(), list.end(), borders.begin()));
}
