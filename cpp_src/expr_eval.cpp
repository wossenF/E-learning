#include <string>
#include <cctype>
#include <stdexcept>
#include <sstream>

// Small arithmetic expression evaluator supporting + - * / and parentheses.
// Shunting-yard style tokenizer + recursive descent for evaluation.

class ExprParser {
public:
    explicit ExprParser(const std::string& s) : s_(s), p_(0) {}
    double parse() {
        double v = parse_expr();
        skip_ws();
        if (p_ != s_.size()) throw std::runtime_error("trailing characters");
        return v;
    }
private:
    std::string s_;
    size_t p_;
    void skip_ws() { while (p_ < s_.size() && std::isspace((unsigned char)s_[p_])) ++p_; }
    double parse_number() {
        skip_ws(); size_t start = p_;
        if (p_ < s_.size() && (s_[p_] == '+' || s_[p_] == '-')) ++p_;
        while (p_ < s_.size() && (std::isdigit((unsigned char)s_[p_]) || s_[p_]=='.')) ++p_;
        if (start == p_) throw std::runtime_error("expected number");
        return std::stod(s_.substr(start, p_-start));
    }
    double parse_factor() {
        skip_ws();
        if (p_ < s_.size() && s_[p_] == '(') { ++p_; double v = parse_expr(); skip_ws(); if (p_>=s_.size()||s_[p_] != ')') throw std::runtime_error("missing )"); ++p_; return v; }
        return parse_number();
    }
    double parse_term() {
        double v = parse_factor();
        while (true) {
            skip_ws(); if (p_>=s_.size()) break;
            char op = s_[p_];
            if (op == '*' || op == '/') { ++p_; double r = parse_factor(); if (op=='*') v *= r; else v /= r; }
            else break;
        }
        return v;
    }
    double parse_expr() {
        double v = parse_term();
        while (true) {
            skip_ws(); if (p_>=s_.size()) break;
            char op = s_[p_];
            if (op == '+' || op == '-') { ++p_; double r = parse_term(); if (op=='+') v += r; else v -= r; }
            else break;
        }
        return v;
    }
};

double eval_expression(const std::string& s) {
    ExprParser p(s); return p.parse();
}

// quick CLI runner when built into example binaries
#ifdef EXPR_EVAL_MAIN
#include <iostream>
int main_expr(int argc, char** argv) {
    std::string in = argc>1?argv[1]:"1+2*(3+4)/2";
    try { std::cout << eval_expression(in) << "\n"; return 0; } catch (std::exception& e) { std::cerr<<e.what(); return 2; }
}
#endif
