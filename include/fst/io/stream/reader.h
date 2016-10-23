//#pragma once
//
//#include "tk/asio/dispatcher.hh"
//#include "tk/asio/status.hh"
//#include "tk/console/print.hh"
//#include "tk/console/file_print.hh"
//#include <unistd.h>
//#include <errno.h>
//#include <cstring>
//
//namespace tk {
//namespace asio {
//
//class Dispatcher;
//namespace stream {
//
//template <typename T>
//class Reader {
// public:
//  typedef void (*ReadCallback)(T*, void*, Status, Error, long);
//
//  Reader(Dispatcher* dispatcher) : _dispatcher(dispatcher) {}
//
//  static void ReaderHandler(void* data) {
//    Reader* reader = (Reader*)data;
//
//    // Could not read from file descriptor.
//    //    if (reader->_status == -1) {
//    // Try to read again.
//    reader->_status = read(reader->_fd, reader->_data, reader->_data_size);
//
//    if (reader->_status == -1) {
//      // Resource temporarily unavailable.
//      if (errno == EAGAIN) {
//#ifdef __TK_CONSOLE__
//        console::Error(TK_TRACER, "Resource is still unavailable (THIS SHOULD NEVER HAPPEND).");
//        console::FError<console::FLogType::ASIO>(TK_TRACER,
//                                                 "Resource is still unavailable (THIS SHOULD NEVER HAPPEND).");
//#endif  // __TK_CONSOLE__
//        return;
//      } else if (errno == EPIPE) {  // Broken pipe.
//#ifdef __TK_CONSOLE__
//        console::FError<console::FLogType::ASIO>(TK_TRACER, "Broken pipe.");
//#endif  // __TK_CONSOLE__
//        reader->CallUserCallback(Status::kBad, Error::kBrokenPipe);
//        return;
//      }
//
//// Unhandled error (for now).
//#ifdef __TK_CONSOLE__
//      console::Error(TK_TRACER, "Could not read from socket :", std::strerror(errno));
//      console::FError<console::FLogType::ASIO>(TK_TRACER, "Could not read from socket :", std::strerror(errno));
//#endif  // __TK_CONSOLE__
//      reader->CallUserCallback(Status::kBad, Error::kUnknown);
//      return;
//    }
//
//    if (reader->_status == 0) {
///// @todo Don't know exactly what to do there.
//#ifdef __TK_CONSOLE__
//      console::Warning(TK_TRACER, "Could not read from socket again ( read size :", 0, ").", std::strerror(errno));
//      console::FWarning<console::FLogType::ASIO>(TK_TRACER, "Could not read from socket again ( read size :", 0, ").",
//                                                 std::strerror(errno));
//
//      /// @todo Add timeout ???
//      console::Error(TK_TRACER, "@@@@TODO MAYBE ADD TIMEOUT ??? Socket got disconnected ?????");
//#endif  // __TK_CONSOLE__
//      reader->CallUserCallback(Status::kBad, Error::kSenderWasClosed);
//      return;
//    }
//
//    reader->CallUserCallback(Status::kGood, Error::kNone);
//    return;
//    //    }
//  }
//
//  long AsyncRead(T* owner, int fd, void* data, std::size_t data_size, ReadCallback callback, void* user_data) {
//    _fd = fd;
//    _owner = owner;
//    _data_size = data_size;
//    _data = data;
//    _callback = callback;
//    _user_data = user_data;
//
//    if (!_callback) {
//#ifdef __TK_CONSOLE__
//      console::Warning(TK_TRACER, "Received data but no callback was set.");
//      console::FWarning<console::FLogType::ASIO>(TK_TRACER, "Received data but no callback was set.");
//#endif  // __TK_CONSOLE__
//      return -1;
//    }
//
//    // Invalid file descriptor.
//    if (_fd == -1) {
//#ifdef __TK_CONSOLE__
//      console::Error(TK_TRACER, "Try to read on empty socket.");
//      console::FError<console::FLogType::ASIO>(TK_TRACER, "Try to read on empty socket.");
//#endif  // __TK_CONSOLE__
//      _status = -1;
//      return CallUserCallback(Status::kBad, Error::kInvalidFileDescriptor);
//    }
//
//    _status = read(_fd, _data, _data_size);
//
//    if (_status == -1) {
//      // Resource temporarily unavailable (which is normal since socket is on non-blocking mode).
//      if (errno == EAGAIN) {
//        _dispatcher->AddHandler(_fd, &Reader::ReaderHandler, this, Dispatcher::HandleType::Input);
//        return _status;
//      } else if (errno == EPIPE) {  // Broken pipe.
//#ifdef __TK_CONSOLE__
//        console::FError<console::FLogType::ASIO>(TK_TRACER, "Broken pipe.");
//#endif  // __TK_CONSOLE__
//        return CallUserCallback(Status::kBad, Error::kBrokenPipe);
//      }
//
//// Unhandled error (for now).
//#ifdef __TK_CONSOLE__
//      console::Error(TK_TRACER, "Read :", std::strerror(errno));
//      console::FError<console::FLogType::ASIO>(TK_TRACER, "Read :", std::strerror(errno));
//#endif  // __TK_CONSOLE__
//      return CallUserCallback(Status::kBad, Error::kUnknown);
//    }
//
//    if (_status == 0) {
///// @todo Don't know exactly what to do there.
//#ifdef __TK_CONSOLE__
//      console::Warning(TK_TRACER, "Could not read from socket again ( read size :", 0, ").", std::strerror(errno));
//      console::FWarning<console::FLogType::ASIO>(TK_TRACER, "Could not read from socket again ( read size :", 0, ").",
//                                                 std::strerror(errno));
//
//      /// @todo Add timeout ???
//      console::Error(TK_TRACER, "@@@@TODO MAYBE ADD TIMEOUT ??? Socket got disconnected ?????");
//#endif  // __TK_CONSOLE__
//      return CallUserCallback(Status::kBad, Error::kSenderWasClosed);
//    }
//
//    return CallUserCallback(Status::kGood, Error::kNone);
//  }
//
// private:
//  long _status = 0;
//  int _fd = -1;
//  std::size_t _data_size = 0;
//  ReadCallback _callback = nullptr;
//  void* _data = nullptr;
//  void* _user_data = nullptr;
//  T* _owner = nullptr;
//  Dispatcher* _dispatcher;
//
//  long CallUserCallback(Status status, Error error) {
//    _callback(_owner, _user_data, status, error, _status);
//    return _status;
//  }
//};
//
//}  // stream.
//}  // asio.
//}  // tk.
