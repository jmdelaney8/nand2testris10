#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "JackTokenizer.h"
namespace fs = std::filesystem;


void tokenizeFile(const fs::path& p) {
  Tokenizer tokenizer = Tokenizer(p.string());
  while (tokenizer.hasMoreTokens()) {
    tokenizer.advance();
    std::cout << tokenizer.tokenType() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::string source = argv[1];
  fs::path path(source);
  if (fs::is_directory(path)) {
    fs::directory_iterator path_iter(path);
    for (auto& p : path_iter) {
      if (p.is_regular_file() && ".jack" == p.path().extension()) {
        tokenizeFile(p.path());
      }
    }
  }
  else {
    tokenizeFile(path);
  }
  return 0;
}
