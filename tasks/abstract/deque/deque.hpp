#include <cstdlib>
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <memory>

const size_t BUCKET = 32;

template <typename T, typename Alloc = std::allocator<T>>
class deque {
    struct index {
        using difference_type   = std::ptrdiff_t;

        T** i;
        long long j;

        bool operator==(const index& other) {
            return i == other.i and j == other.j;
        }

        bool operator!=(const index& other) {
            return i != other.i or j != other.j;
        }

        T& operator*() {
            return *(*i + j);
        }

        T* operator->() {
            return *i + j;
        }

        T* ptr() {
            return *i + j;
        }

        index& operator++() {
            ++j;
            if (j == BUCKET) {
                ++i;
                j = 0;
            }
            return *this;
        }

        index operator++(int) {
            index copy = *this;
            this->operator++();
            return copy;
        }

        index& operator+=(difference_type n) {
            i += n / BUCKET;
            j += n % BUCKET;
            if (j < 0) {
                --i;
                j += BUCKET;
            }
            if (j >= BUCKET) {
                ++i;
                j -= BUCKET;
            }
            return *this;
        }

        index operator+(difference_type n) {
            index copy = *this;
            copy += n;
            return copy;
        }

        index& operator--() {
            --j;
            if (j < 0) {
                --i;
                j = BUCKET - 1;
            }
            return *this;
        }

        index operator--(int) {
            index copy = *this;
            this->operator--();
            return copy;
        }

        index& operator-=(difference_type n) {
            return this->operator+=(-n);
        }

        index operator-(difference_type n) {
            return this->operator+(-n);
        }
    };

public:
    template<typename U>
    struct iterator_base {
        using value_type = U;
        using pointer = U*;
        using reference = U&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        index cur;

        explicit iterator_base(const index& ind) : cur(ind) {}
        iterator_base(const iterator_base& other) : cur(other.cur) {}
        template<typename V>
        iterator_base(const iterator_base<V>& other) : cur(other.cur) {}  // Конструктор преобразования

        reference operator*() {
            return *cur;
        }

        pointer operator->() {
            return cur.operator->();
        }

        bool operator==(const iterator_base& other) {
            return cur == other.cur;
        }

        bool operator!=(const iterator_base& other) {
            return cur != other.cur;
        }

        // Арифметические операции с +

        iterator_base& operator++() {
            ++cur;
            return *this;
        }

        iterator_base operator++(int) {
            iterator_base copy = *this;
            ++cur;
            return copy;
        }

        iterator_base& operator+=(difference_type n) {
            cur += n;
            return *this;
        }

        iterator_base operator+(difference_type n) {
            iterator_base copy = *this;
            copy.cur += n;
            return copy;
        }

        // Арифметические операции с -

        iterator_base& operator--() {
            --cur;
            return *this;
        }

        iterator_base operator--(int) {
            iterator_base copy = *this;
            --cur;
            return copy;
        }

        iterator_base& operator-=(difference_type n) {
            cur += -n;
            return *this;
        }

        iterator_base operator-(difference_type n) {
            return *this + -n;
        }
    };

    using iterator = iterator_base<T>;
    using const_iterator = iterator_base<const T>;

private:
    size_t count;      // количество элементов 
    size_t pool_size;  // размер массива указателей
    T** buckets;       // сам массив указателей
    index start;       // позиция первого элемента
    index finish;      // позиция после последнего элемента

    using alloc_type = Alloc;
    using b_alloc_type = typename std::allocator_traits<alloc_type>::rebind_alloc<T*>;
    using alloc_traits = typename std::allocator_traits<alloc_type>;
    using b_alloc_traits = std::allocator_traits<b_alloc_type>;
    alloc_type alloc;
    b_alloc_type b_alloc;

private:
    void calculate_start_finish(T** bucks, size_t pool, size_t cnt, index& s, index& f) {
        size_t mid = (BUCKET * pool - cnt) / 2;
        s.i = bucks + mid / BUCKET;
        s.j = mid % BUCKET;

        f = s + cnt;
    }

    void default_initialize(T**& bucks, size_t pool) {
        bucks = b_alloc.allocate(pool);
        for (size_t i = 0; i < pool; ++i) {
            b_alloc_traits::construct(b_alloc, bucks + i, alloc.allocate(BUCKET));
        }
    }

public:
    /* ----- КОНСТРУКТОРЫ / ДЕСТРУКТОРЫ ----- */

    explicit deque() : count(0), pool_size(0), buckets(nullptr), start(), finish() {}

    explicit deque(size_t count) : count(count), pool_size(count / BUCKET + (count % BUCKET != 0)) {
        default_initialize(buckets, pool_size);
        calculate_start_finish(buckets, pool_size, count, start, finish);
        for (index it = start; it != finish; ++it) {
            alloc_traits::construct(alloc, it.prt(), std::move(T()));
        }
    }

    deque(size_t count, const T& value) : count(count), pool_size(count / BUCKET + (count % BUCKET != 0)) {
        default_initialize(buckets, pool_size);
        calculate_start_finish(buckets, pool_size, count, start, finish);
        for (index it = start; it != finish; ++it) {
            alloc_traits::construct(alloc, it.ptr(), value);
        }
    }

    deque(std::initializer_list<T> list) : count(list.size()), pool_size(count / BUCKET + (count % BUCKET != 0)) {
        default_initialize(buckets, pool_size);
        calculate_start_finish(buckets, pool_size, count, start, finish);
        index itd = start;
        for (auto itl = list.begin(); itl != list.end(); ++itd, ++itl) {
            alloc_traits::construct(alloc, itd.ptr(), std::move(*itl));
        }
    }

    deque(const deque& other) : count(other.count), pool_size(other.pool_size) {
        default_initialize(buckets, pool_size);
        start.i = buckets + (other.start.i - other.buckets);
        start.j = other.start.j;
        finish.i = buckets + (other.finish.i - other.buckets);
        finish.j = other.finish.j;
        for (index itd = start, ito = other.start; ito != other.finish; ++itd, ++ito) {
            alloc_traits::construct(alloc, itd.ptr(), *ito);
        }
    }

    deque(deque&& other) {
        default_initialize(buckets, pool_size);
        start.i = buckets + (other.start.i - other.buckets);
        start.j = other.start.j;
        finish.i = buckets + (other.finish.i - other.buckets);
        finish.j = other.finish.j;
        for (index itd = start, ito = other.start; ito != other.finish; ++itd, ++ito) {
            alloc_traits::construct(alloc, itd.ptr(), std::move(*ito));
        }
    }

    ~deque() {
        clear();
        for (size_t i = 0; i < pool_size; ++i) {
            alloc.deallocate(*(buckets + i), BUCKET);
        }
        b_alloc.deallocate(buckets, pool_size);
    }

    /* ---- ДОСТУП К ЭЛЕМЕНТАМ ---- */

    T& operator[](size_t i) {
        return *(start + i);
    }

    const T& operator[](size_t i) const {
        return *(start + i);
    }

    T& front() {
        return *start;
    }

    const T& front() const {
        return *start;
    }

    T& back() {
        return *(finish - 1);
    }

    const T& back() const {
        return *(finish - 1);
    }

    /* ---- ИТЕРАТОРЫ ---- */

    iterator begin() noexcept {
        return iterator(start);
    }

    const_iterator begin() const noexcept {
        return const_iterator(start);
    }

    iterator end() noexcept {
        return iterator(finish);
    }

    const_iterator end() const noexcept {
        return const_iterator(finish);
    }


    /* ---- РАЗМЕР / ВМЕСТИМОСТЬ ---- */

private:
    void reserve(size_t new_pool) {
        // создаём новый массив бакетов
        T** new_buckets = b_alloc.allocate(new_pool);
        size_t k = finish.i - start.i;
        index new_start;
        new_start.i = new_buckets + (new_pool - k) / 2;
        new_start.j = start.j;
        index new_finish;
        new_finish.i = new_start.i + k;
        new_finish.j = finish.j;

        // переносим бакеты
        for (auto in = new_start.i, io = start.i; io != finish.i; ++in, ++io) {
            b_alloc_traits::construct(b_alloc, in, *io);
        }

        // удаляем старый массив бакетов
        b_alloc.deallocate(buckets, pool_size);
        buckets = new_buckets;
        start = new_start;
        finish = new_finish;
        pool_size = new_pool;
    }

public:

    bool empty() {
        return count == 0;
    }

    size_t size() {
        return count;
    }

    /* ---- ИЗМЕНЕНИЕ ---- */

    void clear() {
        for (iterator it = begin(); it != end(); ++it) {
            alloc_traits::destroy(alloc, &(*it));
        }
        count = 0;
        calculate_start_finish(buckets, pool_size, count, start, finish);
    }

    // возвращает итератор на вставленный элемент
    iterator insert(const_iterator pos, const T& value) {
        if (finish.i == buckets + (pool_size - 1) and finish.j == BUCKET - 1) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, finish.i, alloc.allocate(BUCKET));
        }

        for (iterator it = end(); it != pos; --it) {
            alloc_traits::construct(alloc, &(*it), std::move(*(it - 1)));
            alloc_traits::destroy(alloc, &(*(it - 1)));
        }
        alloc_traits::construct(alloc, &(*pos), value);
        ++count;
        return pos;;
    }

    iterator insert(const_iterator pos, T&& value) {
        if (finish.i == buckets + (pool_size - 1) and finish.j == BUCKET - 1) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, finish.i, alloc.allocate(BUCKET));
        }

        for (iterator it = end(); it != pos; --it) {
            alloc_traits::construct(alloc, &(*it), std::move(*(it - 1)));
            alloc_traits::destroy(alloc, &(*(it - 1)));
        }
        alloc_traits::construct(alloc, &(*pos), std::move(value));
        ++count;
        return pos;
    }

    template< class... Args >
    iterator emplace(const_iterator pos, Args&&... args ) {
        if (finish.i == buckets + (pool_size - 1) and finish.j == BUCKET - 1) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, finish.i, alloc.allocate(BUCKET));
        }

        for (iterator it = end(); it != pos; --it) {
            alloc_traits::construct(alloc, &(*it), std::move(*(it - 1)));
            alloc_traits::destroy(alloc, &(*(it - 1)));
        }
        alloc_traits::construct(alloc, &(*pos), std::forward<Args>(args)...);
        ++count;
        ++finish;
        return pos;
    }

    // возвращает итератор на элемент, идущий после удалённого элемента
    iterator erase(iterator pos) {
        for (iterator it = pos + 1; it != end(); ++it) {
            alloc_traits::destroy(alloc, &(*(it - 1)));
            alloc_traits::construct(alloc, &(*(it - 1)), std::move(*it));
        }
        --count;
        --finish;
        return pos;
    }


    void push_back(const T& value) {
        if (finish.i == buckets + (pool_size - 1) and finish.j == BUCKET - 1) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, finish.i, alloc.allocate(BUCKET));
        }
        alloc_traits::construct(alloc, &(*end()), value);
        ++finish;
        ++count;
    }

    void push_back(T&& value) {
        if (finish.i == buckets + pool_size and finish.j == 0) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, finish.i, alloc.allocate(BUCKET));
        }
        alloc_traits::construct(alloc, &(*end()), std::move(value));
        ++finish;
        ++count;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (finish.i == buckets + pool_size and finish.j == 0) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, finish.i, alloc.allocate(BUCKET));
        }
        alloc_traits::construct(alloc, &(*end()), std::forward<Args>(args)...);
        ++finish;
        ++count;
    }

    void pop_back() {
        --finish;
        alloc_traits::destroy(alloc, &(*end()));
        --count;
    }


    void push_front(const T& value) {
        if (start.i == buckets and start.j == 0) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, start.i - 1, alloc.allocate(BUCKET));
        }
        --start;
        alloc_traits::construct(alloc, &(*begin()), value);
        ++count;
    }

    void push_front(T&& value) {
        if (start.i == buckets and start.j == 0) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, start.i - 1, alloc.allocate(BUCKET));
        }
        --start;
        alloc_traits::construct(alloc, &(*begin()), std::move(value));
        ++count;
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        if (start.i == buckets and start.j == 0) {
            reserve(2 * pool_size + pool_size % 2);
            b_alloc_traits::construct(b_alloc, start.i - 1, alloc.allocate(BUCKET));
        }
        --start;
        alloc_traits::construct(alloc, &(*begin()), std::forward<Args>(args)...);
        ++count;
    }

    void pop_front() {
        alloc_traits::destroy(alloc, *start.i + start.j);
        ++start;
        --count;
    }
};