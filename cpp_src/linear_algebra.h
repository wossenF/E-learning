#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

#include <vector>
#include <iostream>
#include <cmath>
#include <stdexcept>

namespace temene {

class Matrix {
public:
  Matrix(size_t rows, size_t cols, double initialVal = 0.0);

  static Matrix identity(size_t n);
  static Matrix random(size_t rows, size_t cols);

  size_t rows() const { return rows_; }
  size_t cols() const { return cols_; }

  double& at(size_t r, size_t c);
  double at(size_t r, size_t c) const;

  Matrix transpose() const;
  Matrix add(const Matrix& other) const;
  Matrix multiply(const Matrix& other) const;
  Matrix scale(double factor) const;

  double determinant() const;
  Matrix inverse() const;

  std::pair<Matrix, Matrix> luDecomposition() const; // L and U
  std::pair<Matrix, Matrix> qrDecomposition() const; // Q and R

private:
  size_t rows_;
  size_t cols_;
  std::vector<double> data_;
};

} // namespace temene

#endif // LINEAR_ALGEBRA_H
