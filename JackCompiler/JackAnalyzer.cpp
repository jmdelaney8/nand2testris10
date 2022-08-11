#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "JackTokenizer.h"
namespace fs = std::filesystem;

std::ofstream outfile;


void tokenizeFile(const fs::path& p) {
  Tokenizer tokenizer = Tokenizer(p.string());
  outfile << "<tokens>" << std::endl;
  while (tokenizer.hasMoreTokens()) {
    tokenizer.advance();
    std::string token_type = toString(tokenizer.tokenType());
    outfile << "<" << token_type << "> ";
    outfile << tokenizer.value();
    outfile << " </" << token_type << ">" << std::endl;
  }
  outfile << "</tokens>" << std::endl;
}

int main(int argc, char* argv[]) {
  std::string source = argv[1];
  fs::path path(source);
  
  // Tokenize input files
  if (fs::is_directory(path)) {
    
    fs::directory_iterator path_iter(path);
    for (auto& p : path_iter) {
      if (p.is_regular_file() && ".jack" == p.path().extension()) {
        // Create output file
        fs::path outpath = path.parent_path() / path.stem() / p.path().stem().string().append("T_test.xml");
        outfile.open(outpath.string());
        tokenizeFile(p.path());
        outfile.close();
      }
    }
  }
  else {
    // Create output file
    fs::path outpath = path.parent_path() / path.stem().string().append("T_test.xml");
    outfile.open(outpath.string());
    tokenizeFile(path);
  }
  return 0;
}
