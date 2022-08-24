#pragma once

#include <string>
#include <fstream>
#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"

class Compiler {
  std::ofstream outfile;
  Tokenizer tokenizer;
  SymbolTable local_table;
  SymbolTable class_table;
  VMWriter writer;
  std::string subroutine_name;
  std::string current_class;
  int label_counter;
  Keyword subroutine_type;
  void advance();
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