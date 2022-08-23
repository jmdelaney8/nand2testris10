#pragma once

#include <string>
#include <unordered_map>

enum class Kind { STATIC, FIELD, ARG, VAR, NONE };
const int n_kinds = 4;


class SymbolTable {
  struct Symbol {
    std::string name;
    std::string type;
    Kind kind;
    int index;
  };
  std::unordered_map<std::string, Symbol> table;
  int indices[n_kinds];
public:
  SymbolTable();
  void reset();
  void define(const std::string& name, const std::string& type, const Kind& kind);
  int varCount(const Kind &kind);
  Kind kindOf(const std::string& name);
  std::string typeOf(const std::string& name);
  int indexOf(const std::string& name);
};