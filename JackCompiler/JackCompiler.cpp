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
  // Create class-level symbol table
  class_table.reset();
  tag("class");
  processKeyword();  // class
  std::string name = tokenizer.identifier();
  processIdentifier(name, "className", 0, "declared");  // className
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
  // Initialize local symbol table
  local_table.reset();
  if (tokenizer.keyword() == Keyword::METHOD || tokenizer.keyword() == Keyword::CONSTRUCTOR) {
    local_table.define("this", "className", "arg");
  }
  tag("subroutineDec");
  processKeyword();  // function | method | constructor
  compileType();
  processIdentifier(tokenizer.identifier(), "subroutine", 0, "declared");
  processSymbol();  // (
  compileParameterList();
  processSymbol(); // )
  compileSubroutineBody();
  tag("/subroutineDec");
}

void Compiler::compileParameterList() {
  tag("parameterList");
  while (tokenizer.tokenType() != TokenType::SYMBOL) {
    std::string type = toString(tokenizer.keyword());
    processKeyword();  // type
    std::string name = tokenizer.identifier();
    local_table.define(name, type, "arg");
    processIdentifier(name, "arg", local_table.indexOf(name), "declared");  // varName
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
  tag("/subroutineBody");
}

void Compiler::compileClassVarDec() {
  tag("classVarDec");
  Keyword kind_type = tokenizer.keyword();
  std::string kind;
  if (kind_type == Keyword::FIELD) {
    kind = "field";
  }
  else {
    kind = "static";
  }

  processKeyword();  // static | field
  std::string type = tokenizer.value();
  compileType();
  // Compile 1 or more var names
  std::string name = tokenizer.identifier();
  class_table.define(name, type, kind);
  processIdentifier(name, kind, class_table.indexOf(name), "declared");  // varName
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    processSymbol();  // ,
    name = tokenizer.identifier();
    class_table.define(name, type, kind);
    processIdentifier(name, kind, class_table.indexOf(name), "declared");  // varName
  }
  processSymbol();  // ;
  tag("/classVarDec");
}

void Compiler::compileVarDec() {
  tag("varDec");
  processKeyword();  // var
  std::string type = tokenizer.value();
  compileType();
  // Compile 1 or more var names
  std::string name = tokenizer.identifier();
  local_table.define(name, type, "var");
  processIdentifier(name, "var", local_table.indexOf(name), "declared");  // varName
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    processSymbol();  // ,
    name = tokenizer.identifier();
    local_table.define(name, type, "var");
    processIdentifier(name, "var", local_table.indexOf(name), "declared");  // varName
  }
  processSymbol();  // ;
  tag("/varDec");
}

void Compiler::compileType() {
  if (tokenizer.tokenType() == TokenType::KEYWORD) {
    processKeyword();  // type
  }
  else {
    process("identifier", tokenizer.identifier());  // custom type
    advance();
  }
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
  std::string name = tokenizer.identifier();
  std::string kind;
  int index;
  if (local_table.kindOf(name) != "none") {
    kind = local_table.kindOf(name);
    index = local_table.indexOf(name);
  }
  else {
    kind = class_table.kindOf(name);
    index = class_table.indexOf(name);
  }
  processIdentifier(tokenizer.identifier(), kind, index, "used");  // varName
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
  std::string name = tokenizer.identifier();  // function name | class name
  advance();
  // compile sub class or method calls
  if (tokenizer.symbol() == '.') {
    processIdentifier(name, "className", 0, "used", false);
    processSymbol();  // .
    processIdentifier(tokenizer.identifier(), "subroutine", 0, "used");  // method name
  }
  else {
    processIdentifier(name, "subroutine", 0, "used", false);  // subroutine name
  }
  processSymbol(); // (
  // argument list
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
    if (tokenizer.tokenType() == TokenType::IDENTIFIER) {
      std::string name = tokenizer.identifier();
      advance();
      if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '.') {  // method call
        processIdentifier(name, "className", 0, "used", false);
        processSymbol();  // .
        processIdentifier(tokenizer.identifier(), "subroutine", 0, "used"); // methodName;
        processSymbol();  // (
        compileExpressionList();
        processSymbol();  // )
      }
      else if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '(') {
        processIdentifier(name, "subroutine", 0, "used", false);
        processSymbol();  // (
        compileExpressionList();
        processSymbol();  // )
      }
      else if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[') { // array index
        SymbolTable* table = NULL;
        if (local_table.kindOf(name) == "none") {
          table = &class_table;
        }
        else {
          table = &local_table;
        }
        processIdentifier(name, table->kindOf(name), table->indexOf(name), "used", false);
        processSymbol();  // [
        compileExpression();
        processSymbol();  // ]
      }
      else {
        SymbolTable* table = NULL;
        if (local_table.kindOf(name) == "none") {
          table = &class_table;
        }
        else {
          table = &local_table;
        }
        processIdentifier(name, table->kindOf(name), table->indexOf(name), "used", false);
      }
    } 
    else {
      process(toString(tokenizer.tokenType()), tokenizer.value());
      advance();
    }
  }
  tag("/term");
}


void Compiler::process(std::string token_type, std::string token) {
  outfile << "<" << token_type << "> " << token << " </" << token_type << ">" << std::endl;
}

void Compiler::processIdentifier(const std::string& name, const std::string& category, const int &index,
  const std::string& usage, bool do_advance)
{
  outfile << "<identifier> " << name << " " << category << " " << index << " " << usage
    << " </identifier>" << std::endl;
  if (do_advance) {
    advance();
  }
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


