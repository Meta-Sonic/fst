/////
///// BSD 3-Clause License
/////
///// Copyright (c) 2021, Alexandre Arsenault
///// All rights reserved.
/////
///// Redistribution and use in source and binary forms, with or without
///// modification, are permitted provided that the following conditions are met:
/////
///// * Redistributions of source code must retain the above copyright notice, this
/////   list of conditions and the following disclaimer.
/////
///// * Redistributions in binary form must reproduce the above copyright notice,
/////   this list of conditions and the following disclaimer in the documentation
/////   and/or other materials provided with the distribution.
/////
///// * Neither the name of the copyright holder nor the names of its
/////   contributors may be used to endorse or promote products derived from
/////   this software without specific prior written permission.
/////
///// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
///// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
///// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
///// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
///// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
///// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
///// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
///// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
///// POSSIBILITY OF SUCH DAMAGE.
/////
//
//#pragma once
//#include <fst/config>
//#include <cstdint>
//#include <string>
//
// namespace fst {
// class shared_memory {
// public:
//  enum class error_type {
//    none = 0,
//    creation_failed = 100,
//    mapping_failed = 110,
//    opening_failed = 120,
//  };
//
//  shared_memory() = default;
//  shared_memory(const shared_memory&) = delete;
//  shared_memory(shared_memory&&);
//  ~shared_memory();
//
//  shared_memory& operator=(const shared_memory&) = delete;
//  shared_memory& operator=(shared_memory&&);
//
//  error_type create(const std::string& __name, std::size_t __size);
//  error_type open(const std::string& __name, std::size_t __size);
//  void close();
//
//  inline bool is_valid() const { return _data && _size; }
//  inline std::size_t size() { return _size; };
//  inline const std::string& name() { return _name; }
//  inline std::uint8_t* data() { return _data; }
//
// private:
//  std::string _name;
//  std::uint8_t* _data = nullptr;
//  std::size_t _size = 0;
//
//#if __FST_WINDOWS__
//  using handle_type = void*;
//  static constexpr handle_type handle_default_value = nullptr;
//#else
//  using handle_type = int;
//  static constexpr handle_type handle_default_value = -1;
//#endif
//
//  handle_type _handle = handle_default_value;
//};
//} // namespace fst.
