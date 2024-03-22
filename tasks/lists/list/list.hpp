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
        Node() : data(new T) {
        }
        Node(T value) : data(new T(value)) {
        }
        ~Node() {
            delete data;
        }

        T* data;
        Node* next;
        Node* prev;
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

        // ListIterator& operator=(ListIterator& other) {
        //     current = other.current;
        //     return *this;
        // }

        inline bool operator==(const ListIterator& other) const {
            return this->current == other.current;
        };

        inline bool operator!=(const ListIterator& other) const {
            return this->current != other.current;
        };

        // *End() work incorrect (valgrind)
        inline reference operator*() const {
            return *current->data;
        };

        ListIterator& operator++() {
            current = current->next;
            return *this;
        };

        ListIterator operator++(int) {
            ListIterator copy = *this;
            current = current->next;
            return copy;
        };

        ListIterator& operator--() {
            current = current->prev;
            return *this;
        };

        ListIterator operator--(int) {
            ListIterator copy = *this;
            current = current->prev;
            return copy;
        };

        inline pointer operator->() const {
            return current->data;
        };

    private:
        ListIterator(Node* other_node) {
            current = other_node;
        }
        ListIterator(const Node* other_node) {
            current = other_node;
        }

    private:
        Node* current;
    };

public:
    List() : size(0), head(new Node()) {
        head->next = head->prev = head;
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
        for (List<T>::ListIterator current = other.Begin(); current != other.End(); ++current) {
            PushBack(*current);
        }
        size = other.size;
    }

    List& operator=(const List& other) {
        List copy = other;
        Swap(copy);
        return *this;
    }

    ListIterator Begin() const noexcept {
        ListIterator begin_iterator(head->next);
        return begin_iterator;
    }

    ListIterator End() const noexcept {
        ListIterator end_iterator(head);
        return end_iterator;
    }

    inline T& Front() const {
        return *head->next->data;
    }

    inline T& Back() const {
        return *head->prev->data;
    }

    inline bool IsEmpty() const noexcept {
        return (size == 0);
    }

    inline size_t Size() const noexcept {
        return size;
    }

    void Swap(List& other) {
        std::swap(head, other.head);
        std::swap(size, other.size);
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
        Node* node_to_del = pos.current;
        node_to_del->prev->next = node_to_del->next;
        node_to_del->next->prev = node_to_del->prev;
        --size;
        delete node_to_del;
    }

    void Insert(ListIterator pos, const T& value) {
        Node* node_to_add = new Node(value);
        // node_to_add->data = value;
        Node* current = pos.current;

        node_to_add->prev = current->prev;
        node_to_add->next = current;

        current->prev->next = node_to_add;
        current->prev = node_to_add;

        ++size;
    }

    void Clear() noexcept {
        while (size) {
            PopBack();
        }
    }

    void PushBack(const T& value) {
        Node* node_to_add = new Node(value);
        // node_to_add->data = value;

        node_to_add->next = head;
        node_to_add->prev = head->prev;

        head->prev->next = node_to_add;
        head->prev = node_to_add;

        ++size;
    }

    void PushFront(const T& value) {
        Node* node_to_add = new Node(value);
        // node_to_add->data = value;

        node_to_add->prev = head;
        node_to_add->next = head->next;

        head->next->prev = node_to_add;
        head->next = node_to_add;

        ++size;
    }

    void PopBack() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty, can't pop last element.");
        }
        Node* node_to_del = head->prev;
        node_to_del->prev->next = head;
        head->prev = node_to_del->prev;
        --size;
        delete node_to_del;
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("List is empty, can't pop first element.");
        }
        Node* node_to_del = head->next;
        node_to_del->next->prev = head;
        head->next = node_to_del->next;
        --size;
        delete node_to_del;
    }

    ~List() {
        Clear();
        delete head;
    }

private:
    size_t size;
    Node* head;
};

namespace std {
// Global swap overloading
template <typename T>
void swap(List<T>& a, List<T>& b) {
    a.Swap(b);
}
}  // namespace std
