#include "fst/archive.h"
#include "zip.h"

namespace fst {
archive::~archive() { close(); }

archive::error_type archive::open(const std::filesystem::path& path) {
  close();

  int err = 0;
  _archive = zip_open(path.string().c_str(), ZIP_CREATE, &err);

  if (_archive == nullptr) {
    return error_type::invalid_archive;
  }

  if (err != ZIP_ER_OK) {
    fst::errprint("Can't open archive : " + std::string(zip_strerror(_archive)));
    return error_type::open_file_error;
  }

  return error_type::no_error;
}

archive::error_type archive::open(const fst::byte_view& data) {
  close();

  zip_error_t error;
  zip_error_init(&error);

  // Create source from buffer.
  zip_source* src = zip_source_buffer_create(data.data(), data.size(), 0, &error);
  if (src == nullptr) {
    fst::errprint("Can't create buffer : " + std::string(zip_error_strerror(&error)));
    //      error_type err(true, );
    zip_error_fini(&error);
    return error_type::buffer_creation_error;
  }

  // Open zip archive from source.
  _archive = zip_open_from_source(src, ZIP_CREATE, &error);
  if (_archive == nullptr) {
    //      error_type err(true, "Can't open zip from source : " + std::string(zip_error_strerror(&error)));
    fst::errprint("Can't open zip from source : " + std::string(zip_error_strerror(&error)));
    zip_source_free(src);
    src = nullptr;
    zip_error_fini(&error);
    return error_type::open_from_source_error;
  }

  zip_error_fini(&error);
  return error_type::no_error;
}

archive::error_type archive::create() {
  zip_error_t error;
  zip_error_init(&error);

  _src = zip_source_buffer_create(0, 0, 0, &error);
  if (_src == nullptr) {
    fst::errprint("Can't create buffer : " + std::string(zip_error_strerror(&error)));
    //      error_type err(true, "Can't create buffer : " + std::string(zip_error_strerror(&error)));
    zip_error_fini(&error);
    return error_type::buffer_creation_error;
  }

  zip_source_keep(_src);
  _archive = zip_open_from_source(_src, ZIP_TRUNCATE, &error);

  if (_archive == nullptr) {
    //      error_type err(true, "Can't open zip from source : " + std::string(zip_error_strerror(&error)));
    fst::errprint("Can't open zip from source : " + std::string(zip_error_strerror(&error)));
    zip_source_free(_src);
    _src = nullptr;
    zip_error_fini(&error);
    return error_type::open_from_source_error;
  }

  return error_type::no_error;
}

void archive::close() {
  if (_archive) {
    zip_close(_archive);
    _archive = nullptr;
  }

  if (_src) {
    zip_source_close(_src);
    zip_source_free(_src);
    _src = nullptr;
  }
}

fst::byte_vector archive::close_with_data() {
  if (_archive) {
    zip_close(_archive);
    _archive = nullptr;
  }

  zip_source_open(_src);
  zip_source_seek(_src, 0, SEEK_END);
  zip_int64_t sz = zip_source_tell(_src);
  zip_source_seek(_src, 0, SEEK_SET);
  fst::byte_vector archive_data(sz, 0);
  zip_source_read(_src, archive_data.data(), sz);
  close();
  return archive_data;
}

bool archive::add_file_content(const char* name, const fst::byte_view& data) {
  zip_source* s = zip_source_buffer(_archive, (const void*)data.data(), data.size(), 0);

  if (s == nullptr) {
    zip_source_free(s);
    fst::errprint("Can't add file:", zip_strerror(_archive));
    return false;
  }

  if (zip_file_add(_archive, name, s, ZIP_FL_OVERWRITE) < 0) {
    zip_source_free(s);
    fst::errprint("Can't add file:", zip_strerror(_archive));
    return false;
  }

  return true;
}

bool archive::replace_file_content(const char* name, const fst::byte_view& data) {
  zip_source* s = zip_source_buffer(_archive, (const void*)data.data(), data.size(), 0);

  if (s == nullptr) {
    zip_source_free(s);
    fst::errprint("Can't add file:", zip_strerror(_archive));
    return false;
  }

  zip_int64_t f_id = zip_name_locate(_archive, name, 0);

  if (zip_file_replace(_archive, f_id, s, ZIP_FL_OVERWRITE) < 0) {
    zip_source_free(s);
    fst::errprint("Can't add file:", zip_strerror(_archive));
    return false;
  }

  return true;
}

// True on success.
bool archive::add_directory(const char* name) { return zip_dir_add(_archive, name, ZIP_FL_ENC_UTF_8) >= 0; }

std::int64_t archive::get_file_index(const char* f_name) const {
  zip_int64_t file_index = zip_name_locate(_archive, f_name, 0);
  if (file_index < 0) {
    return -1;
  }
  return file_index;
}

int archive::get_file_count() const { return zip_get_num_files(_archive); }

const char* archive::get_file_name(std::int64_t file_index) const { return zip_get_name(_archive, file_index, 0); }

fst::byte_vector archive::get_file_content(const char* f_name) const {
  struct zip_stat stat;
  zip_int64_t file_index = zip_name_locate(_archive, f_name, 0);
  if (file_index < 0) {
    return {};
  }

  if (zip_stat_index(_archive, file_index, 0, &stat) < 0) {
    //    std::cerr << "File not found." << std::endl;
    // std::cout << "error file: " << zip_strerror(_archive) << std::endl;

    return {};
  }

  //  f_name = stat.name;

  //  zip_file* file = zip_fopen(_archive, f_name.c_str(), 0);
  zip_file* file = zip_fopen_index(_archive, file_index, 0);

  if (file == nullptr) {
    return {};
  }

  fst::byte_vector buffer(stat.size);
  zip_fread(file, buffer.data(), stat.size);
  zip_fclose(file);
  return buffer;
}

fst::byte_vector archive::get_file_content(std::int64_t file_index) const {
  struct zip_stat stat;
  if (zip_stat_index(_archive, file_index, 0, &stat) < 0) {
    //    std::cerr << "File not found." << std::endl;
    // std::cout << "error file: " << zip_strerror(_archive) << std::endl;

    return {};
  }

  zip_file* file = zip_fopen_index(_archive, file_index, 0);

  if (file == nullptr) {
    return {};
  }

  fst::byte_vector buffer(stat.size);
  zip_fread(file, buffer.data(), stat.size);
  zip_fclose(file);
  return buffer;
}
} // namespace fst.
