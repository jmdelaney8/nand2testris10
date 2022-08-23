#pragma once
#include <string>
#include <fstream>

enum class Segment {CONSTANT, ARGUMENT, LOCAL, STATIC, THIS, THAT, POINTER, TEMP, NONE};
enum class Arithmetic { ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT };
const Arithmetic ariths[] = { Arithmetic::ADD, Arithmetic::SUB, Arithmetic::NEG,
                                 Arithmetic::EQ, Arithmetic::GT, Arithmetic::LT,
                                 Arithmetic::AND, Arithmetic::OR, Arithmetic::NOT };


Arithmetic toArith(char op);
bool isArith(char symbol);

class VMWriter {
  std::ofstream outfile;
public:
  VMWriter(std::string outfilename);
  void writePush(Segment segment, int index);
  void writePop(Segment segment, int index);
  void writeArithmetic(Arithmetic command);
  void writeLabel(std::string label);
  void writeGoto(std::string label);
  void writeIf(std::string label);
  void writeCall(std::string name, int nArgs);
  void writeFunction(std::string name, int nArgs);
  void writeReturn();
  void close();
};