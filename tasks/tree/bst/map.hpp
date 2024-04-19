#pragma once

#include <fmt/core.h>

#include <cstdlib>
#include <functional>
#include <utility>
#include <vector>

#include "exceptions.hpp"

template <typename U, typename V, typename Compare = std::less<U>,
          typename Alloc = std::allocator<std::pair<const U, V>>>
class Map {
public:
    // NOLINTNEXTLINE
    using key_type = U;
    // NOLINTNEXTLINE
    using value_type = V;

private:
    struct Node {
        Node* left = nullptr;
        Node* right = nullptr;

        key_type key;
        value_type value;
        explicit Node(const key_type& key) : key(key), value() {
        }

        void Swap(Node& other) {
            std::swap(this->key, other.key);
            std::swap(this->value, other.value);
        }
    };

public:
    Map() : sz_(0), head_(nullptr) {
    }

    explicit Map(const Alloc& a) : sz_(0), head_(nullptr), alloc_(a) {
    }

    value_type& operator[](const key_type& key) {
        if (this->sz_ == 0) {
            ++this->sz_;
            this->head_ = alloc_.allocate(1);
            std::allocator_traits<node_allocator>::construct(alloc_, this->head_, key);
            return this->head_->value;
        }
        return this->Find(this->head_, key)->value;
    }

    inline bool IsEmpty() const noexcept {
        return this->sz_ == 0;
    }

    inline size_t Size() const noexcept {
        return this->sz_;
    }

    void Swap(Map& other) {
        static_assert(std::is_same<decltype(this->cmp_), decltype(other.cmp_)>::value,
                      "The compare function types are different");

        std::swap(this->head_, other.head_);
        std::swap(this->sz_, other.sz_);
    }

private:
    void InOrder(Node* u, std::vector<std::pair<key_type, value_type>>& values, bool is_increase) const {
        if (u == nullptr) {
            return;
        }
        if (is_increase) {
            InOrder(u->left, values, is_increase);
            values.emplace_back(u->key, u->value);
            InOrder(u->right, values, is_increase);
        } else {
            InOrder(u->right, values, is_increase);
            values.emplace_back(u->key, u->value);
            InOrder(u->left, values, is_increase);
        }
    }

    Node* Find(Node*& current, const key_type& key) {
        if (current == nullptr) {
            ++this->sz_;
            current = alloc_.allocate(1);
            std::allocator_traits<node_allocator>::construct(alloc_, current, key);
            return current;
        }
        if (this->cmp_(key, current->key)) {
            return Find(current->left, key);
        } else if (this->cmp_(current->key, key)) {
            return Find(current->right, key);
        } else {
            return current;
        }
    }

    Node* Erase(Node* p, const key_type& key) {
        if (p == nullptr) {
            throw MapIsEmptyException("You are trying to erase not existing key.");
        }
        if (this->cmp_(key, p->key)) {
            p->left = Erase(p->left, key);
            return p;
        }
        if (this->cmp_(p->key, key)) {
            p->right = Erase(p->right, key);
            return p;
        }
        if (p->left == nullptr and p->right == nullptr) {
            std::allocator_traits<node_allocator>::destroy(alloc_, p);
            alloc_.deallocate(p, 1);
            return nullptr;
        }
        if (p->left == nullptr) {
            Node* t = p->right;
            std::allocator_traits<node_allocator>::destroy(alloc_, p);
            alloc_.deallocate(p, 1);
            return t;
        }
        if (p->right == nullptr) {
            Node* t = p->left;
            std::allocator_traits<node_allocator>::destroy(alloc_, p);
            alloc_.deallocate(p, 1);
            return t;
        }

        Node* prev_max = nullptr;
        Node* max = p->left;
        while (max->right != nullptr) {
            prev_max = max;
            max = max->right;
        }

        p->key = max->key;
        p->value = max->value;
        if (prev_max == nullptr) {
            p->left = nullptr;
        } else {
            prev_max->right = nullptr;
        }

        std::allocator_traits<node_allocator>::destroy(alloc_, max);
        alloc_.deallocate(max, 1);
        return p;
    }

public:
    void Clear() noexcept {
        if (!this->IsEmpty()) {
            ClearSubtree(this->head_);
        }
        this->sz_ = 0;
    }

    void ClearSubtree(Node* root) {
        if (root == nullptr) {
            return;
        }

        ClearSubtree(root->left);
        root->left = nullptr;
        ClearSubtree(root->right);
        root->right = nullptr;

        std::allocator_traits<node_allocator>::destroy(alloc_, root);
        alloc_.deallocate(root, 1);
    }

    bool Find(const key_type& key) const {
        Node* current = this->head_;
        while (current != nullptr) {
            if (this->cmp_(key, current->key)) {
                current = current->left;
            } else if (this->cmp_(current->key, key)) {
                current = current->right;
            } else {
                return true;
            }
        }
        return false;
    }

    void Insert(const std::pair<const key_type, value_type>& val) {
        this->operator[](val.first) = val.second;
    }

    void Insert(const std::initializer_list<std::pair<const key_type, value_type>>& values) {
        for (auto it = values.begin(); it != values.end(); ++it) {
            Insert(*it);
        }
    }

    void Erase(const key_type& key) {
        this->head_ = this->Erase(this->head_, key);
        --this->sz_;
    }

    std::vector<std::pair<key_type, value_type>> Values(bool is_increase = true) const noexcept {
        std::vector<std::pair<key_type, value_type>> answer;
        this->InOrder(this->head_, answer, is_increase);
        return answer;
    }

    ~Map() {
        this->Clear();
    }

private:
    size_t sz_;
    Node* head_;
    Compare cmp_;
    // NOLINTNEXTLINE
    using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    node_allocator alloc_;
};

namespace std {
// Global swap overloading
template <typename U, typename V>
// NOLINTNEXTLINE
void swap(Map<U, V>& a, Map<U, V>& b) {
    a.Swap(b);
}
}  // namespace std
