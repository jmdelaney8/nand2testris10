#include "VMWriter.h"
#include <assert.h>

const int n_segments = 8;
std::string segment_strs[n_segments] = { "constant", "argument", "local", "static", "this", "that", 
                                          "pointer" , "temp" };

const int n_arith = 9;
std::string arithmetic_strs[n_arith] = { "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not" };
const char arith_chars[n_arith] = { '+', '-', '-', '=', '>', '<', '&', '|', '~' };

bool isArith(char symbol) {
  for (int i = 0; i < n_arith; ++i) {
    if (symbol == arith_chars[i]) return true;
  }
  return false;
}

Arithmetic toArith(char op) {
  for (int i = 0; i < n_arith; ++i) {
    if (op == arith_chars[i]) return ariths[i];
  }
  assert(false);
}

std::string toString(Arithmetic op) {
  return arithmetic_strs[(int)op];
}

std::string toString(Segment segment) {
  return segment_strs[(int)segment];
}

VMWriter::VMWriter(std::string outfilename) {
  outfile.open(outfilename);
}

void VMWriter::writePush(Segment segment, int index) {
  outfile << "push " << toString(segment) << " " << index << std::endl;
}

void VMWriter::writePop(Segment segment, int index) {
  outfile << "pop " << toString(segment) << " " << index << std::endl;
}

void VMWriter::writeArithmetic(Arithmetic op) {
  outfile << toString(op) << std::endl;
}

void VMWriter::writeLabel(std::string label) {
  outfile << "label " << label << std::endl;
}

void VMWriter::writeGoto(std::string label) {
  outfile << "goto " << label << std::endl;
}

void VMWriter::writeIf(std::string label) {
  outfile << "if-goto " << label << std::endl;
}

void VMWriter::writeCall(std::string name, int nArgs) {
  outfile << "call " << name << " " << nArgs << std::endl;
}

void VMWriter::writeFunction(std::string name, int nVars) {
  outfile << "function " << name << " " << nVars << std::endl;
}

void VMWriter::writeReturn() {
  outfile << "return" << std::endl;
}

void VMWriter::close() {
  outfile.close();
}

