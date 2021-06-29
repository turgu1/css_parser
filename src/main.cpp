#include <iostream>
#include <fstream>
#include <filesystem>

#include "css.hpp"

bool do_file(const char * filename) {

  std::uintmax_t fsize;

  try {
    fsize = std::filesystem::file_size(filename);  
  }
  catch (const std::filesystem::filesystem_error & err) {
    std::cerr << "filesystem error! " << err.what() << '\n';
    if (!err.path1().empty())
      std::cerr << "path1: " << err.path1().string() << '\n';
    if (!err.path2().empty())
      std::cerr << "path2: " << err.path2().string() << '\n';
    return false;
  }
  catch (const std::exception & ex) {
    std::cerr << "general exception: " << ex.what() << '\n';
    return false;
  }

  std::ifstream file;

  file.open(filename, std::ifstream::in);
  if (!file.is_open()) {
    std::cerr << "Unable to open file " << filename << std::endl;
    return false;
  }
  else {
    char * buffer = new char[fsize+1];

    file.read((char *)buffer, fsize);
    file.close();

    CSS * css = new CSS(std::string(filename), std::string("./"), buffer, fsize);
    // css->show();
    delete css;
    
    return true;
  }
}

const int FILE_COUNT = 11;

const char * files[FILE_COUNT] = {
  "test/test1.css",
  "test/test2.css",
  "test/test3.css",
  "test/test4.css",
  "test/test5.css",
  "test/test6.css",
  "test/test7.css",
  "test/test8.css",
  "test/test9.css",
  "test/test10.css",
  "test/test11.css",
};

int main() {

  int count = 0;
  int qty = 0;

  for (int i = 0; i < 9; i++) {
    std::cout << "---- File: " << files[i] << " ----" << std::endl;
    if (do_file(files[i])) count++;
    qty++;
  }

  std::cout << "Test completed, success count: " << count << " out of " << qty << std::endl;

  return 0;
}