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
#include <cstddef>
#include <cstdint>
#include <cstdlib>

///
/// Macro options.
/// FST_CONFIG_ASSERT_IN_RELEASE :
/// FST_CONFIG_NO_EXCEPTIONS :
///

#define __FST_VERSION_MAJOR__ 1
#define __FST_VERSION_MINOR__ 0
#define __FST_VERSION_PATCH__ 0

// clang-format off
#ifndef __has_include
  #define __has_include(x) 0
#endif

#ifndef __has_feature
  #define __has_feature(x) 0
#endif

#ifndef __has_extension
  #define __has_extension(x) 0
#endif

#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

//
// Trace.
//
#undef __FST_USE_TRACE__
#undef __FST_USE_TRACE_INTERFACE__

#ifdef FST_CONFIG_USE_TRACE
  #define __FST_USE_TRACE__ 1

  #ifdef FST_CONFIG_USE_TRACE_INTERFACE
    #define __FST_USE_TRACE_INTERFACE__ 1
  #else
    #define __FST_USE_TRACE_INTERFACE__ 0
  #endif

#else
  #define __FST_USE_TRACE__ 0
  #define __FST_USE_TRACE_INTERFACE__ 0
#endif // FST_CONFIG_USE_TRACE.

namespace fst::config {

  //
  // Exceptions.
  //
  #undef __FST_HAS_EXCEPTIONS__

  #if defined(FST_CONFIG_NO_EXCEPTIONS) || !(defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND))
    inline constexpr bool has_exceptions = false;
    #define __FST_HAS_EXCEPTIONS__ 0
  #else
    inline constexpr bool has_exceptions = true;
    #define __FST_HAS_EXCEPTIONS__ 1
  #endif // __FST_NO_EXCEPTIONS__

// "https://web.archive.org/web/20140625123925/http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system"

  inline constexpr std::uint8_t version[3] = {1, 0, 0};
  enum class build_type { unknown, debug, release };
  enum class cpp_version_type { unknown, cpp_17, cpp_20 };
  enum class bitness_type { unknown, b_32, b_64 };
  enum class compiler_type { unknown, clang, wasm, gcc, intel, mingw, msvc };
  enum class platform_type { unknown, android, bsd, ios, linux, macos, solaris, windows };
  enum class architecture_type { unknown, x86, x64, arm };

  //
  // Build type.
  //
  #undef __FST_DEBUG_BUILD__
  #undef __FST_RELEASE_BUILD__

  #if defined(NDEBUG) && !defined(FST_CONFIG_ASSERT_IN_RELEASE)
    // Release.
    inline constexpr bool is_debug_build = false;
    inline constexpr bool is_release_build = true;
    inline constexpr build_type build = build_type::release;
    #define __FST_RELEASE_BUILD__ 1
    #define __FST_DEBUG_BUILD__ 0
  #else
    // Debug.
    inline constexpr bool is_debug_build = true;
    inline constexpr bool is_release_build = false;
    inline constexpr build_type build = build_type::debug;
    #define __FST_RELEASE_BUILD__ 0
    #define __FST_DEBUG_BUILD__ 1
  #endif

  //
  // C++ version.
  //
  #undef __FST_CPP_20__
  #undef __FST_CPP_17__

  // C++ 20.
  #if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
    #define __FST_CPP_20__ 1
    #define __FST_CPP_17__ 1
    inline constexpr cpp_version_type cpp_version = cpp_version_type::cpp_20;

  // C++ 17.
  #elif __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    #define __FST_CPP_20__ 0
    #define __FST_CPP_17__ 1
    inline constexpr cpp_version_type cpp_version = cpp_version_type::cpp_17;

  #else
    #define __FST_CPP_20__ 0
    #define __FST_CPP_17__ 0
    #error "fst only support c++17 and up."
  #endif

  //
  // Bitness type.
  //
  #undef __FST_32_BIT__
  #undef __FST_64_BIT__

  // 32 bit.
  #if INTPTR_MAX == INT32_MAX
    #define __FST_32_BIT__ 1
    #define __FST_64_BIT__ 0
    inline constexpr bitness_type bitness = bitness_type::b_32;
    inline constexpr std::size_t bitness_byte_size = 4;

  // 64 bit.
  #elif INTPTR_MAX == INT64_MAX
    #define __FST_32_BIT__ 0
    #define __FST_64_BIT__ 1
    inline constexpr bitness_type bitness = bitness_type::b_64;
    inline constexpr std::size_t bitness_byte_size = 8;

  // Unknown.
  #else
    #define __FST_32_BIT__ 0
    #define __FST_64_BIT__ 0
    inline constexpr bitness_type bitness = bitness_type::unknown;
    inline constexpr std::size_t bitness_byte_size = 0;
    #error "fst only support 32 and 64 bit architecture."
  #endif

  //
  // Compiler type.
  //
  #undef __FST_CLANG__
  #undef __FST_GCC__
  #undef __FST_INTEL__
  #undef __FST_MINGW__
  #undef __FST_MSVC__
  #undef __FST_WASM__

  // Clang.
  #if defined(__clang__)
    #define __FST_CLANG__ 1
    #define __FST_GCC__   0
    #define __FST_INTEL__ 0
    #define __FST_MINGW__ 0
    #define __FST_MSVC__  0
    #define __FST_WASM__  0
    inline constexpr compiler_type compiler = compiler_type::clang;

  // GCC.
  #elif defined(__GNUC__) || defined(__GNUG__)
    #define __FST_CLANG__ 0
    #define __FST_GCC__   1
    #define __FST_INTEL__ 0
    #define __FST_MINGW__ 0
    #define __FST_MSVC__  0
    #define __FST_WASM__  0
    inline constexpr compiler_type compiler = compiler_type::gcc;

  // Intel.
  #elif (defined(SYCL_LANGUAGE_VERSION) && defined (__INTEL_LLVM_COMPILER)) || defined(__INTEL_COMPILER)
    #define __FST_CLANG__ 0
    #define __FST_GCC__   0
    #define __FST_INTEL__ 1
    #define __FST_MINGW__ 0
    #define __FST_MSVC__  0
    #define __FST_WASM__  0
    inline constexpr compiler_type compiler = compiler_type::intel;

  // MinGW.
  #elif defined(__MINGW32__) || defined(__MINGW64__)
    #define __FST_CLANG__ 0
    #define __FST_GCC__   0
    #define __FST_INTEL__ 0
    #define __FST_MINGW__ 1
    #define __FST_MSVC__  0
    #define __FST_WASM__  0
    inline constexpr compiler_type compiler = compiler_type::mingw;

  // Microsoft visual studio.
  #elif defined(_MSC_VER)
    #define __FST_CLANG__ 0
    #define __FST_GCC__   0
    #define __FST_INTEL__ 0
    #define __FST_MINGW__ 0
    #define __FST_MSVC__  1
    #define __FST_WASM__  0
    inline constexpr compiler_type compiler = compiler_type::msvc;

  // Web assembly.
  #elif defined(__EMSCRIPTEN__)
    #define __FST_CLANG__ 0
    #define __FST_GCC__   0
    #define __FST_INTEL__ 0
    #define __FST_MINGW__ 0
    #define __FST_MSVC__  0
    #define __FST_WASM__  1
    inline constexpr compiler_type compiler = compiler_type::wasm;

  // Unknown compiler.
  #else
    #define __FST_CLANG__ 0
    #define __FST_GCC__   0
    #define __FST_INTEL__ 0
    #define __FST_MINGW__ 0
    #define __FST_MSVC__  0
    #define __FST_WASM__  0
    inline constexpr compiler_type compiler = compiler_type::unknown;
    #error "fst unsupported compiler."
  #endif

  //
  // Platform type.
  //
  #undef __FST_ANDROID__
  #undef __FST_BSD__
  #undef __FST_IOS__
  #undef __FST_LINUX__
  #undef __FST_MACOS__
  #undef __FST_SOLARIS__
  #undef __FST_WINDOWS__

  // Android.
  #if defined(__ANDROID__)
    #define __FST_ANDROID__ 1
    #define __FST_BSD__     0
    #define __FST_IOS__     0
    #define __FST_LINUX__   0
    #define __FST_MACOS__   0
    #define __FST_SOLARIS__ 0
    #define __FST_WINDOWS__ 0
    inline constexpr platform_type platform = platform_type::android;

  // Linux.
  #elif defined(__linux__) || defined(__linux) || defined(linux)
    #define __FST_ANDROID__ 0
    #define __FST_BSD__     0
    #define __FST_IOS__     0
    #define __FST_LINUX__   1
    #define __FST_MACOS__   0
    #define __FST_SOLARIS__ 0
    #define __FST_WINDOWS__ 0
    inline constexpr platform_type platform = platform_type::linux;

  // Apple macos or ios.
  #elif defined( __APPLE__ )
    // Apple.
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
      // ios.
      #define __FST_ANDROID__ 0
      #define __FST_BSD__     0
      #define __FST_IOS__     1
      #define __FST_LINUX__   0
      #define __FST_MACOS__   0
      #define __FST_SOLARIS__ 0
      #define __FST_WINDOWS__ 0
      inline constexpr platform_type platform = platform_type::ios;

    #elif TARGET_OS_MAC
      // Mac OS.
      #define __FST_ANDROID__ 0
      #define __FST_BSD__     0
      #define __FST_IOS__     0
      #define __FST_LINUX__   0
      #define __FST_MACOS__   1
      #define __FST_SOLARIS__ 0
      #define __FST_WINDOWS__ 0
      inline constexpr platform_type platform = platform_type::macos;

    // BSD.
  #elif defined(BSD) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    #define __FST_ANDROID__ 0
    #define __FST_BSD__     1
    #define __FST_IOS__     0
    #define __FST_LINUX__   0
    #define __FST_MACOS__   0
    #define __FST_SOLARIS__ 0
    #define __FST_WINDOWS__ 0
    inline constexpr platform_type platform = platform_type::bsd;
    
    #else
      // Unknown apple platform.
      #define __FST_ANDROID__ 0
      #define __FST_BSD__     0
      #define __FST_IOS__     0
      #define __FST_LINUX__   0
      #define __FST_MACOS__   0
      #define __FST_SOLARIS__ 0
      #define __FST_WINDOWS__ 0
      inline constexpr platform_type platform = platform_type::unknown;
      #error "fst unsupported platform."
    #endif

  // Solaris.
  #elif defined(__sun) && defined(__SVR4)
    #define __FST_ANDROID__ 0
    #define __FST_BSD__     0
    #define __FST_IOS__     0
    #define __FST_LINUX__   0
    #define __FST_MACOS__   0
    #define __FST_SOLARIS__ 1
    #define __FST_WINDOWS__ 0
    inline constexpr platform_type platform = platform_type::solaris;

  // Windows.
  #elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
    #define __FST_ANDROID__ 0
    #define __FST_BSD__     0
    #define __FST_IOS__     0
    #define __FST_LINUX__   0
    #define __FST_MACOS__   0
    #define __FST_SOLARIS__ 0
    #define __FST_WINDOWS__ 1
    inline constexpr platform_type platform = platform_type::windows;

  // Unknown platform.
  #else
    #define __FST_ANDROID__ 0
    #define __FST_BSD__     0
    #define __FST_IOS__     0
    #define __FST_LINUX__   0
    #define __FST_MACOS__   0
    #define __FST_SOLARIS__ 0
    #define __FST_WINDOWS__ 0
    inline constexpr platform_type platform = platform_type::unknown;
    #error "fst unsupported platform."
  #endif
  
  //
  // Architecture type.
  //
  #undef __FST_X86__
  #undef __FST_X64__
  #undef __FST_ARM__

  #if defined(__i386__) || defined(_M_IX86) || defined(_X86_)
    #define __FST_X86__ 1
    #define __FST_X64__ 0
    #define __FST_ARM__ 0
    inline constexpr architecture_type arch = architecture_type::x86;
  #elif defined(__x86_64__) || defined(_M_X64) || defined(__IA64__) || defined(_M_IA64)
    #define __FST_X86__ 0
    #define __FST_X64__ 1
    #define __FST_ARM__ 0
    inline constexpr architecture_type arch = architecture_type::x64;
  #elif defined(__arm__) || defined(_M_ARM)
    #define __FST_X86__ 0
    #define __FST_X64__ 0
    #define __FST_ARM__ 1
    inline constexpr architecture_type arch = architecture_type::arm;
  #else
    #define __FST_X86__ 0
    #define __FST_X64__ 0
    #define __FST_ARM__ 0
    inline constexpr architecture_type arch = architecture_type::unknown;
  #endif

  //
  // unistd.h
  //
  #undef __FST_UNISTD__

  #if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
    #define __FST_UNISTD__ 1
    inline constexpr bool has_unistd = true;
  #else
    #define __FST_UNISTD__ 0
    inline constexpr bool has_unistd = false;
  #endif
} // namespace fst::config.

 /// @macro FST_GNUC_PREREQ
 /// Extend the default __GNUC_PREREQ even if glibc's features.h isn't
 /// available.
#ifndef FST_GNUC_PREREQ
  #if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
    #define FST_GNUC_PREREQ(maj, min, patch) \
        ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= ((maj) << 20) + ((min) << 10) + (patch))
  #elif defined(__GNUC__) && defined(__GNUC_MINOR__)
    #define FST_GNUC_PREREQ(maj, min, patch) \
        ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
  #else
    #define FST_GNUC_PREREQ(maj, min, patch) 0
  #endif
#endif

/// Only use __has_cpp_attribute in C++ mode. GCC defines __has_cpp_attribute in
/// C mode, but the :: in __has_cpp_attribute(scoped::attribute) is invalid.
#ifndef FST_HAS_CPP_ATTRIBUTE
  #if defined(__cplusplus) && defined(__has_cpp_attribute)
    #define FST_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
  #else
    #define FST_HAS_CPP_ATTRIBUTE(x) 0
  #endif
#endif

/// @macro FST_PACKED
/// Used to specify a packed structure.
/// FST_PACKED(
///   struct A {
///      int i;
///      int j;
///      int k;
///      long long l;
///   });
///
/// FST_PACKED_START
/// struct B {
///   int i;
///   int j;
///   int k;
///   long long l;
/// };
/// FST_PACKED_END
#if __FST_MSVC__
  #define FST_PACKED(d)     __pragma(pack(push, 1)) d __pragma(pack(pop))
  #define FST_PACKED_START  __pragma(pack(push, 1))
  #define FST_PACKED_END    __pragma(pack(pop))
#else
  #define FST_PACKED(d)     d __attribute__((packed))
  #define FST_PACKED_START  _Pragma("pack(push, 1)")
  #define FST_PACKED_END    _Pragma("pack(pop)")
#endif
 
/// @macro FST_ALWAYS_INLINE
/// On compilers where we have a directive to do so, mark a method "always inline"
/// because it is performance sensitive. GCC 3.4 supported this but is buggy in
/// various cases and produces unimplemented errors, just use it in GCC 4.0 and later.
#if __has_attribute(always_inline) || FST_GNUC_PREREQ(4, 0, 0)
  #define FST_ALWAYS_INLINE inline __attribute__((always_inline))
#elif __FST_MSVC__
  #define FST_ALWAYS_INLINE __forceinline
#else
  #define FST_ALWAYS_INLINE inline
#endif
 
/// @macro FST_PRETTY_FUNCTION
/// Gets a user-friendly looking function signature for the current scope
/// using the best available method on each platform.  The exact format of the
/// resulting string is implementation specific and non-portable, so this should
/// only be used, for example, for logging or diagnostics.
#if __FST_MSVC__
  #define FST_PRETTY_FUNCTION __FUNCSIG__
#elif __FST_GCC__ || __FST_CLANG__ || __FST_MINGW__
  #define FST_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
  #define FST_PRETTY_FUNCTION __func__
#endif

/// @macro FST_LIKELY and FST_UNLIKELY
#if __has_builtin(__builtin_expect) || FST_GNUC_PREREQ(4, 0, 0)
  #define FST_LIKELY(EXPR) __builtin_expect((bool)(EXPR), true)
  #define FST_UNLIKELY(EXPR) __builtin_expect((bool)(EXPR), false)
#else
  #define FST_LIKELY(EXPR) (EXPR)
  #define FST_UNLIKELY(EXPR) (EXPR)
#endif

/// @macro FST_ATTRIBUTE_RETURNS_NONNULL.
#if __has_attribute(returns_nonnull) || FST_GNUC_PREREQ(4, 9, 0)
  #define FST_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#elif __FST_MSVC__
  #define FST_ATTRIBUTE_RETURNS_NONNULL _Ret_notnull_
#else
  #define FST_ATTRIBUTE_RETURNS_NONNULL
#endif

/// @macro FST_BUILTIN_TRAP
/// On compilers which support it, expands to an expression which causes the program to exit abnormally.
#if __has_builtin(__builtin_trap) || FST_GNUC_PREREQ(4, 3, 0)
  #define FST_BUILTIN_TRAP __builtin_trap()

#elif defined(_MSC_VER)
  // The __debugbreak intrinsic is supported by MSVC, does not require forward
  // declarations involving platform-specific typedefs (unlike RaiseException),
  // results in a call to vectored exception handlers, and encodes to a short
  // instruction that still causes the trapping behavior we want.
  #define FST_BUILTIN_TRAP __debugbreak()

#else
  #define FST_BUILTIN_TRAP *(volatile int*)0x11 = 0
#endif

/// @macro FST_DEBUGTRAP
/// On compilers which support it, expands to an expression which causes the program to break
/// while running under a debugger.
#if __has_builtin(__builtin_debugtrap)
  #define FST_DEBUGTRAP() __builtin_debugtrap()

#elif __FST_MSVC__
  // The __debugbreak intrinsic is supported by MSVC and breaks while
  // running under the debugger, and also supports invoking a debugger
  // when the OS is configured appropriately.
  #define FST_DEBUGTRAP() __debugbreak()

#elif __FST_CLANG__ && (defined(unix) || defined(__unix__) || defined(__unix) || defined(__MACH__))
  #include <signal.h>
  #define FST_DEBUGTRAP() raise(SIGTRAP)

#else
  #define FST_DEBUGTRAP() std::abort()
#endif

/// @macro FST_NOP().
#if __FST_X86__ || __FST_X64__
  #if __FST_MSVC__
    #define FST_NOP() _mm_pause()
  #else
    #define FST_NOP() __builtin_ia32_pause()
  #endif
#elif __FST_ARM__
  #define FST_NOP() __yield()
#else
  #define FST_NOP() ({ (void)0; })
#endif


// Some compilers warn about unused functions. When a function is sometimes
// used or not depending on build settings (e.g. a function only called from
// within "assert"), this attribute can be used to suppress such warnings.
//
// However, it shouldn't be used for unused *variables*, as those have a much
// more portable solution:
//   (void)unused_var_name;
// Prefer cast-to-void wherever it is sufficient.
#if defined(__cplusplus) && __cplusplus > 201402L && FST_HAS_CPP_ATTRIBUTE(maybe_unused)
  #define FST_ATTRIBUTE_UNUSED [[maybe_unused]]

#elif __has_attribute(unused) || FST_GNUC_PREREQ(3, 1, 0)
  #define FST_ATTRIBUTE_UNUSED __attribute__((__unused__))

#else
  #define FST_ATTRIBUTE_UNUSED
#endif


/// FST_NODISCARD - Warn if a type or return value is discarded.
#if defined(__cplusplus) && __cplusplus > 201402L && FST_HAS_CPP_ATTRIBUTE(nodiscard)
  // Use the 'nodiscard' attribute in C++17 or newer mode.
  #define FST_NODISCARD [[nodiscard]]

#elif FST_HAS_CPP_ATTRIBUTE(clang::warn_unused_result)
  // Clang in C++14 mode claims that it has the 'nodiscard' attribute, but also
  // warns in the pedantic mode that 'nodiscard' is a C++17 extension (PR33518).
  // Use the 'nodiscard' attribute in C++14 mode only with GCC.
  // TODO: remove this workaround when PR33518 is resolved.
  #define FST_NODISCARD [[clang::warn_unused_result]]

#elif defined(__GNUC__) && FST_HAS_CPP_ATTRIBUTE(nodiscard)
  #define FST_NODISCARD [[nodiscard]]

#else
  #define FST_NODISCARD
#endif

// clang-format on
