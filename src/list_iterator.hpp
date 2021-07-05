#pragma once

#include "fwd.hpp"

#include "list_node.hpp"

#include <iterator>
#include <mutex>

template <typename List>
class list_iterator {
public:
    friend List;
    using list_type = const List;
    using write_lock = typename List::write_lock;
    using read_lock = typename List::read_lock;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename list_type::value_type;
    using difference_type = size_t;
    using pointer = value_type*;
    using reference = value_type&;
    list_iterator() = default;
    list_iterator(const list_iterator& other) {
        read_lock lock(other.list->rw_mutex);
        list = other.list;
        node = other.node;
    }
    list_iterator& operator=(const list_iterator& other) {
        read_lock lock(other.list->rw_mutex);
        if (node == other.node) {
            return *this;
        }
        list = other.list;
        node = other.node;
        return *this;
    }
    value_type& operator*() {
        read_lock lock(list->rw_mutex);
        return node->value;
    }
    value_type* operator->() {
        read_lock lock(list->rw_mutex);
        return &(node->value);
    }
    list_iterator& operator++() {
        read_lock lock(list->rw_mutex);
        node = node->next;
        while (node->is_deleted) {
            node = node->next;
        }
        return *this;
    }
    list_iterator operator++(int) {
        list_iterator other(*this);
        ++*this;
        return other;
    }
    list_iterator& operator--() {
        read_lock lock(list->rw_mutex);
        node = node->prev;
        while (node->is_deleted) {
            node = node->prev;
        }
        return *this;
    }
    list_iterator operator--(int) {
        list_iterator other(*this);
        --*this;
        return other;
    }
    bool operator==(const list_iterator& rhs) {
        read_lock lock(list->rw_mutex);
        return node == rhs.node;
    }
    bool operator!=(const list_iterator& rhs) {
        read_lock lock(list->rw_mutex);
        return node != rhs.node;
    }
    ~list_iterator() = default;
private:
    list_iterator(list_type* list, node_pointer<List> other_node) {
        read_lock lock(list->rw_mutex);
        this->list = list;
        node = other_node;
    }
    list_type* list = nullptr;
    node_pointer<List> node = nullptr;
};