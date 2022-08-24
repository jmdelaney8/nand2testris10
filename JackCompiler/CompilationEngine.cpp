#include "CompilationEngine.h"
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

Segment toSegment(Kind kind) {
  Segment segment = Segment::NONE;
  switch (kind){
  case Kind::ARG:
    segment = Segment::ARGUMENT;
    break;
  case Kind::FIELD:
    segment = Segment::THIS;
    break;
  case Kind::STATIC:
    segment = Segment::STATIC;
    break;
  case Kind::VAR:
    segment = Segment::LOCAL;
    break;
  }
  assert(segment != Segment::NONE);
  return segment;
}

Compiler::Compiler(std::string infile_name, std::string outfile_name) :
  tokenizer(infile_name), writer(outfile_name)
{
  //outfile.open(outfile_name);
  label_counter = 0;
  advance();
}

void Compiler::compileClass() {
  // Create class-level symbol table
  class_table.reset();
  local_table.reset();
  advance();  // class
  current_class = tokenizer.identifier();
  advance();  // className
  advance();  // {
  while (tokenizer.keyword() == Keyword::STATIC || tokenizer.keyword() == Keyword::FIELD) {
    compileClassVarDec();
  }
  while (tokenizer.tokenType() == TokenType::KEYWORD) {
    compileSubroutine();
  }
  // Don't use function that consumes because it's the end of the file.
}

void Compiler::compileSubroutine() {
  // Initialize local symbol table
  local_table.reset();
  subroutine_type = tokenizer.keyword();
  if (subroutine_type == Keyword::METHOD) {
    local_table.define("this", "className", Kind::ARG);
  }
  advance();  // function | method | constructor
  advance();  // type
  subroutine_name = tokenizer.identifier();
  advance();  // subroutineName
  advance();  // (
  compileParameterList();
  advance();  // )
  compileSubroutineBody();
}

void Compiler::compileParameterList() {
  while (tokenizer.tokenType() != TokenType::SYMBOL) {
    // TODO: handle custom type or built in classes
    std::string type = tokenizer.value();
    advance();  // type
    std::string name = tokenizer.identifier();
    local_table.define(name, type, Kind::ARG);
    //outfile << "// added " << name << " to class symbol table" << std::endl;
    advance();  // varName
    if (tokenizer.symbol() == ',') {
      advance();  // ,
    }
  }
}


void Compiler::compileSubroutineBody() {
  advance();  // {
  while (tokenizer.keyword() == Keyword::VAR) {
    // Var decs
    while (tokenizer.keyword() == Keyword::VAR) {
      compileVarDec();
    }
  }
  writer.writeFunction(current_class + "." + subroutine_name, local_table.varCount(Kind::VAR));
  if (subroutine_type == Keyword::CONSTRUCTOR) {
    writer.writePush(Segment::CONSTANT, class_table.varCount(Kind::FIELD));
    writer.writeCall("Memory.alloc", 1);
    writer.writePop(Segment::POINTER, 0);
  }
  if (subroutine_type == Keyword::METHOD) {
    writer.writePush(Segment::ARGUMENT, 0);
    writer.writePop(Segment::POINTER, 0);
  }
  // Statements
  compileStatements();
  advance();  // }
}

void Compiler::compileClassVarDec() {
  Keyword kind_type = tokenizer.keyword();
  Kind kind;
  if (kind_type == Keyword::FIELD) {
    kind = Kind::FIELD;
  }
  else {
    kind = Kind::STATIC;
  }
  advance();  // static | field
  std::string type = tokenizer.value();
  advance();  // static | field
  // Compile 1 or more var names
  std::string name = tokenizer.identifier();
  class_table.define(name, type, kind);
  //outfile << "added " << name << " to class symbol table" << std::endl;
  advance();  // varName
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    advance();  // ,
    name = tokenizer.identifier();
    class_table.define(name, type, kind);
    //outfile << "// added " << name << " to class symbol table" << std::endl;
    advance();  // varName
  }
  advance();  // ;
}

void Compiler::compileVarDec() {
  advance();  // var
  std::string type = tokenizer.value();
  advance();  // type
  // Compile 1 or more var names
  std::string name = tokenizer.identifier();
  local_table.define(name, type, Kind::VAR);
  advance();  // varName
  while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
    advance();  // ,
    name = tokenizer.identifier();
    local_table.define(name, type, Kind::VAR);
    advance();  // varName
  }
  advance();  // ;
}

int Compiler::compileExpressionList() {
  int count = 0;
  // assume all elements of list are just an identifier
  while (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != ')') {
    compileExpression();
    count++;
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
      advance();  // ,
    }
  }
  return count;
}

void Compiler::compileStatements() {
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
}

void Compiler::compileIf() {
  std::string t = "true" + std::to_string(label_counter);
  std::string f = "false" + std::to_string(label_counter++);
  advance();  // if
  advance();  // (
  compileExpression();
  writer.writeArithmetic(Arithmetic::NOT);
  writer.writeIf(f);
  advance();  // )
  advance();  // {
  compileStatements();
  advance();  // }
  writer.writeGoto(t);
  writer.writeLabel(f);
  if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyword() == Keyword::ELSE) {
    advance(); // else
    advance();  // {
    compileStatements();
    advance(); // }
  }
  writer.writeLabel(t);
}

void Compiler::compileWhile() {
  advance();  // while
  advance();  // (
  std::string loop = "loop" + std::to_string(label_counter);
  std::string out = "out" + std::to_string(label_counter);
  label_counter++;
  writer.writeLabel(loop);
  compileExpression();
  advance();  // )
  writer.writeArithmetic(Arithmetic::NOT);
  writer.writeIf(out);
  advance();  // {
  compileStatements();
  advance();  // }
  writer.writeGoto(loop);
  writer.writeLabel(out);
}

void Compiler::compileReturn() {
  // return
  advance();  // return
  if (tokenizer.tokenType() != TokenType::SYMBOL) {
    compileExpression();
  }
  else {
    writer.writePush(Segment::CONSTANT, 0);
  }
  advance();  // ;
  writer.writeReturn();
}

void Compiler::compileLet() {
  advance();  // let
  std::string name = tokenizer.identifier();
  Kind kind;
  int index;
  SymbolTable* table;
  if (local_table.kindOf(name) != Kind::NONE) {
    table = &local_table;
  }
  else {
    table = &class_table;
  }
  kind = table->kindOf(name);
  index = table->indexOf(name);
  advance();  // varName
  // Process array indexing
  bool assign_to_array = tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[';
  if (assign_to_array) {
    advance();  // [
    writer.writePush(toSegment(kind), index);
    compileExpression();
    writer.writeArithmetic(Arithmetic::ADD);
    advance();  // ]
  }
  advance();  // =
  compileExpression();
  advance();  // ;
  if (assign_to_array) {
    writer.writePop(Segment::TEMP, 0);  // right side of = expression
    writer.writePop(Segment::POINTER, 1);  // left hand of = index address
    writer.writePush(Segment::TEMP, 0);  // put right hand expression on stack
    writer.writePop(Segment::THAT, 0);  // pop right hand to left hand
  }
  else {
    writer.writePop(toSegment(kind), index);
  }
}

void Compiler::compileDo() {
  advance();  // do
  compileExpression();
  advance();  // ;
  writer.writePop(Segment::TEMP, 0);  // pop return value of void function
}

void Compiler::compileExpression() {
  while (tokenizer.tokenType() != TokenType::SYMBOL || (tokenizer.symbol() != ';'
    && tokenizer.symbol() != ')' && tokenizer.symbol() != ',' && tokenizer.symbol() != ']'))
  {
    if (tokenizer.tokenType() == TokenType::SYMBOL
      && tokenizer.symbol() == '-')
    {
      advance();  // -
      compileTerm();
      writer.writeArithmetic(Arithmetic::NEG);
    }
    else if (tokenizer.tokenType() == TokenType::SYMBOL
      && tokenizer.symbol() == '~')
    {
      advance();  // ~
      compileTerm();
      writer.writeArithmetic(Arithmetic::NOT);
    }
    else {
      compileTerm();
      if (tokenizer.tokenType() == TokenType::SYMBOL
        && isArith(tokenizer.symbol()))
      {
        char op = tokenizer.symbol();
        advance();  // op
        compileTerm();
        writer.writeArithmetic(toArith(op));
      }
      else if (tokenizer.tokenType() == TokenType::SYMBOL
        && (tokenizer.symbol() == '*' || tokenizer.symbol() == '/'))
      {
        char op = tokenizer.symbol();
        advance();  // op
        compileTerm();
        std::string fname;
        if (op == '*') {
          fname = "Math.multiply";
        }
        else {
          fname = "Math.divide";
        }
        writer.writeCall(fname, 2);
      }
    }
  }
}

void Compiler::compileTerm() {
  // subexpression
  if (tokenizer.tokenType() == TokenType::SYMBOL && (tokenizer.symbol() == '(' || tokenizer.symbol() == '[')) {
    advance();  // [ | (
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '-') { // negation
      advance();  // -
      compileTerm();
      writer.writeArithmetic(Arithmetic::NEG);
    }
    else {
      compileExpression();
    }
    advance();  // ] | )
  }
  else {
    // Process term
    if (tokenizer.tokenType() == TokenType::IDENTIFIER) {
      std::string name = tokenizer.identifier();
      advance();  // varName | className | functionName
      if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '.') {  // method call
        // Put class location on stack.
        advance();  // .
        bool is_OS_call = (local_table.kindOf(name) == Kind::NONE && class_table.kindOf(name) == Kind::NONE);
        if (!is_OS_call) {
          SymbolTable* table = NULL;
          if (local_table.kindOf(name) == Kind::NONE) {
            table = &class_table;
          }
          else {
            table = &local_table;
          }
          writer.writePush(toSegment(table->kindOf(name)), table->indexOf(name));
          name = table->typeOf(name) + "." + tokenizer.identifier(); // full function name
        }
        else {
          name = name + "." + tokenizer.identifier();  // full function name
        }
        advance();  // methodName
        advance();  // (
        int n_args = compileExpressionList();  // first arg is class address
        if (!is_OS_call) {
          n_args++;
        }
        advance();  // )
        writer.writeCall(name, n_args);
      }
      else if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '(') {  // method call in this class (i think?)
        //assert(local_table.kindOf("this") != Kind::NONE);
        //writer.writePush(toSegment(local_table.kindOf("this")), local_table.indexOf("this"));
        // I think we are safe to push pointer 0 since an unamed method call will only happen within
        // the same object.
        writer.writePush(Segment::POINTER, 0);
        advance();  // (
        int n_args = compileExpressionList();
        n_args++;
        advance();  // )
        writer.writeCall(current_class + "." + name, n_args);

      }
      else if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[') { // array index
        SymbolTable* table = NULL;
        if (local_table.kindOf(name) == Kind::NONE) {
          table = &class_table;
        }
        else {
          table = &local_table;
        }
        writer.writePush(toSegment(table->kindOf(name)), table->indexOf(name));  // push array base
        advance();  // [
        compileExpression();  // push index
        advance();  // ]
        writer.writeArithmetic(Arithmetic::ADD);  // compute array index address
        writer.writePop(Segment::POINTER, 1);
        writer.writePush(Segment::THAT, 0);
      }
      else {  // variable
        SymbolTable* table = NULL;
        if (local_table.kindOf(name) == Kind::NONE) {
          table = &class_table;
        }
        else {
          table = &local_table;
        }
        writer.writePush(toSegment(table->kindOf(name)), table->indexOf(name));
      }
    } 
    else if (tokenizer.tokenType() == TokenType::INT_CONST) {
      int i = tokenizer.intVal();
      advance();  // int_const
      writer.writePush(Segment::CONSTANT, i);
    }
    else if (tokenizer.tokenType() == TokenType::STRING_CONST) {
      std::string str = tokenizer.stringVal();
      advance();  // string_const
      writer.writePush(Segment::CONSTANT, str.length());
      writer.writeCall("String.new", 1);
      for (int i = 0; i < str.length(); ++i) {
        // TODO: May need to convert to hack char set.
        writer.writePush(Segment::CONSTANT, str[i]);
        writer.writeCall("String.appendChar", 2);
      }
    }
    else if (tokenizer.tokenType() == TokenType::KEYWORD && (tokenizer.keyword() == Keyword::FALSE
      || tokenizer.keyword() == Keyword::_NULL))
    {
      writer.writePush(Segment::CONSTANT, 0);
      advance();  // false | null
    }
    else if(tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyword() == Keyword::TRUE) {
      writer.writePush(Segment::CONSTANT, 1);
      writer.writeArithmetic(Arithmetic::NEG);
      advance();  // true;
    }
    else if (tokenizer.tokenType() == TokenType::KEYWORD
      && tokenizer.keyword() == Keyword::THIS)
    {
      writer.writePush(Segment::POINTER, 0);
      advance(); // this
    }
    // TODO: handle this -> output: push pointer 0
    else {
      assert(false);
    }
  }
}

void Compiler::advance() {
  assert(tokenizer.hasMoreTokens());
  tokenizer.advance();
}