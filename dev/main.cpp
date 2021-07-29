#include <fst/assert>
#include <fst/print>
#include <fst/uuid>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
int main() {
  fst::print("Test");

  auto start = std::chrono::system_clock::now();
  std::filesystem::path p = std::filesystem::temp_directory_path() / "example.bin";
  std::ofstream(p.c_str()).put('a'); // create file

  //     auto print_last_write_time = [](std::filesystem::file_time_type const& ftime) {
  //      ftime.time_since_epoch()
  ////        std::time_t cftime = std::chrono::system_clock::to_time_t(
  ////            std::chrono::file_clock::to_sys(ftime));
  ////        std::cout << "File write time is " << std::asctime(std::localtime(&cftime));
  //    };
  //      std::this_thread::sl
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(200ms);

  std::filesystem::file_time_type ftime = std::filesystem::last_write_time(p);
  fst::print("A", (std::size_t)ftime.time_since_epoch().count());
  //     fst::print(start.);
  fst::print(start.time_since_epoch().count());

  {
    const std::time_t t_c = std::chrono::system_clock::to_time_t(start);
    std::cout << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;
  }

  {
    //      time_t
    //      const Clock::duration since_epoch = point.time_since_epoch();
    //    std::cout << std::chrono::duration_cast<Ms>(since_epoch).count() << " ms\n";

    //    ` const Clock::duration since_epoch = point.time_since_epoch();
    //    std::cout << std::chrono::duration_cast<Ms>(since_epoch).count() << " ms\n";

    //      using time_point = std::chrono::time_point<std::chrono::system_clock>;
    //      time_point tt = std::chrono::time_point_cast<time_point>(ftime.time_since_epoch().count());
    //      time_point tt = ftime.time_since_epoch();
    //      std::chrono::system_clock::duration tt =
    //      std::chrono::duration_cast<time_point>(ftime.time_since_epoch().count());
    //      std::chrono::duration_cast<std::chrono::time_point<std::chrono::system_clock>>(

    //     const std::time_t t_c = std::chrono::system_clock::to_time_t();
    //    std::cout << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;
  }
  return 0;
}

// int main() {
//  fst::print("Test");
//
//  fst::uuid u0 = fst::uuid::create();
//  fst::print("GG", u0);
//
//  fst::print("FSLKJFS", fst::uuid::is_valid("717432bc-4f8e-4934-a302-e76fad0cfb45"));
//
//  fst::uuid u1 = fst::uuid::from_string("c8119cad-bb6e-4f32-aecb-f662315473aa");
//  fst::print("GG", u1);
//  fst::print("FSLKJFS", u1.is_valid());
//
//  fst::uuid u2;
//  fst::print("GG", u2);
//  fst::print("FSLKJFS", u2.is_valid());
//
//  return 0;
//}
