#pragma once

#include "fwd.hpp"

#include <utility>
#include <atomic>
#include <shared_mutex>
#include <stack>

namespace polyndrom::detail {

template<class List>
class consistent_node_ptr {
public:
    using list_type = List;

    friend list_iterator<list_type>;

    using write_lock = typename list_type::write_lock;
    using read_lock = typename list_type::read_lock;
    using value_type = typename list_type::value_type;

    class consistent_node {
    private:
        friend list_type;
        friend list_iterator<list_type>;
        friend consistent_node_ptr<list_type>;

        using value_type = typename list_type::value_type;

        consistent_node() = default;

        template<typename U>
        explicit consistent_node(U&& value) : value(std::forward<U>(value)) {}

        consistent_node_ptr<list_type> prev = nullptr;
        consistent_node_ptr<list_type> next = nullptr;
        value_type value;
        std::atomic_size_t ref_count = 0;
        std::atomic_bool is_deleted = false;
        std::shared_mutex mutex;
    };

    consistent_node_ptr() = default;

    consistent_node_ptr(const consistent_node_ptr& other) {
        acquire(other.owned_node);
    }

    explicit consistent_node_ptr(const value_type& value) {
        acquire(new consistent_node(value));
    }

    explicit consistent_node_ptr(value_type&& value) {
        acquire(new consistent_node(std::move(value)));
    }

    consistent_node_ptr& operator=(const consistent_node_ptr& other) {
        if (owned_node == other.owned_node) {
            return *this;
        }
        consistent_node* new_node = other.owned_node;
        release();
        acquire(new_node);
        return *this;
    }

    consistent_node_ptr(std::nullptr_t) {
    }

    consistent_node_ptr& operator=(std::nullptr_t) {
        release();
        return *this;
    }

    consistent_node* operator->() const {
        return owned_node;
    }

    bool operator==(const consistent_node_ptr& rhs) const {
        return owned_node == rhs.owned_node;
    }

    bool operator!=(const consistent_node_ptr& rhs) const {
        return owned_node != rhs.owned_node;
    }

    consistent_node_ptr locked_read_next() const {
        read_lock lock(owned_node->mutex);
        return owned_node->next;
    }

    consistent_node_ptr locked_read_prev() const {
        read_lock lock(owned_node->mutex);
        return owned_node->prev;
    }

    std::pair<consistent_node_ptr, consistent_node_ptr> locked_read_nodes() const {
        read_lock lock(owned_node->mutex);
        return {owned_node->prev, owned_node->next};
    }

    ~consistent_node_ptr() {
        release();
    }

private:
    void acquire(consistent_node* node) {
        owned_node = node;
        if (owned_node != nullptr) {
            owned_node->ref_count += 1;
        }
    }

    void release() {
        if (owned_node != nullptr) {
            if (owned_node->ref_count-- == 1) {
                // std::stack<consistent_node*> nodes_stack;
                nodes_stack.push(owned_node);
                while (!nodes_stack.empty()) {
                    consistent_node* node = nodes_stack.top();
                    nodes_stack.pop();
                    consistent_node* prev = node->prev.owned_node;
                    consistent_node* next = node->next.owned_node;
                    if (prev != nullptr && prev->ref_count-- == 1) {
                        nodes_stack.push(prev);
                    }
                    if (next != nullptr && next->ref_count-- == 1) {
                        nodes_stack.push(next);
                    }
                    node->prev.owned_node = nullptr;
                    node->next.owned_node = nullptr;
                    delete node;
                }
            }
            owned_node = nullptr;
        }
    }

private:
    static thread_local std::stack<consistent_node*> nodes_stack;
    consistent_node* owned_node = nullptr;
};

} // polyndrom::detail

namespace polyndrom::detail {
    template <class List>
    thread_local std::stack<typename consistent_node_ptr<List>::consistent_node*> consistent_node_ptr<List>::nodes_stack{};
} // polyndrom::detail
