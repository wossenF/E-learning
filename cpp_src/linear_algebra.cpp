#include "linear_algebra.h"
#include <random>

namespace temene {

Matrix::Matrix(size_t rows, size_t cols, double initialVal)
  : rows_(rows), cols_(cols), data_(rows * cols, initialVal) {}

Matrix Matrix::identity(size_t n) {
  Matrix m(n, n, 0.0);
  for (size_t i = 0; i < n; ++i) m.at(i, i) = 1.0;
  return m;
}

Matrix Matrix::random(size_t rows, size_t cols) {
  Matrix m(rows, cols);
  std::mt19937 rng(42);
  std::uniform_real_distribution<double> dist(-1.0, 1.0);
  for (size_t i = 0; i < rows * cols; ++i) {
    m.data_[i] = dist(rng);
  }
  return m;
}

double& Matrix::at(size_t r, size_t c) {
  if (r >= rows_ || c >= cols_) throw std::out_of_range("Matrix index out of bounds.");
  return data_[r * cols_ + c];
}

double Matrix::at(size_t r, size_t c) const {
  if (r >= rows_ || c >= cols_) throw std::out_of_range("Matrix index out of bounds.");
  return data_[r * cols_ + c];
}

Matrix Matrix::transpose() const {
  Matrix res(cols_, rows_);
  for (size_t r = 0; r < rows_; ++r) {
    for (size_t c = 0; c < cols_; ++c) {
      res.at(c, r) = at(r, c);
    }
  }
  return res;
}

Matrix Matrix::add(const Matrix& other) const {
  if (rows_ != other.rows_ || cols_ != other.cols_) throw std::invalid_argument("Matrix dimensions mismatch.");
  Matrix res(rows_, cols_);
  for (size_t i = 0; i < data_.size(); ++i) {
    res.data_[i] = data_[i] + other.data_[i];
  }
  return res;
}

Matrix Matrix::multiply(const Matrix& other) const {
  if (cols_ != other.rows_) throw std::invalid_argument("Matrix inner dimensions mismatch.");
  Matrix res(rows_, other.cols_, 0.0);

  for (size_t i = 0; i < rows_; ++i) {
    for (size_t k = 0; k < cols_; ++k) {
      double r = at(i, k);
      for (size_t j = 0; j < other.cols_; ++j) {
        res.at(i, j) += r * other.at(k, j);
      }
    }
  }

  return res;
}

Matrix Matrix::scale(double factor) const {
  Matrix res(rows_, cols_);
  for (size_t i = 0; i < data_.size(); ++i) {
    res.data_[i] = data_[i] * factor;
  }
  return res;
}

std::pair<Matrix, Matrix> Matrix::luDecomposition() const {
  if (rows_ != cols_) throw std::invalid_argument("LU decomposition requires square matrix.");
  size_t n = rows_;
  Matrix L(n, n, 0.0);
  Matrix U(n, n, 0.0);

  for (size_t i = 0; i < n; ++i) {
    for (size_t k = i; k < n; ++k) {
      double sum = 0;
      for (size_t j = 0; j < i; ++j) sum += (L.at(i, j) * U.at(j, k));
      U.at(i, k) = at(i, k) - sum;
    }

    for (size_t k = i; k < n; ++k) {
      if (i == k) L.at(i, i) = 1.0;
      else {
        double sum = 0;
        for (size_t j = 0; j < i; ++j) sum += (L.at(k, j) * U.at(j, i));
        L.at(k, i) = (at(k, i) - sum) / (U.at(i, i) != 0.0 ? U.at(i, i) : 1e-9);
      }
    }
  }

  return {L, U};
}

std::pair<Matrix, Matrix> Matrix::qrDecomposition() const {
  size_t m = rows_;
  size_t n = cols_;
  Matrix Q(m, n, 0.0);
  Matrix R(n, n, 0.0);

  for (size_t j = 0; j < n; ++j) {
    std::vector<double> v(m);
    for (size_t i = 0; i < m; ++i) v[i] = at(i, j);

    for (size_t i = 0; i < j; ++i) {
      double dot = 0.0;
      for (size_t k = 0; k < m; ++k) dot += Q.at(k, i) * at(k, j);
      R.at(i, j) = dot;
      for (size_t k = 0; k < m; ++k) v[k] -= dot * Q.at(k, i);
    }

    double norm = 0.0;
    for (size_t k = 0; k < m; ++k) norm += v[k] * v[k];
    norm = std::sqrt(norm);
    R.at(j, j) = norm;

    for (size_t k = 0; k < m; ++k) {
      Q.at(k, j) = (norm != 0.0) ? (v[k] / norm) : 0.0;
    }
  }

  return {Q, R};
}

double Matrix::determinant() const {
  if (rows_ != cols_) throw std::invalid_argument("Determinant requires square matrix.");
  auto [L, U] = luDecomposition();
  double det = 1.0;
  for (size_t i = 0; i < rows_; ++i) {
    det *= U.at(i, i);
  }
  return det;
}

Matrix Matrix::inverse() const {
  if (rows_ != cols_) throw std::invalid_argument("Inverse requires square matrix.");
  size_t n = rows_;
  Matrix aug(n, 2 * n, 0.0);

  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) aug.at(i, j) = at(i, j);
    aug.at(i, n + i) = 1.0;
  }

  for (size_t i = 0; i < n; ++i) {
    double pivot = aug.at(i, i);
    if (pivot == 0.0) throw std::runtime_error("Matrix is singular and cannot be inverted.");

    for (size_t j = 0; j < 2 * n; ++j) aug.at(i, j) /= pivot;

    for (size_t k = 0; k < n; ++k) {
      if (k != i) {
        double factor = aug.at(k, i);
        for (size_t j = 0; j < 2 * n; ++j) {
          aug.at(k, j) -= factor * aug.at(i, j);
        }
      }
    }
  }

  Matrix inv(n, n);
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
      inv.at(i, j) = aug.at(i, n + j);
    }
  }

  return inv;
}

} // namespace temene
