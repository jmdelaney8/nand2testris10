#pragma once

#include <iostream>
#include <fstream>
#include <string>

enum TokenType { KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST };

enum Keyword {
  CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR, STATIC, FIELD, LET, DO, IF, ELSE,
  WHILE, RETURN, TRUE, FALSE, _NULL, THIS
};

std::string toString(const TokenType &type);
std::string toString(const Keyword& keyword);
std::string escapeXML(const char& c);


class Tokenizer {
  std::ifstream file;
  std::string token;
  bool initialized;

public:
  Tokenizer(std::string filename) {
    file.open(filename);
    token = "";
  }
  TokenType tokenType();
  bool hasMoreTokens();
  void advance();
  Keyword keyword();
  char symbol();
  std::string identifier();
  int intVal();
  std::string stringVal();
  std::string value();
  /*
    Checks if next characters in file are a comment or not. Does not remove comment.
  */
  bool nextIsSingleComment();
  bool nextIsMultiComment();
  bool nextIsSpace();
};