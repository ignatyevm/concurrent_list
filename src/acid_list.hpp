#pragma once

#include "fwd.hpp"
#include "list_node.hpp"
#include "list_iterator.hpp"

#include <iostream>
#include <utility>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace polyndrom {

template<class T>
class acid_list {
    friend node_pointer<acid_list<T>>;
    friend list_iterator<acid_list<T>>;
    using self_type = acid_list<T>;
    using node_ptr = node_pointer<self_type>;
    using read_lock = std::shared_lock<std::shared_mutex>;
    using write_lock = std::unique_lock<std::shared_mutex>;
public:
    using value_type = T;
    using iterator = list_iterator<self_type>;
    acid_list() : rw_mutex(), node_mutex(), first(this, T()), last(this, T()) {
        first->next = last;
        last->prev = first;
    }
    template <typename U>
    void push_back(U&& value) {
        write_lock wlock(rw_mutex);
        insert(last, std::forward<U>(value));
    }
    template <typename U>
    void push_front(U&& value) {
        write_lock wlock(rw_mutex);
        insert(first->next, std::forward<U>(value));
    }
    template <typename U>
    iterator insert(iterator pos, U&& value) {
        write_lock wlock(rw_mutex);
        node_ptr node = insert(pos.node, std::forward<U>(value));
        wlock.unlock();
        read_lock rlock(rw_mutex);
        iterator res(this, node);
        rlock.unlock();
        return res;
    }
    iterator erase(iterator pos) {
        iterator next(this, pos.node->next);
        write_lock wlock(rw_mutex);
        erase(pos.node);
        wlock.unlock();
        return next;
    }
    iterator begin() const {
        read_lock rlock(rw_mutex);
        node_ptr begin_node = first->next;
        // rlock.unlock();
        return iterator(this, begin_node);
    }
    iterator end() const {
        read_lock rlock(rw_mutex);
        node_ptr last_node = last;
        // rlock.unlock();
        return iterator(this, last_node);
    }
    size_t size() const {
        read_lock rlock(rw_mutex);
        return elements_count;
    }
    void clear() {
        write_lock wlock(rw_mutex);
        node_ptr node = first->next;
        while (node != last) {
            erase(node);
            node = node->next;
        }
    }
    ~acid_list() {
        clear();
        // write_lock wlock(rw_mutex);
        first->next = nullptr;
        last->prev = nullptr;
    }
private:
    template <typename U>
    node_ptr insert(node_ptr pos, U&& value) {
        ++elements_count;
        node_ptr node(this, std::forward<U>(value));
        node->next = pos;
        node->prev = pos->prev;
        pos->prev->next = node;
        pos->prev = node;
        return node;
    }
    void erase(node_ptr pos) {
        --elements_count;
        pos->is_deleted = true;
        pos->next->prev = pos->prev;
        pos->prev->next = pos->next;
    }
    mutable std::shared_mutex rw_mutex;
    mutable std::recursive_mutex node_mutex;
    node_ptr first;
    node_ptr last;
    size_t elements_count = 0;
};

} // polyndrom