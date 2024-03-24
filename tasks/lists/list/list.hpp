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
    class Node {
        friend class ListIterator;
        friend class List;

    private:
        Node() : data_() {
        }

        explicit Node(const T& value) : data_(value) {
        }

        explicit Node(T&& value) : data_(std::move(value)) {
        }

        ~Node() {
        }

        T data_;
        Node* next_;
        Node* prev_;
    };

public:
    class ListIterator {
        friend class List;

    public:
        // NOLINTNEXTLINE
        using iterator_category = std::bidirectional_iterator_tag;
        // NOLINTNEXTLINE
        using difference_type = std::ptrdiff_t;
        // NOLINTNEXTLINE
        using value_type = T;
        // NOLINTNEXTLINE
        using reference = value_type&;
        // NOLINTNEXTLINE
        using pointer = value_type*;

        inline bool operator==(const ListIterator& other) const {
            return this->current_ == other.current_;
        };

        inline bool operator!=(const ListIterator& other) const {
            return this->current_ != other.current_;
        };

        // *End() work incorrect (valgrind)
        inline reference operator*() const {
            return current_->data_;
        };

        ListIterator& operator++() {
            current_ = current_->next_;
            return *this;
        };

        ListIterator operator++(int) {
            ListIterator copy = *this;
            current_ = current_->next_;
            return copy;
        };

        ListIterator& operator--() {
            current_ = current_->prev_;
            return *this;
        };

        ListIterator operator--(int) {
            ListIterator copy = *this;
            current_ = current_->prev_;
            return copy;
        };

        inline pointer operator->() const {
            return &current_->data_;
        };

    private:
        explicit ListIterator(Node* other_node) {
            current_ = other_node;
        }
        explicit ListIterator(const Node* other_node) {
            current_ = other_node;
        }

    private:
        Node* current_;
    };

public:
    List() : size_(0), head_(new Node()) {
        head_->next_ = head_->prev_ = head_;
    }

    explicit List(size_t sz) : List() {
        for (size_t _ = 0; _ < sz; ++_) {
            PushBack(T());
        }
    }

    List(const std::initializer_list<T>& values) : List() {
        for (T value : values) {
            PushBack(value);
        }
    }

    List(const List& other) : List() {
        for (List<T>::ListIterator it = other.Begin(); it != other.End(); ++it) {
            PushBack(*it);
        }
        size_ = other.size_;
    }

    List& operator=(const List& other) {
        List copy = other;
        Swap(copy);
        return *this;
    }

    ListIterator Begin() const noexcept {
        ListIterator begin_iterator(head_->next_);
        return begin_iterator;
    }

    ListIterator End() const noexcept {
        ListIterator end_iterator(head_);
        return end_iterator;
    }

    inline T& Front() const {
        return head_->next_->data_;
    }

    inline T& Back() const {
        return head_->prev_->data_;
    }

    inline bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    inline size_t Size() const noexcept {
        return size_;
    }

    void Swap(List& other) {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
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
        Node* node_to_del = pos.current_;
        node_to_del->prev_->next_ = node_to_del->next_;
        node_to_del->next_->prev_ = node_to_del->prev_;
        --size_;
        delete node_to_del;
    }

    void Insert(ListIterator pos, const T& value) {
        Node* node_to_add = new Node(value);
        Node* current = pos.current_;

        node_to_add->prev_ = current->prev_;
        node_to_add->next_ = current;

        current->prev_->next_ = node_to_add;
        current->prev_ = node_to_add;

        ++size_;
    }

    void Clear() noexcept {
        while (size_) {
            PopBack();
        }
    }

    void PushBack(const T& value) {
        Node* node_to_add = new Node(value);

        node_to_add->next_ = head_;
        node_to_add->prev_ = head_->prev_;

        head_->prev_->next_ = node_to_add;
        head_->prev_ = node_to_add;

        ++size_;
    }

    void PushBack(T&& value) {
        Node* node_to_add = new Node(std::move(value));

        node_to_add->next_ = head_;
        node_to_add->prev_ = head_->prev_;

        head_->prev_->next_ = node_to_add;
        head_->prev_ = node_to_add;

        ++size_;
    }

    void PushFront(const T& value) {
        Node* node_to_add = new Node(value);

        node_to_add->prev_ = head_;
        node_to_add->next_ = head_->next_;

        head_->next_->prev_ = node_to_add;
        head_->next_ = node_to_add;

        ++size_;
    }

    void PushFront(T&& value) {
        Node* node_to_add = new Node(std::move(value));

        node_to_add->prev_ = head_;
        node_to_add->next_ = head_->next_;

        head_->next_->prev_ = node_to_add;
        head_->next_ = node_to_add;

        ++size_;
    }

    void PopBack() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty, can't pop last element.");
        }
        Node* node_to_del = head_->prev_;
        node_to_del->prev_->next_ = head_;
        head_->prev_ = node_to_del->prev_;
        --size_;
        delete node_to_del;
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty, can't pop first element.");
        }
        Node* node_to_del = head_->next_;
        node_to_del->next_->prev_ = head_;
        head_->next_ = node_to_del->next_;
        --size_;
        delete node_to_del;
    }

    ~List() {
        Clear();
        delete head_;
    }

private:
    size_t size_;
    Node* head_;
};

namespace std {
// Global swap overloading
template <typename T>
// NOLINTNEXTLINE
void swap(List<T>& a, List<T>& b) {  // Здесь clippy просил, чтобы я функцию назвал с большой буквы
    a.Swap(b);
}
}  // namespace std
