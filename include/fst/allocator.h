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

#include <memory>
#include <type_traits>
#include <cstdlib>

#define RAPIDJSON_ALIGN(x) (((x) + static_cast<std::size_t>(7u)) & ~static_cast<std::size_t>(7u))
#define RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY (64 * 1024)
#define RAPIDJSON_NEW(TypeName) new TypeName
#define RAPIDJSON_DELETE(x) delete x

namespace fst {

struct default_allocator {
  inline static void* malloc(std::size_t size) { return std::malloc(size); }

  inline static void* realloc(void* ptr, std::size_t new_size) { return std::realloc(ptr, new_size); }

  inline static void free(void* ptr) { std::free(ptr); }
};

//! C-runtime library allocator.
/*! This class is just wrapper for standard C library memory routines.
    \note implements Allocator concept
*/
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

///////////////////////////////////////////////////////////////////////////////
// MemoryPoolAllocator

//! Default memory allocator used by the parser and DOM.
/*! This allocator allocate memory blocks from pre-allocated memory chunks.
    It does not free memory blocks. And Realloc() only allocate new memory.
    The memory chunks are allocated by BaseAllocator, which is CrtAllocator by default.
    User may also supply a buffer as the first chunk.
    If the user-buffer is full then additional chunks are allocated by BaseAllocator.
    The user-buffer is not deallocated by this allocator.
    \tparam BaseAllocator the allocator type for allocating memory chunks. Default is CrtAllocator.
    \note implements Allocator concept
*/
template <typename BaseAllocator = crt_allocator>
class memory_pool_allocator {
  //! Chunk header for perpending to each chunk.
  /*! Chunks are stored as a singly linked list.
   */
  struct ChunkHeader {
    size_t capacity; //!< Capacity of the chunk in bytes (excluding the header itself).
    size_t size; //!< Current size of allocated memory in bytes.
    ChunkHeader* next; //!< Next chunk in the linked list.
  };

  struct SharedData {
    ChunkHeader* chunkHead; //!< Head of the chunk linked-list. Only the head chunk serves allocation.
    BaseAllocator* ownBaseAllocator; //!< base allocator created by this object.
    size_t refcount;
    bool ownBuffer;
  };

  static constexpr std::size_t default_alignement = 8;
  static constexpr std::size_t SIZEOF_SHARED_DATA = fst::aligned_size<default_alignement, SharedData>();
  static constexpr std::size_t SIZEOF_CHUNK_HEADER = fst::aligned_size<default_alignement, ChunkHeader>();
  static constexpr std::size_t default_chunk_capacity
      = RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY; //!< Default chunk capacity.

  static inline ChunkHeader* GetChunkHead(SharedData* shared) {
    return reinterpret_cast<ChunkHeader*>(reinterpret_cast<uint8_t*>(shared) + SIZEOF_SHARED_DATA);
  }

  static inline uint8_t* GetChunkBuffer(SharedData* shared) {
    return reinterpret_cast<uint8_t*>(shared->chunkHead) + SIZEOF_CHUNK_HEADER;
  }

public:
  /// Tell users that no need to call Free() with this allocator. (concept Allocator)
  static constexpr bool need_free = false;

  /// Tell users that this allocator is reference counted on copy.
  static constexpr bool is_ref_counted = true;

  //! Constructor with chunkSize.
  /*! \param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
      \param baseAllocator The allocator for allocating memory chunks.
  */
  explicit memory_pool_allocator(size_t chunkSize = default_chunk_capacity, BaseAllocator* baseAllocator = 0)
      : chunk_capacity_(chunkSize)
      , baseAllocator_(baseAllocator ? baseAllocator : RAPIDJSON_NEW(BaseAllocator)())
      , shared_(static_cast<SharedData*>(
            baseAllocator_ ? baseAllocator_->allocate(SIZEOF_SHARED_DATA + SIZEOF_CHUNK_HEADER) : 0)) {

    fst_assert(baseAllocator_ != 0, "");
    fst_assert(shared_ != 0, "");

    if (baseAllocator) {
      shared_->ownBaseAllocator = 0;
    }
    else {
      shared_->ownBaseAllocator = baseAllocator_;
    }
    shared_->chunkHead = GetChunkHead(shared_);
    shared_->chunkHead->capacity = 0;
    shared_->chunkHead->size = 0;
    shared_->chunkHead->next = 0;
    shared_->ownBuffer = true;
    shared_->refcount = 1;
  }

  //! Constructor with user-supplied buffer.
  /*! The user buffer will be used firstly. When it is full, memory pool allocates new chunk with chunk size.
      The user buffer will not be deallocated when this allocator is destructed.
      \param buffer User supplied buffer.
      \param size Size of the buffer in bytes. It must at least larger than sizeof(ChunkHeader).
      \param chunkSize The size of memory chunk. The default is kDefaultChunkSize.
      \param baseAllocator The allocator for allocating memory chunks.
  */
  memory_pool_allocator(
      void* buffer, size_t size, size_t chunkSize = default_chunk_capacity, BaseAllocator* baseAllocator = 0)
      : chunk_capacity_(chunkSize)
      , baseAllocator_(baseAllocator)
      , shared_(static_cast<SharedData*>(AlignBuffer(buffer, size))) {

    fst_assert(size >= SIZEOF_SHARED_DATA + SIZEOF_CHUNK_HEADER, "");

    shared_->chunkHead = GetChunkHead(shared_);
    shared_->chunkHead->capacity = size - SIZEOF_SHARED_DATA - SIZEOF_CHUNK_HEADER;
    shared_->chunkHead->size = 0;
    shared_->chunkHead->next = 0;
    shared_->ownBaseAllocator = 0;
    shared_->ownBuffer = false;
    shared_->refcount = 1;
  }

  memory_pool_allocator(const memory_pool_allocator& rhs) noexcept
      : chunk_capacity_(rhs.chunk_capacity_)
      , baseAllocator_(rhs.baseAllocator_)
      , shared_(rhs.shared_) {
    fst_noexcept_assert(shared_->refcount > 0, "");
    ++shared_->refcount;
  }

  memory_pool_allocator& operator=(const memory_pool_allocator& rhs) noexcept {
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    ++rhs.shared_->refcount;
    this->~memory_pool_allocator();
    baseAllocator_ = rhs.baseAllocator_;
    chunk_capacity_ = rhs.chunk_capacity_;
    shared_ = rhs.shared_;
    return *this;
  }

  memory_pool_allocator(memory_pool_allocator&& rhs) noexcept
      : chunk_capacity_(rhs.chunk_capacity_)
      , baseAllocator_(rhs.baseAllocator_)
      , shared_(rhs.shared_) {
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    rhs.shared_ = 0;
  }

  memory_pool_allocator& operator=(memory_pool_allocator&& rhs) noexcept {
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    this->~memory_pool_allocator();
    baseAllocator_ = rhs.baseAllocator_;
    chunk_capacity_ = rhs.chunk_capacity_;
    shared_ = rhs.shared_;
    rhs.shared_ = 0;
    return *this;
  }

  //! Destructor.
  /*! This deallocates all memory chunks, excluding the user-supplied buffer.
   */
  ~memory_pool_allocator() noexcept {
    if (!shared_) {
      // do nothing if moved
      return;
    }
    if (shared_->refcount > 1) {
      --shared_->refcount;
      return;
    }

    clear();

    BaseAllocator* a = shared_->ownBaseAllocator;
    if (shared_->ownBuffer) {
      baseAllocator_->free(shared_);
    }
    RAPIDJSON_DELETE(a);
  }

  //! Deallocates all memory chunks, excluding the first/user one.
  void clear() noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");

    for (;;) {
      ChunkHeader* c = shared_->chunkHead;
      if (!c->next) {
        break;
      }
      shared_->chunkHead = c->next;
      baseAllocator_->free(c);
    }

    shared_->chunkHead->size = 0;
  }

  //! Computes the total capacity of allocated memory chunks.
  /*! \return total capacity in bytes.
   */
  size_t Capacity() const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    size_t capacity = 0;
    for (ChunkHeader* c = shared_->chunkHead; c != 0; c = c->next) {
      capacity += c->capacity;
    }
    return capacity;
  }

  //! Computes the memory blocks allocated.
  /*! \return total used bytes.
   */
  size_t Size() const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    size_t size = 0;
    for (ChunkHeader* c = shared_->chunkHead; c != 0; c = c->next) {
      size += c->size;
    }
    return size;
  }

  //! Whether the allocator is shared.
  /*! \return true or false.
   */
  bool Shared() const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    return shared_->refcount > 1;
  }

  //! Allocates a memory block. (concept Allocator)
  void* allocate(size_t size) {
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

  //! Resizes a memory block (concept Allocator)
  void* realloc(void* originalPtr, size_t originalSize, size_t newSize) {
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
      size_t increment = static_cast<size_t>(newSize - originalSize);
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

  //! Frees a memory block (concept Allocator)
  inline static void free(void* ptr) noexcept { fst::unused(ptr); } // Do nothing

  //! Compare (equality) with another MemoryPoolAllocator
  inline bool operator==(const memory_pool_allocator& rhs) const noexcept {
    fst_noexcept_assert(shared_->refcount > 0, "");
    fst_noexcept_assert(rhs.shared_->refcount > 0, "");
    return shared_ == rhs.shared_;
  }
  //! Compare (inequality) with another MemoryPoolAllocator
  inline bool operator!=(const memory_pool_allocator& rhs) const noexcept { return !operator==(rhs); }

private:
  //! Creates a new chunk.
  /*! \param capacity Capacity of the chunk in bytes.
      \return true if success.
  */
  bool AddChunk(size_t capacity) {
    if (!baseAllocator_)
      shared_->ownBaseAllocator = baseAllocator_ = RAPIDJSON_NEW(BaseAllocator)();
    if (ChunkHeader* chunk = static_cast<ChunkHeader*>(baseAllocator_->allocate(SIZEOF_CHUNK_HEADER + capacity))) {
      chunk->capacity = capacity;
      chunk->size = 0;
      chunk->next = shared_->chunkHead;
      shared_->chunkHead = chunk;
      return true;
    }

    return false;
  }

  static inline void* AlignBuffer(void* buf, size_t& size) {
    fst_noexcept_assert(buf != 0, "");
    const uintptr_t mask = sizeof(void*) - 1;
    const uintptr_t ubuf = reinterpret_cast<uintptr_t>(buf);

    if (FST_UNLIKELY(ubuf & mask)) {
      const uintptr_t abuf = (ubuf + mask) & ~mask;
      fst_noexcept_assert(size >= abuf - ubuf, "");
      buf = reinterpret_cast<void*>(abuf);
      size -= abuf - ubuf;
    }
    return buf;
  }

  size_t chunk_capacity_; //!< The minimum capacity of chunk when they are allocated.
  BaseAllocator* baseAllocator_; //!< base allocator for allocating memory chunks.
  SharedData* shared_; //!< The shared data of the allocator
};
} // namespace fst.
