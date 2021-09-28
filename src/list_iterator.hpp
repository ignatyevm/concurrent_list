#pragma once

#include "fwd.hpp"
#include "list_node.hpp"

#include <iterator>

namespace polyndrom {

template<typename List>
class list_iterator {
private:
    using list_type = List;

    friend list_type;

    using write_lock = typename list_type::write_lock;
    using read_lock = typename list_type::read_lock;
    using node_ptr = detail::consistent_node_ptr<List>;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename list_type::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    list_iterator() = default;

    list_iterator(const list_iterator& other) : node(other.node) {
    }

    list_iterator& operator=(const list_iterator& other) {
        if (node == other.node) {
            return *this;
        }
        node = other.node;
        return *this;
    }

    value_type& operator*() {
        read_lock lock(node->mutex);
        return node->value;
    }

    value_type* operator->() {
        read_lock lock(node->mutex);
        return &(node->value);
    }

    list_iterator& operator++() {
        node = node.locked_read_next();
        while (node->is_deleted) {
            node = node.locked_read_next();
        }
        return *this;
    }

    list_iterator operator++(int) {
        list_iterator other(*this);
        ++*this;
        return other;
    }

    list_iterator& operator--() {
        node = node.locked_read_prev();
        while (node->is_deleted) {
            node = node.locked_read_prev();
        }
        return *this;
    }

    list_iterator operator--(int) {
        list_iterator other(*this);
        --*this;
        return other;
    }

    bool operator==(const list_iterator& rhs) const {
        return node == rhs.node;
    }

    bool operator!=(const list_iterator& rhs) const {
        return node != rhs.node;
    }

    ~list_iterator() = default;

private:
    explicit list_iterator(node_ptr other_node) : node(other_node) {
    }

private:
    node_ptr node = nullptr;
};

} // polyndrom