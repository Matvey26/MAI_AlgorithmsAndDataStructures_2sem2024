#pragma once

#include <fmt/core.h>

#include <cstdlib>
#include <functional>
#include <iterator>
#include <utility>

#include "exceptions.hpp"

// Просто комментарий, чтобы были какие-то изменения. Я просто пытаюсь заставить клиппи самому обновить тесты в моём
// пр'е.

template <typename T>
class ForwardList {
private:
    struct BaseNode {
        BaseNode* next_ = nullptr;
    };

    struct Node : public BaseNode {
        T data_;
        explicit Node(const T& value) : data_(value) {
        }
    };

public:
    template <bool IsConst>
    class BaseIterator {
        friend class ForwardList;

    public:
        // NOLINTNEXTLINE
        using basenode_ptr_type = std::conditional_t<IsConst, const BaseNode*, BaseNode*>;
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
        using iterator_category = std::forward_iterator_tag;

        inline bool operator==(const BaseIterator& other) const {
            return this->current_ == other.current_;
        }

        inline bool operator!=(const BaseIterator& other) const {
            return this->current_ != other.current_;
        }

        inline reference_type operator*() const {
            return static_cast<node_ptr_type>(this->current_)->data_;
        }

        BaseIterator& operator++() noexcept {
            this->current_ = this->current_->next_;
            return *this;
        }

        BaseIterator operator++(int) noexcept {
            BaseIterator temp = *this;
            this->current_ = this->current_->next_;
            return temp;
        }

        inline pointer_type operator->() const {
            return &static_cast<node_ptr_type>(this->current_)->data_;
        }

    private:
        explicit BaseIterator(basenode_ptr_type node) noexcept : current_(node) {
        }

    private:
        basenode_ptr_type current_;
    };

public:
    using Iterator = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    ForwardList() : sz_(0) {
    }

    explicit ForwardList(size_t sz) : ForwardList() {
        while (sz--) {
            this->PushFront(T());
        }
    }

    ForwardList(const std::initializer_list<T>& values) : ForwardList() {
        this->RangeInitialize(values.begin(), values.end());
    }

    ForwardList(const ForwardList& other) : ForwardList() {
        this->RangeInitialize(other.Begin(), other.End());
    }

    template <typename InputIterator>
    void RangeInitialize(InputIterator first, InputIterator last) {
        BaseNode* to = &this->head_;
        for (; first != last; ++first) {
            to->next_ = static_cast<BaseNode*>(new Node(*first));
            to = to->next_;
            ++this->sz_;
        }
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this != &other) {
            ForwardList copy = other;
            this->Swap(copy);
        }
        return *this;
    }

    Iterator BeforeBegin() noexcept {
        return Iterator(&this->head_);
    }

    ConstIterator BeforeBegin() const noexcept {
        return ConstIterator(&this->head_);
    }

    ConstIterator CBeforeBegin() const noexcept {
        return ConstIterator(&this->head_);
    }

    Iterator Begin() noexcept {
        return Iterator(this->head_.next_);
    }

    ConstIterator Begin() const noexcept {
        return ConstIterator(this->head_.next_);
    }

    Iterator End() noexcept {
        return Iterator(nullptr);
    }

    ConstIterator End() const noexcept {
        return ConstIterator(nullptr);
    }

    inline T& Front() const {
        return static_cast<Node*>(this->head_.next_)->data_;
    }

    inline bool IsEmpty() const noexcept {
        return this->sz_ == 0;
    }

    inline size_t Size() const noexcept {
        return this->sz_;
    }

    void Swap(ForwardList& other) {
        std::swap(this->head_.next_, other.head_.next_);
        std::swap(this->sz_, other.sz_);
    }

    void EraseAfterNode(BaseNode* current) {
        Node* node_to_del = static_cast<Node*>(current->next_);
        current->next_ = node_to_del->next_;
        delete node_to_del;
        --this->sz_;
    }

    void InsertAfterNode(BaseNode* current, const T& value) {
        Node* node_to_add = new Node(value);
        node_to_add->next_ = current->next_;
        current->next_ = static_cast<BaseNode*>(node_to_add);
        ++this->sz_;
    }

    void EraseAfter(ConstIterator pos) {
        // NOLINTNEXTLINE
        EraseAfterNode(const_cast<BaseNode*>(pos.current_));
    }

    void EraseAfter(Iterator pos) {
        EraseAfterNode(pos.current_);
    }

    void InsertAfter(ConstIterator pos, const T& value) {
        // NOLINTNEXTLINE
        InsertAfterNode(const_cast<BaseNode*>(pos.current_), value);
    }

    void InsertAfter(Iterator pos, const T& value) {
        InsertAfterNode(pos.current_, value);
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

    void Clear() noexcept {
        while (this->sz_) {
            this->PopFront();
        }
    }

    void PushFront(const T& value) {
        this->InsertAfter(CBeforeBegin(), value);
    }

    void PopFront() {
        if (this->sz_ == 0) {
            throw ListIsEmptyException("Can't delete element from empty forward list!");
        }
        this->EraseAfter(CBeforeBegin());
    }

    ~ForwardList() {
        this->Clear();
    }

private:
    size_t sz_;
    BaseNode head_;
};

namespace std {
// Global swap overloading
template <typename T>
// NOLINTNEXTLINE
void swap(ForwardList<T>& a, ForwardList<T>& b) {
    a.Swap(b);
}
}  // namespace std
