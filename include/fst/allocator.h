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

#include <memory>
#include <type_traits>
#include <cstdlib>

#define RAPIDJSON_NEW(TypeName) new TypeName
#define RAPIDJSON_DELETE(x) delete x

// https://github.com/Tencent/rapidjson/blob/master/include/rapidjson/allocators.h
namespace fst {
struct default_allocator {
  inline static void* malloc(std::size_t size) { return std::malloc(size); }
  inline static void* realloc(void* ptr, std::size_t new_size) { return std::realloc(ptr, new_size); }
  inline static void free(void* ptr) { std::free(ptr); }
};

/// C-runtime library allocator.
/// This class is just wrapper for standard C library memory routines.
/// @note implements Allocator concept
class crt_allocator {
public:
  static constexpr bool need_free = true;

  void* allocate(std::size_t size) { return size ? default_allocator::malloc(size) : nullptr; }

  void* realloc(void* original_ptr, std::size_t original_size, std::size_t new_size) {
    fst::unused(original_size);

    if (new_size == 0) {
      default_allocator::free(original_ptr);
      return nullptr;
    }

    return default_allocator::realloc(original_ptr, new_size);
  }

  static void free(void* ptr) noexcept { default_allocator::free(ptr); }

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
class memory_pool_allocator : private detail::memory_pool_base<BaseAllocator> {
  using base = detail::memory_pool_base<BaseAllocator>;
  static constexpr std::size_t default_alignement = 8;
  static constexpr std::size_t default_chunk_capacity = 64 * 1024;
  static constexpr bool is_base_empty = std::is_empty_v<BaseAllocator>;

  /// Chunk header for perpending to each chunk.
  /// Chunks are stored as a singly linked list.
  ///
  struct alignas(default_alignement) chunk_header {
    /// Capacity of the chunk in bytes (excluding the header itself).
    std::size_t capacity;
    /// Current size of allocated memory in bytes.
    std::size_t size;
    /// Next chunk in the linked list.
    chunk_header* next;
  };

  struct alignas(default_alignement) shared_data : detail::shared_data_base<BaseAllocator> {
    /// Head of the chunk linked-list. Only the head chunk serves allocation.
    chunk_header* chunkHead;

    /// base allocator created by this object.
    //    BaseAllocator* ownBaseAllocator;
    std::size_t refcount;
    bool ownBuffer;
  };

  static inline chunk_header* GetChunkHead(shared_data* shared) {
    return reinterpret_cast<chunk_header*>(reinterpret_cast<std::uint8_t*>(shared) + sizeof(shared_data));
  }

  static inline std::uint8_t* GetChunkBuffer(shared_data* shared) {
    return reinterpret_cast<std::uint8_t*>(shared->chunkHead) + sizeof(chunk_header);
  }

public:
  /// Tell users that no need to call Free() with this allocator. (concept Allocator)
  static constexpr bool need_free = false;

  /// Tell users that this allocator is reference counted on copy.
  static constexpr bool is_ref_counted = true;

  static constexpr std::size_t minimum_content_size = sizeof(shared_data) + sizeof(chunk_header);

  /// Constructor with chunkSize.
  /// @param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
  /// @param baseAllocator The allocator for allocating memory chunks.
  explicit memory_pool_allocator(std::size_t chunkSize = default_chunk_capacity, BaseAllocator* baseAllocator = 0)
      : chunk_capacity_(chunkSize) {

    //      , baseAllocator_(baseAllocator ? baseAllocator : RAPIDJSON_NEW(BaseAllocator)())
    //      , shared_(static_cast<shared_data*>(baseAllocator_ ? baseAllocator_->allocate(minimum_content_size) : 0)) {

    if constexpr (is_base_empty) {
      shared_ = static_cast<shared_data*>(BaseAllocator().allocate(minimum_content_size));
    }
    else {
      base::baseAllocator_ = baseAllocator ? baseAllocator : RAPIDJSON_NEW(BaseAllocator)();
      shared_
          = static_cast<shared_data*>(base::baseAllocator_ ? base::baseAllocator_->allocate(minimum_content_size) : 0);

      fst_assert(base::baseAllocator_ != 0, "");
    }

    fst_assert(shared_ != 0, "");

    if constexpr (!is_base_empty) {
      if (baseAllocator) {
        shared_->ownBaseAllocator = 0;
      }
      else {
        shared_->ownBaseAllocator = base::baseAllocator_;
      }
    }

    shared_->chunkHead = GetChunkHead(shared_);
    shared_->chunkHead->capacity = 0;
    shared_->chunkHead->size = 0;
    shared_->chunkHead->next = 0;
    shared_->ownBuffer = true;
    shared_->refcount = 1;
  }

  /// Constructor with user-supplied buffer.
  /// The user buffer will be used firstly. When it is full, memory pool allocates new chunk with chunk size.
  /// The user buffer will not be deallocated when this allocator is destructed.
  /// @param buffer User supplied buffer.
  /// @param size Size of the buffer in bytes. It must at least larger than sizeof(ChunkHeader).
  /// @param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
  /// @param baseAllocator The allocator for allocating memory chunks.
  memory_pool_allocator(
      void* buffer, std::size_t size, std::size_t chunkSize = default_chunk_capacity, BaseAllocator* baseAllocator = 0)
      : chunk_capacity_(chunkSize)
      //      , baseAllocator_(baseAllocator)
      , shared_(static_cast<shared_data*>(AlignBuffer(buffer, size))) {

    if constexpr (!is_base_empty) {
      base::baseAllocator_ = baseAllocator;
    }

    fst_assert(size >= minimum_content_size, "");

    shared_->chunkHead = GetChunkHead(shared_);
    shared_->chunkHead->capacity = size - minimum_content_size;
    shared_->chunkHead->size = 0;
    shared_->chunkHead->next = 0;
    if constexpr (!is_base_empty) {
      shared_->ownBaseAllocator = 0;
    }
    shared_->ownBuffer = false;
    shared_->refcount = 1;
  }

  memory_pool_allocator(const memory_pool_allocator& rhs) noexcept
      : chunk_capacity_(rhs.chunk_capacity_)
      //      , baseAllocator_(rhs.baseAllocator_)
      , shared_(rhs.shared_) {

    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }

    fst_noexcept_assert(shared_->refcount > 0, "");
    ++shared_->refcount;
  }

  memory_pool_allocator& operator=(const memory_pool_allocator& rhs) noexcept {
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    ++rhs.shared_->refcount;
    this->~memory_pool_allocator();
    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }
    chunk_capacity_ = rhs.chunk_capacity_;
    shared_ = rhs.shared_;
    return *this;
  }

  memory_pool_allocator(memory_pool_allocator&& rhs) noexcept
      : chunk_capacity_(rhs.chunk_capacity_)
      //      , baseAllocator_(rhs.baseAllocator_)
      , shared_(rhs.shared_) {

    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }

    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    rhs.shared_ = 0;
  }

  memory_pool_allocator& operator=(memory_pool_allocator&& rhs) noexcept {
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    this->~memory_pool_allocator();
    if constexpr (!is_base_empty) {
      base::baseAllocator_ = rhs.baseAllocator_;
    }
    chunk_capacity_ = rhs.chunk_capacity_;
    shared_ = rhs.shared_;
    rhs.shared_ = 0;
    return *this;
  }

  /// Destructor.
  /// This deallocates all memory chunks, excluding the user-supplied buffer.
  inline ~memory_pool_allocator() noexcept {
    if (!shared_) {
      // do nothing if moved
      return;
    }
    if (shared_->refcount > 1) {
      --shared_->refcount;
      return;
    }

    clear();

    if constexpr (!is_base_empty) {
      BaseAllocator* a = shared_->ownBaseAllocator;
      if (shared_->ownBuffer) {
        base::baseAllocator_->free(shared_);
      }
      RAPIDJSON_DELETE(a);
    }
    else {
      if (shared_->ownBuffer) {
        BaseAllocator().free(shared_);
      }
    }

    //    BaseAllocator* a = shared_->ownBaseAllocator;
    //    if (shared_->ownBuffer) {
    //      baseAllocator_->free(shared_);
    //    }
    //    RAPIDJSON_DELETE(a);
  }

  /// Deallocates all memory chunks, excluding the first/user one.
  void clear() noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");

    for (;;) {
      chunk_header* c = shared_->chunkHead;
      if (!c->next) {
        break;
      }
      shared_->chunkHead = c->next;

      if constexpr (is_base_empty) {
        BaseAllocator().free(c);
      }
      else {
        base::baseAllocator_->free(c);
      }
      //      baseAllocator_->free(c);
    }

    shared_->chunkHead->size = 0;
  }

  /// Computes the total capacity of allocated memory chunks.
  /// @return total capacity in bytes.
  std::size_t Capacity() const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    std::size_t capacity = 0;
    for (chunk_header* c = shared_->chunkHead; c != 0; c = c->next) {
      capacity += c->capacity;
    }
    return capacity;
  }

  /// Computes the memory blocks allocated.
  /// @return total used bytes.
  std::size_t Size() const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    std::size_t size = 0;
    for (chunk_header* c = shared_->chunkHead; c != 0; c = c->next) {
      size += c->size;
    }
    return size;
  }

  /// Whether the allocator is shared.
  /// @return true or false.
  bool Shared() const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    return shared_->refcount > 1;
  }

  /// Allocates a memory block. (concept Allocator)
  void* allocate(std::size_t size) {
    fst_noexcept_assert(shared_->refcount > 0, "");

    if (!size) {
      return nullptr;
    }

    size = fst::aligned_size<default_alignement>(size);

    if (FST_UNLIKELY(shared_->chunkHead->size + size > shared_->chunkHead->capacity))
      if (!AddChunk(chunk_capacity_ > size ? chunk_capacity_ : size)) {
        return nullptr;
      }

    void* buffer = GetChunkBuffer(shared_) + shared_->chunkHead->size;
    shared_->chunkHead->size += size;
    return buffer;
  }

  /// Resizes a memory block (concept Allocator)
  void* realloc(void* originalPtr, std::size_t originalSize, std::size_t newSize) {
    if (originalPtr == 0) {
      return allocate(newSize);
    }

    fst_noexcept_assert(shared_->refcount > 0, "");
    if (newSize == 0) {
      return nullptr;
    }

    originalSize = fst::aligned_size<default_alignement>(originalSize);
    newSize = fst::aligned_size<default_alignement>(newSize);

    // Do not shrink if new size is smaller than original
    if (originalSize >= newSize) {
      return originalPtr;
    }

    // Simply expand it if it is the last allocation and there is sufficient space
    if (originalPtr == GetChunkBuffer(shared_) + shared_->chunkHead->size - originalSize) {
      std::size_t increment = static_cast<std::size_t>(newSize - originalSize);
      if (shared_->chunkHead->size + increment <= shared_->chunkHead->capacity) {
        shared_->chunkHead->size += increment;
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
    fst_noexcept_assert(shared_->refcount > 0, "");
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    return shared_ == rhs.shared_;
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
        chunk->next = shared_->chunkHead;
        shared_->chunkHead = chunk;
        return true;
      }
    }
    else {

      if (!base::baseAllocator_) {
        shared_->ownBaseAllocator = base::baseAllocator_ = RAPIDJSON_NEW(BaseAllocator)();
      }

      if (chunk_header* chunk
          = static_cast<chunk_header*>(base::baseAllocator_->allocate(sizeof(shared_data) + capacity))) {
        chunk->capacity = capacity;
        chunk->size = 0;
        chunk->next = shared_->chunkHead;
        shared_->chunkHead = chunk;
        return true;
      }
    }

    //    if (!baseAllocator_) {
    //      if constexpr (is_base_empty) {
    //        baseAllocator_ = RAPIDJSON_NEW(BaseAllocator)();
    //      }
    //      else {
    //        shared_->ownBaseAllocator = baseAllocator_ = RAPIDJSON_NEW(BaseAllocator)();
    //      }
    //      //      shared_->ownBaseAllocator = baseAllocator_ = RAPIDJSON_NEW(BaseAllocator)();
    //    }

    //    if (chunk_header* chunk = static_cast<chunk_header*>(baseAllocator_->allocate(sizeof(shared_data) +
    //    capacity))) {
    //      chunk->capacity = capacity;
    //      chunk->size = 0;
    //      chunk->next = shared_->chunkHead;
    //      shared_->chunkHead = chunk;
    //      return true;
    //    }

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

  /// base allocator for allocating memory chunks.
  //  BaseAllocator* baseAllocator_;

  /// The shared data of the allocator.
  shared_data* shared_;
};

namespace internal {
  template <typename, typename = void>
  struct IsRefCounted : public std::false_type {};

  template <typename T>
  struct IsRefCounted<T, typename fst::enable_if_cond<T::is_ref_counted>::type> : public std::true_type {};
} // namespace internal

template <typename T, typename A>
inline T* Realloc(A& a, T* old_p, size_t old_n, size_t new_n) {
  fst_noexcept_assert(old_n <= SIZE_MAX / sizeof(T) && new_n <= SIZE_MAX / sizeof(T), "");
  return static_cast<T*>(a.realloc(old_p, old_n * sizeof(T), new_n * sizeof(T)));
}

template <typename T, typename A>
inline T* Malloc(A& a, size_t n = 1) {
  return Realloc<T, A>(a, NULL, 0, n);
}

template <typename T, typename A>
inline void Free(A& a, T* p, size_t n = 1) {
  static_cast<void>(Realloc<T, A>(a, p, n, 0));
}

template <typename T, typename _BaseAllocator = crt_allocator>
class allocator : public std::allocator<T> {
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

  /// rapidjson Allocator concept
  static constexpr bool need_free = base_allocator_type::need_free;
  static constexpr bool is_ref_counted = internal::IsRefCounted<base_allocator_type>::value;

  allocator() noexcept = default;

  allocator(const allocator& rhs) noexcept
      : allocator_type(rhs)
      , baseAllocator_(rhs.baseAllocator_) {}

  template <typename U>
  allocator(const allocator<U, base_allocator_type>& rhs) noexcept
      : allocator_type(rhs)
      , baseAllocator_(rhs.baseAllocator_) {}

  allocator(allocator&& rhs) noexcept
      : allocator_type(std::move(rhs))
      , baseAllocator_(std::move(rhs.baseAllocator_)) {}

  /// implicit.
  allocator(const base_allocator_type& allocator) noexcept
      : allocator_type()
      , baseAllocator_(allocator) {}

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
    return fst::Malloc<U>(baseAllocator_, n);
  }

  template <typename U>
  inline void deallocate(U* p, size_type n = 1) {
    fst::Free<U>(baseAllocator_, p, n);
  }

  inline pointer allocate(size_type n = 1, const void* = 0) { return allocate<value_type>(n); }
  inline void deallocate(pointer p, size_type n = 1) { deallocate<value_type>(p, n); }

  template <typename U>
  inline bool operator==(const allocator<U, base_allocator_type>& rhs) const noexcept {
    return baseAllocator_ == rhs.baseAllocator_;
  }

  template <typename U>
  inline bool operator!=(const allocator<U, base_allocator_type>& rhs) const noexcept {
    return !operator==(rhs);
  }

  void* Malloc(size_t size) { return baseAllocator_.allocate(size); }

  void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
    return baseAllocator_.realloc(originalPtr, originalSize, newSize);
  }

  static void Free(void* ptr) noexcept { base_allocator_type::Free(ptr); }

private:
  template <typename, typename>
  friend class allocator; // access to allocator<!T>.*

  base_allocator_type baseAllocator_;
};

} // namespace fst.
