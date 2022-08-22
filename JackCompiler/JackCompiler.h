#pragma once

#include <string>
#include <fstream>
#include "JackTokenizer.h"
#include "SymbolTable.h"

class Compiler {
  std::ofstream outfile;
  Tokenizer tokenizer;
  SymbolTable local_table;
  SymbolTable class_table;
  void process(std::string token_type, std::string token);
  void advance();
  void processIdentifier(const std::string&, const std::string&, const int&, const std::string&, bool do_advance=true);
  void processSymbol();
  void processKeyword();
  void tag(std::string);
  void compileType();
public:
  Compiler(std::string infile_name, std::string outfile_name);
  void compileClass();
  void compileClassVarDec();
  void compileSubroutine();
  void compileParameterList();
  void compileSubroutineBody();
  void compileVarDec();
  void compileStatements();
  void compileLet();
  void compileIf();
  void compileWhile();
  void compileDo();
  void compileReturn();
  void compileExpression();
  void compileTerm();
  int compileExpressionList();
};