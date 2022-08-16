#include "JackCompiler.h"
#include <assert.h>

Compiler::Compiler(std::string infile_name, std::string outfile_name) : tokenizer(infile_name) {
  outfile.open(outfile_name);
  advance();
}

void Compiler::compileClass() {
  outfile << "<class>" << std::endl;
  processKeyword();
  if (!tokenizer.hasMoreTokens()) return;
  tokenizer.advance();
  processIdentifier();
  if (!tokenizer.hasMoreTokens()) return;
  tokenizer.advance();
  processSymbol();
  if (!tokenizer.hasMoreTokens()) return;

  tokenizer.advance();
  while (tokenizer.keyword() == Keyword::STATIC || tokenizer.keyword() == Keyword::FIELD) {
    compileClassVarDec();
    advance();
  }
  while (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyword() == Keyword::FUNCTION) {
    compileSubroutine();
    advance();
  }

  processSymbol();  // }
  outfile << "</class>" << std::endl;
}

void Compiler::compileSubroutine() {
  outfile << "<subroutineDec>" << std::endl;
  // function
  processKeyword();
  advance();
  // type
  processKeyword();
  advance();
  // identifier
  processIdentifier();
  advance();
  // parameter list
  processSymbol();
  outfile << "<parameterList>" << std::endl;
  advance();
  while (tokenizer.tokenType() != TokenType::SYMBOL) {
    // type
    processKeyword();
    advance();
    // identifier
    processIdentifier();
    advance();
    if (tokenizer.symbol() == ',') {
      processSymbol();
      advance();
    }
  }
  outfile << "</parameterList>" << std::endl;
  processSymbol();
  advance();
  // Body
  outfile << "<subroutineBody>" << std::endl;
  processSymbol();
  advance();
  // Var decs
  outfile << "<varDec>" << std::endl;
  while (tokenizer.keyword() == Keyword::VAR) {
    compileVarDec();
    advance();
  }
  outfile << "</varDec>" << std::endl;
  // Statements
  compileStatements();
  processSymbol();  // }
  outfile << "</subroutineBody>" << std::endl;

  outfile << "</subroutineDec>" << std::endl;
}

void Compiler::compileClassVarDec() {
  outfile << "<classVarDec>" << std::endl;
  processKeyword();  // static | field
  advance();
  processKeyword();  // type TODO: could this possibly be custom and therefore an identifier?
  advance();
  processIdentifier();  // varName
  advance();
  processSymbol();  // ;
  outfile << "</classVarDec>" << std::endl;
}

void Compiler::compileVarDec() {
  process("keyword", "var");
  advance();
  if (tokenizer.tokenType() == TokenType::KEYWORD) {
    processKeyword();  // type
  }
  else {
    processIdentifier();  // custom type
  }
  advance();
  processIdentifier();  // var name
  advance();
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    processSymbol();
    advance();
    processIdentifier();
    advance();
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
    advance();
  }
  outfile << "</statements>" << std::endl;
}

void Compiler::compileIf() {
  tag("ifStatement");
  processKeyword();  // if
  advance();
  processSymbol();  // (
  advance();
  compileExpression();
  advance();
  processSymbol();  // )
  advance();
  processSymbol();  // {
  advance();
  compileStatements();
  // Don't advance if there were no statements to compile
  if (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != '}') {
    advance();
  }
  processSymbol();  // }
  advance();
  if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyword() == Keyword::ELSE) {
    processKeyword(); // else
    advance();
    processSymbol();  // {
    advance();
    compileStatements();
    if (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != '}') {
      advance();
    }
    processSymbol(); // }
  }

  tag("/ifStatement");
}

void Compiler::compileWhile() {
  tag("whileStatement");
  processKeyword();  // while
  advance();
  processSymbol();  // (
  advance();
  compileExpression();
  advance();
  processSymbol();  // )
  advance();
  processSymbol();  // {
  compileStatements();
  advance();
  processSymbol();  // }
  tag("/whileStatement");
}

void Compiler::compileReturn() {
  tag("returnStatement");
  // return
  processKeyword();
  advance();
  // assume no expression for now
  // ;
  processSymbol();
  tag("/returnStatement");
}

void Compiler::compileLet() {
  outfile << "<letStatement>" << std::endl;
  process("keyword", "let");
  advance();
  processIdentifier();
  advance();
  processSymbol();
  advance();
  compileExpression();
  advance();
  // ;
  processSymbol();
  outfile << "</letStatement>" << std::endl;
}

void Compiler::compileDo() {
  outfile << "<doStatement>" << std::endl;
  process("keyword", "do");
  advance();
  // subroutine call
  processIdentifier();
  advance();
  // either ( or .
  processSymbol();
  // compile 'identifier.' structure
  while (tokenizer.symbol() == '.') {
    advance();
    processIdentifier();
    advance();
    processSymbol();
  }
  advance();
  // parameter list
  compileExpressionList();
  if (tokenizer.tokenType() != TokenType::SYMBOL) {
    advance();
  }
  // )
  processSymbol();
  advance();
  // ;
  processSymbol();

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
}


void Compiler::processSymbol() {
  process("symbol", escapeXML(tokenizer.symbol()));
}

void Compiler::processKeyword() {
  process("keyword", toString(tokenizer.keyword()));
}

void Compiler::advance() {
  assert(tokenizer.hasMoreTokens());
  tokenizer.advance();
}


void Compiler::tag(std::string tag) {
  outfile << "<" << tag << ">" << std::endl;
}


