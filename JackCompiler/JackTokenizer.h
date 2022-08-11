#pragma once

#include <iostream>
#include <fstream>
#include <string>

enum TokenType { KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST };

enum Keyword {
  CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR, STATIC, FIELD, LET, DO, IF, ELSE,
  WHILE, RETURN, TRUE, FALSE, _NULL, THIS
};

bool isSymbol(const char& c);

class Tokenizer {
  std::ifstream file;
  std::string token;
  TokenType token_type;
  bool initialized;

public:
  Tokenizer(std::string filename) {
    file.open(filename);
    token = "";
    token_type = KEYWORD;
  }
  TokenType tokenType();
  bool hasMoreTokens();
  void advance();
  Keyword keyword();
  char symbol();
  std::string identifier();
  int intVal();
  std::string stringVal();
  /*
    Checks if next characters in file are a comment or not. Does not remove comment.
  */
  bool nextIsComment();
  bool nextIsSpace();
};