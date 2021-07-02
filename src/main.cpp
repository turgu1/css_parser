#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring> 

#include "css.hpp"
#include "dom.hpp"

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

const char * css_test_str = 
  R"END(
    div.toto {
        width: 25;
    }

    body {
        margin-top: 10px;
    }

    h1 {
        width: 25;
        text-align: center;
    }

    body > h1 + p {
        text-indent: 10px;
        font-size: 20px;
    }

    p {
      width: 15;
    }

    @font-face {
        font-family: "Fontbase";
        font-weight: normal;
        font-style: normal;
        src: url(Fonts/AveriaSerif-Light.ttf)
        }
    @font-face {
        font-family: "SmallCaps";
        font-weight: normal;
        font-style: normal;
        src: url(Fonts/Justus-Versalitas.ttf)
        }
    @font-face {
        font-family: "Lettrines";
        font-weight: normal;
        font-style: normal;
        src: url(Fonts/ELZEVIER_C.ttf)
        }
    @font-face {
        font-family: "Ornements";
        font-weight: normal;
        font-style: normal;
        src: url(Fonts/Swinging.ttf)
        }
  )END";

void test()
{
  DOM dom;

  DOM::Node * div = dom.body->add_child(DOM::Tag::DIV)->add_id("the_div")->add_class("toto");
  div->add_child(DOM::Tag::H1);
  DOM::Node * p = div->add_child(DOM::Tag::P)->add_id("para1");

  dom.show();

  DOM dom2;

  CSS * css = new CSS("test", "./", css_test_str, strlen(css_test_str));
  css->show();

  CSS::RulesMap rm;
  css->match(p, rm);

  css->show(rm);

  DOM::Node * ff_node = dom2.body->add_child(DOM::Tag::FONT_FACE);
  CSS::RulesMap rm2;
  css->match(ff_node, rm2);

  css->show(rm2);
  
  delete css;
}


int main() {

  test();
  return 0;

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