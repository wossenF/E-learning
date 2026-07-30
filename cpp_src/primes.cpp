#include <vector>
#include <cstdint>

// Prime sieve utilities and a function to factor small integers. Useful to
// exercise numeric and memory-intensive code paths.

std::vector<int> sieve_primes(int n) {
    if (n < 2) return {};
    std::vector<char> is_prime(n+1, 1);
    is_prime[0] = is_prime[1] = 0;
    for (int p=2;p*p<=n;++p) if (is_prime[p]) for (int q=p*p;q<=n;q+=p) is_prime[q]=0;
    std::vector<int> primes; for (int i=2;i<=n;++i) if (is_prime[i]) primes.push_back(i);
    return primes;
}

std::vector<std::pair<int,int>> factor_range(int n) {
    auto primes = sieve_primes(n);
    std::vector<std::pair<int,int>> out;
    for (int p : primes) {
        int cnt = 0; int x = n;
        while (x % p == 0) { x /= p; ++cnt; }
        if (cnt>0) out.emplace_back(p, cnt);
    }
    return out;
}
