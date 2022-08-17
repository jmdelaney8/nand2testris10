#pragma once

#include <string>
#include <fstream>
#include "JackTokenizer.h"


class Compiler {
  std::ofstream outfile;
  Tokenizer tokenizer;
  void process(std::string token_type, std::string token);
  void advance();
  void processIdentifier();
  void processSymbol();
  void processKeyword();
  void tag(std::string);
  void compileVarNameDec();
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