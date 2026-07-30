// Small math utilities: gcd, lcm, sieve for primes, and vector dot product
#include <vector>
#include <numeric>

long long gcd_ll(long long a, long long b) {
    if (a<0) a=-a; if (b<0) b=-b; while (b) { long long t=a%b; a=b; b=t; } return a;
}

long long lcm_ll(long long a, long long b) { if (a==0||b==0) return 0; return (a / gcd_ll(a,b)) * b; }

std::vector<int> sieve_primes(int n) {
    if (n < 2) return {};
    std::vector<char> is(n+1, true); is[0]=is[1]=false;
    for (int p=2;p*p<=n;++p) if (is[p]) for (int q=p*p;q<=n;q+=p) is[q]=false;
    std::vector<int> out; for (int i=2;i<=n;++i) if (is[i]) out.push_back(i); return out;
}

long long dot_product(const std::vector<long long>& a, const std::vector<long long>& b) {
    long long s=0; size_t n = std::min(a.size(), b.size()); for (size_t i=0;i<n;++i) s += a[i]*b[i]; return s;
}
