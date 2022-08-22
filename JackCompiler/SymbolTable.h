#pragma once

#include <string>
#include <unordered_map>

const int n_kinds = 4;
const std::string kinds[] = { "static", "field", "arg", "var", "none" };


class SymbolTable {
  struct Symbol {
    std::string name;
    std::string type;
    std::string kind;
    int index;
  };
  std::unordered_map<std::string, Symbol> table;
  int indices[n_kinds];
public:
  SymbolTable();
  void reset();
  void define(const std::string& name, const std::string& type, const std::string& kind);
  int varCount(const std::string &kind);
  std::string kindOf(const std::string& name);
  std::string typeOf(const std::string& name);
  int indexOf(const std::string& name);
};