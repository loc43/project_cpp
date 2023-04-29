#pragma once
#include <memory>
#include <iterator>
#include <stdexcept>
#include <exception>
#include <initializer_list>

template <typename T>
class Vector {
 public:
  using ValueType = T;
  using Pointer = T*;
  using ConstPointer = const T*;
  using Reference = T&;
  using ConstReference = const T&;
  using SizeType = size_t;
  using Iterator = Pointer;
  using ConstIterator = ConstPointer;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  T* buffer_{nullptr};
  size_t size_{0}, capacity_{0};

  Vector() noexcept : size_(0), capacity_(0) {
    buffer_ = nullptr;
  }

  ~Vector() noexcept {
    delete[] buffer_;
    size_ = 0;
    capacity_ = 0;
  }

  Vector(const Vector& other) {
    size_ = other.size_;
    capacity_ = other.capacity_;
    try {
      buffer_ = capacity_ > 0 ? new T[capacity_] : nullptr;
      for (size_t i = 0; i < size_; ++i) {
        buffer_[i] = other.buffer_[i];
      }
    } catch (...) {
      delete[] buffer_;
      buffer_ = nullptr;
      size_ = capacity_ = 0;

      throw;
    }
  }

  Vector(Vector&& other) noexcept
      : buffer_(std::exchange(other.buffer_, nullptr))
      , size_(std::exchange(other.size_, 0))
      , capacity_(std::exchange(other.capacity_, 0)) {
  }

  Vector(std::initializer_list<T>&& init_list) {
    try {
      capacity_ = std::distance(init_list.begin(), init_list.end());
      buffer_ = capacity_ > 0 ? new T[capacity_] : nullptr;
      auto iterator = init_list.begin();
      size_t i = 0;
      while (iterator != init_list.end()) {
        buffer_[i] = *iterator;
        ++iterator;
        ++i;
      }
      size_ = i;
    } catch (...) {
      delete[] buffer_;
      buffer_ = nullptr;
      size_ = capacity_ = 0;

      throw;
    }
  }

  template <class Iterator, class = std::enable_if_t<std::is_base_of_v<
                                std::forward_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>

  Vector(Iterator first, Iterator last) {
    try {
      capacity_ = std::distance(first, last);
      buffer_ = capacity_ > 0 ? new T[capacity_] : nullptr;
      auto iterator = first;
      size_t i = 0;
      while (iterator != last) {
        buffer_[i] = *iterator;
        ++iterator;
        ++i;
      }
      size_ = i;
      capacity_ = i;
    } catch (...) {
      delete[] buffer_;
      buffer_ = nullptr;
      size_ = capacity_ = 0;

      throw;
    }
  }

  explicit Vector(size_t size) {
    Resize(size);
  }

  Vector(size_t size, const T& value) {
    Resize(size, value);
  }

  Vector& operator=(const Vector& other) {
    if (this != &other) {
      auto copy_buffer = std::exchange(buffer_, nullptr);
      auto copy_size = std::exchange(size_, 0);
      auto copy_capacity = std::exchange(capacity_, 0);
      try {
        Vector(other).Swap(*this);
        delete[] copy_buffer;
      } catch (...) {
        buffer_ = copy_buffer;
        size_ = copy_size;
        capacity_ = copy_capacity;
        throw;
      }
    }
    return *this;
  }

  Vector& operator=(Vector&& other) noexcept {
    if (this != &other) {
      Vector(std::move(other)).Swap(*this);
    }
    return *this;
  }

  [[nodiscard]] size_t Size() const noexcept {
    return size_;
  }

  [[nodiscard]] size_t Capacity() const noexcept {
    return capacity_;
  }

  [[nodiscard]] bool Empty() const noexcept {
    return size_ == 0;
  }

  [[nodiscard]] const T& Front() const noexcept {
    return buffer_[0];
  }

  [[nodiscard]] T& Front() noexcept {
    return const_cast<T&>(buffer_[0]);
  }

  [[nodiscard]] const T& Back() const noexcept {
    return buffer_[size_ - 1];
  }

  [[nodiscard]] T& Back() noexcept {
    return const_cast<T&>(buffer_[size_ - 1]);
  }

  [[nodiscard]] const T& operator[](size_t idx) const noexcept {
    return buffer_[idx];
  }

  [[nodiscard]] T& operator[](size_t idx) noexcept {
    return const_cast<T&>(buffer_[idx]);
  }

  [[nodiscard]] const T& At(size_t idx) const {
    if (idx >= size_) {
      throw std::out_of_range("");
    }
    return (*this)[idx];
  }

  [[nodiscard]] T& At(size_t idx) {
    if (idx >= size_) {
      throw std::out_of_range("");
    }
    return const_cast<T&>((*this)[idx]);
  }

  [[nodiscard]] const T* Data() const noexcept {
    return buffer_;
  }

  [[nodiscard]] T* Data() noexcept {
    return const_cast<T*>(const_cast<const Vector&>(*this).Data());
  }

  void Swap(Vector& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  void Resize(size_t size, const T& value) {
    if (size == size_) {
      return;
    }
    auto copy_buffer = buffer_;
    auto copy_size = size_;
    auto copy_capacity = capacity_;
    if (size <= capacity_) {
      try {
        for (size_t i = size_; i < size; ++i) {
          buffer_[i] = value;
        }
        size_ = size;
      } catch (...) {
        size_ = copy_size;
        throw;
      }
    } else {
      T* new_buffer = nullptr;
      try {
        auto new_cap = std::max(capacity_ * 2, size);
        new_buffer = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) {
          new_buffer[i] = std::move_if_noexcept(buffer_[i]);
        }
        for (size_t i = size_; i < size; ++i) {
          new_buffer[i] = value;
        }
        delete[] copy_buffer;
        buffer_ = new_buffer;
        size_ = size;
        capacity_ = new_cap;
      } catch (...) {
        delete[] new_buffer;
        size_ = copy_size;
        capacity_ = copy_capacity;
        buffer_ = copy_buffer;
        //        delete[] copy_buffer;
        throw;
      }
    }
  }

  void Resize(size_t size) {
    if (size == size_) {
      return;
    }
    auto copy_buffer = buffer_;
    auto copy_size = size_;
    auto copy_capacity = capacity_;
    if (size <= capacity_) {
      try {
        for (size_t i = size_; i < size; ++i) {
          buffer_[i] = T();
        }
        size_ = size;
      } catch (...) {
        size_ = copy_size;
        throw;
      }
    } else {
      T* new_buffer = nullptr;
      try {
        auto new_cap = std::max(capacity_ * 2, size);
        new_buffer = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) {
          new_buffer[i] = std::move_if_noexcept(buffer_[i]);
        }
        for (size_t i = size_; i < size; ++i) {
          new_buffer[i] = T();
        }
        delete[] copy_buffer;
        buffer_ = new_buffer;
        size_ = size;
        capacity_ = new_cap;
      } catch (...) {
        delete[] new_buffer;
        size_ = copy_size;
        capacity_ = copy_capacity;
        buffer_ = copy_buffer;
        //        delete[] copy_buffer;
        throw;
      }
    }
  }

  void Reserve(size_t capacity) {
    if (capacity <= capacity_) {
      return;
    }
    auto copy_size = size_;
    auto copy_capacity = capacity_;
    auto copy_buffer = buffer_;
    T* new_buffer = nullptr;
    try {
      if (capacity_ < capacity) {
        new_buffer = new T[capacity];
        for (size_t i = 0; i < size_; ++i) {
          new_buffer[i] = std::move_if_noexcept(buffer_[i]);
        }
        delete[] copy_buffer;
        buffer_ = new_buffer;
      }
      capacity_ = capacity;
    } catch (...) {
      size_ = copy_size;
      capacity_ = copy_capacity;
      buffer_ = copy_buffer;
      delete[] new_buffer;
      throw;
    }
  }

  void ShrinkToFit() {
    if (size_ == capacity_) {
      return ;
    }
    auto copy_size = size_;
    auto copy_capacity = capacity_;
    auto copy_buffer = buffer_;
    T* new_buffer = nullptr;
    try {
      new_buffer = size_ == 0 ? nullptr : new T[size_];
      for (size_t i = 0; i < size_; ++i) {
        new_buffer[i] = std::move_if_noexcept(buffer_[i]);
      }
      capacity_ = size_;
      buffer_ = new_buffer;
      delete[] copy_buffer;
    } catch (...) {
      delete new_buffer;
      size_ = copy_size;
      capacity_ = copy_capacity;
      buffer_ = copy_buffer;
      throw;
    }

  }

  void Clear() noexcept(std::is_nothrow_destructible_v<T>) {
    size_ = 0;
  }

  void PushBack(const T& value) {
    auto copy_buffer = buffer_;
    auto copy_size = size_;
    auto copy_capacity = capacity_;
    if (capacity_ == 0) {
      try {
        buffer_ = new T[1];
        buffer_[0] = value;
        ++size_;
        ++capacity_;
      } catch (...) {
        delete buffer_;
        buffer_ = nullptr;
      }
    } else if (size_ < capacity_) {
      try {
        buffer_[size_] = value;
        ++size_;
      } catch (...) {
        size_ = copy_size;
        throw;
      }
    } else {
      T* new_buffer = nullptr;
      try {
        auto new_cap = capacity_ * 2;
        new_buffer = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) {
          new_buffer[i] = std::move_if_noexcept(buffer_[i]);
        }
        new_buffer[size_] = value;
        delete[] copy_buffer;
        buffer_ = new_buffer;
        ++size_;
        capacity_ = new_cap;
      } catch (...) {
        delete[] new_buffer;
        size_ = copy_size;
        capacity_ = copy_capacity;
        buffer_ = copy_buffer;
        throw;
      }
    }
  }

  void PushBack(T&& value) {
    auto copy_buffer = buffer_;
    auto copy_size = size_;
    auto copy_capacity = capacity_;
    if (capacity_ == 0) {
      try {
        buffer_ = new T[1];
        buffer_[0] = std::move(value);
        ++size_;
        ++capacity_;
      } catch (...) {
        delete buffer_;
        buffer_ = nullptr;
      }
    } else if (size_ < capacity_) {
      try {
        buffer_[size_] = std::move(value);
        ++size_;
      } catch (...) {
        size_ = copy_size;
        throw;
      }
    } else {
      T* new_buffer = nullptr;
      try {
        auto new_cap = capacity_ * 2;
        new_buffer = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) {
          new_buffer[i] = std::move_if_noexcept(buffer_[i]);
        }
        new_buffer[size_] = std::move(value);

        delete[] copy_buffer;
        buffer_ = new_buffer;
        ++size_;
        capacity_ = new_cap;
      } catch (...) {
        delete[] new_buffer;
        size_ = copy_size;
        capacity_ = copy_capacity;
        buffer_ = copy_buffer;
        //        delete[] copy_buffer;
        throw;
      }
    }
  }

  void PopBack() noexcept(std::is_nothrow_destructible_v<T>) {
    --size_;
  }

  [[nodiscard]] const T* cbegin() const noexcept {  // NOLINT
    return Data();
  }

  [[nodiscard]] const T* begin() const noexcept {  // NOLINT
    return cbegin();
  }

  [[nodiscard]] T* begin() noexcept {  // NOLINT
    return Data();
  }

  [[nodiscard]] const T* cend() const noexcept {  // NOLINT
    return Data() + size_;
  }

  [[nodiscard]] const T* end() const noexcept {  // NOLINT
    return cend();
  }

  [[nodiscard]] T* end() noexcept {  // NOLINT
    return Data() + size_;
  }

  [[nodiscard]] std::reverse_iterator<const T*> crbegin() const noexcept {  // NOLINT
    return std::reverse_iterator<const T*>(cend());
  }

  [[nodiscard]] std::reverse_iterator<const T*> rbegin() const noexcept {  // NOLINT
    return crbegin();
  }

  [[nodiscard]] std::reverse_iterator<T*> rbegin() noexcept {  // NOLINT
    return std::reverse_iterator<T*>(end());
  }

  [[nodiscard]] std::reverse_iterator<const T*> crend() const noexcept {  // NOLINT
    return std::reverse_iterator<const T*>(cbegin());
  }

  [[nodiscard]] std::reverse_iterator<const T*> rend() const noexcept {  // NOLINT
    return crend();
  }

  [[nodiscard]] std::reverse_iterator<T*> rend() noexcept {  // NOLINT
    return std::reverse_iterator<T*>(begin());
  }
};

template <typename T>
[[nodiscard]] bool operator==(const Vector<T>& first, const Vector<T>& second) noexcept {
  return first <= second && second <= first;
}

template <typename T>
[[nodiscard]] bool operator<(const Vector<T>& first, const Vector<T>& second) noexcept {
  return std::lexicographical_compare(first.buffer_, (first.buffer_ + first.size_), second.buffer_,
                                      (second.buffer_ + second.size_));
}

template <typename T>
[[nodiscard]] inline bool operator!=(const Vector<T>& first, const Vector<T>& second) noexcept {
  return !(first == second);
}

template <typename T>
[[nodiscard]] inline bool operator>(const Vector<T>& first, const Vector<T>& second) noexcept {
  return std::lexicographical_compare(second.buffer_, (second.buffer_ + second.size_), first.buffer_,
                                      (first.buffer_ + first.size_));
}

template <typename T>
[[nodiscard]] inline bool operator<=(const Vector<T>& first, const Vector<T>& second) noexcept {
  return !(first > second);
}

template <typename T>
[[nodiscard]] inline bool operator>=(const Vector<T>& first, const Vector<T>& second) noexcept {
  return !(first < second);
}