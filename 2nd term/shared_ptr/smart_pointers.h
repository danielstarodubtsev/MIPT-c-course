#include <iostream>
#include <memory>
#include <type_traits>

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

struct BaseControlBlock {
  size_t shared_count = 1;
  size_t weak_count = 0;

  virtual void apply_deleter() {};
  virtual void apply_dealloc() {};

  virtual ~BaseControlBlock() = default;
};

template <typename T, typename Deleter, typename Alloc>
struct ControlBlockRegular : BaseControlBlock {
  T* value;
  Deleter del;
  Alloc alloc;

  explicit ControlBlockRegular(T* value) : value(value) {}

  ControlBlockRegular(T* value, Deleter del, Alloc alloc) : value(value), del(del), alloc(alloc) {}

  void apply_deleter() override {
    del(value);
  }

  void apply_dealloc() override {
    using AllocToUse = typename std::allocator_traits<Alloc>::template
    rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    AllocToUse alloc_to_use = alloc;
    std::allocator_traits<AllocToUse>::deallocate(alloc_to_use, this, 1);
  }
};

template <typename T, typename Alloc>
struct ControlBlockMakeShared : BaseControlBlock {
  T value;
  Alloc alloc;

  ControlBlockMakeShared() = default;

  template <typename... Args>
  ControlBlockMakeShared(const Alloc& alloc, Args&&... args) : value(std::forward<Args>(args)...),
                                                               alloc(alloc) {}

  void apply_deleter() override {
    std::allocator_traits<Alloc>::destroy(alloc, &value);
  }

  void apply_dealloc() override {
    using AllocToUse = typename std::allocator_traits<Alloc>::template
    rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
    AllocToUse alloc_to_use = alloc;
    std::allocator_traits<AllocToUse>::deallocate(alloc_to_use, this, 1);
  }
};


template <typename T>
class SharedPtr {
 private:
  T* ptr = nullptr;
  BaseControlBlock* control_block = nullptr;

  template <typename Alloc>
  SharedPtr(ControlBlockMakeShared<T, Alloc>* control_block) : ptr(&control_block->value),
                                                               control_block(control_block) {}

 public:
  void swap(SharedPtr& other) {
    std::swap(ptr, other.ptr);
    std::swap(control_block, other.control_block);
  }

  T& operator*() const {
    return *ptr;
  }

  T* operator->() const {
    return ptr;
  }

  size_t use_count() const {
    return control_block->shared_count;
  }

  void reset(T* new_ptr) {
    SharedPtr<T>(new_ptr).swap(*this);
  }

  void reset() {
    SharedPtr<T>().swap(*this);
  }

  T* get() const {
    return ptr;
  }

  SharedPtr() = default;

  template <typename U>
  SharedPtr(U* new_ptr) : ptr(new_ptr),
                          control_block(new ControlBlockRegular<T,
                                        std::default_delete<T>, std::allocator<T>>(ptr)) {}

  template <typename U, typename Deleter, typename Alloc = std::allocator<T>>
  SharedPtr(U* new_ptr, Deleter del, Alloc alloc = std::allocator<T>()) : ptr(new_ptr) {
    using AllocToUse = typename std::allocator_traits<Alloc>::template
    rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    AllocToUse alloc_to_use = alloc;
    control_block = std::allocator_traits<AllocToUse>::allocate(alloc_to_use, 1);
    new (control_block) ControlBlockRegular<T, Deleter, Alloc>(ptr, del, alloc);
  }

  SharedPtr(const SharedPtr& other) : ptr(other.ptr), control_block(other.control_block) {
    if (control_block) {
      ++control_block->shared_count;
    }
  }

  template <typename U>
  SharedPtr(const SharedPtr<U>& other) : ptr(other.ptr), control_block(other.control_block) {
    if (control_block) {
      ++control_block->shared_count;
    }
  }

  template <typename U>
  explicit SharedPtr(const WeakPtr<U>& other) : ptr(other.ptr),
                                                control_block(other.control_block) {
    if (control_block) {
      ++control_block->shared_count;
    }
  }

  SharedPtr(SharedPtr&& other) : ptr(other.ptr), control_block(other.control_block) {
    other.ptr = nullptr;
    other.control_block = nullptr;
  }

  template <typename U>
  SharedPtr(SharedPtr<U>&& other) : ptr(other.ptr), control_block(other.control_block) {
    other.ptr = nullptr;
    other.control_block = nullptr;
  }

  SharedPtr& operator=(const SharedPtr& other) {
    SharedPtr copy = other;
    copy.swap(*this);
    return *this;
  }

  template <typename U>
  SharedPtr& operator=(const SharedPtr<U>& other) {
    SharedPtr copy = other;
    copy.swap(*this);
    return *this;
  }

  template <typename U>
  SharedPtr& operator=(SharedPtr<U>&& other) {
    SharedPtr copy = std::move(other);
    copy.swap(*this);
    return *this;
  }

  ~SharedPtr() {
    if (control_block) {
      --control_block->shared_count;

      if (control_block->shared_count == 0) {
        control_block->apply_deleter();

        if (control_block->weak_count == 0) {
          control_block->apply_dealloc();
        }
      }
    }
  }

  template <typename U>
  friend class SharedPtr;

  template <typename U>
  friend class WeakPtr;

  template <typename U, typename... Args>
  friend SharedPtr<U> makeShared(Args&&...);

  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(const Alloc&, Args&&...);
};

template <typename T>
class WeakPtr {
 private:
  T* ptr = nullptr;
  BaseControlBlock* control_block = nullptr;

 public:
  void swap(WeakPtr& other) {
    std::swap(ptr, other.ptr);
    std::swap(control_block, other.control_block);
  }

  size_t use_count() const {
    return control_block ? control_block->shared_count : 0;
  }

  bool expired() const {
    return control_block->shared_count == 0;
  }

  SharedPtr<T> lock() const {
    return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
  }

  WeakPtr() = default;

  WeakPtr(const WeakPtr& other) : ptr(other.ptr), control_block(other.control_block) {
    if (control_block) {
      ++control_block->weak_count;
    }
  }

  template <typename U>
  WeakPtr(const WeakPtr<U>& other) : ptr(other.ptr), control_block(other.control_block) {
    if (control_block) {
      ++control_block->weak_count;
    }
  }

  template <typename U>
  WeakPtr(const SharedPtr<U>& other) : ptr(other.ptr), control_block(other.control_block) {
    if (control_block) {
      ++control_block->weak_count;
    }
  }

  template <typename U>
  WeakPtr& operator=(const WeakPtr<U>& other) {
    WeakPtr copy = other;
    copy.swap(*this);
    return *this;
  }

  WeakPtr& operator=(const SharedPtr<T>& other) {
    WeakPtr copy = other;
    copy.swap(*this);
    return *this;
  }

  ~WeakPtr() {
    if (control_block) {
      --control_block->weak_count;

      if (control_block->shared_count == 0 && control_block->weak_count == 0) {
        control_block->apply_dealloc();
      }
    }
  }

  template <typename U>
  friend class SharedPtr;

  template <typename U>
  friend class WeakPtr;
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    auto control_block = new ControlBlockMakeShared<T, std::allocator<T>>(
                             std::allocator<T>(), std::forward<Args>(args)...);
    return SharedPtr<T>(control_block);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    using AllocToUse = typename std::allocator_traits<Alloc>::template
    rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
    AllocToUse alloc_to_use = alloc;
    auto where = std::allocator_traits<AllocToUse>::allocate(alloc_to_use, 1);
    std::allocator_traits<AllocToUse>::construct(alloc_to_use, where, alloc,
                                                 std::forward<Args>(args)...);
    return SharedPtr<T>(where);
}

template <typename T>
class EnableSharedFromThis {
 private:
  WeakPtr<T> ptr;

 public:
  SharedPtr<T> shared_from_this() const {
    return ptr.lock();
  }

  template <typename U>
  friend class SharedPtr;
};