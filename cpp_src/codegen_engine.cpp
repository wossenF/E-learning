#include "codegen_engine.h"

namespace temene {

std::string CodeGeneratorEngine::regToString(Register r) const {
  switch (r) {
    case Register::R0: return "r0";
    case Register::R1: return "r1";
    case Register::R2: return "r2";
    case Register::R3: return "r3";
    case Register::R4: return "r4";
    case Register::R5: return "r5";
    case Register::RAX: return "rax";
    case Register::RBX: return "rbx";
    case Register::RCX: return "rcx";
    case Register::RDX: return "rdx";
  }
  return "r0";
}

void CodeGeneratorEngine::emitMov(Register dst, Register src) {
  instructions_.push_back({"mov", regToString(dst), regToString(src), "Register copy"});
}

void CodeGeneratorEngine::emitMovImm(Register dst, int64_t imm) {
  instructions_.push_back({"mov", regToString(dst), "$" + std::to_string(imm), "Immediate load"});
}

void CodeGeneratorEngine::emitAdd(Register dst, Register src) {
  instructions_.push_back({"add", regToString(dst), regToString(src), "Addition"});
}

void CodeGeneratorEngine::emitSub(Register dst, Register src) {
  instructions_.push_back({"sub", regToString(dst), regToString(src), "Subtraction"});
}

void CodeGeneratorEngine::emitMul(Register dst, Register src) {
  instructions_.push_back({"imul", regToString(dst), regToString(src), "Multiplication"});
}

void CodeGeneratorEngine::emitDiv(Register dst, Register src) {
  instructions_.push_back({"idiv", regToString(dst), regToString(src), "Division"});
}

void CodeGeneratorEngine::emitJmp(const std::string& label) {
  instructions_.push_back({"jmp", label, "", "Unconditional jump"});
}

void CodeGeneratorEngine::emitLabel(const std::string& label) {
  instructions_.push_back({label + ":", "", "", "Label definition"});
}

std::string CodeGeneratorEngine::generateAssemblyText() const {
  std::ostringstream ss;
  ss << ".global main\n.text\n";
  for (const auto& inst : instructions_) {
    if (inst.opcode.back() == ':') {
      ss << inst.opcode << "\n";
    } else {
      ss << "    " << inst.opcode << " " << inst.op1;
      if (!inst.op2.empty()) ss << ", " << inst.op2;
      if (!inst.comment.empty()) ss << " ; " << inst.comment;
      ss << "\n";
    }
  }
  return ss.str();
}

void CodeGeneratorEngine::optimizeAssembly() {
  auto it = instructions_.begin();
  while (it != instructions_.end()) {
    if (it->opcode == "mov" && it->op1 == it->op2) {
      it = instructions_.erase(it); // Redundant self-assignment
    } else {
      ++it;
    }
  }
}

} // namespace temene
