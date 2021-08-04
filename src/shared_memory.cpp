//#include "fst/shared_memory.h"
//#include <fst/print>
//
//#if __FST_UNISTD__
//#include <fcntl.h> // for O_* constants
//#include <sys/mman.h> // mmap, munmap
//#include <sys/stat.h> // for mode constants
//#include <unistd.h> // unlink
//
//#if defined(__APPLE__)
//#include <errno.h>
//#endif // __APPLE__
//
//#include <stdexcept>
//
//#elif __FST_WINDOWS__
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#undef WIN32_LEAN_AND_MEAN
//#endif
//
// namespace fst {
// shared_memory::shared_memory(shared_memory&& sm)
//    : _name(sm._name)
//    , _data(sm._data)
//    , _size(sm._size)
//    , _handle(sm._handle) {
//  sm._name = "";
//  sm._data = nullptr;
//  sm._size = 0;
//  sm._handle = handle_default_value;
//}
//
// shared_memory& shared_memory::operator=(shared_memory&& sm) {
//  close();
//
//  _name = sm._name;
//  _data = sm._data;
//  _size = sm._size;
//  _handle = sm._handle;
//
//  sm._name = "";
//  sm._data = nullptr;
//  sm._size = 0;
//  sm._handle = handle_default_value;
//
//  return *this;
//}
//
//#if __FST_UNISTD__
//// https://github.com/itchio/shoom/blob/master/src/shoom_unix_darwin.cc
//
// void shared_memory::close() {
//  if (!is_valid()) {
//    return;
//  }
//
//  munmap(_data, _size);
//  ::close(_handle);
//  shm_unlink(_name.c_str());
//
//  _data = nullptr;
//  _size = 0;
//}
//
// shared_memory::error_type shared_memory::create(const std::string& __name, std::size_t __size) {
//  if (is_valid()) {
//    munmap(_data, _size);
//    ::close(_handle);
//    _data = nullptr;
//    _size = 0;
//    _handle = -1;
//
//    // shm segments persist across runs, and macOS will refuse
//    // to ftruncate an existing shm segment, so to be on the safe
//    // side, we unlink it beforehand.
//    // TODO(amos) check errno while ignoring ENOENT?
//
//    // If one or more references to the shared memory object exist when the object is unlinked,
//    // the name shall be removed before shm_unlink() returns, but the removal of the memory object contents
//    // shall be postponed until all open and map references to the shared memory object have been removed.
//    if (shm_unlink(_name.c_str()) < 0 && errno != ENOENT) {
//      return error_type::creation_failed;
//    }
//  }
//
//  // Make sure data is empty here.
//  _data = nullptr;
//  _name = '/' + __name;
//  _size = __size;
//
//  // O_CREAT :
//  // If the shared memory object exists, this flag has no effect, except as noted under O_EXCL below.
//  // Otherwise, the shared memory object is created; the user ID of the shared memory object shall be
//  // set to the effective user ID of the process; the group ID of the shared memory object is set to a
//  // system default group ID or to the effective group ID of the process. The permission bits of the
//  // shared memory object shall be set to the value of the mode argument except those set in the file
//  // mode creation mask of the process. When bits in mode other than the file permission bits are set,
//  // the effect is unspecified. The mode argument does not affect whether the shared memory object is
//  // opened for reading, for writing, or for both. The shared memory object has a size of zero.
//
//  // O_EXCL :
//  // If O_EXCL and O_CREAT are set, shm_open() fails if the shared memory object exists.
//  // The check for the existence of the shared memory object and the creation of the object if it does
//  // not exist is atomic with respect to other processes executing shm_open() naming the same shared
//  // memory object with O_EXCL and O_CREAT set. If O_EXCL is set and O_CREAT is not set, the result
//  // is undefined.
//  _handle = shm_open(_name.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
//
//  //  _fd = shm_open(_name.c_str(), flags, 0755);
//  if (_handle < 0) {
//    _size = 0;
//    return error_type::creation_failed;
//  }
//
//  // this is the only way to specify the size of a
//  // newly-created POSIX shared memory object
//  int trunc_ret = ftruncate(_handle, _size);
//  if (trunc_ret != 0) {
//    ::close(_handle);
//    shm_unlink(_name.c_str());
//    _size = 0;
//    return error_type::creation_failed;
//  }
//
//  int prot = PROT_READ | PROT_WRITE;
//
//  void* memory = mmap(nullptr, // addr
//      _size, // length.
//      prot, // prot.
//      MAP_SHARED, // flags.
//      _handle, // fd.
//      0 // offset.
//  );
//
//  if (memory == MAP_FAILED) {
//    ::close(_handle);
//    shm_unlink(_name.c_str());
//    _size = 0;
//    return error_type::mapping_failed;
//  }
//
//  _data = static_cast<std::uint8_t*>(memory);
//  if (!_data) {
//    //    munmap(_data, _size);
//    ::close(_handle);
//    shm_unlink(_name.c_str());
//    _size = 0;
//    return error_type::mapping_failed;
//  }
//
//  return error_type::none;
//}
//
// shared_memory::error_type shared_memory::open(const std::string& __name, std::size_t __size) {
//  if (is_valid()) {
//    munmap(_data, _size);
//    ::close(_handle);
//    _data = nullptr;
//    _size = 0;
//    _handle = -1;
//
//    // shm segments persist across runs, and macOS will refuse
//    // to ftruncate an existing shm segment, so to be on the safe
//    // side, we unlink it beforehand.
//    // TODO(amos) check errno while ignoring ENOENT?
//
//    // If one or more references to the shared memory object exist when the object is unlinked,
//    // the name shall be removed before shm_unlink() returns, but the removal of the memory object contents
//    // shall be postponed until all open and map references to the shared memory object have been removed.
//    if (shm_unlink(_name.c_str()) < 0 && errno != ENOENT) {
//      return error_type::opening_failed;
//    }
//  }
//
//  // Make sure data is empty here.
//  _data = nullptr;
//  _name = '/' + __name;
//  _size = __size;
//
//  _handle = shm_open(_name.c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
//  if (_handle < 0) {
//    _size = 0;
//    return error_type::opening_failed;
//  }
//
//  int prot = PROT_READ | PROT_WRITE;
//
//  void* memory = mmap(nullptr, // addr
//      _size, // length.
//      prot, // prot.
//      MAP_SHARED, // flags.
//      _handle, // fd.
//      0 // offset.
//  );
//
//  if (memory == MAP_FAILED) {
//    ::close(_handle);
//    shm_unlink(_name.c_str());
//    _size = 0;
//    return error_type::mapping_failed;
//  }
//
//  _data = static_cast<std::uint8_t*>(memory);
//  if (!_data) {
//    //    munmap(_data, _size);
//    ::close(_handle);
//    shm_unlink(_name.c_str());
//    _size = 0;
//    return error_type::mapping_failed;
//  }
//
//  return error_type::none;
//
//  //  _data = static_cast<std::uint8_t*>(memory);
//  //  return _data ? error_type::none : error_type::mapping_failed;
//}
//
// shared_memory::~shared_memory() { close(); }
//#endif // __FST_UNISTD__
//} // namespace fst
