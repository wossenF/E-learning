#ifndef EXPRESSION_VM_H
#define EXPRESSION_VM_H

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <iostream>

namespace temene {

enum class VmOpcode : uint8_t {
  PUSH_DOUBLE,
  PUSH_INT,
  LOAD_REG,
  STORE_REG,
  ADD,
  SUB,
  MUL,
  DIV,
  JMP,
  JZ,
  HALT
};

struct VmInstruction {
  VmOpcode opcode;
  double dVal{0.0};
  int iVal{0};
};

class ExpressionVmEngine {
public:
  void loadProgram(std::vector<VmInstruction> program);
  double run(const std::unordered_map<int, double>& registers);

private:
  std::vector<VmInstruction> program_;
};

} // namespace temene

#endif // EXPRESSION_VM_H
