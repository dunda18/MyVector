#ifndef MY_VECTOR_MY_VECTOR_H
#define MY_VECTOR_MY_VECTOR_H

#include <memory>
#include <initializer_list>
#include <stdexcept>

template <typename T>
class Vector {
public:
    class Iterator {
        using IteratorTag = std::random_access_iterator_tag;
        T* ptr_;

    public:
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;
        typedef IteratorTag iterator_category;

        Iterator(T* ptr) : ptr_(ptr) {
        }

        Iterator& operator++() {
            ++ptr_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator--() {
            --ptr_;
            return *this;
        }
        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        Iterator& operator+=(ptrdiff_t offset) {
            ptr_ += offset;
            return *this;
        }

        Iterator& operator-=(ptrdiff_t offset) {
            ptr_ -= offset;
            return *this;
        }

        Iterator operator+(ptrdiff_t offset) const {
            return Iterator(ptr_ + offset);
        }

        Iterator operator-(ptrdiff_t offset) const {
            return Iterator(ptr_ - offset);
        }

        ptrdiff_t operator-(const Iterator& other) const {
            return ptr_ - other.ptr_;
        }

        T& operator*() const {
            return *ptr_;
        }
        T* operator->() const {
            return ptr_;
        }

        bool operator==(const Iterator& rhs) const {
            return ptr_ == rhs.ptr_;
        }
        bool operator!=(const Iterator& rhs) const {
            return ptr_ != rhs.ptr_;
        }
        bool operator<(const Iterator& rhs) const {
            return ptr_ < rhs.ptr_;
        }
        bool operator<=(const Iterator& rhs) const {
            return ptr_ <= rhs.ptr_;
        }
        bool operator>(const Iterator& rhs) const {
            return ptr_ > rhs.ptr_;
        }
        bool operator>=(const Iterator& rhs) const {
            return ptr_ >= rhs.ptr_;
        }
    };

    Vector() : data_(reinterpret_cast<T*>(new std::byte[capacity_ * sizeof(T)])) {
    }

    Vector(size_t size) : size_(size), capacity_(size_),
                          data_(reinterpret_cast<T*>(new std::byte[capacity_ * sizeof(T)])) {
        try {
            std::uninitialized_default_construct_n(data_, size_);
        } catch (...) {
            delete[] reinterpret_cast<std::byte*>(data_);
            throw;
        }
    }

    Vector(size_t size, const T& value) : size_(size), capacity_(size_),
                                          data_(reinterpret_cast<T*>(new std::byte[capacity_ * sizeof(T)])) {
        try {
            std::uninitialized_fill_n(data_, size_, value);
        } catch (...) {
            delete[] reinterpret_cast<std::byte*>(data_);
            throw;
        }
    }

    Vector(std::initializer_list<T> init) : size_(init.size()), capacity_(size_),
                                            data_(reinterpret_cast<T*>(new std::byte[capacity_ * sizeof(T)])) {
        try {
            std::uninitialized_copy(init.begin(), init.end(), data_);
        } catch (...) {
            delete[] reinterpret_cast<std::byte*>(data_);
            throw;
        }
    }

    Vector(const Vector& other) : size_(other.size_), capacity_(size_),
                                  data_(reinterpret_cast<T*>(new std::byte[capacity_ * sizeof(T)])) {
        try {
            std::uninitialized_copy_n(other.data_, size_, data_);
        } catch (...) {
            delete[] reinterpret_cast<std::byte*>(data_);
            throw;
        }
    }

    Vector(Vector&& other) : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    ~Vector() {
        Clear();
        if (data_) {
            delete[] reinterpret_cast<std::byte*>(data_);
        }
    }

    Vector& operator=(const Vector& other) {
        Clear();
        Reserve(other.size_);
        std::uninitialized_copy_n(other.data_, other.size_, data_);
        size_ = other.size_;
        return *this;
    }

    Vector& operator=(Vector&& other) {
        Clear();
        delete[] reinterpret_cast<std::byte*>(data_);
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = other.data_;
        other.data_ = nullptr;
        other.size_ = 0;
        return *this;
    }

    Vector& operator=(std::initializer_list<T> ilist) {
        Clear();
        Reserve(ilist.size());
        std::uninitialized_copy_n(ilist.begin(), ilist.size(), data_);
        size_ = ilist.size();
        return *this;
    }

    void Swap(Vector& other) {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(data_, other.data_);
    }

    void Assign(size_t count, const T& value) {
        Clear();
        Reserve(count);
        std::uninitialized_fill_n(data_, count, value);
        size_ = count;
    }

    void Assign(std::initializer_list<T> ilist) {
        Clear();
        Reserve(ilist.size());
        std::uninitialized_copy_n(ilist.begin(), ilist.size(), data_);
        size_ = ilist.size();
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        T* new_data = reinterpret_cast<T*>(new std::byte[new_capacity * sizeof(T)]);
        try {
            std::uninitialized_copy_n(data_, size_, new_data);
        } catch (...) {
            delete[] reinterpret_cast<std::byte*>(new_data);
            throw;
        }
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        delete[] reinterpret_cast<std::byte*>(data_);
        data_ = new_data;
        capacity_ = new_capacity;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            for (size_t i = new_size; i < size_; ++i) {
                data_[i].~T();
            }
            size_ = new_size;
            return;
        }
        if (new_size > capacity_) {
            Reserve(new_size);
        }
        std::uninitialized_default_construct_n(data_ + size_, new_size - size_);
        size_ = new_size;
    }

    void Resize(size_t new_size, const T& value) {
        if (new_size <= size_) {
            for (size_t i = new_size; i < size_; ++i) {
                data_[i].~T();
            }
            size_ = new_size;
            return;
        }
        if (new_size > capacity_) {
            Reserve(new_size);
        }
        std::uninitialized_fill_n(data_ + size_, new_size - size_, value);
        size_ = new_size;
    }

    void PushBack(const T& value) {
        if (size_ + 1 > capacity_) {
            Reserve(2 * capacity_ + 1);
        }
        new (data_ + size_) T(value);
        size_++;
    }

    void PushBack(T&& value) {
        if (size_ + 1 > capacity_) {
            Reserve(2 * capacity_ + 1);
        }
        new (data_ + size_) T(std::move(value));
        size_++;
    }

    template <typename... Args>
    T& EmplaceBack(Args&&... args) {
        if (size_ + 1 > capacity_) {
            Reserve(2 * capacity_ + 1);
        }
        new (data_ + size_) T(std::forward<Args>(args)...);
        return data_[size_++];
    }

    void PopBack() {
        if (Empty()) {
            throw std::underflow_error("PopBack on empty Vector");
        }
        data_[--size_].~T();
    }

    Iterator Insert(const Iterator& pos, const T& value) {
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Out of range");
        }
        if (size_ + 1 > capacity_) {
            Reserve(2 * capacity_ + 1);
        }
        if (index == size_) {
            new (data_ + size_) T(value);
            return Iterator(data_ + size_++);
        }
        new (data_ + size_) T(Back());
        for (size_t i = size_ - 1; i > index; --i) {
            data_[i] = data_[i - 1];
        }
        data_[index] = value;
        ++size_;
        return Iterator(data_ + index);
    }

    Iterator Insert(const Iterator& pos, T&& value) {
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Out of range");
        }
        if (size_ + 1 > capacity_) {
            Reserve(2 * capacity_ + 1);
        }
        if (index == size_) {
            new (data_ + size_) T(std::move(value));
            return Iterator(data_ + size_++);
        }
        new (data_ + size_) T(Back());
        for (size_t i = size_ - 1; i > index; --i) {
            data_[i] = data_[i - 1];
        }
        data_[index] = std::move(value);
        ++size_;
        return Iterator(data_ + index);
    }

    Iterator Insert(const Iterator& pos, size_t count, const T& value) {
        if (count == 0) {
            return pos;
        }
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Out of range");
        }
        if (size_ + count > capacity_) {
            Reserve(2 * (size_ + count) + 1);
        }
        if (index == size_) {
            std::uninitialized_fill_n(data_ + size_, count, value);
            size_ += count;
            return Iterator(data_ + index);
        }
        for (size_t i = size_ + count - 1; i >= index + count; --i) {
            if (i >= size_) {
                new (data_ + i) T(data_[i - count]);
            } else {
                data_[i] = data_[i - count];
            }
        }
        for (size_t i = index; i < index + count; ++i) {
            if (i >= size_) {
                new (data_ + i) T(value);
            } else {
                data_[i] = value;
            }
        }
        size_ += count;
        return Iterator(data_ + index);
    }

    Iterator Insert(const Iterator& pos, std::initializer_list<T> ilist) {
        if (ilist.size() == 0) {
            return pos;
        }
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Out of range");
        }
        if (size_ + ilist.size() > capacity_) {
            Reserve(2 * (size_ + ilist.size()) + 1);
        }
        if (index == size_) {
            std::uninitialized_copy_n(ilist.begin(), ilist.size(), data_ + size_);
            size_ += ilist.size();
            return Iterator(data_ + index);
        }
        for (size_t i = size_ + ilist.size() - 1; i >= index + ilist.size(); --i) {
            if (i >= size_) {
                new (data_ + i) T(data_[i - ilist.size()]);
            } else {
                data_[i] = data_[i - ilist.size()];
            }
        }
        for (size_t i = index; i < index + ilist.size(); ++i) {
            if (i >= size_) {
                new (data_ + i) T(ilist[i - index]);
            } else {
                data_[i] = ilist[i - index];
            }
        }
        size_ += ilist.size();
        return Iterator(data_ + index);
    }

    template <typename... Args>
    Iterator Emplace(const Iterator& pos, Args&&... args) {
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Out of range");
        }
        if (size_ + 1 > capacity_) {
            Reserve(2 * capacity_ + 1);
        }
        if (index == size_) {
            new (data_ + size_) T(std::forward<Args>(args)...);
            return Iterator(data_ + size_++);
        }
        new (data_ + size_) T(Back());
        for (size_t i = size_ - 1; i > index; --i) {
            data_[i] = data_[i - 1];
        }
        T value(std::forward<Args>(args)...);
        data_[index] = std::move(value);
        ++size_;
        return Iterator(data_ + index);
    }

    Iterator Erase(const Iterator& pos) {
        size_t index = pos - begin();
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        for (size_t i = index; i + 1 < size_; ++i) {
            data_[i] = data_[i + 1];
        }
        data_[--size_].~T();
        return Iterator(data_ + index);
    }

    Iterator Erase(const Iterator& first, const Iterator& last) {
        if (first == last) {
            return first;
        }
        size_t index = first - begin();
        size_t count = last - first;
        if (index + count > size_) {
            throw std::out_of_range("Out of range");
        }
        for (size_t i = index; i + count < size_; ++i) {
            data_[i] = data_[i + count];
        }
        for (size_t i = size_ - count; i < size_; ++i) {
            data_[i].~T();
        }
        size_ -= count;
        return Iterator(data_ + index);
    }

    void Clear() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    T& operator[](size_t pos) {
        return data_[pos];
    }

    const T& operator[](size_t pos) const {
        return data_[pos];
    }

    T& At(size_t pos) {
        if (pos >= size_) {
            throw std::out_of_range("Out of range");
        }
        return data_[pos];
    }

    const T& At(size_t pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Out of range");
        }
        return data_[pos];
    }

    T& Front() {
        if (Empty()) {
            throw std::underflow_error("Front on empty Vector");
        }
        return data_[0];
    }

    const T& Front() const {
        if (Empty()) {
            throw std::underflow_error("Front on empty Vector");
        }
        return data_[0];
    }

    T& Back() {
        if (Empty()) {
            throw std::underflow_error("Front on empty Vector");
        }
        return data_[size_ - 1];
    }

    const T& Back() const {
        if (Empty()) {
            throw std::underflow_error("Front on empty Vector");
        }
        return data_[size_ - 1];
    }

    T* Data() {
        return data_;
    }

    const T* Data() const {
        return data_;
    }

    bool Empty() const {
        return size_ == 0;
    }

    size_t Capacity() const {
        return capacity_;
    }

    size_t Size() const {
        return size_;
    }

    Iterator begin() const {  // NOLINT
        return Iterator(data_);
    }

    Iterator end() const {  // NOLINT
        return Iterator(data_ + size_);
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    T* data_;
};

template <typename T>
bool operator==(const Vector<T>& lhs, const Vector<T>& rhs) {
    if (lhs.Size() != rhs.Size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.Size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool operator!=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs == rhs);
}

template <typename T>
bool operator<(const Vector<T>& lhs, const Vector<T>& rhs) {
    for (size_t i = 0; i < std::min(lhs.Size(), rhs.Size()); ++i) {
        if (lhs[i] != rhs[i]) {
            return lhs[i] < rhs[i];
        }
    }
    return lhs.Size() < rhs.Size();
}

template <typename T>
bool operator<=(const Vector<T>& lhs, const Vector<T>& rhs) {
    for (size_t i = 0; i < std::min(lhs.Size(), rhs.Size()); ++i) {
        if (lhs[i] != rhs[i]) {
            return lhs[i] < rhs[i];
        }
    }
    return lhs.Size() <= rhs.Size();
}

template <typename T>
bool operator>(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs <= rhs);
}

template <typename T>
bool operator>=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs < rhs);
}

#endif //MY_VECTOR_MY_VECTOR_H
