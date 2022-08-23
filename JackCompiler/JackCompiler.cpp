#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "JackTokenizer.h"
#include "CompilationEngine.h"
namespace fs = std::filesystem;

std::ofstream outfile;


void compileFile(const fs::path& inpath, const fs::path& outpath) {
  Compiler compiler(inpath.string(), outpath.string());
  compiler.compileClass();
}


int main(int argc, char* argv[]) {
  std::string source = argv[1];
  fs::path path(source);
  
  if (fs::is_directory(path)) {
    
    fs::directory_iterator path_iter(path);
    for (auto& p : path_iter) {
      if (p.is_regular_file() && ".jack" == p.path().extension()) {
        // Create output file
        fs::path outpath = path.parent_path() / path.stem() / p.path().stem().string().append(".vm");
        compileFile(p.path(), outpath);
      }
    }
  }
  else {
    // Create output file
    fs::path outpath = path.parent_path() / path.stem().string().append(".vm");
    compileFile(path, outpath);
  }
  return 0;
}
