#include "utils.hpp"

#include <vector>
#include <random>
#include <algorithm>
#include <map>
#include <thread>
#include <cassert>

std::vector<int> RandomIntVector(int n) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> d(std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max());
    std::vector<int> values(n);
    std::generate(values.begin(), values.end(), [&mt, &d] {
        return d(mt);
    });
    return values;
}

WorkerPool::WorkerPool(size_t pool_size) : pool_size_(pool_size) {
    pool_.reserve(pool_size);
    workers_.reserve(pool_size);
}

void WorkerPool::SubmitWorker(const Worker& worker) {
    assert(pool_.size() + 1 <= pool_size_);
    workers_.push_back(worker);
}

void WorkerPool::Run() {
    assert(workers_.size() == pool_size_);
    for (auto& worker : workers_) {
        pool_.emplace_back(worker);
    }
}

void WorkerPool::Join() {
    for (auto& thread : pool_) {
        thread.join();
    }
}
