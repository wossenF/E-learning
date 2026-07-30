#ifndef CODEGEN_ENGINE_H
#define CODEGEN_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>

namespace temene {

enum class Register { R0, R1, R2, R3, R4, R5, RAX, RBX, RCX, RDX };

struct AssemblyInstruction {
  std::string opcode;
  std::string op1;
  std::string op2;
  std::string comment;
};

class CodeGeneratorEngine {
public:
  void emitMov(Register dst, Register src);
  void emitMovImm(Register dst, int64_t imm);
  void emitAdd(Register dst, Register src);
  void emitSub(Register dst, Register src);
  void emitMul(Register dst, Register src);
  void emitDiv(Register dst, Register src);
  void emitJmp(const std::string& label);
  void emitLabel(const std::string& label);

  std::string generateAssemblyText() const;
  void optimizeAssembly();

private:
  std::vector<AssemblyInstruction> instructions_;
  int labelCounter_{0};

  std::string regToString(Register r) const;
};

} // namespace temene

#endif // CODEGEN_ENGINE_H
