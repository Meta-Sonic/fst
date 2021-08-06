///
/// BSD 3-Clause License
///
/// Copyright (c) 2021, Alexandre Arsenault
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// * Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
/// * Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// * Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
///

#pragma once
#include <fst/assert>
#include <fst/unused>
#include <fst/memory>
#include <fst/traits>
#include <fst/pointer>
#include <cstring>

// https://github.com/Tencent/rapidjson/blob/master/include/rapidjson/allocators.h
namespace fst {

template <typename _InternalAllocatorType, bool _Freeable, bool _RefCounted>
class internal_allocator_base {
public:
  using internal_allocator_type = _InternalAllocatorType;
  static constexpr bool is_freeable = _Freeable;
  static constexpr bool is_ref_counted = _RefCounted;
};

/// Runtime library allocator.
/// This class is just wrapper for standard C library memory routines.
class crt_allocator : public internal_allocator_base<crt_allocator, true, false> {
public:
  inline void* allocate(std::size_t size) { return size ? fst::memory::malloc(size) : nullptr; }

  inline void* realloc(void* original_ptr, std::size_t original_size, std::size_t new_size) {
    fst::unused(original_size);

    if (new_size == 0) {
      fst::memory::free(original_ptr);
      return nullptr;
    }

    return fst::memory::realloc(original_ptr, new_size);
  }

  inline static void free(void* ptr) noexcept { fst::memory::free(ptr); }

  inline bool operator==(const crt_allocator&) const noexcept { return true; }
  inline bool operator!=(const crt_allocator&) const noexcept { return false; }
};

namespace detail {
  template <typename _BaseAllocator>
  struct shared_data_base_impl {
    _BaseAllocator* ownBaseAllocator;

    static constexpr bool is_empty = false;
  };

  struct empty_shared_data_base {
    static constexpr bool is_empty = true;
  };

  template <typename _BaseAllocator>
  using shared_data_base = std::conditional_t<std::is_empty_v<_BaseAllocator>, empty_shared_data_base,
      shared_data_base_impl<_BaseAllocator>>;

  template <typename _BaseAllocator>
  struct memory_pool_base_impl {
    memory_pool_base_impl() = default;
    memory_pool_base_impl(_BaseAllocator* b)
        : baseAllocator_(b) {}

    memory_pool_base_impl(const memory_pool_base_impl&) = default;
    memory_pool_base_impl(memory_pool_base_impl&&) = default;

    memory_pool_base_impl& operator=(const memory_pool_base_impl&) = default;
    memory_pool_base_impl& operator=(memory_pool_base_impl&&) = default;

    _BaseAllocator* baseAllocator_;

    static constexpr bool is_empty = false;
  };

  struct empty_memory_pool_base {
    static constexpr bool is_empty = true;
  };

  template <typename _BaseAllocator>
  using memory_pool_base = std::conditional_t<std::is_empty_v<_BaseAllocator>, empty_memory_pool_base,
      memory_pool_base_impl<_BaseAllocator>>;
} // namespace detail.

/// MemoryPoolAllocator
///
/// Default memory allocator used by the parser and DOM.
/// This allocator allocate memory blocks from pre-allocated memory chunks.
/// It does not free memory blocks. And Realloc() only allocate new memory.
/// The memory chunks are allocated by BaseAllocator, which is CrtAllocator by default.
/// User may also supply a buffer as the first chunk.
/// If the user-buffer is full then additional chunks are allocated by BaseAllocator.
/// The user-buffer is not deallocated by this allocator.
/// @tparam BaseAllocator the allocator type for allocating memory chunks. Default is CrtAllocator.
/// @note implements Allocator concept
///
template <typename BaseAllocator = crt_allocator>
class memory_pool_allocator : private detail::memory_pool_base<BaseAllocator>,
                              public internal_allocator_base<memory_pool_allocator<BaseAllocator>, false, true> {

  using base = detail::memory_pool_base<BaseAllocator>;
  static constexpr std::size_t default_alignement = 8;
  static constexpr std::size_t default_chunk_capacity = 64 * 1024;

  using base_empty = std::bool_constant<std::is_empty_v<BaseAllocator>>;
  using base_not_empty = std::bool_constant<!std::is_empty_v<BaseAllocator>>;
  static constexpr bool is_base_empty = base_empty::value;

  /// Chunk header for perpending to each chunk.
  /// Chunks are stored as a singly linked list.
  struct alignas(default_alignement) chunk_header {
    /// Capacity of the chunk in bytes (excluding the header itself).
    std::size_t capacity;
    /// Current size of allocated memory in bytes.
    std::size_t size;
    /// Next chunk in the linked list.
    chunk_header* next;
  };

public:
  struct refcount_type {
    bool own_buffer : 1; // = false;
    std::uint32_t refcount : 31; // = 0;
  };

  struct alignas(default_alignement) shared_data : detail::shared_data_base<BaseAllocator> {
    /// Head of the chunk linked-list. Only the head chunk serves allocation.
    chunk_header* chunk_head;
    refcount_type rc;
    //    std::uint32_t refcount;
    //    bool own_buffer;
  };

  static inline chunk_header* GetChunkHead(shared_data* shared) {
    return reinterpret_cast<chunk_header*>(reinterpret_cast<std::uint8_t*>(shared) + sizeof(shared_data));
  }

  static inline std::uint8_t* GetChunkBuffer(shared_data* shared) {
    return reinterpret_cast<std::uint8_t*>(shared->chunk_head) + sizeof(chunk_header);
  }

  template <bool _Dummy, class _D = dependent_type_condition<_Dummy, base_empty>>
  using enable_if_has_empty_base = enable_if_same<_Dummy, _D>;

  template <bool _Dummy, class _D = dependent_type_condition<_Dummy, base_not_empty>>
  using enable_if_has_base = enable_if_same<_Dummy, _D>;

public:
  /// Tell users that no need to call Free() with this allocator. (concept Allocator)
  //  static constexpr bool need_free = false;

  /// Tell users that this allocator is reference counted on copy.
  //  static constexpr bool is_ref_counted = true;

  static constexpr std::size_t minimum_content_size = sizeof(shared_data) + sizeof(chunk_header);

  /// Constructor with chunkSize.
  /// @param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
  /// @param baseAllocator The allocator for allocating memory chunks.
  template <bool _Dummy = true, class = enable_if_has_empty_base<_Dummy>>
  explicit memory_pool_allocator(std::size_t chunkSize = default_chunk_capacity)
      : chunk_capacity_(chunkSize)
      , _shared(static_cast<shared_data*>(BaseAllocator().allocate(minimum_content_size))) {

    fst_assert(_shared != 0, "");
    _shared->chunk_head = GetChunkHead(_shared);
    _shared->chunk_head->capacity = 0;
    _shared->chunk_head->size = 0;
    _shared->chunk_head->next = 0;
    _shared->rc.own_buffer = true;
    _shared->rc.refcount = 1;
  }

  template <bool _Dummy = true, class = enable_if_has_base<_Dummy>>
  explicit memory_pool_allocator(std::size_t chunkSize = default_chunk_capacity, BaseAllocator* baseAllocator = nullptr)
      : base(baseAllocator ? baseAllocator : fst::memory::__new<BaseAllocator>())
      , chunk_capacity_(chunkSize)
      , _shared(static_cast<shared_data*>(
            base::baseAllocator_ ? base::baseAllocator_->allocate(minimum_content_size) : 0)) {

    fst_assert(base::baseAllocator_ != 0, "");
    fst_assert(_shared != 0, "");

    _shared->ownBaseAllocator = baseAllocator ? 0 : base::baseAllocator_;
    _shared->chunk_head = GetChunkHead(_shared);
    _shared->chunk_head->capacity = 0;
    _shared->chunk_head->size = 0;
    _shared->chunk_head->next = 0;
    _shared->rc.own_buffer = true;
    _shared->rc.refcount = 1;
  }

  /// Constructor with user-supplied buffer.
  /// The user buffer will be used firstly. When it is full, memory pool allocates new chunk with chunk size.
  /// The user buffer will not be deallocated when this allocator is destructed.
  /// @param buffer User supplied buffer.
  /// @param size Size of the buffer in bytes. It must at least larger than sizeof(ChunkHeader).
  /// @param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
  /// @param baseAllocator The allocator for allocating memory chunks.

  template <bool _Dummy = true, class = enable_if_has_empty_base<_Dummy>>
  memory_pool_allocator(void* buffer, std::size_t size, std::size_t chunkSize = default_chunk_capacity)
      : chunk_capacity_(chunkSize)
      , _shared(static_cast<shared_data*>(AlignBuffer(buffer, size))) {

    fst_assert(size >= minimum_content_size, "");

    _shared->chunk_head = GetChunkHead(_shared);
    chunk_header& h = *_shared->chunk_head;
    h.capacity = size - minimum_content_size;
    h.size = 0;
    h.next = 0;
    _shared->rc.own_buffer = false;
    _shared->rc.refcount = 1;
  }

  template <bool _Dummy = true, class = enable_if_has_base<_Dummy>>
  memory_pool_allocator(
      void* buffer, std::size_t size, std::size_t chunkSize = default_chunk_capacity, BaseAllocator* baseAllocator = 0)
      : base(baseAllocator)
      , chunk_capacity_(chunkSize)
      , _shared(static_cast<shared_data*>(AlignBuffer(buffer, size))) {

    fst_assert(size >= minimum_content_size, "");

    _shared->chunk_head = GetChunkHead(_shared);
    _shared->chunk_head->capacity = size - minimum_content_size;
    _shared->chunk_head->size = 0;
    _shared->chunk_head->next = 0;
    _shared->ownBaseAllocator = 0;
    _shared->rc.own_buffer = false;
    _shared->rc.refcount = 1;
  }

  memory_pool_allocator(const memory_pool_allocator& rhs) noexcept
      : chunk_capacity_(rhs.chunk_capacity_)
      //      , baseAllocator_(rhs.baseAllocator_)
      , _shared(rhs._shared) {

    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }

    fst_noexcept_assert(_shared->rc.refcount > 0, "");
    ++_shared->rc.refcount;
  }

  memory_pool_allocator& operator=(const memory_pool_allocator& rhs) noexcept {
    fst_noexcept_assert(rhs._shared->refcount > 0, "");
    ++rhs._shared->refcount;
    this->~memory_pool_allocator();
    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }
    chunk_capacity_ = rhs.chunk_capacity_;
    _shared = rhs._shared;
    return *this;
  }

  memory_pool_allocator(memory_pool_allocator&& rhs) noexcept
      : chunk_capacity_(rhs.chunk_capacity_)
      //      , baseAllocator_(rhs.baseAllocator_)
      , _shared(rhs._shared) {

    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }

    fst_noexcept_assert(rhs._shared->rc.refcount > 0, "");
    rhs._shared = 0;
  }

  memory_pool_allocator& operator=(memory_pool_allocator&& rhs) noexcept {
    fst_noexcept_assert(rhs._shared->refcount > 0, "");
    this->~memory_pool_allocator();
    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }
    chunk_capacity_ = rhs.chunk_capacity_;
    _shared = rhs._shared;
    rhs._shared = 0;
    return *this;
  }

  /// Destructor.
  /// This deallocates all memory chunks, excluding the user-supplied buffer.
  inline ~memory_pool_allocator() noexcept {
    if (!_shared) {
      // do nothing if moved
      return;
    }
    if (_shared->rc.refcount > 1) {
      --_shared->rc.refcount;
      return;
    }

    clear();

    if constexpr (!is_base_empty) {
      BaseAllocator* a = _shared->ownBaseAllocator;
      if (_shared->own_buffer) {
        base::baseAllocator_->free(_shared);
      }
      fst::memory::__delete(a);
    }
    else {
      if (_shared->rc.own_buffer) {
        BaseAllocator().free(_shared);
      }
    }
  }

  /// Deallocates all memory chunks, excluding the first/user one.
  void clear() noexcept {
    fst_noexcept_assert(_shared->rc.refcount > 0, "");

    for (;;) {
      chunk_header* c = _shared->chunk_head;
      if (!c->next) {
        break;
      }
      _shared->chunk_head = c->next;

      if constexpr (is_base_empty) {
        BaseAllocator().free(c);
      }
      else {
        base::baseAllocator_->free(c);
      }
      //      baseAllocator_->free(c);
    }

    _shared->chunk_head->size = 0;
  }

  /// Computes the total capacity of allocated memory chunks.
  /// @return total capacity in bytes.
  std::size_t capacity() const noexcept {
    fst_noexcept_assert(_shared->refcount > 0, "");
    std::size_t capacity = 0;
    for (chunk_header* c = _shared->chunk_head; c != 0; c = c->next) {
      capacity += c->capacity;
    }
    return capacity;
  }

  /// Computes the memory blocks allocated.
  /// @return total used bytes.
  std::size_t size() const noexcept {
    fst_noexcept_assert(_shared->refcount > 0, "");
    std::size_t size = 0;
    for (chunk_header* c = _shared->chunk_head; c != 0; c = c->next) {
      size += c->size;
    }
    return size;
  }

  /// Whether the allocator is shared.
  bool is_shared() const noexcept {
    fst_noexcept_assert(_shared->rc.refcount > 0, "");
    return _shared->rc.refcount > 1;
  }

  /// Allocates a memory block. (concept Allocator)
  void* allocate(std::size_t size) {
    fst_noexcept_assert(_shared->rc.refcount > 0, "");

    if (!size) {
      return nullptr;
    }

    size = fst::memory::aligned_size<default_alignement>(size);

    if (FST_UNLIKELY(_shared->chunk_head->size + size > _shared->chunk_head->capacity))
      if (!AddChunk(chunk_capacity_ > size ? chunk_capacity_ : size)) {
        return nullptr;
      }

    void* buffer = GetChunkBuffer(_shared) + _shared->chunk_head->size;
    _shared->chunk_head->size += size;
    return buffer;
  }

  /// Resizes a memory block (concept Allocator)
  void* realloc(void* originalPtr, std::size_t originalSize, std::size_t newSize) {
    if (originalPtr == 0) {
      return allocate(newSize);
    }

    fst_noexcept_assert(_shared->rc.refcount > 0, "");
    if (newSize == 0) {
      return nullptr;
    }

    originalSize = fst::memory::aligned_size<default_alignement>(originalSize);
    newSize = fst::memory::aligned_size<default_alignement>(newSize);

    // Do not shrink if new size is smaller than original
    if (originalSize >= newSize) {
      return originalPtr;
    }

    // Simply expand it if it is the last allocation and there is sufficient space.
    if (originalPtr == GetChunkBuffer(_shared) + _shared->chunk_head->size - originalSize) {
      std::size_t increment = static_cast<std::size_t>(newSize - originalSize);
      if (_shared->chunk_head->size + increment <= _shared->chunk_head->capacity) {
        _shared->chunk_head->size += increment;
        return originalPtr;
      }
    }

    // Realloc process: allocate and copy memory, do not free original buffer.
    if (void* newBuffer = allocate(newSize)) {
      if (originalSize) {
        std::memcpy(newBuffer, originalPtr, originalSize);
      }
      return newBuffer;
    }

    return nullptr;
  }

  /// Frees a memory block (concept Allocator)
  /// Does nothing.
  inline static void free(void* ptr) noexcept { fst::unused(ptr); }

  /// Compare (equality) with another memory_pool_allocator
  inline bool operator==(const memory_pool_allocator& rhs) const noexcept {
    fst_noexcept_assert(_shared->refcount > 0, "");
    fst_noexcept_assert(rhs._shared->refcount > 0, "");
    return _shared == rhs._shared;
  }

  /// Compare (inequality) with another memory_pool_allocator
  inline bool operator!=(const memory_pool_allocator& rhs) const noexcept { return !operator==(rhs); }

private:
  /// Creates a new chunk.
  /// @param capacity Capacity of the chunk in bytes.
  /// @return true if success.
  bool AddChunk(std::size_t capacity) {
    if constexpr (is_base_empty) {
      if (chunk_header* chunk = static_cast<chunk_header*>(BaseAllocator().allocate(sizeof(shared_data) + capacity))) {
        chunk->capacity = capacity;
        chunk->size = 0;
        chunk->next = _shared->chunk_head;
        _shared->chunk_head = chunk;
        return true;
      }
    }
    else {

      if (!base::baseAllocator_) {
        _shared->ownBaseAllocator = base::baseAllocator_ = fst::memory::__new<BaseAllocator>();
      }

      if (chunk_header* chunk
          = static_cast<chunk_header*>(base::baseAllocator_->allocate(sizeof(shared_data) + capacity))) {
        chunk->capacity = capacity;
        chunk->size = 0;
        chunk->next = _shared->chunk_head;
        _shared->chunk_head = chunk;
        return true;
      }
    }
    return false;
  }

  static inline void* AlignBuffer(void* buf, std::size_t& size) {
    fst_noexcept_assert(buf != 0, "");
    const std::uintptr_t mask = sizeof(void*) - 1;
    const std::uintptr_t ubuf = reinterpret_cast<std::uintptr_t>(buf);

    if (FST_UNLIKELY(ubuf & mask)) {
      const std::uintptr_t abuf = (ubuf + mask) & ~mask;
      fst_noexcept_assert(size >= abuf - ubuf, "");
      buf = reinterpret_cast<void*>(abuf);
      size -= abuf - ubuf;
    }
    return buf;
  }

  /// The minimum capacity of chunk when they are allocated.
  std::size_t chunk_capacity_;

  /// The shared data of the allocator.
  shared_data* _shared;
};

namespace internal {
  template <typename, typename = void>
  struct IsRefCounted : public std::false_type {};

  template <typename T>
  struct IsRefCounted<T, typename fst::enable_if_cond<T::is_ref_counted>::type> : public std::true_type {};
} // namespace internal

namespace allocator_detail {
  template <typename T, typename A>
  inline T* realloc(A& a, T* old_p, std::size_t old_n, std::size_t new_n) {
    fst_noexcept_assert(old_n <= SIZE_MAX / sizeof(T) && new_n <= SIZE_MAX / sizeof(T), "");
    return static_cast<T*>(a.realloc(old_p, old_n * sizeof(T), new_n * sizeof(T)));
  }

  template <typename T, typename A>
  inline T* malloc(A& a, std::size_t n = 1) {
    return realloc<T, A>(a, nullptr, 0, n);
  }

  template <typename T, typename A>
  inline void free(A& a, T* p, std::size_t n = 1) {
    static_cast<void>(realloc<T, A>(a, p, n, 0));
  }
} // namespace allocator_detail.

template <typename T, typename _BaseAllocator = crt_allocator>
class allocator : public std::allocator<T>,
                  public internal_allocator_base<allocator<T, _BaseAllocator>, _BaseAllocator::is_freeable,
                      _BaseAllocator::is_ref_counted> {
  using allocator_type = std::allocator<T>;
  using traits_type = std::allocator_traits<allocator_type>;

public:
  using base_allocator_type = _BaseAllocator;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;

  using size_type = typename traits_type::size_type;
  using difference_type = typename traits_type::difference_type;

  using value_type = typename traits_type::value_type;
  using pointer = typename traits_type::pointer;
  using const_pointer = typename traits_type::const_pointer;

  using reference = std::add_lvalue_reference_t<value_type>&;
  using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>&;

  using is_always_equal = std::is_empty<base_allocator_type>;

  allocator() noexcept = default;
  allocator(const allocator& rhs) noexcept = default;
  allocator(allocator&& rhs) noexcept = default;

  template <typename U>
  allocator(const allocator<U, base_allocator_type>& rhs) noexcept
      : allocator_type(rhs)
      , _base_allocator(rhs._base_allocator) {}

  /// implicit.
  allocator(const base_allocator_type& allocator) noexcept
      : allocator_type()
      , _base_allocator(allocator) {}

  ~allocator() noexcept = default;

  template <typename U>
  struct rebind {
    using other = allocator<U, base_allocator_type>;
  };

  inline pointer address(reference r) const noexcept { return std::addressof(r); }
  inline const_pointer address(const_reference r) const noexcept { return std::addressof(r); }

  FST_NODISCARD inline size_type max_size() const noexcept { return traits_type::max_size(*this); }

  template <typename... Args>
  inline void construct(pointer p, Args&&... args) {
    traits_type::construct(*this, p, std::forward<Args>(args)...);
  }

  inline void destroy(pointer p) { traits_type::destroy(*this, p); }

  template <typename U>
  inline U* allocate(size_type n = 1, const void* = 0) {
    return fst::allocator_detail::malloc<U>(_base_allocator, n);
  }

  template <typename U>
  inline void deallocate(U* p, size_type n = 1) {
    fst::allocator_detail::free<U>(_base_allocator, p, n);
  }

  inline pointer allocate(size_type n = 1, const void* = 0) { return allocate<value_type>(n); }
  inline void deallocate(pointer p, size_type n = 1) { deallocate<value_type>(p, n); }

  template <typename U>
  inline bool operator==(const allocator<U, base_allocator_type>& rhs) const noexcept {
    return _base_allocator == rhs._base_allocator;
  }

  template <typename U>
  inline bool operator!=(const allocator<U, base_allocator_type>& rhs) const noexcept {
    return !operator==(rhs);
  }

private:
  template <typename, typename>
  friend class allocator; // access to allocator<!T>.*

  base_allocator_type _base_allocator;
};

} // namespace fst.
