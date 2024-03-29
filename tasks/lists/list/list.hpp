#pragma once

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

#include "exceptions.hpp"

// #include <fmt/core.h>

template <typename T>
class List {
private:
    struct NodeBase {
        NodeBase* next_;
        NodeBase* prev_;
    };

    struct NodeHeader : public NodeBase {
        size_t sz_;

        // Improves code readability.
        NodeHeader* base() {
            return this;
        }

        // default ctor
        NodeHeader() noexcept {
            init();
        }

        // copy ctor
        NodeHeader(const NodeHeader& other) noexcept : NodeBase{other.next_, other.prev_}, sz_(other.sz_) {
            if (other.base()->next_ == other.base()) {
                this->next_ = this->prev_ = this;
            } else {
                this->next_->prev_ = this->prev_->next_ = this;
            }
        }

        // void Swap(NodeHeader& other) {
        //     std::swap(base()->next_, other.base().next_);
        //     std::swap(base()->prev_, other.base().prev_);
        // }

        // move ctor
        NodeHeader(NodeHeader&& other) noexcept : NodeBase{other.next_, other.prev_}, sz_(other.sz_) {
            if (other.base()->next_ == other.base()) {
                this->next_ = this->prev_ = this;
            } else {
                this->next_->prev_ = this->prev_->next_ = this;
                other.init();
            }
        }

        // initializes (or resets) the node
        void init() {
            this->next_ = this->prev_ = this;
            sz_ = 0;
        }
    };

    struct Node : public NodeBase {
        T data_;

        explicit Node(const T& value) : data_(value) {
        }
    };

public:
    class ListIterator {
        friend List;

    public:
        // NOLINTNEXTLINE
        using value_type = T;
        // NOLINTNEXTLINE
        using reference_type = value_type&;
        // NOLINTNEXTLINE
        using pointer_type = value_type*;
        // NOLINTNEXTLINE
        using difference_type = std::ptrdiff_t;
        // NOLINTNEXTLINE
        using iterator_category = std::bidirectional_iterator_tag;

        inline bool operator==(const ListIterator& other) const {
            return current_ == other.current_;
        };

        inline bool operator!=(const ListIterator& other) const {
            return current_ != other.current_;
        };

        inline reference_type operator*() const {
            return static_cast<Node*>(current_)->data_;
        };

        ListIterator& operator++() noexcept {
            current_ = current_->next_;
            return *this;
        };

        ListIterator operator++(int) noexcept {
            ListIterator temp = *this;
            current_ = current_->next_;
            return temp;
        };

        ListIterator& operator--() {
            current_ = current_->prev_;
            return *this;
        };

        ListIterator operator--(int) {
            ListIterator temp = *this;
            current_ = current_->prev_;
            return temp;
        };

        inline pointer_type operator->() const {
            return &static_cast<Node*>(current_)->data_;
        };

    private:
        ListIterator() noexcept : current_() {
        }
        explicit ListIterator(const NodeBase* node) noexcept : current_(const_cast<NodeBase*>(node)) {
        }
        ListIterator ConstCast() const noexcept {
            return *this;
        }

    private:
        NodeBase* current_;
    };

public:
    List() = default;

    explicit List(size_t sz) {
        while (sz--) {
            PushBack(T());
        }
    }

    List(const std::initializer_list<T>& values) {
        for (auto value : values) {
            PushBack(value);
        }
    }

    List(const List& other) : List() {
        for (auto it = other.Begin(); it != other.End(); ++it) {
            PushBack(*it);
        }
    }

    List& operator=(const List& other) {
        List copy = other;
        Swap(copy);
        return *this;
    }

    ListIterator Begin() const noexcept {
        return ListIterator(head_.next_);
    }

    ListIterator End() const noexcept {
        return ListIterator(&head_);
    }

    inline T& Front() const {
        return static_cast<Node*>(head_.next_)->data_;
    }

    inline T& Back() const {
        return static_cast<Node*>(head_.prev_)->data_;
    }

    inline bool IsEmpty() const noexcept {
        return head_.sz_ == 0;
    }

    inline size_t Size() const noexcept {
        return head_.sz_;
    }

    void Swap(List& other) {
        head_.prev_->next_ = &other.head_;
        head_.next_->prev_ = &other.head_;
        other.head_.prev_->next_ = &head_;
        other.head_.next_->prev_ = &head_;
        // std::swap(head_.base()->prev_->next_, other.head_.base()->prev_->next_);
        // std::swap(head_.base()->next_->prev_, other.head_.base()->next_->prev_);
        // std::swap(head_.base(), other.head_.base());
        std::swap(head_.prev_, other.head_.prev_);
        std::swap(head_.next_, other.head_.next_);
        std::swap(head_.sz_, other.head_.sz_);
    }

    ListIterator Find(const T& value) const {
        auto it = Begin();
        for (; it != End(); ++it) {
            if (*it == value) {
                break;
            }
        }
        return it;
    }

    void Erase(ListIterator pos) {
        Node* node_to_del = static_cast<Node*>(pos.current_);
        pos.current_->prev_->next_ = pos.current_->next_;
        pos.current_->next_->prev_ = pos.current_->prev_;
        --head_.sz_;
        delete node_to_del;
    }

    void Insert(ListIterator pos, const T& value) {
        Node* node_to_add = new Node(value);
        node_to_add->next_ = pos.current_;
        node_to_add->prev_ = pos.current_->prev_;
        pos.current_->prev_->next_ = static_cast<NodeBase*>(node_to_add);
        pos.current_->prev_ = static_cast<NodeBase*>(node_to_add);
        ++head_.sz_;
    }

    void Clear() noexcept {
        while (head_.sz_) {
            PopBack();
        }
    }

    void PushBack(const T& value) {
        Insert(End(), value);
    }

    void PushFront(const T& value) {
        Insert(Begin(), value);
    }

    void PopBack() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        Erase(ListIterator(head_.prev_));
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty");
        }
        Erase(Begin());
    }

    ~List() {
        Clear();
    }

private:
    NodeHeader head_;
};

namespace std {
// Global swap overloading
template <typename T>
// NOLINTNEXTLINE
void swap(List<T>& a, List<T>& b) {
    a.Swap(b);
}
}  // namespace std