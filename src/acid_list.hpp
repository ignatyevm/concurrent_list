#pragma once

#include "fwd.hpp"
#include "list_node.hpp"
#include "list_iterator.hpp"

#include <utility>
#include <memory>

namespace polyndrom {

template<class T>
class acid_list {
    using self_type = acid_list<T>;
    using node_ptr = node_pointer<self_type>;
public:
    using value_type = T;
    using iterator = list_iterator<self_type>;
    acid_list() : first(T()), last(T()) {
        first->next = last;
        last->prev = first;
    }
    template <typename U>
    void push_back(U&& value) {
        insert(last, std::forward<U>(value));
    }
    template <typename U>
    void push_front(U&& value) {
        insert(first->next, std::forward<U>(value));
    }
    template <typename U>
    iterator insert(iterator pos, U&& value) {
        return iterator(insert(pos.node, std::forward<U>(value)));
    }
    iterator erase(iterator pos) {
        iterator next(pos.node->next);
        erase(pos.node);
        return next;
    }
    iterator begin() const {
        return iterator(first->next);
    }
    iterator end() const {
        return iterator(last);
    }
    size_t size() const {
        return elements_count;
    }
    ~acid_list() {
        auto it = begin();
        while (it != end()) {
            it = erase(it);
        }
        first->next = nullptr;
        last->prev = nullptr;
    }
private:
    template <typename U>
    node_ptr insert(node_ptr pos, U&& value) {
        ++elements_count;
        node_ptr node(std::forward<U>(value));
        node->next = pos;
        node->prev = pos->prev;
        pos->prev->next = node;
        pos->prev = node;
        return node;
    }
    void erase(node_ptr pos) {
        --elements_count;
        pos->next->prev = pos->prev;
        pos->prev->next = pos->next;
    }
    node_ptr first;
    node_ptr last;
    size_t elements_count = 0;
};

} // polyndrom