#pragma once

#include "fwd.hpp"

#include <cstddef>
#include <utility>
#include <atomic>
#include <mutex>

template <class List>
class node_type {
public:
    node_type() = default;
    using value_type = typename List::value_type;
    template <typename U>
    explicit node_type(U&& value) : value(std::forward<U>(value)) {}
    node_pointer<List> prev = nullptr;
    node_pointer<List> next = nullptr;
    value_type value;
    std::atomic_size_t ref_count = 0;
    std::atomic_bool is_deleted = false;
};

template <class List>
class node_pointer {
public:
    using list_type = const List;
    using value_type = typename List::value_type;
    node_pointer() = default;
    template <typename T>
    explicit node_pointer(list_type* list, T&& value) {
        this->list = list;
        acquire(new node_type<List>(std::forward<T>(value)));
        owned_node->next.list = list;
        owned_node->prev.list = list;
    }
    node_pointer(const node_pointer& other) {
        list = other.list;
        acquire(other.owned_node);
    }
    node_pointer& operator=(const node_pointer& other) {
        if (owned_node == other.owned_node) {
            return *this;
        }
        list = other.list;
        node_type<List>* new_node = other.owned_node;
        release();
        acquire(new_node);
        return *this;
    }
    node_pointer(std::nullptr_t) {}
    node_pointer& operator=(std::nullptr_t) {
        release();
        return *this;
    }
    node_type<List>* operator->() const {
        return owned_node;
    }
    bool operator==(const node_pointer& rhs) const {
        return owned_node == rhs.owned_node;
    }
    bool operator!=(const node_pointer& rhs) const {
        return owned_node != rhs.owned_node;
    }
    ~node_pointer() {
        release();
    }
    void acquire(node_type<List>* node) {
        owned_node = node;
        if (owned_node != nullptr) {
            owned_node->ref_count += 1;
        }
    }
    void release() {
        if (owned_node != nullptr) {
            owned_node->ref_count -= 1;
            if (owned_node->ref_count == 0) {
                delete owned_node;
            }
            owned_node = nullptr;
        }
    }
    list_type* list = nullptr;
    node_type<List>* owned_node = nullptr;
};