#include "expression_vm.h"
#include <stdexcept>

namespace temene {

void ExpressionVmEngine::loadProgram(std::vector<VmInstruction> program) {
  program_ = std::move(program);
}

double ExpressionVmEngine::run(const std::unordered_map<int, double>& registers) {
  std::vector<double> stack;
  std::unordered_map<int, double> regMap = registers;
  size_t ip = 0;

  while (ip < program_.size()) {
    const auto& inst = program_[ip++];
    switch (inst.opcode) {
      case VmOpcode::PUSH_DOUBLE:
        stack.push_back(inst.dVal);
        break;
      case VmOpcode::PUSH_INT:
        stack.push_back(static_cast<double>(inst.iVal));
        break;
      case VmOpcode::LOAD_REG:
        stack.push_back(regMap[inst.iVal]);
        break;
      case VmOpcode::STORE_REG:
        if (!stack.empty()) {
          regMap[inst.iVal] = stack.back();
          stack.pop_back();
        }
        break;
      case VmOpcode::ADD: {
        if (stack.size() < 2) break;
        double b = stack.back(); stack.pop_back();
        double a = stack.back(); stack.pop_back();
        stack.push_back(a + b);
        break;
      }
      case VmOpcode::SUB: {
        if (stack.size() < 2) break;
        double b = stack.back(); stack.pop_back();
        double a = stack.back(); stack.pop_back();
        stack.push_back(a - b);
        break;
      }
      case VmOpcode::MUL: {
        if (stack.size() < 2) break;
        double b = stack.back(); stack.pop_back();
        double a = stack.back(); stack.pop_back();
        stack.push_back(a * b);
        break;
      }
      case VmOpcode::DIV: {
        if (stack.size() < 2) break;
        double b = stack.back(); stack.pop_back();
        double a = stack.back(); stack.pop_back();
        stack.push_back(b != 0.0 ? a / b : 0.0);
        break;
      }
      case VmOpcode::JMP:
        ip = static_cast<size_t>(inst.iVal);
        break;
      case VmOpcode::JZ: {
        if (!stack.empty()) {
          double v = stack.back(); stack.pop_back();
          if (v == 0.0) ip = static_cast<size_t>(inst.iVal);
        }
        break;
      }
      case VmOpcode::HALT:
        return stack.empty() ? 0.0 : stack.back();
    }
  }

  return stack.empty() ? 0.0 : stack.back();
}

} // namespace temene
