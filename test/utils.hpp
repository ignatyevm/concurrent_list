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
    for (auto value: container) {
        ++result[value];
    }
    return result;
}

template <class Container, class FwdIt>
auto NextCircularIt(const Container& container, int step, FwdIt prev_it) {
    auto it = prev_it;
    int step_abs = step > 0 ? step : -step;
    for (int i = 0; i < step_abs; i++) {
        if (container.size() == 0) {
            continue;
        }
        if (step > 0) {
            if (it == container.end()) {
                it = container.begin();
            }
            ++it;
        } else {
            if (it == container.begin()) {
                it = container.end();
            }
            --it;
        }
    }
    return it;
}

template <class Container, class FwdIt>
auto RandomIterator(const Container& container, FwdIt prev_it) {
    static thread_local std::mt19937 engine(std::random_device{}());
    static thread_local std::uniform_int_distribution dist(-100, 100);
    return NextCircularIt(container, dist(engine), prev_it);
}

template <class Container>
auto MakeIteratorsVector(const Container& container, int n) {
    assert(n <= container.size());
    std::vector<typename Container::iterator> its;
    its.reserve(container.size());
    for (auto it = container.begin(); it != container.end(); it++) {
        its.push_back(it);
    }
    return its;
}

template <class Container>
auto MakeRandomIteratorsVector(const Container& container, int n) {
    assert(n <= container.size());
    std::vector<typename Container::iterator> its = MakeIteratorsVector(container, n);
    std::mt19937 engine(std::random_device{}());
    std::shuffle(its.begin(), its.end(), engine);
    its.resize(n);
    return its;
}

template <class Container>
bool IsContainsUnique(const Container& container) {
    using ValueType = typename Container::value_type;
    std::map<ValueType, size_t> counts;
    std::for_each(container.begin(), container.end(), [&counts](const ValueType& value) {
        ++counts[value];
    });
    return std::all_of(counts.begin(), counts.end(), [](const std::pair<ValueType, size_t> value_count) {
        return value_count.second == 1;
    });
}
