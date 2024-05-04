#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <utility>

const size_t MIN_CAP = 10;

template <typename T, typename Alloc = std::allocator<T>>
class Vector {
private:
    // NOLINTNEXTLINE
    using allocator_type = Alloc;
    // NOLINTNEXTLINE
    using alloc_traits = std::allocator_traits<allocator_type>;
    allocator_type alloc_;

    size_t sz_;
    size_t cap_;
    T* arr_;
    // using T_alloc_type = typename std::allocator_traits<allocator_type>::template rebind<T>::other;

public:
    Vector() : alloc_(), sz_(0), cap_(0), arr_(nullptr) {
    }

    explicit Vector(const allocator_type& a) : alloc_(a), sz_(0), cap_(0), arr_(alloc_.allocate(cap_)) {
    }

    explicit Vector(size_t count, const allocator_type& a = allocator_type())
        : alloc_(a), sz_(0), cap_((count > MIN_CAP ? count : MIN_CAP)), arr_(alloc_.allocate(cap_)) {
    }

    Vector(size_t count, const T& value, const allocator_type& a = allocator_type()) : Vector(count, a) {
        this->sz_ = count;
        std::uninitialized_fill_n(this->arr_, count, value);
    }

    Vector(const Vector& other) : Vector(other.sz_, other.alloc_) {
        this->sz_ = other.sz_;
        // переносим элементы и отслеживаем исключения (чтобы почистить память)
        for (size_t i = 0; i < this->sz_; ++i) {
            try {
                alloc_traits::construct(alloc_, this->arr_ + i, other.arr_[i]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    alloc_traits::destroy(alloc_, this->arr_ + j);
                }
                this->alloc_.deallocate(this->arr_, this->cap_);
                throw;
            }
        }
    }

    Vector(Vector&& other) noexcept : alloc_(std::move(other.alloc_)), sz_(0), cap_(0), arr_(nullptr) {
        this->Swap(other);
    }

    Vector(std::initializer_list<T> init) : Vector(init.size()) {
        this->sz_ = init.size();
        auto it = init.begin();
        for (size_t i = 0; i < this->sz_; ++i) {
            this->arr_[i] = std::move(*it);
            ++it;
        }
    }

    Vector& operator=(const Vector& other) {
        auto copy = other;
        this->Swap(copy);
        return *this;
    }

    Vector& operator=(Vector&& other) {
        for (size_t i = 0; i < this->sz_; ++i) {
            alloc_traits::destroy(this->alloc_, &this->arr_[i]);
        }
        this->alloc_.deallocate(this->arr_, this->cap_);
        this->arr_ = nullptr;
        this->sz_ = 0;
        this->cap_ = 0;
        this->Swap(other);
        return *this;
    }

    T& operator[](size_t pos) {
        return this->arr_[pos];
    }

    const T& operator[](size_t pos) const {
        return this->arr_[pos];
    }

    T& Front() const noexcept {
        return this->arr_[0];
    }

    T& Back() const noexcept {
        return this->arr_[this->sz_ - 1];
    }

    T* Data() const noexcept {
        return this->arr_;
    }

    bool IsEmpty() const noexcept {
        return this->sz_ == 0;
    }

    size_t Size() const noexcept {
        return this->sz_;
    }

    size_t Capacity() const noexcept {
        return this->cap_;
    }

    void Reserve(size_t new_cap) {
        if (new_cap <= this->cap_) {
            return;
        }

        // выделяем память для нового массива
        T* new_arr = alloc_.allocate(new_cap);

        // переносим элементы и отслеживаем исключения (чтобы почистить память)
        for (size_t i = 0; i < this->sz_; ++i) {
            try {
                alloc_traits::construct(alloc_, new_arr + i, std::move(this->arr_[i]));
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    alloc_traits::destroy(alloc_, new_arr + j);
                }
                this->alloc_.deallocate(new_arr, new_cap);
                throw;
            }
        }
        for (size_t i = 0; i < this->sz_; ++i) {
            alloc_traits::destroy(alloc_, &this->arr_[i]);
        }

        this->alloc_.deallocate(this->arr_, this->cap_);
        this->cap_ = new_cap;
        this->arr_ = new_arr;
    }

    void Clear() noexcept {
        while (!this->IsEmpty()) {
            this->PopBack();
        }
    }

    void Insert(size_t pos, T&& value) {
        this->CheckCapacity();

        for (size_t i = sz_; i > pos; --i) {
            alloc_traits::construct(alloc_, this->arr_ + i, std::move_if_noexcept(this->arr_[i - 1]));
            alloc_traits::destroy(alloc_, this->arr_ + i - 1);
        }

        alloc_traits::construct(alloc_, this->arr_ + pos, std::move(value));
        ++sz_;
    }

    void Erase(size_t begin_pos, size_t end_pos) {
        begin_pos = std::max(0ul, std::min(this->sz_, begin_pos));
        end_pos = std::max(0ul, std::min(this->sz_, end_pos));
        for (size_t i = begin_pos; i < end_pos; ++i) {
            alloc_traits::destroy(this->alloc_, this->arr_ + i);
        }
        for (size_t i = 0; i < this->sz_ - end_pos; ++i) {
            alloc_traits::construct(alloc_, this->arr_ + begin_pos + i, this->arr_[end_pos + i]);
            alloc_traits::destroy(alloc_, this->arr_ + end_pos + i);
        }
        this->sz_ -= end_pos - begin_pos;
    }

    void PushBack(const T& value) {
        this->CheckCapacity();
        alloc_traits::construct(this->alloc_, this->arr_ + this->sz_, value);
        ++this->sz_;
    }

    void PushBack(T&& value) {
        this->CheckCapacity();
        alloc_traits::construct(this->alloc_, this->arr_ + this->sz_, std::move(value));
        ++this->sz_;
    }

    template <class... Args>
    void EmplaceBack(Args&&... args) {
        this->CheckCapacity();
        alloc_traits::construct(this->alloc_, this->arr_ + this->sz_, std::forward<Args>(args)...);
        ++this->sz_;
    }

    void PopBack() {
        --this->sz_;
        alloc_traits::destroy(alloc_, this->arr_ + this->sz_);
    }

    void Resize(size_t count, const T& value) {
        if (count < this->sz_) {
            for (size_t i = count; i < this->sz_; ++i) {
                alloc_traits::destroy(this->alloc_, this->arr_ + i);
            }
            this->sz_ = count;
        } else {
            count -= this->sz_;
            for (size_t i = 0; i < count; ++i) {
                this->PushBack(value);
            }
        }
    }

    void Swap(Vector<T>& other) {
        std::swap(sz_, other.sz_);
        std::swap(cap_, other.cap_);
        std::swap(arr_, other.arr_);
    }

    void Swap(Vector<T>&& other) {
        std::swap(sz_, other.sz_);
        std::swap(cap_, other.cap_);
        std::swap(arr_, other.arr_);
    }

private:
    void CheckCapacity() {
        if (this->sz_ >= this->cap_) {
            this->Reserve(std::max(MIN_CAP, 2 * this->cap_));
        }
    }

public:
    ~Vector() {
        this->Clear();
        this->alloc_.deallocate(this->arr_, this->cap_);
    }
};

template <>
class Vector<void*, std::allocator<void*>> {
private:
    using T = void*;
    size_t sz_;
    size_t cap_;
    T* arr_;

public:
    Vector() : sz_(0), cap_(0), arr_(nullptr) {
    }
    void PushBack(T value) {
        this->CheckCapacity();
        this->arr_[this->sz_] = value;
        ++this->sz_;
    }

    void Reserve(size_t new_cap) {
        if (new_cap <= this->cap_) {
            return;
        }

        T* new_arr = static_cast<T*>(malloc(new_cap * sizeof(T)));

        for (size_t i = 0; i < this->sz_; ++i) {
            new (new_arr + i) T(this->arr_[i]);
        }
        for (size_t i = this->sz_; i < new_cap; ++i) {
            new (new_arr + i) T(nullptr);
        }

        // alloc_.deallocate(this->arr_, this->cap_ * sizeof(void*));
        operator delete(this->arr_, this->cap_);
        this->cap_ = new_cap;
        this->arr_ = new_arr;
    }

private:
    void CheckCapacity() {
        if (this->sz_ >= this->cap_) {
            this->Reserve(std::max(MIN_CAP, 2 * this->cap_));
        }
    }

public:
    ~Vector() noexcept {
        for (size_t i = 0; i < this->sz_; ++i) {
            if (this->arr_[i] != nullptr) {
                free(this->arr_[i]);
            }
        }
        free(this->arr_);
    }
};