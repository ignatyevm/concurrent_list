#pragma once

#include "fwd.hpp"

#include <utility>
#include <atomic>
#include <mutex>

namespace polyndrom::detail {

template<class List>
class consistent_node_ptr {
public:
    using list_type = List;
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
    };

    consistent_node_ptr() = default;

    template<typename T>
    explicit consistent_node_ptr(list_type* list, T&& value) noexcept {
        this->list = list;
        acquire(new consistent_node(std::forward<T>(value)));
        owned_node->next.list = list;
        owned_node->prev.list = list;
    }

    consistent_node_ptr(const consistent_node_ptr& other) {
        list = other.list;
        acquire(other.owned_node);
    }

    consistent_node_ptr& operator=(const consistent_node_ptr& other) {
        if (owned_node == other.owned_node) {
            return *this;
        }
        list = other.list;
        consistent_node* new_node = other.owned_node;
        release();
        acquire(new_node);
        return *this;
    }

    consistent_node_ptr(std::nullptr_t) {}

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
            owned_node->ref_count -= 1;
            if (owned_node->ref_count == 0) {
                delete owned_node;
            }
            owned_node = nullptr;
        }
    }

private:
    list_type* list = nullptr;
    consistent_node* owned_node = nullptr;
};

} // polyndrom::detail