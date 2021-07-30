// https://llvm.org/doxygen/SmallVector_8cpp_source.html
#include "fst/small_vector.h"
#include <cstdlib>
#include <cstdint>
#include <string>

#if __FST_HAS_EXCEPTIONS__
#include <stdexcept>
#endif // __FST_HAS_EXCEPTIONS__

#if __FST_MSVC__
#pragma warning(push)
#pragma warning(disable : 4324)
#endif

namespace fst {
FST_ATTRIBUTE_RETURNS_NONNULL inline void* safe_malloc(size_t Sz) {
  void* result = std::malloc(Sz);
  if (result == nullptr) {
    // It is implementation-defined whether allocation occurs if the space
    // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
    // non-zero, if the space requested was zero.
    if (Sz == 0) {
      return safe_malloc(1);
      //     report_bad_alloc_error("Allocation failed");
    }
  }
  return result;
}

FST_ATTRIBUTE_RETURNS_NONNULL inline void* safe_calloc(size_t Count, size_t Sz) {
  void* result = std::calloc(Count, Sz);
  if (result == nullptr) {
    // It is implementation-defined whether allocation occurs if the space
    // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
    // non-zero, if the space requested was zero.
    if (Count == 0 || Sz == 0) {
      return safe_malloc(1);
      //     report_bad_alloc_error("Allocation failed");
    }
  }

  return result;
}

FST_ATTRIBUTE_RETURNS_NONNULL inline void* safe_realloc(void* Ptr, size_t Sz) {
  void* result = std::realloc(Ptr, Sz);
  if (result == nullptr) {
    // It is implementation-defined whether allocation occurs if the space
    // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
    // non-zero, if the space requested was zero.
    if (Sz == 0) {
      return safe_malloc(1);
      //     report_bad_alloc_error("Allocation failed");
    }
  }

  return result;
}
} // namespace fst.

using namespace fst;

// Check that no bytes are wasted and everything is well-aligned.
namespace {
struct Struct16B {
  alignas(16) void* X;
};
struct Struct32B {
  alignas(32) void* X;
};
} // namespace

static_assert(
    sizeof(small_vector<void*, 0>) == sizeof(unsigned) * 2 + sizeof(void*), "wasted space in SmallVector size 0");
static_assert(alignof(small_vector<Struct16B, 0>) >= alignof(Struct16B), "wrong alignment for 16-byte aligned T");
static_assert(alignof(small_vector<Struct32B, 0>) >= alignof(Struct32B), "wrong alignment for 32-byte aligned T");
static_assert(sizeof(small_vector<Struct16B, 0>) >= alignof(Struct16B), "missing padding for 16-byte aligned T");
static_assert(sizeof(small_vector<Struct32B, 0>) >= alignof(Struct32B), "missing padding for 32-byte aligned T");
static_assert(
    sizeof(small_vector<void*, 1>) == sizeof(unsigned) * 2 + sizeof(void*) * 2, "wasted space in SmallVector size 1");

static_assert(sizeof(small_vector<char, 0>) == sizeof(void*) * 2 + sizeof(void*),
    "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] static void report_size_overflow(size_t MinSize, size_t MaxSize);
static void report_size_overflow(size_t MinSize, size_t MaxSize) {
  std::string reason = "SmallVector unable to grow. Requested capacity (" + std::to_string(MinSize)
      + ") is larger than maximum value for size type (" + std::to_string(MaxSize) + ")";

#if __FST_HAS_EXCEPTIONS__
  throw std::length_error(reason);
#else
  //   report_fatal_error(Reason);
#endif
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.

[[noreturn]] static void report_at_maximum_capacity(size_t MaxSize);
static void report_at_maximum_capacity(size_t MaxSize) {
  std::string reason = "SmallVector capacity unable to grow. Already at maximum size " + std::to_string(MaxSize);
#if __FST_HAS_EXCEPTIONS__
  throw std::length_error(reason);
#else
  //   report_fatal_error(Reason);
#endif
}

// Note: Moving this function into the header may cause performance regression.
template <class Size_T>
static size_t getNewCapacity(size_t MinSize, size_t /*TSize*/, size_t OldCapacity) {
  constexpr size_t MaxSize = std::numeric_limits<Size_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  if (MinSize > MaxSize)
    report_size_overflow(MinSize, MaxSize);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  if (OldCapacity == MaxSize)
    report_at_maximum_capacity(MaxSize);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t NewCapacity = 2 * OldCapacity + 1; // Always grow.
  return std::min(std::max(NewCapacity, MinSize), MaxSize);
}

// Note: Moving this function into the header may cause performance regression.
template <class Size_T>
void* sv_detail::small_vector_base<Size_T>::mallocForGrow(size_t MinSize, size_t TSize, size_t& NewCapacity) {
  NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, this->capacity());
  return fst::safe_malloc(NewCapacity * TSize);
}

// Note: Moving this function into the header may cause performance regression.
template <class Size_T>
void sv_detail::small_vector_base<Size_T>::grow_pod(void* FirstEl, size_t MinSize, size_t TSize) {
  size_t NewCapacity = getNewCapacity<Size_T>(MinSize, TSize, this->capacity());
  void* NewElts;
  if (BeginX == FirstEl) {
    NewElts = safe_malloc(NewCapacity * TSize);

    // Copy the elements over.  No need to run dtors on PODs.
    memcpy(NewElts, this->BeginX, size() * TSize);
  }
  else {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = safe_realloc(this->BeginX, NewCapacity * TSize);
  }

  this->BeginX = NewElts;
  this->Capacity = (unsigned int)NewCapacity;
}

template class fst::sv_detail::small_vector_base<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#if SIZE_MAX > UINT32_MAX
template class fst::sv_detail::small_vector_base<uint64_t>;

// Assertions to ensure this #if stays in sync with small_vector_size_type.
static_assert(sizeof(sv_detail::small_vector_size_type<char>) == sizeof(uint64_t),
    "Expected small_vector_base<uint64_t> variant to be in use.");
#else
static_assert(sizeof(sv_detail::small_vector_size_type<char>) == sizeof(uint32_t),
    "Expected small_vector_base<uint32_t> variant to be in use.");
#endif

#if __FST_MSVC__
#pragma warning(pop)
#endif
