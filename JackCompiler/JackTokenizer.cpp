#include "JackTokenizer.h"

#include<assert.h>


const std::string token_types[] = {"keyword", "symbol", "identifier", "integerConstant", "stringConstant"};
const int n_keywords = 21;
const std::string keyword_tokens[n_keywords] = {
  "class", "method", "function", "constructor", "int", "boolean", "char", "void",
  "var", "static", "field", "let", "do", "if", "else", "while", "return", "true",
  "false", "null", "this"
};
const int n_symbols = 19;
char symbols[n_symbols] = { '{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>',
                            '=', '~' };

bool isSymbol(const char& c) {
  for (int i = 0; i < n_symbols; ++i) {
    if (symbols[i] == c) return true;
  }
  return false;
}

bool isKeyword(const std::string& t) {
  for (int i = 0; i < n_keywords; ++i) {
    if (t == keyword_tokens[i]) return true;
  }
  return false;
}

std::string toString(const TokenType& type) {
  return token_types[(int)type];
}

std::string toString(const Keyword& keyword) {
  return keyword_tokens[(int)keyword];
}

bool Tokenizer::nextIsSpace() {
  int c = file.peek();
  return c == ' ' || c == '\n' || c == '\t';
}

bool Tokenizer::hasMoreTokens() {
  int c = file.peek();
  while (nextIsComment() || nextIsSpace()) {
    // Ignore empty space
    while (c != EOF && nextIsSpace()) {
      c = file.get();
      c = file.peek();
    }
    if (c == EOF) {
      return false;
    }
    // Ignore comments
    while (c != EOF && nextIsComment()) {
      char line[256];
      file.getline(line, 256);
      c = file.peek();
    }
  }
  return c != EOF;
}

Keyword Tokenizer::keyword() {
  Keyword keywords[n_keywords] = { Keyword::CLASS, Keyword::METHOD, Keyword::FUNCTION,  Keyword::CONSTRUCTOR, Keyword::INT, Keyword::BOOLEAN,
                  Keyword::CHAR, Keyword::VOID, Keyword::VAR, Keyword::STATIC, Keyword::FIELD,
                  Keyword::LET, Keyword::DO, Keyword::IF, Keyword::ELSE, Keyword::WHILE, Keyword::RETURN,
                  Keyword::TRUE, Keyword::FALSE, Keyword::_NULL, Keyword::THIS };
  for (int i = 0; i < n_keywords; ++i) {
    if (token == keyword_tokens[i]) return keywords[i];
  }
  assert (false);
}

char Tokenizer::symbol() {
  return token.at(0);
}

std::string Tokenizer::identifier() {
  return token;
}

int Tokenizer::intVal() {
  return stoi(token);
}

std::string Tokenizer::stringVal() {
  return token;
}

bool Tokenizer::nextIsComment() {
  int c = file.peek();
  if (c == '/') {
    file.get();
    c = file.peek();
    file.unget();
    if (c == '/') return true;
    if (c == '*') {
      c = file.get();
      c = file.get();
      c = file.peek();
      file.unget();
      file.unget();
      return c == '*';
    }
  }
  return false;
}

void Tokenizer::advance() {
  token = "";
  char c0, c;
  c0 = file.get();
  // Consume string
  if (c0 == '\"') {
    token.push_back(c0);
    c = file.peek();
    while (c != '\"') {
      c = file.get();
      token.push_back(c);
      c = file.peek();
    }
    c =file.get();
    token.push_back(c);
    return;
  }

  token.push_back(c0);
  // TODO: This can be done cleaner.
  if (!isSymbol(c0)) {
    c = file.peek();
    // Read in multicharacter token
    while (!isSymbol(c) && c != EOF && !nextIsSpace() && !Tokenizer::nextIsComment()) {
      c = file.get();
      token.push_back(c);
      c = file.peek();
    }
  }
}

TokenType Tokenizer::tokenType() {
  if (token.length() == 1 && isSymbol(token.at(0))) return TokenType::SYMBOL;
  if (isKeyword(token)) return TokenType::KEYWORD;
  if (isdigit(token.at(0))) return TokenType::INT_CONST;
  if (token.at(0) == '\"' || token.at(0) == '\'') return TokenType::STRING_CONST;
  if (isalpha(token.at(0))) return TokenType::IDENTIFIER;
  else {
    assert (false);
  }
}

std::string Tokenizer::value() {
  std::string val;
  switch (tokenType()) {
    case TokenType::IDENTIFIER:
      val = identifier();
      break;
    case TokenType::INT_CONST:
      val = std::to_string(intVal());
      break;
    case TokenType::KEYWORD:
      val = toString(keyword());
      break;
    case TokenType::STRING_CONST:
      val = stringVal();
      val = val.substr(1, val.length() - 2);
      break;
    case TokenType::SYMBOL:
      val.push_back(symbol());
      break;
  }
  return val;
}
