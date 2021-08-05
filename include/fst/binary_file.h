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

namespace detail {
  struct chunk_info {
    static constexpr std::size_t uid_size = 8;
    char uid[uid_size];
    std::uint32_t size;
    std::uint32_t offset;
  };

  struct header {
    static constexpr std::size_t uid_size = 4;
    char uid[uid_size];
    std::uint32_t n_chunk;
  };

  inline static std::ptrdiff_t get_chunk_info_offset(std::size_t index) {
    return sizeof(header) + index * sizeof(chunk_info);
  }

  inline static const chunk_info* get_chunk_info(const header* h, std::size_t index) {
    return reinterpret_cast<const chunk_info*>(reinterpret_cast<const std::uint8_t*>(h) + get_chunk_info_offset(index));
  }

  inline static const chunk_info* get_chunk_info_from_offset(const header* h, std::size_t offset) {
    return reinterpret_cast<const chunk_info*>(reinterpret_cast<const std::uint8_t*>(h) + offset);
  }

} // namespace detail.

/// Loader.
class loader {
public:
  bool load(const std::filesystem::path& file_path) {
    _file.close();

    if (!_file.open(file_path)) {
      return false;
    }

    if (!_file.is_valid()) {
      return false;
    }

    if (_file.size() < sizeof(detail::header)) {
      return false;
    }

    return load(fst::byte_view(_file.data(), _file.size()));
  }

  bool load(const fst::byte_view& bv) {
    const detail::header& h = bv.as_ref<detail::header>(0);

    if (fst::string::to_string_view_n(h.uid, detail::header::uid_size) != "fstb") {
      return false;
    }

    if (h.n_chunk == 0) {
      return false;
    }

    for (std::size_t i = 0; i < h.n_chunk; i++) {
      std::size_t c_offset = detail::get_chunk_info_offset(i);
      if (c_offset + sizeof(detail::chunk_info) > bv.size()) {
        return false;
      }

      const detail::chunk_info* c = detail::get_chunk_info_from_offset(&h, c_offset);

      if (c->size == 0) {
        continue;
      }

      if (c->offset + c->size > bv.size()) {
        return false;
      }

      _names.push_back(fst::string::to_string_view_n(c->uid, detail::chunk_info::uid_size));
      _data.emplace_back(bv.data() + c->offset, c->size);
    }

    return true;
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

/// Writer.
class writer {
public:
  using string_type = fst::small_string<detail::chunk_info::uid_size>;

  inline bool add_chunk(const string_type& name, const fst::byte_vector& data) {
    // Make sure data is not empty.
    if (data.empty()) {
      return false;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return false;
    }

    _chunk_name.push_back(name_info{ name, name_info::index_t{ false, (std::uint32_t)_chunk_data.size() } });
    _chunk_data.push_back(data);
    return true;
  }

  inline bool add_chunk(const string_type& name, fst::byte_vector&& data) {
    // Make sure data is not empty.
    if (data.empty()) {
      return false;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return false;
    }

    _chunk_name.push_back(name_info{ name, name_info::index_t{ false, (std::uint32_t)_chunk_data.size() } });
    _chunk_data.push_back(std::move(data));
    return true;
  }

  template <typename T>
  inline bool add_chunk(const string_type& name, const T& value) {
    // Make sure data is not empty.
    if (std::is_empty_v<T>) {
      return false;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return false;
    }

    fst::byte_vector data;
    data.push_back(value);

    _chunk_name.push_back(name_info{ name, name_info::index_t{ false, (std::uint32_t)_chunk_data.size() } });
    _chunk_data.push_back(std::move(data));
    return true;
  }

  inline bool add_chunk_ref(const string_type& name, const fst::byte_view& data) {
    // Make sure data is not empty.
    if (data.empty()) {
      return false;
    }

    // Make sure name doesn't already exist.
    if (contains(name)) {
      return false;
    }

    _chunk_name.push_back(name_info{ name, name_info::index_t{ true, (std::uint32_t)_chunk_view.size() } });
    _chunk_view.push_back(data);
    return true;
  }

  template <typename T>
  inline bool add_chunk_ref(const string_type& name, const T& value) {
    return add_chunk_ref(name, fst::byte_view((const std::uint8_t*)&value, sizeof(T)));
  }

  inline bool contains(const fst::small_string<8>& name) {
    for (const auto& n : _chunk_name) {
      if (n.name == name) {
        return true;
      }
    }

    return false;
  }

  inline bool write_to_file(const std::filesystem::path& filepath) const {
    std::ofstream output_file(filepath, std::ios::binary);
    if (!output_file.is_open()) {
      return false;
    }

    if (!internal_write<std::ofstream, const char*, std::streamsize>(output_file)) {
      output_file.close();
      return false;
    }

    output_file.close();
    return true;
  }

  inline bool write_to_buffer(fst::byte_vector& buffer) const {
    return internal_write<fst::byte_vector, const std::uint8_t*, std::size_t>(buffer);
  }

  inline fst::byte_vector write_to_buffer() const {
    fst::byte_vector buffer;
    write_to_buffer(buffer);
    return buffer;
  }

private:
  struct name_info {
    fst::small_string<8> name;

    struct index_t {
      bool is_view : 1 = false;
      std::uint32_t index : 31 = 0;
    };

    index_t index;
  };

  std::vector<name_info> _chunk_name;
  std::vector<fst::byte_vector> _chunk_data;
  std::vector<fst::byte_view> _chunk_view;

  template <typename _Writer, typename _DataPtrType = const char*, typename _DataSizeType = std::size_t>
  inline bool internal_write(_Writer& w) const {
    using data_ptr_type = _DataPtrType;
    using data_size_type = _DataSizeType;

    detail::header h{ { 'f', 's', 't', 'b' }, (std::uint32_t)_chunk_name.size() };
    w.write((data_ptr_type)&h, (data_size_type)sizeof(detail::header));

    std::uint32_t offset = (std::uint32_t)(sizeof(detail::header) + _chunk_name.size() * sizeof(detail::chunk_info));

    for (std::size_t i = 0; i < _chunk_name.size(); i++) {
      detail::chunk_info c_info;
      std::memset((void*)&c_info.uid, 0, detail::chunk_info::uid_size);
      std::memcpy((void*)&c_info.uid, _chunk_name[i].name.data(), _chunk_name[i].name.size());

      std::size_t chunk_index = _chunk_name[i].index.index;
      std::size_t chunk_size
          = _chunk_name[i].index.is_view ? _chunk_view[chunk_index].size() : _chunk_data[chunk_index].size();

      c_info.size = chunk_size;
      c_info.offset = offset;

      offset += chunk_size;

      w.write((data_ptr_type)&c_info, (data_size_type)sizeof(detail::chunk_info));
    }

    for (std::size_t i = 0; i < _chunk_name.size(); i++) {
      std::size_t chunk_index = _chunk_name[i].index.index;

      if (_chunk_name[i].index.is_view) {
        w.write((data_ptr_type)_chunk_view[chunk_index].data(), (data_size_type)_chunk_view[chunk_index].size());
      }
      else {
        w.write((data_ptr_type)_chunk_data[chunk_index].data(), (data_size_type)_chunk_data[chunk_index].size());
      }
    }

    return true;
  }
};
} // namespace fst::binary_file.
