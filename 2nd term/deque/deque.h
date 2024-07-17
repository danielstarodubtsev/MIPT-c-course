#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>

template <typename T>
class Deque {
 private:
  static constexpr size_t kBucketSize = 32;

  struct IndexPair {
    size_t out_index = 0;
    size_t in_index = 0;

    bool operator==(const IndexPair& other) const {
      return out_index == other.out_index && in_index == other.in_index;
    }

    bool operator!=(const IndexPair& other) const {
      return !(other == *this);
    }

    IndexPair& operator+=(size_t shift) {
      size_t overall_index = out_index * kBucketSize + in_index + shift;
      out_index = overall_index / kBucketSize;
      in_index = overall_index % kBucketSize;
      return *this;
    }

    IndexPair operator+(size_t shift) const {
      IndexPair result = *this;
      result += shift;
      return result;
    }

    IndexPair& operator-=(size_t shift) {
      size_t overall_index = out_index * kBucketSize + in_index - shift;
      out_index = overall_index / kBucketSize;
      in_index = overall_index % kBucketSize;
      return *this;
    }

    IndexPair operator-(size_t shift) const {
      IndexPair result = *this;
      result -= shift;
      return result;
    }

    IndexPair& operator=(const IndexPair& other) {
      out_index = other.out_index;
      in_index = other.in_index;
      return *this;
    }

    size_t operator-(const IndexPair& other) const {
      return kBucketSize * out_index + in_index - kBucketSize * other.out_index - other.in_index;
    }

    IndexPair() = default;

    IndexPair(const IndexPair& other) = default;

    IndexPair(size_t out_index, size_t in_index) : out_index(out_index), in_index(in_index) {}
  };

  size_t size_ = 0;
  std::vector<T*> out_array_;
  IndexPair begin_index_;

  void ClearMemory() {
    for (size_t i = 0; i < out_array_.size(); ++i) {
      delete[] reinterpret_cast<char*>(out_array_[i]);
    }
  }

  void RemoveFirst(size_t cnt) {
    for (size_t i = 0; i < cnt; ++i) {
      IndexPair cur_index = begin_index_ + i;
      (out_array_[cur_index.out_index] + cur_index.in_index)->~T();
    }
  }

  void AllocateMemory(size_t need_size) {
    std::vector<T*> new_out_array(need_size);

    for (size_t i = 0; i < need_size; ++i) {
      new_out_array[i] = reinterpret_cast<T*>(new char[kBucketSize * sizeof(T)]);
    }

    std::swap(out_array_, new_out_array);
  }

  template <bool is_const>
  class inside_iterator {
   private:
    IndexPair index_pair_;
    typename std::conditional_t<is_const, T const*, T*> pointer_;
    typename std::conditional_t<is_const, T* const*, T**> pointer2_;
   public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::random_access_iterator_tag;

    inside_iterator& operator+=(size_t shift) {
      size_t old_out = index_pair_.out_index;
      index_pair_ += shift;

      if (index_pair_.out_index != old_out) {
        pointer_ = *(pointer2_ + index_pair_.out_index);
      }

      return *this;
    }

    inside_iterator operator+(size_t shift) const {
      inside_iterator result = *this;
      result += shift;
      return result;
    }

    inside_iterator& operator-=(size_t shift) {
      size_t old_out = index_pair_.out_index;
      index_pair_ -= shift;

      if (index_pair_.out_index != old_out && old_out != 0) {
        pointer_ = *(pointer2_ + index_pair_.out_index);
      }

      return *this;
    }

    inside_iterator operator-(size_t shift) const {
      inside_iterator result = *this;
      result -= shift;
      return result;
    }

    inside_iterator& operator++() {
      *this += 1;
      return *this;
    }

    inside_iterator& operator--() {
      *this -= 1;
      return *this;
    }

    inside_iterator operator++(int) {
      inside_iterator copy = *this;
      *this += 1;
      return copy;
    }

    inside_iterator operator--(int) {
      inside_iterator copy = *this;
      *this -= 1;
      return copy;
    }

    inside_iterator& operator=(const inside_iterator& other) {
      inside_iterator copy(other);
      std::swap(copy.index_pair_, index_pair_);
      std::swap(copy.pointer_, pointer_);
      std::swap(copy.pointer2_, pointer2_);
      return *this;
    }

    std::ptrdiff_t operator-(const inside_iterator& other) const {
      return index_pair_ - other.index_pair_;
    }

    std::conditional_t<is_const, const T*, T*> operator->() const {
      return pointer_ + index_pair_.in_index;
    }

    std::conditional_t<is_const, const T&, T&> operator*() const {
      return *(operator->());
    }

    bool operator==(const inside_iterator& other) const {
      return index_pair_ == other.index_pair_;
    }

    bool operator!=(const inside_iterator& other) const {
      return !(other == *this);
    }

    bool operator<(const inside_iterator& other) const {
      return (other - *this) > 0;
    }

    bool operator>(const inside_iterator& other) const {
      return other < *this;
    }

    bool operator<=(const inside_iterator& other) const {
      return !(*this > other);
    }

    bool operator>=(const inside_iterator& other) const {
      return !(*this < other);
    }

    operator inside_iterator<true>() const {
      inside_iterator<true> result(index_pair_, pointer_, pointer2_);
      return result;
    }

    inside_iterator() = default;

    inside_iterator(const inside_iterator& other) = default;

    inside_iterator(IndexPair index_pair, typename std::conditional_t<is_const, T const*,
                    T*> pointer) : index_pair_(index_pair), pointer_(pointer) {}

    inside_iterator(IndexPair index_pair, typename std::conditional_t<is_const, T const*,
                    T*> pointer, typename std::conditional_t<is_const, T* const*,
                    T**> pointer2) : index_pair_(index_pair), pointer_(pointer),
                    pointer2_(pointer2) {}
  };

 public:
  size_t size() const {
    return size_;
  }

  T& operator[](size_t index) {
    IndexPair new_index = begin_index_ + index;
    return out_array_[new_index.out_index][new_index.in_index];
  }

  const T& operator[](size_t index) const {
    IndexPair new_index = begin_index_ + index;
    return out_array_[new_index.out_index][new_index.in_index];
  }

  T& at(size_t index) {
    if (index < size_) {
      return (*this)[index];
    }

    throw std::out_of_range("aboba");
  }

  const T& at(size_t index) const {
    if (index < size_) {
      return (*this)[index];
    }

    throw std::out_of_range("aboba");
  }

  void pop_front() {
    --size_;
    (out_array_[begin_index_.out_index] + begin_index_.in_index)->~T();
    begin_index_ += 1;
  }

  void pop_back() {
    --size_;
    (out_array_[(begin_index_ + size_).out_index] + (begin_index_ + size_).in_index)->~T();
  }

  void push_back(const T& element) {
    if ((begin_index_ + size_ + 1).out_index < out_array_.size()) {
      new (out_array_[(begin_index_ + size_).out_index] + 
          (begin_index_ + size_).in_index) T(element);
      ++size_;
    } else {
      size_t new_size = out_array_.empty() ? 1 : out_array_.size() * 2;
      size_t to_where = 0;
      std::vector<T*> new_out_array(new_size);

      try {
        std::copy(out_array_.begin(), out_array_.end(), new_out_array.begin());

        for (size_t index = out_array_.size(); index < new_size; ++index) {
          to_where = index;
          new_out_array[index] = reinterpret_cast<T*>(new char[kBucketSize * sizeof(T)]);
        }

        new(new_out_array[(begin_index_ + size_).out_index] +
                          (begin_index_ + size_).in_index) T(element);
        std::swap(new_out_array, out_array_);
        ++size_;
      } catch (...) {
        for (int i = out_array_.size(); i <= to_where; ++i) {
          delete[] reinterpret_cast<char*>(new_out_array[i]);
        }

        throw;
      }
    }
  }

  void push_front(const T& element) {
    if (begin_index_.out_index != 0 || begin_index_.in_index != 0) {
      new (out_array_[(begin_index_ - 1).out_index] + (begin_index_ - 1).in_index) T(element);
      begin_index_ -= 1;
      ++size_;
    } else {
      size_t new_size = out_array_.empty() ? 1 : out_array_.size() * 2;
      size_t to_where = 0;
      std::vector<T*> new_out_array(new_size);

      try {
        std::copy(out_array_.begin(), out_array_.end(), new_out_array.begin() + out_array_.size());

        for (size_t index = 0; index < out_array_.size(); ++index) {
          to_where = index;
          new_out_array[index] = reinterpret_cast<T*>(new char[kBucketSize * sizeof(T)]);
        }

        new(new_out_array[(begin_index_ + kBucketSize * out_array_.size() - 1).out_index] +
                          (begin_index_ + kBucketSize * out_array_.size() - 1).in_index) T(element);
        begin_index_ += (out_array_.size() * kBucketSize - 1);
        std::swap(new_out_array, out_array_);
        ++size_;
      } catch (...) {
        for (int i = 0; i <= to_where; ++i) {
          delete[] reinterpret_cast<char*>(new_out_array[i]);
        }

        throw;
      }
    }
  }

  Deque& operator=(const Deque& other) {
    Deque<T> copy(other);
    std::swap(size_, copy.size_);
    std::swap(begin_index_, copy.begin_index_);
    std::swap(out_array_, copy.out_array_);

    return *this;
  }

  using iterator = inside_iterator<false>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  inside_iterator<false> begin() {
    return inside_iterator<false>(begin_index_, out_array_[begin_index_.out_index],
                                  out_array_.data());
  }

  inside_iterator<true> begin() const {
    return inside_iterator<true>(begin_index_, out_array_[begin_index_.out_index],
                                 out_array_.data());
  }

  inside_iterator<true> cbegin() const {
    return inside_iterator<true>(begin_index_, out_array_[begin_index_.out_index],
                                 out_array_.data());
  }

  inside_iterator<false> end() {
    return inside_iterator<false>(begin_index_ + size_,
                                  out_array_[(begin_index_ + size_).out_index], out_array_.data());
  }

  inside_iterator<true> end() const {
    return inside_iterator<true>(begin_index_ + size_,
                                 out_array_[(begin_index_ + size_).out_index], out_array_.data());
  }

  inside_iterator<true> cend() const {
    return inside_iterator<true>(begin_index_ + size_,
                                 out_array_[(begin_index_ + size_).out_index], out_array_.data());
  }

  reverse_iterator rbegin() {
    return reverse_iterator(end());
  }

  std::reverse_iterator<inside_iterator<true>> rbegin() const {
    return std::reverse_iterator<inside_iterator<true>>(end());
  }

  std::reverse_iterator<inside_iterator<true>> crbegin() const {
    return std::reverse_iterator<inside_iterator<true>>(end());
  }

  std::reverse_iterator<inside_iterator<false>> rend() {
    return std::reverse_iterator<inside_iterator<false>>(begin());
  }

  std::reverse_iterator<inside_iterator<true>> rend() const {
    return std::reverse_iterator<inside_iterator<true>>(begin());
  }

  std::reverse_iterator<inside_iterator<true>> crend() const {
    return std::reverse_iterator<inside_iterator<true>>(begin());
  }

  void insert(const inside_iterator<false>& iter, const T& element) {
    push_back(element);
    size_t index = iter - begin();

    for (size_t i = size_ - 1; i > index; --i) {
      std::swap((*this)[i], (*this)[i - 1]);
    }
  }

  void erase(const inside_iterator<false>& iter) {
    size_t index = iter - begin();

    for (size_t i = index; i < size_ - 1; ++i) {
      std::swap((*this)[i], (*this)[i + 1]);
    }
    
    pop_back();
  }

  Deque(int size, const T& object) : size_(size) {
    AllocateMemory(size_ / kBucketSize + 1);

    for (size_t shift = 0; shift < size_; ++shift) {
      IndexPair index_pair = begin_index_ + shift;

      try {
        new(out_array_[index_pair.out_index] + index_pair.in_index) T(object);
      } catch (...) {
        RemoveFirst(shift);
        ClearMemory();
        throw;
      }
    }
  }

  Deque(int size) : Deque(size, T()) {}

  Deque() {
    AllocateMemory(1);
  };

  Deque(const Deque& other) : size_(other.size_), begin_index_(other.begin_index_) {
    AllocateMemory(size_ / kBucketSize + 1);

    for (size_t shift = 0; shift < size_; ++shift) {
      IndexPair index_pair = begin_index_ + shift;

      try {
        new(out_array_[index_pair.out_index] + index_pair.in_index)T(other[(begin_index_
                + shift).out_index * kBucketSize + (begin_index_ + shift).in_index]);
      } catch (...) {
        RemoveFirst(shift);
        ClearMemory();
        throw;
      }
    }
  }

  ~Deque() {
    RemoveFirst(size_);
    ClearMemory();
  }
};
