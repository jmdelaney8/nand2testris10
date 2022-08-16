#include "JackCompiler.h"
#include <assert.h>

// All process and compile functions return with the tokenizer on the next token after the function's jurisdiction.

Compiler::Compiler(std::string infile_name, std::string outfile_name) : tokenizer(infile_name) {
  outfile.open(outfile_name);
  advance();
}

void Compiler::compileClass() {
  tag("class");
  processKeyword();  // class
  processIdentifier();  // className
  processSymbol();  // {
  tag("classVarDec");
  while (tokenizer.keyword() == Keyword::STATIC || tokenizer.keyword() == Keyword::FIELD) {
    compileClassVarDec();
  }
  tag("/classVarDec");
  while (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyword() == Keyword::FUNCTION) {
    compileSubroutine();
  }
  // Don't use function that consumes because it's the end of the file.
  process("symbol", "}");  // }
  tag("/class");
}

void Compiler::compileSubroutine() {
  outfile << "<subroutineDec>" << std::endl;
  processKeyword();  // function
  processKeyword();  // type
  processIdentifier();  // function name
  processSymbol();  // (
  tag("parameterList");
  while (tokenizer.tokenType() != TokenType::SYMBOL) {
    processKeyword();  // type
    processIdentifier();  // varName
    if (tokenizer.symbol() == ',') {
      processSymbol();  // ,
    }
  }
  tag("/parameterList");
  processSymbol(); // )
  // Body
  outfile << "<subroutineBody>" << std::endl;
  processSymbol();
  // Var decs
  outfile << "<varDec>" << std::endl;
  while (tokenizer.keyword() == Keyword::VAR) {
    compileVarDec();
  }
  outfile << "</varDec>" << std::endl;
  // Statements
  compileStatements();
  processSymbol();  // }
  outfile << "</subroutineBody>" << std::endl;

  outfile << "</subroutineDec>" << std::endl;
}

void Compiler::compileClassVarDec() {
  processKeyword();  // static | field
  processKeyword();  // type TODO: could this possibly be custom and therefore an identifier?
  processIdentifier();  // varName
  processSymbol();  // ;
}

void Compiler::compileVarDec() {
  processKeyword();  // var
  if (tokenizer.tokenType() == TokenType::KEYWORD) {
    processKeyword();  // type
  }
  else {
    processIdentifier();  // custom type
  }
  processIdentifier();  // varName
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    processSymbol();  // ,
    processIdentifier();  // varName
  }
  processSymbol();  // ;
}

int Compiler::compileExpressionList() {
  outfile << "<expressionList>" << std::endl;

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
  // assume no expression for now
  processSymbol();  // ;
  tag("/returnStatement");
}

void Compiler::compileLet() {
  outfile << "<letStatement>" << std::endl;
  processKeyword();  // let
  processIdentifier();  // varName
  processSymbol();  // =
  compileExpression();
  processSymbol();  // ;
  outfile << "</letStatement>" << std::endl;
}

void Compiler::compileDo() {
  outfile << "<doStatement>" << std::endl;
  processKeyword();  // do
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
  outfile << "<expression>" << std::endl;
  // Assume we statement is an identifier
  outfile << "<term>" << std::endl;
  processIdentifier();
  outfile << "</term>" << std::endl;
  outfile << "</expression>" << std::endl;
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


