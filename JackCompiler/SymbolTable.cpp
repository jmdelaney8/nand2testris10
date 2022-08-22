#include "SymbolTable.h"
#include <assert.h>

int toIndex(std::string kind) {
  for (int i = 0; i < n_kinds; ++i) {
    if (kind == kinds[i]) return i;
  }
  assert(false);
}


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

void SymbolTable::define(const std::string& name, const std::string& type, const std::string& kind) {
  Symbol symbol;
  symbol.name = name;
  symbol.type = type;
  symbol.kind = kind;
  symbol.index = indices[toIndex(kind)];
  table[name] = symbol;
  indices[toIndex(kind)] += 1;
}

int SymbolTable::varCount(const std::string& kind) {
  return indices[toIndex(kind)];
}

std::string SymbolTable::kindOf(const std::string& name) {
  auto symbol = table.find(name);
  if (symbol == table.end()) {
    return "none";
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