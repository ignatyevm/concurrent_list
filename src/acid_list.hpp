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

    acid_list() : first(T()), last(T()) {
        first->next = last;
        last->prev = first;
    }

    template<typename U>
    void push_back(U&& value) {
        insert(last, std::forward<U>(value));
    }

    template<typename U>
    void push_front(U&& value) {
        insert(first.locked_read_next(), std::forward<U>(value));
    }

    template<typename U>
    iterator insert(iterator pos, U&& value) {
        node_ptr node = insert(pos.node, std::forward<U>(value));
        return iterator(node);
    }

    iterator erase(iterator pos) {
        node_ptr node = erase_node(pos.node);
        return iterator(node);
    }

    iterator begin() const {
        return iterator(first.locked_read_next());
    }

    iterator end() const {
        return iterator(last);
    }

    int size() const {
        return elements_count;
    }

    void clear() {
        for (auto it = begin(); it != end(); it = erase(it));
    }

    ~acid_list() {
        clear();
        first->next = nullptr;
        last->prev = nullptr;
    }

private:
    template<typename U>
    node_ptr insert(node_ptr node, U&& value) {
        node_ptr new_node(std::forward<U>(value));
        while (true) {
            while (node->is_deleted) {
                node = node.locked_read_next();
            }

            node_ptr prev = node.locked_read_prev();

            write_lock prev_lock(prev->mutex);
            write_lock current_lock(node->mutex);

            if (node->is_deleted || node->prev != prev) {
                continue;
            }

            new_node->prev = prev;
            new_node->next = node;
            prev->next = new_node;
            node->prev = new_node;
            ++elements_count;
            return new_node;
        }
    }

    node_ptr erase_node(node_ptr node) {
        while (!node->is_deleted) {
            auto [prev, next] = node.locked_read_nodes();

            write_lock prev_lock(prev->mutex);
            read_lock current_lock(node->mutex);
            write_lock next_lock(next->mutex);

            if (node->is_deleted) {
                return last;
            }

            if (node->prev != prev || node->next != next) {
                continue;
            }

            node->is_deleted = true;
            next->prev = prev;
            prev->next = next;
            --elements_count;
            return next;
        }
        return last;
    }

private:
    node_ptr first;
    node_ptr last;
    std::atomic_int elements_count = 0;
};

} // polyndrom