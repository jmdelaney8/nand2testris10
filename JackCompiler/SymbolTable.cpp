#include "SymbolTable.h"
#include <assert.h>


const std::string kinds[] = { "static", "field", "arg", "var", "none" };

SymbolTable::SymbolTable() {
  for (int i = 0; i < n_kinds; ++i) {
    indices[i] = 0;
  }
}

void SymbolTable::reset() {
  table.clear();
  for (int i = 0; i < n_kinds; ++i) {
    indices[i] = 0;
  }
}

void SymbolTable::define(const std::string& name, const std::string& type, const Kind& kind) {
  Symbol symbol;
  symbol.name = name;
  symbol.type = type;
  symbol.kind = kind;
  symbol.index = indices[(int)kind];
  table[name] = symbol;
  indices[(int)kind] += 1;
}

int SymbolTable::varCount(const Kind& kind) {
  return indices[(int)kind];
}

Kind SymbolTable::kindOf(const std::string& name) {
  auto symbol = table.find(name);
  if (symbol == table.end()) {
    return Kind::NONE;
  }
  else {
    return symbol->second.kind;
  }
}

std::string SymbolTable::typeOf(const std::string& name) {
  // Assumes element is in table.
  return table[name].type;
}

int SymbolTable::indexOf(const std::string & name) {
  // Assumes element is in table.
  return table[name].index;
}