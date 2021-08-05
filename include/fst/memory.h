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
#include <fst/config>
#include <fst/math>
#include <cstdlib>
#include <memory>
#include <type_traits>

// clang-format off
#if __FST_UNISTD__
  #include <unistd.h>
#endif // __FST_UNISTD__

#if __FST_MACOS__
  #include <sys/sysctl.h>

#elif __FST_WINDOWS__
  #include <stdlib.h>
  #include FST_BEGIN_WINDOWS_H
  #include <sysinfoapi.h>
  #include FST_END_WINDOWS_H

#elif __FST_LINUX__
  #include <cstdio>
#endif
// clang-format on

namespace fst::memory {
inline void* malloc(std::size_t size) { return std::malloc(size); }
inline void* realloc(void* ptr, std::size_t new_size) { return std::realloc(ptr, new_size); }
inline void free(void* ptr) { std::free(ptr); }

template <class T, class... Args>
std::enable_if_t<!std::is_array<T>::value, T*> __new(Args&&... args) {
  return new T(std::forward<Args>(args)...);
}

template <class T>
std::enable_if_t<std::is_unbounded_array_v<T>, std::remove_extent_t<T>*> __new(std::size_t n) {
  return new std::remove_extent_t<T>[n]();
}

template <class T>
std::enable_if_t<std::is_bounded_array_v<T>, std::remove_extent_t<T>*> __new() {
  return new std::remove_extent_t<T>[std::extent_v<T>]();
}

template <class T, class... Args>
T* new_pointer(Args&&... args) {
  return new T(std::forward<Args>(args)...);
}

template <class T>
T* new_array(std::size_t n) {
  return new T[n]();
}

template <class T>
std::enable_if_t<!std::is_array<T>::value, void> __delete(std::remove_extent_t<T>* value) {
  delete value;
}

template <class T>
std::enable_if_t<std::is_array<T>::value, void> __delete(std::remove_extent_t<T>* value) {
  delete[] value;
}

template <class T>
void delete_array(T* value) {
  delete[] value;
}

// template <class T>
// std::enable_if_t<std::is_unbounded_array_v<T>, void> __delete(std::remove_extent_t<T>* value) {
//  delete[] value;
//}

// template <class T>
// std::enable_if_t<std::is_bounded_array_v<T>, void> __delete(std::remove_extent_t<T>* value) {
//  delete[] value;
//}

template <std::size_t N>
inline constexpr std::size_t aligned_size(std::size_t size) {
  static_assert(fst::math::is_power_of_two(N), "N must be a power of two.");
  return (size + (N - 1)) & ~(N - 1);
}

template <std::size_t N, typename T>
inline constexpr std::size_t aligned_size() {
  static_assert(fst::math::is_power_of_two(N), "N must be a power of two.");
  return (sizeof(T) + (N - 1)) & ~(N - 1);
}

namespace detail {
  inline constexpr std::size_t default_page_size = 4096;
  inline constexpr std::size_t default_cache_size = 64;

  /// get_page_size.
  inline std::size_t get_page_size() {
#if __FST_UNISTD__
    long pagesize = sysconf(_SC_PAGE_SIZE);
    return pagesize >= 0 ? (std::size_t)pagesize : 0;

#elif __FST_WINDOWS__
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwPageSize >= 0 ? (std::size_t)sys_info.dwPageSize : 0;

#else
    return default_page_size;
#endif
  }

  /// get_cache_size.
  /// https://stackoverflow.com/questions/794632/programmatically-get-the-cache-line-size
  inline std::size_t get_cache_size() {
#if __FST_MACOS__
    std::size_t line_size = 0;
    std::size_t sizeof_line_size = sizeof(line_size);
    sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
    return line_size;

#elif __FST_WINDOWS__
    DWORD buffer_size = 0;

    if (!GetLogicalProcessorInformation(nullptr, &buffer_size)) {
      return default_cache_size;
    }

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer
        = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)fst::memory::malloc(buffer_size);

    if (!buffer) {
      return default_cache_size;
    }

    if (!GetLogicalProcessorInformation(&buffer[0], &buffer_size)) {
      fst::memory::free(buffer);
      return default_cache_size;
    }

    std::size_t line_size = 0;
    const std::size_t size = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

    for (std::size_t i = 0; i < size; i++) {
      if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
        line_size = buffer[i].Cache.LineSize;
        break;
      }
    }

    //    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
    //      if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
    //        line_size = buffer[i].Cache.LineSize;
    //        break;
    //      }
    //    }

    fst::memory::free(buffer);
    return line_size;

#elif __FST_LINUX__
    std::FILE* p = std::fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");

    if (!p) {
      return default_cache_size;
    }

    unsigned int i = 0;
    bool is_valid = std::fscanf(p, "%u", &i) == 1;
    std::fclose(p);

    return is_valid ? (std::size_t)i : default_cache_size;
#else

#warning "Unsupported platform for cache size."
    return default_cache_size;
#endif
  }
} // namespace detail.

inline std::size_t get_page_size() {
  // In C++11, the following is guaranteed to perform thread-safe initialisation.
  static std::size_t size = detail::get_page_size();
  return size;
}

inline std::size_t get_cache_size() {
  // In C++11, the following is guaranteed to perform thread-safe initialisation.
  static std::size_t size = detail::get_cache_size();
  return size;
}
} // namespace fst::memory
