#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class Vector {
private:
    using allocator_type = Alloc;
    using alloc_traits = std::allocator_traits<allocator_type>;
    allocator_type alloc_;

    size_t sz_;
    size_t cap_;
    T* arr_;
    // using T_alloc_type = typename std::allocator_traits<allocator_type>::template rebind<T>::other;

public:
    Vector() : alloc_(), sz_(0), cap_(10), arr_(alloc_.allocate(cap_)) {
    }

    Vector(const allocator_type& a) : alloc_(a), sz_(0), cap_(10), arr_(alloc_.allocate(cap_)) {
    }

    Vector(size_t count, const T& value, const allocator_type& a = allocator_type())
        : alloc_(a), sz_(count), cap_(count), arr_(alloc_.allocate(cap_)) {
        std::uninitialized_fill_n(this->arr_, count, value);
    }

    Vector(const Vector& other)
        : alloc_(other.alloc_), sz_(other.sz_), cap_(other.cap_), arr_(other.alloc_.allocate(other.cap_)) {
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
        std::swap(sz_, other.sz_);
        std::swap(cap_, other.cap_);
        std::swap(arr_, other.arr_);
    }

    Vector(std::initializer_list<T> init) {
    }

    Vector& operator=(const Vector& other);

    Vector& operator=(Vector&& other);

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

    void Insert(size_t pos, T value);

    void Erase(size_t begin_pos, size_t end_pos);

    void PushBack(T value) {
        // TODO: exception safety
        if (this->sz_ >= this->cap_) {
            this->Reserve(2 * this->cap_);
        }
        new (this->arr_ + this->sz_) T(value);  // TODO: использовать аллокаторы
        ++this->sz_;
    }

    template <class... Args>
    void EmplaceBack(Args&&... args);

    void PopBack() {
        --this->sz_;
        alloc_traits::destroy(alloc_, &this->arr_[sz_]);
    }

    void Resize(size_t count, const T& value);

    ~Vector() {
        this->Clear();
        this->alloc_.deallocate(this->arr_, this->cap_);
    }
};
