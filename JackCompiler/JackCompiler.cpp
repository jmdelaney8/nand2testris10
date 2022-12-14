#include "JackCompiler.h"
#include <assert.h>

// All process and compile functions return with the tokenizer on the next token after the function's jurisdiction.
const int num_ops = 9;
char ops[num_ops] = { '+', '-', '*', '/', '&', '|', '<', '>', '=' };

bool isOp(char symbol) {
  for (int i = 0; i < num_ops; ++i) {
    if (ops[i] == symbol) return true;
  }
  return false;
}

bool isUnaryOp(char symbol) {
  return symbol == '-' || symbol == '~';
}

Compiler::Compiler(std::string infile_name, std::string outfile_name) : tokenizer(infile_name) {
  outfile.open(outfile_name);
  advance();
}

void Compiler::compileClass() {
  tag("class");
  processKeyword();  // class
  processIdentifier();  // className
  processSymbol();  // {
  while (tokenizer.keyword() == Keyword::STATIC || tokenizer.keyword() == Keyword::FIELD) {
    compileClassVarDec();
  }
  while (tokenizer.tokenType() == TokenType::KEYWORD) {
    compileSubroutine();
  }
  // Don't use function that consumes because it's the end of the file.
  process("symbol", "}");  // }
  tag("/class");
}

void Compiler::compileSubroutine() {
  tag("subroutineDec");
  processKeyword();  // function
  compileType();
  processIdentifier();  // function name
  processSymbol();  // (
  compileParameterList();
  processSymbol(); // )
  compileSubroutineBody();
  tag("/subroutineDec");;
}

void Compiler::compileParameterList() {
  tag("parameterList");
  while (tokenizer.tokenType() != TokenType::SYMBOL) {
    processKeyword();  // type
    processIdentifier();  // varName
    if (tokenizer.symbol() == ',') {
      processSymbol();  // ,
    }
  }
  tag("/parameterList");
}


void Compiler::compileSubroutineBody() {
  // Body
  tag("subroutineBody");
  processSymbol();  // {
  if (tokenizer.keyword() == Keyword::VAR) {
    // Var decs
    while (tokenizer.keyword() == Keyword::VAR) {
      compileVarDec();
    }
  }
  // Statements
  compileStatements();
  processSymbol();  // }
  tag("/subroutineBody");;
}

void Compiler::compileClassVarDec() {
  tag("classVarDec");
  processKeyword();  // static | field
  compileType();
  compileVarNameDec();
  tag("/classVarDec");
}

void Compiler::compileVarDec() {
  tag("varDec");
  processKeyword();  // var
  compileType();
  compileVarNameDec(); 
  tag("/varDec");
}

void Compiler::compileType() {
  if (tokenizer.tokenType() == TokenType::KEYWORD) {
    processKeyword();  // type
  }
  else {
    processIdentifier();  // custom type
  }
}

void Compiler::compileVarNameDec() {
  processIdentifier();  // varName
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    processSymbol();  // ,
    processIdentifier();  // varName
  }
  processSymbol();  // ;
}

int Compiler::compileExpressionList() {
  outfile << "<expressionList>" << std::endl;
  // assume all elements of list are just an identifier
  while (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != ')') {
    compileExpression();
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
      processSymbol();  // ,
    }
  }
  outfile << "</expressionList>" << std::endl;
  return 0;
}

void Compiler::compileStatements() {
  outfile << "<statements>" << std::endl;
  while (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != '}') {
    if (tokenizer.keyword() == Keyword::DO) {
      compileDo();
    }
    else if (tokenizer.keyword() == Keyword::LET) {
      compileLet();
    }
    else if (tokenizer.keyword() == Keyword::RETURN) {
      compileReturn();
    }
    else if (tokenizer.keyword() == Keyword::IF) {
      compileIf();
    }
    else if (tokenizer.keyword() == Keyword::WHILE) {
      compileWhile();
    }
    else {
      assert(false);
    }
  }
  outfile << "</statements>" << std::endl;
}

void Compiler::compileIf() {
  tag("ifStatement");
  processKeyword();  // if
  processSymbol();  // (
  compileExpression();
  processSymbol();  // )
  processSymbol();  // {
  compileStatements();
  processSymbol();  // }
  if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyword() == Keyword::ELSE) {
    processKeyword(); // else
    processSymbol();  // {
    compileStatements();
    processSymbol(); // }
  }

  tag("/ifStatement");
}

void Compiler::compileWhile() {
  tag("whileStatement");
  processKeyword();  // while
  processSymbol();  // (
  compileExpression();
  processSymbol();  // )
  processSymbol();  // {
  compileStatements();
  processSymbol();  // }
  tag("/whileStatement");
}

void Compiler::compileReturn() {
  tag("returnStatement");
  // return
  processKeyword();  // return
  if (tokenizer.tokenType() != TokenType::SYMBOL) {
    compileExpression();
  }
  processSymbol();  // ;
  tag("/returnStatement");
}

void Compiler::compileLet() {
  outfile << "<letStatement>" << std::endl;
  processKeyword();  // let
  processIdentifier();  // varName
  // Process array indexing
  if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[') {
    processSymbol();  // [
    compileExpression();
    processSymbol();  // ]
  }
  processSymbol();  // =
  compileExpression();
  processSymbol();  // ;
  outfile << "</letStatement>" << std::endl;
}

void Compiler::compileDo() {
  outfile << "<doStatement>" << std::endl;
  processKeyword();  // do
  // TODO: book recommends compiling the rest as an expression.
  processIdentifier();  // subroutine name
  // compile sub class or method calls
  while (tokenizer.symbol() == '.') {
    processSymbol();  // .
    processIdentifier();  // class name | method name
  }
  processSymbol(); // (
  // parameter list
  compileExpressionList();
  processSymbol();  // );
  processSymbol();  // ;

  outfile << "</doStatement>" << std::endl;

}

void Compiler::compileExpression() {
  tag("expression");
  while (tokenizer.tokenType() != TokenType::SYMBOL || (tokenizer.symbol() != ';'
    && tokenizer.symbol() != ')' && tokenizer.symbol() != ',' && tokenizer.symbol() != ']'))
  {
    if (tokenizer.tokenType() == TokenType::SYMBOL
      && isOp(tokenizer.symbol()))
    {
      //tag("op");
      processSymbol();
      //tag("/op");
    }
    else if (tokenizer.tokenType() == TokenType::SYMBOL
      && tokenizer.symbol() == '~')
    {
      tag("term");
      processSymbol();  // ~
      compileTerm();
      tag("/term");
    }
    else {
      compileTerm();
    }
  }
  tag("/expression");
}

void Compiler::compileTerm() {
  tag("term");
  // subexpression
  if (tokenizer.tokenType() == TokenType::SYMBOL && (tokenizer.symbol() == '(' || tokenizer.symbol() == '[')) {
    processSymbol();  // [ | )
    // Compile unary op
    if (tokenizer.tokenType() == TokenType::SYMBOL && isUnaryOp(tokenizer.symbol())) {
      tag("expression");
      tag("term");
      processSymbol();
      compileTerm();
      tag("/term");
      tag("/expression");
    }
    else {
      compileExpression();
    }
    processSymbol();  // ] | )
  }
  else {
    // Process term
    process(toString(tokenizer.tokenType()), tokenizer.value());
    advance();
    if (tokenizer.tokenType() == TokenType::SYMBOL) {  // method call
      if (tokenizer.symbol() == '.') {
        processSymbol();  // .
        processIdentifier(); // methodName;
        processSymbol();  // (
        compileExpressionList();
        processSymbol();  // )
      }
      else if (tokenizer.symbol() == '(') {  // function call
        processSymbol();  // (
        compileExpressionList();
        processSymbol();  // )
      }
      else if (tokenizer.symbol() == '[') { // array index
        processSymbol();  // [
        compileExpression();
        processSymbol();  // ]
      }
    }
  }
  tag("/term");
}


void Compiler::process(std::string token_type, std::string token) {
  outfile << "<" << token_type << "> " << token << " </" << token_type << ">" << std::endl;
}

void Compiler::processIdentifier() {
  process("identifier", tokenizer.identifier());
  advance();
}


void Compiler::processSymbol() {
  process("symbol", escapeXML(tokenizer.symbol()));
  advance();
}

void Compiler::processKeyword() {
  process("keyword", toString(tokenizer.keyword()));
  advance();
}

void Compiler::advance() {
  assert(tokenizer.hasMoreTokens());
  tokenizer.advance();
}


void Compiler::tag(std::string tag) {
  outfile << "<" << tag << ">" << std::endl;
}


