#include <vector>
#include <cassert>

using Matrix = std::vector<std::vector<double>>;

Matrix mat_mul(const Matrix& A, const Matrix& B) {
    assert(!A.empty() && !B.empty());
    size_t n = A.size(); size_t m = B[0].size(); size_t k = B.size();
    Matrix C(n, std::vector<double>(m, 0.0));
    for (size_t i=0;i<n;++i) for (size_t j=0;j<m;++j) for (size_t t=0;t<k;++t) C[i][j] += A[i][t]*B[t][j];
    return C;
}

Matrix mat_identity(size_t n) {
    Matrix I(n, std::vector<double>(n,0.0));
    for (size_t i=0;i<n;++i) I[i][i] = 1.0;
    return I;
}
