#pragma once

#include "fwd.hpp"
#include "list_node.hpp"
#include "list_iterator.hpp"

#include <utility>
#include <mutex>
#include <shared_mutex>

namespace polyndrom {

template<class T>
class acid_list {
private:
    using self_type = acid_list<T>;
    using node_ptr = detail::consistent_node_ptr<self_type>;

    friend node_ptr;
    friend list_iterator<self_type>;

    using read_lock = std::shared_lock<std::shared_mutex>;
    using write_lock = std::unique_lock<std::shared_mutex>;

public:
    using value_type = T;
    using iterator = list_iterator<self_type>;

    acid_list() : rw_mutex(), first(this, T()), last(this, T()) {
        first->next = last;
        last->prev = first;
    }

    template<typename U>
    void push_back(U&& value) {
        write_lock lock(rw_mutex);
        insert(last, std::forward<U>(value));
    }

    template<typename U>
    void push_front(U&& value) {
        write_lock lock(rw_mutex);
        insert(first->next, std::forward<U>(value));
    }

    template<typename U>
    iterator insert(iterator pos, U&& value) {
        write_lock lock(rw_mutex);
        node_ptr node = insert(pos.node, std::forward<U>(value));
        lock.unlock();
        return iterator(this, node);
    }

    iterator erase(iterator pos) {
        iterator next(pos);
        ++next;
        write_lock lock(rw_mutex);
        erase(pos.node);
        lock.unlock();
        return next;
    }

    iterator begin() const {
        read_lock lock(rw_mutex);
        node_ptr begin_node = first->next;
        return iterator(this, begin_node);
    }

    iterator end() const {
        read_lock lock(rw_mutex);
        node_ptr last_node = last;
        return iterator(this, last_node);
    }

    size_t size() const {
        read_lock lock(rw_mutex);
        return elements_count;
    }

    void clear() {
        write_lock lock(rw_mutex);
        node_ptr node = first->next;
        while (node != last) {
            erase(node);
            node = node->next;
        }
    }

    ~acid_list() {
        clear();
        first->next = nullptr;
        last->prev = nullptr;
    }

private:
    template<typename U>
    node_ptr insert(node_ptr pos, U&& value) {
        while (pos->is_deleted) {
            pos = pos->next;
        }
        ++elements_count;
        node_ptr node(this, std::forward<U>(value));
        node->next = pos;
        node->prev = pos->prev;
        pos->prev->next = node;
        pos->prev = node;
        return node;
    }

    void erase(node_ptr pos) {
        if (pos->is_deleted) {
            return;
        }
        --elements_count;
        pos->is_deleted = true;
        pos->next->prev = pos->prev;
        pos->prev->next = pos->next;
    }

private:
    mutable std::shared_mutex rw_mutex;
    node_ptr first;
    node_ptr last;
    size_t elements_count = 0;
};

} // polyndrom