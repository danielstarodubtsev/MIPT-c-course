#include <iostream>

template <size_t N>
class StackStorage {
  static constexpr size_t alignment_ = alignof(::max_align_t);
  alignas(::max_align_t) char storage_[N];
  char* pointer_{storage_};

  static constexpr size_t align(size_t const n) {
    return (n + (alignment_ - 1)) & (-alignment_);
  }
 
 public:
  StackStorage() = default;

  StackStorage(const StackStorage& other) = default;

  StackStorage& operator=(const StackStorage& other) = default;

  char* allocate(size_t n) {
    n = align(n);
    pointer_ += n;
    return pointer_ - n;
  }
};

template <typename T, size_t N>
class StackAllocator {
 public:
  using store_type = StackStorage<N>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using value_type = T;
  
  static size_t constexpr type_size = sizeof(T);

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  StackAllocator() = default;

  StackAllocator(StackStorage<N>& store) : storage_(&store) {}

  template <typename U>
  StackAllocator(const StackAllocator<U, N>& other) : storage_(other.get_storage()) {}

  T* allocate(size_t n) {
    return reinterpret_cast<T*>(storage_->allocate(type_size * n));
  }

  void deallocate(T*, size_t) {}

  template <class U, size_t M>
  bool operator==(const StackAllocator<U, M>& other) const {
    return storage_ == other.get_storage();
  }

  template <class U, size_t M>
  bool operator!=(const StackAllocator<U, M>& other) const {
    return !(*this == other);
  }

 private:
  store_type* storage_;
 
 public:
  store_type* get_storage() const {
    return storage_;
  }
};

struct BaseNode {
  BaseNode* prev = nullptr;
  BaseNode* next = nullptr;

  BaseNode() : prev(this), next(this) {}

  BaseNode(const BaseNode& other) = default;
};

template <typename T>
struct Node : BaseNode {
  alignas(T) char data[sizeof(T)];

  Node() = default;
};

template <typename T, typename Allocator = std::allocator<T>>
class List {
  static void connect_nodes(BaseNode* first, BaseNode* second) {
    first->next = second;
    second->prev = first;
  }

  static void connect_nodes(BaseNode& first, BaseNode* second) {
    first.next = second;
    second->prev = &first;
  }

  static void connect_nodes(BaseNode* first, BaseNode& second) {
    first->next = &second;
    second.prev = first;
  }

  static void connect_nodes(BaseNode& first, BaseNode& second) {
    first.next = &second;
    second.prev = &first;
  }

  using allocator_traits = std::allocator_traits<typename std::allocator_traits<Allocator>
  ::template rebind_alloc<Node<T>>>;
  using AllocatorNode = typename allocator_traits::template rebind_alloc<Node<T>>;

  template <typename... Args>
  void make_node(AllocatorNode alloc, Node<T>* pointer, Args&&... args) {
    allocator_traits::template construct<Node<T>>(alloc, pointer);
    try {
      allocator_traits::template construct<T>(alloc, reinterpret_cast<T*>(pointer->data), std::forward<Args>(args)...);
    } catch (...) {
      allocator_traits::template destroy<Node<T>>(alloc, pointer);
      throw;
    }
  }

  void real_make_node(AllocatorNode alloc, Node<T>* pointer, T* value) {
    if (value) {
      make_node(alloc, pointer, *value);
    } else {
      make_node(alloc, pointer);
    }
  }

  BaseNode zero_node_;
  size_t size_ = 0;
  [[no_unique_address]] AllocatorNode alloc_;

  template <bool is_const>
  class inside_iterator {
    BaseNode* pointer_;

   public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::bidirectional_iterator_tag;
    using node_ptr = std::conditional_t<is_const, const Node<T>*, Node<T>*>;
    using node_ref = std::conditional_t<is_const, const Node<T>&, Node<T>&>;

    inside_iterator& operator++() {
      pointer_ = pointer_->next;
      return *this;
    }

    inside_iterator& operator--() {
      pointer_ = pointer_->prev;
      return *this;
    }

    inside_iterator operator++(int) {
      inside_iterator copy = pointer_;
      pointer_ = pointer_->next;
      return copy;
    }

    inside_iterator operator--(int) {
      inside_iterator copy = pointer_;
      pointer_ = pointer_->prev;
      return copy;
    }

    bool operator==(const inside_iterator& other) const {
      return pointer_ == other.pointer_;
    }

    bool operator!=(const inside_iterator& other) const {
      return !(*this == other);
    }

    inside_iterator& operator=(const inside_iterator& other) = default;

    reference operator*() const {
      return reinterpret_cast<reference>((static_cast<node_ptr>(pointer_))->data);
    }

    pointer operator->() const {
      return reinterpret_cast<pointer>((static_cast<node_ptr>(pointer_))->data);
    }

    operator inside_iterator<true>() const {
      return inside_iterator<true>(pointer_);
    }

    BaseNode* get_pointer() const {
      return pointer_;
    }

    inside_iterator(const BaseNode* pointer) : pointer_(const_cast<BaseNode*>(pointer)) {}

    inside_iterator(const inside_iterator& other) = default;
  };

 public:
  using iterator = inside_iterator<false>;
  using const_iterator = inside_iterator<true>;
  using reverse_iterator = std::reverse_iterator<inside_iterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<inside_iterator<true>>;

  iterator begin() {
    return zero_node_.next;
  }

  const_iterator begin() const {
    return zero_node_.next;
  }

  const_iterator cbegin() const {
    return zero_node_.next;
  }

  iterator end() {
    return &zero_node_;
  }

  const_iterator end() const {
    return &zero_node_;
  }

  const_iterator cend() const {
    return &zero_node_;
  }

  reverse_iterator rbegin() {
    return reverse_iterator(&zero_node_);
  }

  const_reverse_iterator rbegin() const {
    return reverse_iterator(&zero_node_);
  }

  const_reverse_iterator crbegin() const {
    return reverse_iterator(&zero_node_);
  }

  reverse_iterator rend() {
    return reverse_iterator(zero_node_.next);
  }

  const_reverse_iterator rend() const {
    return reverse_iterator(zero_node_.next);
  }

  const_reverse_iterator crend() const {
    return reverse_iterator(zero_node_.next);
  }

  size_t size() const {
    return size_;
  }

 private:
  void real_make_node(AllocatorNode alloc, Node<T>* pointer, const_iterator value) {
    make_node(alloc, pointer, *value);
  }

  void reverse_make(AllocatorNode alloc, Node<T>* pointer) {
    allocator_traits::template destroy<T>(alloc, reinterpret_cast<T*>(pointer->data));
    allocator_traits::template destroy<Node<T>>(alloc, pointer);
  }

  template <typename ValueType = T*>
  void make(size_t size, ValueType val, AllocatorNode alloc) {
    size_ = size;

    if (size == 0) {
      reverse_make(alloc);
    } else {
      size_t index = 1;
      Node<T>* pointer;
      Node<T>* cur_ptr = nullptr;

      try {
        Node<T>* prev_ptr = allocator_traits::allocate(alloc, 1);
        pointer = prev_ptr;
        real_make_node(alloc, pointer, val);

        while (index < size_) {
          if constexpr (std::is_same<const_iterator, ValueType>::value) {
            ++val;
          }

          cur_ptr = allocator_traits::allocate(alloc, 1);
          real_make_node(alloc, cur_ptr, val);
          connect_nodes(prev_ptr, cur_ptr);
          prev_ptr = cur_ptr;
          ++index;
        }
      } catch (...) {
        for (size_t cur_index = 0; cur_index < index; ++cur_index) {
          Node<T>* next = static_cast<Node<T>*>(pointer->next);
          reverse_make(alloc, pointer);
          allocator_traits::deallocate(alloc, pointer, 1);
          pointer = next;
        }
        allocator_traits::deallocate(alloc, cur_ptr, 1);
        throw;
      }

      reverse_make(alloc);
      connect_nodes(zero_node_, pointer);

      for (size_t i = 0; i < size_ - 1; ++i) {
        pointer = static_cast<Node<T>*>(pointer->next);
      }

      connect_nodes(pointer, zero_node_);
    }
  }

  void reverse_make(AllocatorNode alloc) {
    for (iterator iter = begin(); iter != end();) {
      iterator cur = iter++;
      reverse_make(alloc, static_cast<Node<T>*>(cur.get_pointer()));
      allocator_traits::deallocate(alloc, static_cast<Node<T>*>(cur.get_pointer()), 1);
    }
  }

 public:
  List() = default;

  List(size_t size) : size_(size) {
    make(size, nullptr, alloc_);
  }

  List(size_t size, const T& value) : size_(size) {
    make(size, &value, alloc_);
  }

  List(const Allocator& alloc) : alloc_(alloc) {}

  List(size_t size, const Allocator& alloc) : alloc_(alloc) {
    make(size, nullptr, alloc);
  }

  List(size_t size, const T& value, const Allocator& alloc) : alloc_(alloc) {
    make(size, &value, alloc);
  }

  List(const List& other) : size_(other.size_), alloc_(allocator_traits::select_on_container_copy_construction(other.get_allocator())) {
    make<const_iterator>(size_, other.begin(), alloc_);
  }

  List& operator=(const List& other) {
    AllocatorNode new_alloc = (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) ? other.alloc_ : alloc_;
    make<const_iterator>(other.size_, other.begin(), alloc_ = new_alloc);
    return *this;
  }

  ~List() {
    reverse_make(alloc_);
  }

  void push_back(const T& new_value) {
    ++size_;
    Node<T>* pointer = allocator_traits::allocate(alloc_, 1);

    try {
      make_node(alloc_, pointer, new_value);
    } catch (...) {
      allocator_traits::deallocate(alloc_, pointer, 1);
      throw;
    }

    connect_nodes(zero_node_.prev, pointer);
    connect_nodes(pointer, zero_node_);
  }

  void push_front(const T& new_value) {
    ++size_;
    Node<T>* pointer = allocator_traits::allocate(alloc_, 1);

    try {
      make_node(alloc_, pointer, new_value);
    } catch (...) {
      allocator_traits::deallocate(alloc_, pointer, 1);
      throw;
    }

    connect_nodes(pointer, zero_node_.next);
    connect_nodes(zero_node_, pointer);
  }

  Allocator get_allocator() const {
    return Allocator(alloc_);
  }

  void pop_back() {
    --size_;
    Node<T>* copy_remove_node = static_cast<Node<T>*>(zero_node_.prev);
    connect_nodes(zero_node_.prev->prev, zero_node_);
    reverse_make(alloc_, copy_remove_node);
    allocator_traits::deallocate(alloc_, copy_remove_node, 1);
  }

  void pop_front() {
    --size_;
    Node<T>* copy_remove_node = static_cast<Node<T>*>(begin().get_pointer());
    connect_nodes(zero_node_, zero_node_.next->next);
    reverse_make(alloc_, copy_remove_node);
    allocator_traits::deallocate(alloc_, copy_remove_node, 1);
  }

 private:
  void insert_node(const_iterator iter, BaseNode* pointer) {
    ++size_;
    connect_nodes(iter.get_pointer()->prev, pointer);
    connect_nodes(pointer, iter.get_pointer());
  }

  void uninsert_node(BaseNode* pointer) {
    --size_;
    connect_nodes(pointer->prev, pointer->next);
    reverse_make(alloc_, static_cast<Node<T>*>(pointer));
  }

 public:
  void insert(const const_iterator& iter, const T& value) {
    Node<T>* pointer = allocator_traits::allocate(alloc_, 1);

    try {
      make_node(alloc_, pointer, value);
    } catch (...) {
      allocator_traits::deallocate(alloc_, pointer, 1);
      throw;
    }

    insert_node(iter, pointer);
  }

  void erase(const const_iterator& iter) {
    Node<T>* copy_remove_node = static_cast<Node<T>*>(iter.get_pointer());
    uninsert_node(copy_remove_node);
    allocator_traits::deallocate(alloc_, copy_remove_node, 1);
  }
};
