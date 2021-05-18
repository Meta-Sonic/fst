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
#include "fst/byte_view.h"
#include "fst/byte_vector.h"
#include "fst/print.h"

struct zip;
struct zip_source;

namespace fst {
class archive {
public:
  enum class error_type { no_error, invalid_archive, open_file_error, buffer_creation_error, open_from_source_error };

  error_type open(const std::filesystem::path& path);
  error_type open(const fst::byte_view& data);
  error_type create();

  void close();

  fst::byte_vector close_with_data();

  inline bool is_valid() const { return _archive != nullptr; }

  inline bool has_source_data() const { return _src != nullptr; }

  bool add_file_content(const char* name, const fst::byte_view& data);

  bool replace_file_content(const char* name, const fst::byte_view& data);

  // True on success.
  bool add_directory(const char* name);

  std::int64_t get_file_index(const char* f_name) const;

  int get_file_count() const;

  const char* get_file_name(std::int64_t file_index) const;

  fst::byte_vector get_file_content(const char* f_name) const;

  fst::byte_vector get_file_content(std::int64_t file_index) const;

private:
  zip* _archive = nullptr;
  zip_source* _src = nullptr;
};
} // namespace fst.
