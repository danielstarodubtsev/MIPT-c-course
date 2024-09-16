#include <cmath>
#include <iostream>
#include <vector>

struct BaseNode {
  BaseNode* prev = nullptr;
  BaseNode* next = nullptr;

  BaseNode() : prev(this), next(this) {}

  void swap(BaseNode& other) {
    std::swap(prev, other.prev);
    std::swap(next, other.next);
  }
};

template <typename T>
struct Node : BaseNode {
  alignas(T) char data[sizeof(T)];
  size_t hash;

  const T& get_value() const {
    return reinterpret_cast<T&>(data);
  }

  T& get_value() {
    return reinterpret_cast<T&>(data);
  }

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

  using AllocatorNode = typename std::allocator_traits<Allocator>::template rebind_alloc<Node<T>>;
  using allocator_traits = std::allocator_traits<AllocatorNode>;
 
  template <typename... Args>
  static void make_node(AllocatorNode alloc, Node<T>* pointer, Args&&... args) {
    allocator_traits::construct(alloc, pointer);
    try {
      allocator_traits::construct(alloc, &(pointer->get_value()), std::forward<Args>(args)...);
    } catch (...) {
      allocator_traits::destroy(alloc, pointer);
      throw;
    }
  }

  static void real_make_node(AllocatorNode alloc, Node<T>* pointer, T* value) {
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

    inside_iterator() = default;

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
    allocator_traits::destroy(alloc, reinterpret_cast<T*>(pointer->data));
    allocator_traits::destroy(alloc, pointer);
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

  void reset() {
    zero_node_.next = &zero_node_;
    zero_node_.prev = &zero_node_;
    size_ = 0;
  }

  void clear() {
    while (size()) {
      pop_back();
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

  List(const List& other) : size_(other.size_),
       alloc_(allocator_traits::select_on_container_copy_construction(other.get_allocator())) {
    make<const_iterator>(size_, other.begin(), alloc_);
  }

  List(List&& other) : size_(other.size_), alloc_(other.alloc_) {
    connect_nodes(zero_node_, other.zero_node_.next);
    connect_nodes(other.zero_node_.prev, zero_node_);
    other.reset();

    if (size_ == 0) {
      reset();
    }
  }

  List& operator=(const List& other) {
    AllocatorNode new_alloc = 
    (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) ?
                                                                        other.alloc_ : alloc_;
    make<const_iterator>(other.size_, other.begin(), alloc_ = new_alloc);
    return *this;
  }

  List& operator=(List&& other) {
    //! Might be wrong: case when (propagate==false && alloc_ != other.alloc_) is not covered
    clear();
    size_ = other.size_;

    if constexpr
       (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
      alloc_ = std::move(other.alloc_);
    }

    connect_nodes(zero_node_, other.zero_node_.next);
    connect_nodes(other.zero_node_.prev, zero_node_);
    other.reset();

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

  void push_back(T&& new_value) {
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

  void push_front(T&& new_value) {
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

 private:
  AllocatorNode get_allocator_node() const {
    return alloc_;
  }

 public:
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

  const BaseNode& get_fake_node() const {
    return zero_node_;
  }

  void insert_node(const const_iterator& iter, BaseNode* pointer) {
    ++size_;
    connect_nodes(iter.get_pointer()->prev, pointer);
    connect_nodes(pointer, iter.get_pointer());
  }

 private:
  void uninsert_node(BaseNode* pointer) {
    --size_;
    connect_nodes(pointer->prev, pointer->next);
    reverse_make(alloc_, static_cast<Node<T>*>(pointer));
  }

  void swap(List& other) {
    std::swap(size_, other.size_);
    zero_node_.swap(other.zero_node_);
    if constexpr
       (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
      std::swap(alloc_, other.alloc_);
    }
  }

 public:
  template <typename... Args>
  void emplace(const const_iterator& iter, Args&&... args) {
    Node<T>* pointer = allocator_traits::allocate(alloc_, 1);

    try {
      make_node(alloc_, pointer, std::forward<Args>(args)...);
    } catch (...) {
      allocator_traits::deallocate(alloc_, pointer, 1);
      throw;
    }

    insert_node(iter, pointer);
  }

  void insert(const const_iterator& iter, const T& value) {
    emplace(iter, value);
  }

  void insert(const const_iterator& iter, T&& value) {
    emplace(iter, std::move(value));
  }

  void insert(const const_iterator& iter, Node<T>* pointer) {
    insert_node(iter, pointer);
  }

  void erase(const const_iterator& iter) {
    Node<T>* copy_remove_node = static_cast<Node<T>*>(iter.get_pointer());
    uninsert_node(copy_remove_node);
    allocator_traits::deallocate(alloc_, copy_remove_node, 1);
  }

  template <typename, typename, typename, typename, typename>
  friend class UnorderedMap;
};

const std::vector<size_t> sizes = {13, 29, 59, 127, 257, 541, 1'109, 2'357, 5'087, 10'273, 20'753,
                                   42'043, 85'229, 172'933, 351'061, 712'697, 1'447'153, 2'938'679,
                                   5'967'347, 12'117'689, 24'607'243, 49'969'847, 101'473'717};

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
 public:
  using NodeType = std::pair<const Key, Value>;

 private:
  std::vector<Node<NodeType>*> pointers_to_nodes;
  List<NodeType, Alloc> nodes;
  [[no_unique_address]] Hash hash;
  [[no_unique_address]] Equal equal;
  [[no_unique_address]] Alloc alloc;
  float max_load_factor_ = 1;

 public:
  void rehash(size_t count) {
    size_t new_bucket_count = *std::lower_bound(sizes.begin(), sizes.end(),
                               std::max(count, size_t(std::ceil(size() / max_load_factor()))));
    std::vector<Node<NodeType>*> new_pointers(new_bucket_count, nullptr);
    List<NodeType, Alloc> new_nodes;

    BaseNode* cur_node = nodes.get_fake_node().next;

    while (cur_node != &nodes.get_fake_node()) {
      Node<NodeType>* real_cur_node = static_cast<Node<NodeType>*>(cur_node);
      BaseNode* next = cur_node->next;
      size_t cur_hash = real_cur_node->hash;
      size_t cur_hash_mod = cur_hash % new_bucket_count;

      if (new_pointers[cur_hash_mod] == nullptr) {
        new_nodes.insert(new_nodes.begin(), real_cur_node);
      } else {
        new_nodes.insert((typename List<NodeType, Alloc>::iterator)(new_pointers[cur_hash_mod]),
                                                                    real_cur_node);
      }

      new_pointers[cur_hash_mod] = real_cur_node;
      cur_node = next;
    }

    nodes.size_ = 0; // because i destroyed the list without it knowing xD
    pointers_to_nodes = std::move(new_pointers);
    nodes = std::move(new_nodes);
  }

  size_t size() const {
    return nodes.size();
  }
  
  size_t bucket_count() const {
    return pointers_to_nodes.size();
  }

  float load_factor() const {
    return float(size()) / bucket_count();
  }

  float max_load_factor() const {
    return max_load_factor_;
  }
 
 private:
  void rehash_if_needed() {
    if (load_factor() > max_load_factor()) {
      rehash(0);
    }
  }

  Value& create_new_key_pointer(const Key& key, size_t cur_hash) {
    nodes.push_front(NodeType(key, Value()));
    static_cast<Node<NodeType>*>(nodes.get_fake_node().next)->hash = cur_hash;
    pointers_to_nodes[cur_hash % bucket_count()] =
                      static_cast<Node<NodeType>*>(nodes.get_fake_node().next);
    Value& to_return = pointers_to_nodes[cur_hash % bucket_count()]->get_value().second;
    rehash_if_needed();
    return to_return;
  }

  Value& create_new_key_pointer(Key&& key, size_t cur_hash) {
    nodes.push_front(NodeType(std::move(key), Value()));
    static_cast<Node<NodeType>*>(nodes.get_fake_node().next)->hash = cur_hash;
    pointers_to_nodes[cur_hash % bucket_count()] =
                      static_cast<Node<NodeType>*>(nodes.get_fake_node().next);
    Value& to_return = pointers_to_nodes[cur_hash % bucket_count()]->get_value().second;
    rehash_if_needed();
    return to_return; 
  }

 public:
  void swap(UnorderedMap& other) {
    pointers_to_nodes.swap(other.pointers_to_nodes);
    std::swap(hash, other.hash);
    std::swap(equal, other.equal);
    
    if (std::allocator_traits<Alloc>::propagate_on_container_swap::value) {
      std::swap(alloc, other.alloc);
    }

    std::swap(max_load_factor_, other.max_load_factor_);
  }

  Alloc get_allocator() {
    return alloc;
  }

  void max_load_factor(float new_max_load_factor) {
    max_load_factor_ = new_max_load_factor;
    rehash_if_needed();
  }

  void reserve(size_t count) {
    rehash(std::ceil(count / max_load_factor()));
  }

  Value& operator[](const Key& key) {
    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      return create_new_key_pointer(key, cur_hash);
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return cur_node->get_value().second;
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    nodes.insert((typename List<NodeType, Alloc>::iterator)(cur_node), NodeType(key, Value()));
    static_cast<Node<NodeType>*>(cur_node->prev)->hash = cur_hash;
    pointers_to_nodes[cur_hash_mod] = static_cast<Node<NodeType>*>(cur_node->prev);
    Value& to_return = static_cast<Node<NodeType>*>(cur_node->prev)->get_value().second;
    rehash_if_needed();

    return to_return;
  }

  Value& operator[](Key&& key) {
    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      return create_new_key_pointer(std::move(key), cur_hash);
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return cur_node->get_value().second;
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    nodes.insert((typename List<NodeType, Alloc>::iterator)(cur_node), NodeType(std::move(key),
                                                                       Value()));
    pointers_to_nodes[cur_hash_mod] = static_cast<Node<NodeType>*>(cur_node->prev);
    static_cast<Node<NodeType>*>(cur_node->prev)->hash = cur_hash;
    Value& to_return = static_cast<Node<NodeType>*>(cur_node->prev)->get_value().second;
    rehash_if_needed();

    return to_return;
  }

  Value& at(const Key& key) {
    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      throw std::out_of_range("Key doesn't exist");
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return cur_node->get_value().second;
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    throw std::out_of_range("Key doesn't exist");
  }

  const Value& at(const Key& key) const {
    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      throw std::out_of_range("Key doesn't exist");
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return cur_node->get_value().second;
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    throw std::out_of_range("Key doesn't exist");
  }

  void clear() {
    erase(begin(), end());
  }

  UnorderedMap(size_t bucket_cnt) {
    pointers_to_nodes.assign(bucket_cnt, nullptr);
  }

  UnorderedMap() : UnorderedMap(sizes[0]) {}

  UnorderedMap(const UnorderedMap& other) : pointers_to_nodes(other.pointers_to_nodes),
          nodes(other.nodes), hash(other.hash), equal(other.equal),
          alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.alloc)),
          max_load_factor_(other.max_load_factor_) {
    for (auto& el : other) {
      insert(el);
    }
  }

  UnorderedMap(UnorderedMap&& other) = default;

  UnorderedMap& operator=(UnorderedMap&& other) {
    if (!std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value &&
        alloc != other.alloc) {
      // we have no choice but to move every element individually
      clear();
      insert(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
      other.clear();
    } else {
      pointers_to_nodes = std::move(other.pointers_to_nodes);
      nodes = std::move(other.nodes);
      hash = std::move(other.hash);
      equal = std::move(other.equal);
      
      if constexpr (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value) {
        alloc = std::move(other.alloc);
      }

      max_load_factor_ = other.max_load_factor_;
      other.max_load_factor_ = 0;
    }

    return *this;
  }

  UnorderedMap& operator=(const UnorderedMap& other) {
    if constexpr (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
      alloc = other.alloc;
    }

    UnorderedMap copy = other;
    swap(copy);

    return *this;
  }

 private:
  template <bool IsConst>
  class base_iterator {
   private:
    using list_iter_type = typename std::conditional_t<IsConst,
                           typename List<NodeType, Alloc>::const_iterator,
                           typename List<NodeType, Alloc>::iterator>;
    list_iter_type iter;

   public:
    using value_type = NodeType;
    using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
    using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;

    base_iterator& operator++() {
      ++iter;
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator copy = *this;
      ++iter;
      return copy;
    }

    base_iterator& operator=(const base_iterator& other) {
      base_iterator copy(other);
      std::swap(iter, copy.iter);
      return *this;
    }

    pointer operator->() const {
      return iter.operator->();
    }

    reference operator*() const {
      return *(operator->());
    }

    bool operator==(const base_iterator& other) const {
      return iter == other.iter;
    }

    bool operator!=(const base_iterator& other) const {
      return !(*this == other);
    }

    operator base_iterator<true>() const {
      base_iterator<true> result(iter);
      return result;
    }

    list_iter_type get_iter() {
      return iter;
    }

    base_iterator() = default;

    base_iterator(const base_iterator& other) = default;

    base_iterator(typename List<NodeType, Alloc>::const_iterator list_iter) {
      BaseNode* list_node = list_iter.get_pointer();
      typename List<NodeType, Alloc>::iterator new_list_iter(list_node);
      iter = new_list_iter;
    }

    base_iterator(Node<NodeType>* node_ptr) {
      typename List<NodeType, Alloc>::iterator list_iter = node_ptr;
      iter = list_iter;
    }
  };

 public:
  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() {
    return iterator(nodes.begin());
  }

  const_iterator begin() const {
    return const_iterator(nodes.begin());
  }

  const_iterator cbegin() const {
    return const_iterator(nodes.begin());
  }

  iterator end() {
    return iterator(nodes.end());
  }

  const_iterator end() const {
    return const_iterator(nodes.end());
  }

  const_iterator cend() const {
    return const_iterator(nodes.end());
  }

  reverse_iterator rbegin() {
    return reverse_iterator(end());
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(end());
  }

  reverse_iterator rend() {
    return reverse_iterator(begin());
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(begin());
  }


 public:
  iterator find(const Key& key) {
    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      return end();
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return iterator(cur_node);
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    return end();
  }

  const_iterator find(const Key& key) const {
    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      return end();
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return const_iterator(cur_node);
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    return end();
  }

  std::pair<iterator, bool> insert(const NodeType& node_type) {
    const Key& key = node_type.first;

    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      nodes.push_front(node_type);
      static_cast<Node<NodeType>*>(nodes.get_fake_node().next)->hash = cur_hash;
      pointers_to_nodes[cur_hash % bucket_count()] =
                        static_cast<Node<NodeType>*>(nodes.get_fake_node().next);
      iterator to_return = iterator(nodes.get_fake_node().next);
      
      rehash_if_needed();
      return std::make_pair(to_return, true);
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        return std::make_pair(iterator(cur_node), false);
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    nodes.insert((typename List<NodeType, Alloc>::iterator)(cur_node), node_type);
    pointers_to_nodes[cur_hash_mod] = static_cast<Node<NodeType>*>(cur_node->prev);
    static_cast<Node<NodeType>*>(cur_node->prev)->hash = cur_hash;
    iterator to_return = iterator(cur_node->prev);

    rehash_if_needed();
    return std::make_pair(to_return, true);
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    Node<NodeType>* pointer = List<NodeType, Alloc>::allocator_traits::allocate(nodes.alloc_, 1);
    nodes.make_node(nodes.get_allocator_node(), pointer, std::forward<Args>(args)...);
    
    const Key& key = pointer->get_value().first;

    size_t cur_hash = hash(key);
    size_t cur_hash_mod = cur_hash % bucket_count();

    if (pointers_to_nodes[cur_hash_mod] == nullptr) {
      nodes.insert(nodes.begin(), pointer);
      pointers_to_nodes[cur_hash % bucket_count()] =
                        static_cast<Node<NodeType>*>(nodes.get_fake_node().next);
      iterator for_return = iterator(nodes.get_fake_node().next);
      pointer->hash = cur_hash;

      rehash_if_needed();
      return std::make_pair(for_return, true);
    }

    Node<NodeType>* cur_node = pointers_to_nodes[cur_hash_mod];

    while (cur_node->hash % bucket_count() == cur_hash_mod) {
      if (equal(cur_node->get_value().first, key)) {
        nodes.reverse_make(nodes.get_allocator_node(), pointer);
        List<NodeType, Alloc>::allocator_traits::deallocate(nodes.alloc_, pointer, 1);
        return std::make_pair(iterator(cur_node), false);
      }

      if (cur_node->next == &nodes.get_fake_node()) {
        break;
      }

      cur_node = static_cast<Node<NodeType>*>(cur_node->next);
    }

    nodes.insert((typename List<NodeType, Alloc>::iterator)(cur_node), pointer);
    pointers_to_nodes[cur_hash_mod] = static_cast<Node<NodeType>*>(cur_node->prev);
    iterator for_return = iterator(cur_node->prev);
    pointer->hash = cur_hash;

    rehash_if_needed();  
    return std::make_pair(for_return, true);
  }

  std::pair<iterator, bool> insert(NodeType&& node_type) {
    return emplace(std::forward<std::pair<Key, Value>>
    ({std::move(const_cast<Key&>(node_type.first)), std::move(node_type.second)}));
  }

  template <typename InputIterator>
  void insert(InputIterator first, InputIterator last) {
    for (InputIterator iter = first; iter != last; ++iter) {
      insert(std::forward<decltype(*iter)>(*iter));
    }
  }

  iterator erase(const_iterator pos) {
    BaseNode* node = pos.get_iter().get_pointer();
    Node<NodeType>* real_node = static_cast<Node<NodeType>*>(node);
    iterator to_return = iterator(node->next);

    // Cases:
    // 1) there was no pointer to the node that is about to be deleted
    // 2) there was a pointer but this is not the only node with this hash
    // 3) there was a pointer and this node is the only one with this hash

    size_t hash = real_node->hash;
    if (pointers_to_nodes[hash % bucket_count()] == real_node) {
      if (real_node->next != &nodes.get_fake_node() && real_node->hash % bucket_count() ==
          static_cast<Node<NodeType>*>(real_node->next)->hash % bucket_count()) {
        // case 2
        pointers_to_nodes[hash % bucket_count()] = static_cast<Node<NodeType>*>(real_node->next);
      } else {
        // case 3
        pointers_to_nodes[hash % bucket_count()] = nullptr;
      }
    }

    // in case 1 there is no need to change pointers_to_nodes
    nodes.erase(pos.get_iter());

    return to_return;
  }

  iterator erase(const_iterator first, const_iterator last) {
    const_iterator iter = first;

    while (iter != last) {
      const_iterator next = iter;
      ++next;
      erase(iter);
      iter = next;
    }

    return iterator(iter.get_iter());
  }
};
