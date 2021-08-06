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
#include <fst/byte_vector>
#include <fst/byte_view>
#include <fst/enum_error>
#include <fst/mapped_file>
#include <fst/small_string>
#include <fst/string>
#include <fst/print>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <cstring>
#include <map>

namespace fst::binary_file {
inline constexpr std::size_t header_id_size = 4;
inline constexpr std::size_t chunk_id_size = 8;

namespace detail {
  struct chunk_info {
    static constexpr std::size_t uid_size = chunk_id_size;
    char uid[uid_size];
    std::uint32_t size;
  };

  struct header {
    static constexpr std::size_t uid_size = header_id_size;
    char uid[uid_size];
    std::uint32_t n_chunk;

    inline const chunk_info* get_chunk_info_from_offset(std::size_t offset) const {
      return reinterpret_cast<const chunk_info*>(reinterpret_cast<const std::uint8_t*>(this) + offset);
    }
  };

  static_assert(sizeof(chunk_info) == 12, "sizeof(chunk_info) should be 12.");
  static_assert(sizeof(header) == 8, "sizeof(header_id_size) should be 8.");

  inline static constexpr std::ptrdiff_t get_chunk_info_offset(std::size_t index) {
    return sizeof(header) + index * sizeof(chunk_info);
  }
} // namespace detail.

/// Loader.
class loader {
public:
  enum class error_type { none, open_file, invalid_header, invalid_header_id, empty_chunk_size, wrong_chunk_size };

  using error_t = fst::enum_error<error_type, error_type::none>;

  error_t load(const std::filesystem::path& file_path) {
    _file.close();

    if (!_file.open(file_path)) {
      return error_type::open_file;
    }

    if (!_file.is_valid()) {
      return error_type::open_file;
    }

    if (_file.size() < sizeof(detail::header)) {
      return error_type::invalid_header;
    }

    return load(fst::byte_view(_file.data(), _file.size()));
  }

  error_t load(const fst::byte_view& bv) {
    const detail::header& h = bv.as_ref<detail::header>(0);

    if (fst::string::to_string_view_n(h.uid, detail::header::uid_size) != "fstb") {
      return error_type::invalid_header_id;
    }

    if (h.n_chunk == 0) {
      return error_type::empty_chunk_size;
    }

    std::uint32_t offset = (std::uint32_t)(sizeof(detail::header) + h.n_chunk * sizeof(detail::chunk_info));

    for (std::size_t i = 0; i < h.n_chunk; i++) {
      std::size_t c_offset = detail::get_chunk_info_offset(i);
      if (c_offset + sizeof(detail::chunk_info) > bv.size()) {
        return error_type::wrong_chunk_size;
      }

      //      const detail::chunk_info* c = detail::get_chunk_info_from_offset(&h, c_offset);
      const detail::chunk_info* c = h.get_chunk_info_from_offset(c_offset);

      if (c->size == 0) {
        continue;
      }

      if (offset + c->size > bv.size()) {
        return error_type::wrong_chunk_size;
      }

      _names.push_back(fst::string::to_string_view_n(c->uid, detail::chunk_info::uid_size));
      _data.emplace_back(bv.data() + offset, c->size);

      offset += c->size;
    }

    return error_t();
  }

  inline fst::byte_view get_data(const std::string_view& name) const {
    for (std::size_t i = 0; i < _names.size(); i++) {
      if (_names[i] == name) {
        return _data[i];
      }
    }

    return fst::byte_view();
  }

  inline fst::byte_view operator[](const std::string_view& name) const { return get_data(name); }

  inline bool contains(const std::string_view& name) const {
    for (const auto& n : _names) {
      if (n == name) {
        return true;
      }
    }

    return false;
  }

  inline const std::vector<std::string_view>& get_names() const { return _names; }

private:
  fst::mapped_file _file;
  std::vector<std::string_view> _names;
  std::vector<fst::byte_view> _data;
};

enum class write_error {
  none,
  empty_data,
  duplicate_name,
  open_file_error,
  write_error,
};

/// Writer.
template <template <class...> class _VectorType = std::vector>
class writer_t {
public:
  using error_type = write_error;
  using error_t = fst::enum_error<error_type, error_type::none>;
  using string_type = fst::small_string<chunk_id_size>;

  inline error_t add_chunk(const string_type& name, const fst::byte_vector& data) {
    // Make sure data is not empty.
    if (data.empty()) {
      return error_type::empty_data;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return error_type::duplicate_name;
    }

    _chunk_name.push_back(name_info{ name, index_t{ false, (std::uint32_t)_chunk_data.size() } });
    _chunk_data.push_back(data);
    return error_t();
  }

  inline error_t add_chunk(const string_type& name, fst::byte_vector&& data) {
    // Make sure data is not empty.
    if (data.empty()) {
      return error_type::empty_data;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return error_type::duplicate_name;
    }

    _chunk_name.push_back(name_info{ name, index_t{ false, (std::uint32_t)_chunk_data.size() } });
    _chunk_data.push_back(std::move(data));
    return error_t();
  }

  template <typename T>
  inline error_t add_chunk(const string_type& name, const T& value) {
    // Make sure data is not empty.
    if (std::is_empty_v<T>) {
      return error_type::empty_data;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return error_type::duplicate_name;
    }

    fst::byte_vector data;
    data.push_back(value);

    _chunk_name.push_back(name_info{ name, index_t{ false, (std::uint32_t)_chunk_data.size() } });
    _chunk_data.push_back(std::move(data));
    return error_t();
  }

  inline error_t add_chunk_ref(const string_type& name, const fst::byte_view& data) {
    // Make sure data is not empty.
    if (data.empty()) {
      return error_type::empty_data;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return error_type::duplicate_name;
    }

    _chunk_name.push_back(name_info{ name, index_t{ true, (std::uint32_t)_chunk_view.size() } });
    _chunk_view.push_back(data);
    return error_t();
  }

  template <typename T>
  inline error_t add_chunk_ref(const string_type& name, const T& value) {
    return add_chunk_ref(name, fst::byte_view((const std::uint8_t*)&value, sizeof(T)));
  }

  inline bool contains(const string_type& name) noexcept {
    for (const auto& n : _chunk_name) {
      if (n.name == name) {
        return true;
      }
    }

    return false;
  }

  inline error_t write_to_file(const std::filesystem::path& filepath) const {
    std::ofstream output_file(filepath, std::ios::binary);
    if (!output_file.is_open()) {
      return error_type::open_file_error;
    }

    constexpr auto check_error = [](const std::ofstream& stream) { return stream.bad(); };
    error_t err = internal_write<std::ofstream, const char*, std::streamsize, decltype(check_error)>(output_file);

    output_file.close();
    return err;
  }

  inline error_t write_to_buffer(fst::byte_vector& buffer) const {
    return internal_write<fst::byte_vector, const std::uint8_t*, std::size_t>(buffer);
  }

  inline fst::byte_vector write_to_buffer() const {
    fst::byte_vector buffer;
    write_to_buffer(buffer);
    return buffer;
  }

private:
  struct index_t {
    bool is_view : 1 = false;
    std::uint32_t index : 31 = 0;
  };
  static_assert(sizeof(index_t) == 4, "sizeof(index_t) should be 4.");

  struct name_info {
    string_type name;
    index_t index;
  };

  using name_vector_type = _VectorType<name_info>;
  using byte_vector_type = _VectorType<fst::byte_vector>;
  using view_vector_type = _VectorType<fst::byte_view>;

  name_vector_type _chunk_name;
  byte_vector_type _chunk_data;
  view_vector_type _chunk_view;

  template <typename _CheckErrorFct, typename T>
  inline static constexpr auto call_error_function(const T& arg) {
    return _CheckErrorFct()(arg);
  }

  template <typename _Writer, typename _DataPtrType, typename _DataSizeType, typename _CheckErrorFct = void>
  inline error_t internal_write(_Writer& w) const {
    using data_ptr_type = _DataPtrType;
    using data_size_type = _DataSizeType;
    constexpr bool has_error_function = !std::is_same_v<_CheckErrorFct, void>;

    detail::header h{ { 'f', 's', 't', 'b' }, (std::uint32_t)_chunk_name.size() };
    w.write((data_ptr_type)&h, (data_size_type)sizeof(detail::header));

    if constexpr (has_error_function) {
      if (call_error_function<_CheckErrorFct>(w)) {
        return error_type::write_error;
      }
    }

    for (std::size_t i = 0; i < _chunk_name.size(); i++) {
      detail::chunk_info c_info;
      std::memset((void*)&c_info.uid, 0, detail::chunk_info::uid_size);
      std::memcpy((void*)&c_info.uid, _chunk_name[i].name.data(), _chunk_name[i].name.size());

      std::size_t chunk_index = _chunk_name[i].index.index;
      std::size_t chunk_size
          = _chunk_name[i].index.is_view ? _chunk_view[chunk_index].size() : _chunk_data[chunk_index].size();

      c_info.size = (std::uint32_t)chunk_size;

      w.write((data_ptr_type)&c_info, (data_size_type)sizeof(detail::chunk_info));

      if constexpr (has_error_function) {
        if (call_error_function<_CheckErrorFct>(w)) {
          return error_type::write_error;
        }
      }
    }

    for (std::size_t i = 0; i < _chunk_name.size(); i++) {
      std::size_t chunk_index = _chunk_name[i].index.index;

      if (_chunk_name[i].index.is_view) {
        w.write((data_ptr_type)_chunk_view[chunk_index].data(), (data_size_type)_chunk_view[chunk_index].size());
      }
      else {
        w.write((data_ptr_type)_chunk_data[chunk_index].data(), (data_size_type)_chunk_data[chunk_index].size());
      }

      if constexpr (has_error_function) {
        if (call_error_function<_CheckErrorFct>(w)) {
          return error_type::write_error;
        }
      }
    }

    return error_t();
  }
};

using writer = writer_t<>;
} // namespace fst::binary_file.
