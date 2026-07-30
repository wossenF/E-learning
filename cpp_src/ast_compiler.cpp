#include "ast_compiler.h"
#include <cmath>
#include <algorithm>
#include <cctype>

namespace temene {

// Helper string convert
static std::string valueToString(const Value& val) {
  if (std::holds_alternative<double>(val)) {
    return std::to_string(std::get<double>(val));
  } else if (std::holds_alternative<std::string>(val)) {
    return std::get<std::string>(val);
  } else if (std::holds_alternative<bool>(val)) {
    return std::get<bool>(val) ? "true" : "false";
  }
  return "null";
}

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

bool Lexer::isAtEnd() const {
  return current_ >= source_.size();
}

char Lexer::advance() {
  return source_[current_++];
}

char Lexer::peek() const {
  if (isAtEnd()) return '\0';
  return source_[current_];
}

char Lexer::peekNext() const {
  if (current_ + 1 >= source_.size()) return '\0';
  return source_[current_ + 1];
}

bool Lexer::match(char expected) {
  if (isAtEnd() || source_[current_] != expected) return false;
  current_++;
  return true;
}

void Lexer::addToken(std::vector<Token>& tokens, TokenType type) {
  addToken(tokens, type, 0.0);
}

void Lexer::addToken(std::vector<Token>& tokens, TokenType type, double val) {
  std::string text = source_.substr(start_, current_ - start_);
  tokens.push_back({type, text, val, line_});
}

void Lexer::stringToken(std::vector<Token>& tokens) {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') line_++;
    advance();
  }

  if (isAtEnd()) {
    throw std::runtime_error("Unterminated string.");
  }

  advance(); // Closing quote
  std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
  tokens.push_back({TokenType::TOKEN_STRING, value, 0.0, line_});
}

void Lexer::numberToken(std::vector<Token>& tokens) {
  while (std::isdigit(peek())) advance();

  if (peek() == '.' && std::isdigit(peekNext())) {
    advance(); // Consume '.'
    while (std::isdigit(peek())) advance();
  }

  double val = std::stod(source_.substr(start_, current_ - start_));
  addToken(tokens, TokenType::TOKEN_NUMBER, val);
}

void Lexer::identifierToken(std::vector<Token>& tokens) {
  while (std::isalnum(peek()) || peek() == '_') advance();

  std::string text = source_.substr(start_, current_ - start_);
  TokenType type = TokenType::TOKEN_IDENTIFIER;
  if (text == "AND" || text == "and") type = TokenType::TOKEN_AND;
  else if (text == "OR" || text == "or") type = TokenType::TOKEN_OR;
  else if (text == "NOT" || text == "not") type = TokenType::TOKEN_NOT;

  addToken(tokens, type);
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    start_ = current_;
    char c = advance();

    switch (c) {
      case '(': addToken(tokens, TokenType::TOKEN_LPAREN); break;
      case ')': addToken(tokens, TokenType::TOKEN_RPAREN); break;
      case ',': addToken(tokens, TokenType::TOKEN_COMMA); break;
      case '+': addToken(tokens, TokenType::TOKEN_PLUS); break;
      case '-': addToken(tokens, TokenType::TOKEN_MINUS); break;
      case '*': addToken(tokens, TokenType::TOKEN_STAR); break;
      case '/': addToken(tokens, TokenType::TOKEN_SLASH); break;
      case '%': addToken(tokens, TokenType::TOKEN_MODULO); break;
      case '!':
        addToken(tokens, match('=') ? TokenType::TOKEN_NEQ : TokenType::TOKEN_NOT);
        break;
      case '=':
        if (match('=')) addToken(tokens, TokenType::TOKEN_EQ);
        else addToken(tokens, TokenType::TOKEN_EQ);
        break;
      case '<':
        addToken(tokens, match('=') ? TokenType::TOKEN_LTE : TokenType::TOKEN_LT);
        break;
      case '>':
        addToken(tokens, match('=') ? TokenType::TOKEN_GTE : TokenType::TOKEN_GT);
        break;
      case '&':
        if (match('&')) addToken(tokens, TokenType::TOKEN_AND);
        break;
      case '|':
        if (match('|')) addToken(tokens, TokenType::TOKEN_OR);
        break;
      case ' ':
      case '\r':
      case '\t':
        break;
      case '\n':
        line_++;
        break;
      case '"':
        stringToken(tokens);
        break;
      default:
        if (std::isdigit(c)) {
          numberToken(tokens);
        } else if (std::isalpha(c) || c == '_') {
          identifierToken(tokens);
        } else {
          throw std::runtime_error("Unexpected character: " + std::string(1, c));
        }
        break;
    }
  }

  tokens.push_back({TokenType::TOKEN_EOF, "", 0.0, line_});
  return tokens;
}

// AST Nodes evaluation
Value LiteralNode::evaluate(const std::unordered_map<std::string, Value>&) const {
  return value_;
}

std::string LiteralNode::toString() const {
  return valueToString(value_);
}

std::shared_ptr<AstNode> LiteralNode::optimize() {
  return std::make_shared<LiteralNode>(value_);
}

Value IdentifierNode::evaluate(const std::unordered_map<std::string, Value>& context) const {
  auto it = context.find(name_);
  if (it != context.end()) return it->second;
  return std::monostate{};
}

std::string IdentifierNode::toString() const {
  return name_;
}

std::shared_ptr<AstNode> IdentifierNode::optimize() {
  return std::make_shared<IdentifierNode>(name_);
}

Value BinaryOpNode::evaluate(const std::unordered_map<std::string, Value>& context) const {
  Value leftVal = left_->evaluate(context);
  Value rightVal = right_->evaluate(context);

  if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
    double l = std::get<double>(leftVal);
    double r = std::get<double>(rightVal);
    switch (op_) {
      case TokenType::TOKEN_PLUS: return l + r;
      case TokenType::TOKEN_MINUS: return l - r;
      case TokenType::TOKEN_STAR: return l * r;
      case TokenType::TOKEN_SLASH: return r != 0.0 ? l / r : 0.0;
      case TokenType::TOKEN_MODULO: return std::fmod(l, r);
      case TokenType::TOKEN_EQ: return l == r;
      case TokenType::TOKEN_NEQ: return l != r;
      case TokenType::TOKEN_LT: return l < r;
      case TokenType::TOKEN_LTE: return l <= r;
      case TokenType::TOKEN_GT: return l > r;
      case TokenType::TOKEN_GTE: return l >= r;
      default: break;
    }
  } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
    std::string l = std::get<std::string>(leftVal);
    std::string r = std::get<std::string>(rightVal);
    switch (op_) {
      case TokenType::TOKEN_PLUS: return l + r;
      case TokenType::TOKEN_EQ: return l == r;
      case TokenType::TOKEN_NEQ: return l != r;
      default: break;
    }
  } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
    bool l = std::get<bool>(leftVal);
    bool r = std::get<bool>(rightVal);
    switch (op_) {
      case TokenType::TOKEN_AND: return l && r;
      case TokenType::TOKEN_OR: return l || r;
      case TokenType::TOKEN_EQ: return l == r;
      case TokenType::TOKEN_NEQ: return l != r;
      default: break;
    }
  }

  return std::monostate{};
}

std::string BinaryOpNode::toString() const {
  return "(" + left_->toString() + " OP " + right_->toString() + ")";
}

std::shared_ptr<AstNode> BinaryOpNode::optimize() {
  auto optLeft = left_->optimize();
  auto optRight = right_->optimize();

  if (optLeft->getType() == AstNodeType::LITERAL && optRight->getType() == AstNodeType::LITERAL) {
    std::unordered_map<std::string, Value> dummyCtx;
    Value res = BinaryOpNode(op_, optLeft, optRight).evaluate(dummyCtx);
    return std::make_shared<LiteralNode>(res);
  }

  return std::make_shared<BinaryOpNode>(op_, optLeft, optRight);
}

Value UnaryOpNode::evaluate(const std::unordered_map<std::string, Value>& context) const {
  Value val = operand_->evaluate(context);

  if (op_ == TokenType::TOKEN_MINUS && std::holds_alternative<double>(val)) {
    return -std::get<double>(val);
  } else if (op_ == TokenType::TOKEN_NOT) {
    if (std::holds_alternative<bool>(val)) return !std::get<bool>(val);
    if (std::holds_alternative<double>(val)) return std::get<double>(val) == 0.0;
  }

  return std::monostate{};
}

std::string UnaryOpNode::toString() const {
  return "(UNARY " + operand_->toString() + ")";
}

std::shared_ptr<AstNode> UnaryOpNode::optimize() {
  auto optOperand = operand_->optimize();
  if (optOperand->getType() == AstNodeType::LITERAL) {
    std::unordered_map<std::string, Value> dummyCtx;
    Value res = UnaryOpNode(op_, optOperand).evaluate(dummyCtx);
    return std::make_shared<LiteralNode>(res);
  }
  return std::make_shared<UnaryOpNode>(op_, optOperand);
}

Value FunctionCallNode::evaluate(const std::unordered_map<std::string, Value>& context) const {
  std::vector<Value> argVals;
  for (const auto& arg : args_) {
    argVals.push_back(arg->evaluate(context));
  }

  if (funcName_ == "ABS" || funcName_ == "abs") {
    if (!argVals.empty() && std::holds_alternative<double>(argVals[0])) {
      return std::abs(std::get<double>(argVals[0]));
    }
  } else if (funcName_ == "SQRT" || funcName_ == "sqrt") {
    if (!argVals.empty() && std::holds_alternative<double>(argVals[0])) {
      return std::sqrt(std::get<double>(argVals[0]));
    }
  } else if (funcName_ == "CONCAT" || funcName_ == "concat") {
    std::string res;
    for (const auto& v : argVals) res += valueToString(v);
    return res;
  }

  return std::monostate{};
}

std::string FunctionCallNode::toString() const {
  return funcName_ + "(...)";
}

std::shared_ptr<AstNode> FunctionCallNode::optimize() {
  std::vector<std::shared_ptr<AstNode>> optArgs;
  for (const auto& a : args_) optArgs.push_back(a->optimize());
  return std::make_shared<FunctionCallNode>(funcName_, optArgs);
}

// Parser implementation
AstParser::AstParser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

bool AstParser::isAtEnd() const { return peek().type == TokenType::TOKEN_EOF; }
Token AstParser::peek() const { return tokens_[current_]; }
Token AstParser::previous() const { return tokens_[current_ - 1]; }
Token AstParser::advance() { if (!isAtEnd()) current_++; return previous(); }
bool AstParser::check(TokenType type) const { if (isAtEnd()) return false; return peek().type == type; }

bool AstParser::match(const std::vector<TokenType>& types) {
  for (TokenType t : types) {
    if (check(t)) {
      advance();
      return true;
    }
  }
  return false;
}

Token AstParser::consume(TokenType type, const std::string& message) {
  if (check(type)) return advance();
  throw std::runtime_error(message);
}

std::shared_ptr<AstNode> AstParser::parse() {
  return expression();
}

std::shared_ptr<AstNode> AstParser::expression() {
  return logicOr();
}

std::shared_ptr<AstNode> AstParser::logicOr() {
  auto expr = logicAnd();
  while (match({TokenType::TOKEN_OR})) {
    TokenType op = previous().type;
    auto right = logicAnd();
    expr = std::make_shared<BinaryOpNode>(op, expr, right);
  }
  return expr;
}

std::shared_ptr<AstNode> AstParser::logicAnd() {
  auto expr = equality();
  while (match({TokenType::TOKEN_AND})) {
    TokenType op = previous().type;
    auto right = equality();
    expr = std::make_shared<BinaryOpNode>(op, expr, right);
  }
  return expr;
}

std::shared_ptr<AstNode> AstParser::equality() {
  auto expr = comparison();
  while (match({TokenType::TOKEN_EQ, TokenType::TOKEN_NEQ})) {
    TokenType op = previous().type;
    auto right = comparison();
    expr = std::make_shared<BinaryOpNode>(op, expr, right);
  }
  return expr;
}

std::shared_ptr<AstNode> AstParser::comparison() {
  auto expr = term();
  while (match({TokenType::TOKEN_GT, TokenType::TOKEN_GTE, TokenType::TOKEN_LT, TokenType::TOKEN_LTE})) {
    TokenType op = previous().type;
    auto right = term();
    expr = std::make_shared<BinaryOpNode>(op, expr, right);
  }
  return expr;
}

std::shared_ptr<AstNode> AstParser::term() {
  auto expr = factor();
  while (match({TokenType::TOKEN_PLUS, TokenType::TOKEN_MINUS})) {
    TokenType op = previous().type;
    auto right = factor();
    expr = std::make_shared<BinaryOpNode>(op, expr, right);
  }
  return expr;
}

std::shared_ptr<AstNode> AstParser::factor() {
  auto expr = unary();
  while (match({TokenType::TOKEN_STAR, TokenType::TOKEN_SLASH, TokenType::TOKEN_MODULO})) {
    TokenType op = previous().type;
    auto right = unary();
    expr = std::make_shared<BinaryOpNode>(op, expr, right);
  }
  return expr;
}

std::shared_ptr<AstNode> AstParser::unary() {
  if (match({TokenType::TOKEN_NOT, TokenType::TOKEN_MINUS})) {
    TokenType op = previous().type;
    auto right = unary();
    return std::make_shared<UnaryOpNode>(op, right);
  }
  return primary();
}

std::shared_ptr<AstNode> AstParser::primary() {
  if (match({TokenType::TOKEN_NUMBER})) {
    return std::make_shared<LiteralNode>(previous().doubleValue);
  }
  if (match({TokenType::TOKEN_STRING})) {
    return std::make_shared<LiteralNode>(previous().lexeme);
  }
  if (match({TokenType::TOKEN_IDENTIFIER})) {
    std::string name = previous().lexeme;
    if (match({TokenType::TOKEN_LPAREN})) {
      std::vector<std::shared_ptr<AstNode>> args;
      if (!check(TokenType::TOKEN_RPAREN)) {
        do {
          args.push_back(expression());
        } while (match({TokenType::TOKEN_COMMA}));
      }
      consume(TokenType::TOKEN_RPAREN, "Expected ')' after function arguments.");
      return std::make_shared<FunctionCallNode>(name, args);
    }
    return std::make_shared<IdentifierNode>(name);
  }
  if (match({TokenType::TOKEN_LPAREN})) {
    auto expr = expression();
    consume(TokenType::TOKEN_RPAREN, "Expected ')' after expression.");
    return expr;
  }

  throw std::runtime_error("Expected expression.");
}

// Bytecode Compiler
std::vector<Instruction> BytecodeCompiler::compile(const std::shared_ptr<AstNode>& root) {
  std::vector<Instruction> code;
  emit(root, code);
  code.push_back({OpCode::OP_HALT, std::monostate{}, "", 0});
  return code;
}

void BytecodeCompiler::emit(const std::shared_ptr<AstNode>& node, std::vector<Instruction>& code) {
  if (!node) return;

  switch (node->getType()) {
    case AstNodeType::LITERAL: {
      auto lit = std::dynamic_pointer_cast<LiteralNode>(node);
      code.push_back({OpCode::OP_LOAD_CONST, lit->getValue(), "", 0});
      break;
    }
    case AstNodeType::IDENTIFIER: {
      auto id = std::dynamic_pointer_cast<IdentifierNode>(node);
      code.push_back({OpCode::OP_LOAD_VAR, std::monostate{}, id->getName(), 0});
      break;
    }
    case AstNodeType::BINARY_OP: {
      // Traverse binary children
      break;
    }
    default:
      break;
  }
}

Value BytecodeVm::execute(const std::vector<Instruction>& code, const std::unordered_map<std::string, Value>& context) {
  std::vector<Value> stack;

  for (const auto& instr : code) {
    if (instr.op == OpCode::OP_HALT) break;
    if (instr.op == OpCode::OP_LOAD_CONST) {
      stack.push_back(instr.arg);
    } else if (instr.op == OpCode::OP_LOAD_VAR) {
      auto it = context.find(instr.strArg);
      if (it != context.end()) stack.push_back(it->second);
      else stack.push_back(std::monostate{});
    }
  }

  return stack.empty() ? Value(std::monostate{}) : stack.back();
}

} // namespace temene
