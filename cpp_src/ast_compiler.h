#ifndef AST_COMPILER_H
#define AST_COMPILER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace temene {

enum class TokenType {
  TOKEN_EOF,
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_IDENTIFIER,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_MODULO,
  TOKEN_EQ,
  TOKEN_NEQ,
  TOKEN_LT,
  TOKEN_LTE,
  TOKEN_GT,
  TOKEN_GTE,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_NOT,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_COMMA
};

struct Token {
  TokenType type;
  std::string lexeme;
  double doubleValue{0.0};
  int line{1};
};

using Value = std::variant<std::monostate, double, std::string, bool>;

class Lexer {
public:
  explicit Lexer(std::string source);
  std::vector<Token> tokenize();

private:
  std::string source_;
  size_t start_{0};
  size_t current_{0};
  int line_{1};

  bool isAtEnd() const;
  char advance();
  char peek() const;
  char peekNext() const;
  bool match(char expected);
  void addToken(std::vector<Token>& tokens, TokenType type);
  void addToken(std::vector<Token>& tokens, TokenType type, double val);
  void stringToken(std::vector<Token>& tokens);
  void numberToken(std::vector<Token>& tokens);
  void identifierToken(std::vector<Token>& tokens);
};

enum class AstNodeType {
  LITERAL,
  IDENTIFIER,
  BINARY_OP,
  UNARY_OP,
  FUNCTION_CALL
};

class AstNode {
public:
  virtual ~AstNode() = default;
  virtual AstNodeType getType() const = 0;
  virtual Value evaluate(const std::unordered_map<std::string, Value>& context) const = 0;
  virtual std::string toString() const = 0;
  virtual std::shared_ptr<AstNode> optimize() = 0;
};

class LiteralNode : public AstNode {
public:
  explicit LiteralNode(Value val) : value_(std::move(val)) {}
  AstNodeType getType() const override { return AstNodeType::LITERAL; }
  Value evaluate(const std::unordered_map<std::string, Value>& context) const override;
  std::string toString() const override;
  std::shared_ptr<AstNode> optimize() override;
  Value getValue() const { return value_; }

private:
  Value value_;
};

class IdentifierNode : public AstNode {
public:
  explicit IdentifierNode(std::string name) : name_(std::move(name)) {}
  AstNodeType getType() const override { return AstNodeType::IDENTIFIER; }
  Value evaluate(const std::unordered_map<std::string, Value>& context) const override;
  std::string toString() const override;
  std::shared_ptr<AstNode> optimize() override;
  std::string getName() const { return name_; }

private:
  std::string name_;
};

class BinaryOpNode : public AstNode {
public:
  BinaryOpNode(TokenType op, std::shared_ptr<AstNode> left, std::shared_ptr<AstNode> right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) {}
  AstNodeType getType() const override { return AstNodeType::BINARY_OP; }
  Value evaluate(const std::unordered_map<std::string, Value>& context) const override;
  std::string toString() const override;
  std::shared_ptr<AstNode> optimize() override;

private:
  TokenType op_;
  std::shared_ptr<AstNode> left_;
  std::shared_ptr<AstNode> right_;
};

class UnaryOpNode : public AstNode {
public:
  UnaryOpNode(TokenType op, std::shared_ptr<AstNode> operand)
    : op_(op), operand_(std::move(operand)) {}
  AstNodeType getType() const override { return AstNodeType::UNARY_OP; }
  Value evaluate(const std::unordered_map<std::string, Value>& context) const override;
  std::string toString() const override;
  std::shared_ptr<AstNode> optimize() override;

private:
  TokenType op_;
  std::shared_ptr<AstNode> operand_;
};

class FunctionCallNode : public AstNode {
public:
  FunctionCallNode(std::string funcName, std::vector<std::shared_ptr<AstNode>> args)
    : funcName_(std::move(funcName)), args_(std::move(args)) {}
  AstNodeType getType() const override { return AstNodeType::FUNCTION_CALL; }
  Value evaluate(const std::unordered_map<std::string, Value>& context) const override;
  std::string toString() const override;
  std::shared_ptr<AstNode> optimize() override;

private:
  std::string funcName_;
  std::vector<std::shared_ptr<AstNode>> args_;
};

class AstParser {
public:
  explicit AstParser(std::vector<Token> tokens);
  std::shared_ptr<AstNode> parse();

private:
  std::vector<Token> tokens_;
  size_t current_{0};

  bool isAtEnd() const;
  Token peek() const;
  Token previous() const;
  Token advance();
  bool check(TokenType type) const;
  bool match(const std::vector<TokenType>& types);
  Token consume(TokenType type, const std::string& message);

  std::shared_ptr<AstNode> expression();
  std::shared_ptr<AstNode> logicOr();
  std::shared_ptr<AstNode> logicAnd();
  std::shared_ptr<AstNode> equality();
  std::shared_ptr<AstNode> comparison();
  std::shared_ptr<AstNode> term();
  std::shared_ptr<AstNode> factor();
  std::shared_ptr<AstNode> unary();
  std::shared_ptr<AstNode> primary();
};

enum class OpCode {
  OP_LOAD_CONST,
  OP_LOAD_VAR,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_EQ,
  OP_NEQ,
  OP_LT,
  OP_LTE,
  OP_GT,
  OP_GTE,
  OP_AND,
  OP_OR,
  OP_NOT,
  OP_CALL,
  OP_HALT
};

struct Instruction {
  OpCode op;
  Value arg;
  std::string strArg;
  int intArg{0};
};

class BytecodeCompiler {
public:
  std::vector<Instruction> compile(const std::shared_ptr<AstNode>& root);
private:
  void emit(const std::shared_ptr<AstNode>& node, std::vector<Instruction>& code);
};

class BytecodeVm {
public:
  Value execute(const std::vector<Instruction>& code, const std::unordered_map<std::string, Value>& context);
};

} // namespace temene

#endif // AST_COMPILER_H
