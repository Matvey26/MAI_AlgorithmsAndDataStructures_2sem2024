#pragma once

#include <fmt/core.h>

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

#include "exceptions.hpp"

template <typename T>
class List {
private:
    struct NodeBase {
        NodeBase* next_;
        NodeBase* prev_;
    };

    struct Node : public NodeBase {
        T data_;

        explicit Node(const T& value) : data_(value) {
        }
    };

private:
    template <bool IsConst>
    class BaseIterator {
        friend List;

    public:
        // NOLINTNEXTLINE
        using basenode_ptr_type = std::conditional_t<IsConst, const NodeBase*, NodeBase*>;
        // NOLINTNEXTLINE
        using node_ptr_type = std::conditional_t<IsConst, const Node*, Node*>;
        // NOLINTNEXTLINE
        using value_type = T;
        // NOLINTNEXTLINE
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        // NOLINTNEXTLINE
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        // NOLINTNEXTLINE
        using difference_type = std::ptrdiff_t;
        // NOLINTNEXTLINE
        using iterator_category = std::bidirectional_iterator_tag;

        inline bool operator==(const BaseIterator& other) const {
            return this->current_ == other.current_;
        };

        inline bool operator!=(const BaseIterator& other) const {
            return this->current_ != other.current_;
        };

        inline reference_type operator*() const {
            return static_cast<node_ptr_type>(this->current_)->data_;
        };

        BaseIterator& operator++() noexcept {
            this->current_ = this->current_->next_;
            return *this;
        };

        BaseIterator operator++(int) noexcept {
            BaseIterator temp = *this;
            this->current_ = this->current_->next_;
            return temp;
        };

        BaseIterator& operator--() {
            this->current_ = this->current_->prev_;
            return *this;
        };

        BaseIterator operator--(int) {
            BaseIterator temp = *this;
            this->current_ = this->current_->prev_;
            return temp;
        };

        inline pointer_type operator->() const {
            return &static_cast<node_ptr_type>(this->current_)->data_;
        };

    private:
        BaseIterator() noexcept : current_() {
        }
        explicit BaseIterator(basenode_ptr_type node) noexcept : current_(node) {
        }

    private:
        basenode_ptr_type current_;
    };

public:
    using Iterator = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    List() : sz_(0) {
        this->head_.next_ = this->head_.prev_ = &this->head_;
    }

    explicit List(size_t sz) : List() {
        while (sz--) {
            this->PushBack(T());
        }
    }

    List(const std::initializer_list<T>& values) : List() {
        for (auto value : values) {
            this->PushBack(value);
        }
    }

    List(const List& other) : List() {
        for (auto it = other.Begin(); it != other.End(); ++it) {
            this->PushBack(*it);
        }
    }

    List& operator=(const List& other) {
        if (this != &other) {
            List copy = other;
            this->Swap(copy);
        }
        return *this;
    }

    Iterator Begin() noexcept {
        return Iterator(this->head_.next_);
    }

    Iterator End() noexcept {
        return Iterator(&this->head_);
    }

    // Нужны для константых списков
    ConstIterator Begin() const noexcept {
        return ConstIterator(this->head_.next_);
    }

    ConstIterator End() const noexcept {
        return ConstIterator(&this->head_);
    }

    inline T& Front() const {
        return static_cast<Node*>(this->head_.next_)->data_;
    }

    inline T& Back() const {
        return static_cast<Node*>(this->head_.prev_)->data_;
    }

    inline bool IsEmpty() const noexcept {
        return this->sz_ == 0;
    }

    inline size_t Size() const noexcept {
        return this->sz_;
    }

    void Swap(List& other) {
        std::swap(this->head_.prev_, other.head_.prev_);
        std::swap(this->head_.next_, other.head_.next_);
        std::swap(this->sz_, other.sz_);

        if (this->sz_ == 0) {
            this->head_.next_ = this->head_.prev_ = &this->head_;
        } else {
            this->head_.next_->prev_ = this->head_.prev_->next_ = &head_;
        }

        if (other.sz_ == 0) {
            other.head_.next_ = other.head_.prev_ = &other.head_;
        } else {
            other.head_.next_->prev_ = other.head_.prev_->next_ = &other.head_;
        }
    }

    Iterator Find(const T& value) {
        auto it = this->Begin();
        for (; it != this->End(); ++it) {
            if (*it == value) {
                break;
            }
        }
        return it;
    }

    void Erase(Iterator pos) {
        Node* node_to_del = static_cast<Node*>(pos.current_);
        pos.current_->prev_->next_ = pos.current_->next_;
        pos.current_->next_->prev_ = pos.current_->prev_;
        --this->sz_;
        delete node_to_del;
    }

    void Insert(Iterator pos, const T& value) {
        Node* node_to_add = new Node(value);
        node_to_add->next_ = pos.current_;
        node_to_add->prev_ = pos.current_->prev_;
        pos.current_->prev_->next_ = static_cast<NodeBase*>(node_to_add);
        pos.current_->prev_ = static_cast<NodeBase*>(node_to_add);
        ++this->sz_;
    }

    void Clear() noexcept {
        while (this->sz_) {
            this->PopBack();
        }
    }

    void PushBack(const T& value) {
        this->Insert(this->End(), value);
    }

    void PushFront(const T& value) {
        this->Insert(this->Begin(), value);
    }

    void PopBack() {
        if (this->IsEmpty()) {
            throw ListIsEmptyException("List is empty, you can't delete last element from him");
        }
        this->Erase(Iterator(this->head_.prev_));
    }

    void PopFront() {
        if (this->IsEmpty()) {
            throw ListIsEmptyException("List is empty, you can't delete first element from him");
        }
        this->Erase(this->Begin());
    }

    ~List() {
        this->Clear();
    }

private:
    size_t sz_;
    NodeBase head_;
};

namespace std {
// Global swap overloading
template <typename T>
// NOLINTNEXTLINE
void swap(List<T>& a, List<T>& b) {
    a.Swap(b);
}
}  // namespace std