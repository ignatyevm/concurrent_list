#pragma once

#include <vector>
#include <map>
#include <thread>
#include <functional>
#include <random>
#include <cassert>

using Worker = std::function<void()>;

std::vector<int> RandomIntVector(int n);

std::vector<int> RandomUniqueIntVector(int n);

class WorkerPool {
public:
    explicit WorkerPool(size_t pool_size);
    void SubmitWorker(const Worker& worker);
    void Run();
    void Join();
private:
    size_t pool_size_;
    std::vector<std::thread> pool_;
    std::vector<Worker> workers_;
};

template <class IntContainer>
std::map<int, int> CountUniqueValues(const IntContainer& container) {
    std::map<int, int> result;
    for (int value: container) {
        ++result[value];
    }
    return result;
}

template <class Container>
auto NextCircularIt(const Container& container, int step) {
    auto it = container.begin();
    for (int i = 0; i < step; i++) {
        if (it == container.end()) {
            it = container.begin();
        }
        ++it;
    }
    return it;
}

template <class Container>
auto RandomIterator(const Container& container) {
    static thread_local std::mt19937 engine(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, container.size());
    return NextCircularIt(container, dist(engine));
}

template <class Container>
auto RandomIteratorsVector(const Container& container, int n) {
    assert(n <= container.size());
    std::vector<typename Container::iterator> its;
    its.reserve(container.size());
    for (auto it = container.begin(); it != container.end(); it++) {
        its.push_back(it);
    }
    std::mt19937 engine(std::random_device{}());
    std::shuffle(its.begin(), its.end(), engine);
    its.resize(n);
    return its;
}

template <class Data, class F>
auto SplitTaskToWorkers(const Data& data, int workers_count, const F& task) {
    std::vector<std::function<void()>> workers;
    workers.reserve(workers_count);
    int size_per_worker = static_cast<int>(data.size()) / workers_count;
    for (int i = 0; i < workers_count; i++) {
        auto first = data.begin() + size_per_worker * i;
        auto last = i == workers_count - 1 ? data.end() : first + size_per_worker;
        workers.push_back([&task, first, last] () {
            for (auto it = first; it != last; it++) {
                task(*it);
            }
        });
    }
    return workers;
}
