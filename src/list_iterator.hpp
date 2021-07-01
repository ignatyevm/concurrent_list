#pragma once

#include "fwd.hpp"

#include "list_node.hpp"

template <typename List>
class list_iterator {
public:
    friend List;
    using value_type = typename List::value_type;
    list_iterator() = default;
    list_iterator(const list_iterator& other) : node(other.node) {}
    list_iterator& operator=(const list_iterator& other) {
        if (*this == other) {
            return *this;
        }
        node = other.node;
        return *this;
    }
    value_type& operator*() {
        return node->value;
    }
    value_type* operator->() {
        return &(node->value);
    }
    list_iterator& operator++() {
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
        return node == rhs.node;
    }
    bool operator!=(const list_iterator& rhs) {
        return node != rhs.node;
    }
private:
    list_iterator(node_pointer<List> other_node) {
        node = other_node;
    }
    node_pointer<List> node = nullptr;
};