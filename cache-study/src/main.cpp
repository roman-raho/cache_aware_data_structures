#include <bits/stdc++.h> // quick include for std::vector, iostream, iota, etc.
#include <timing.hpp>
#include <traversal.hpp> // vector bs linked list
#include <matmul.hpp>    // matrix multiply experiment

using namespace std;

// traversal benchmark: vector vs linked list

static void bench_traversal()
{
  // problem sizes (#elements)
  vector<size_t> Ns = {10'000, 100'000, 1'000'000, 5'000'000, 10'000'000};

  cout << "TRAVERSAL\n";
  cout << "n,vector_ms,linked_ms,linked_over_vector\n";

  for (auto n : Ns)
  {
    // file vector with 0
    Vec v(n);
    iota(v.begin(), v.end(), 0);

    // build linked list with same #elements
    // pass true to randomise node order (worse for cache locality)
    Node *head = make_linked(n, false);

    // warm up (ensures caches are touched, JIT not an issue in C++ but)
    // still good practice to avoid first call overhead
    traverse_vector(v);
    traverse_linked(head);

    // time each structure
    double tv = time_ms([&]
                        { traverse_vector(v); }, 7);
    double tl = time_ms([&]
                        { traverse_linked(head); }, 7);

    // correctness check: both should sum to same value
    if (traverse_vector(v) != traverse_linked(head))
      cerr << "WARN: sums differe at n=" << n << "\n";

    // CSV output: size, vector time, linked_time, ratio
    cout << n << "," << tv << "," << tl << "," << (tl / tv) << "\n";

    // free memory from linked list
    free_list(head);
  }
}

// mat mul benchmark: naive (bad) vs transpose aided (good)
static void bench_matmul()
{
  // square matrix sizes to test
  vector<int> Ns = {128, 192, 256, 384, 512};

  // RNG for matrix elements
  std::mt19937 rng(123);
  std::uniform_real_distribution<double> U(0.0, 1.0);
  cout << "MATMUL\n";
  cout << "n,bad_ms,good_ms,bad_over_good,max_abs_diff_check\n";

  for (int n : Ns)
  {
    // allocate A, B, and C (flattendeded n*n matrices)
    Mat A((size_t)n * n), B((size_t)n * n), C((size_t)n * n);

    // fill A and B with random [0,1)
    for (auto &x : A)
      x = U(rng);
    for (auto &x : B)
      x = U(rng);

    // precompute the tranpose of B for the good kernel
    Mat BT = transpose(B, n);

    // warm up
    mm_bad(A, B, C, n);
    mm_good(A, BT, C, n);

    auto tb = time_ms([&]
                      { return mm_bad(A, B, C, n); }, 3);
    auto tg = time_ms([&]
                      { return mm_good(A, BT, C, n); }, 3);

    // correctness check
    double maxdiff = 0.0;
    if (n == 128)
    {
      Mat C1((size_t)n * n), C2((size_t)n * n);
      mm_bad(A, B, C1, n);
      mm_good(A, BT, C2, n);
      for (size_t i = 0; i < C1.size(); ++i)
      {
        maxdiff = max(maxdiff, fabs(C1[i] - C2[i]));
      }
    }
    cout << n << "," << tb << "," << tg << "," << (tb / tg) << "," << maxdiff << "\n";
  }
}

int main()
{
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  bench_traversal(); // run vector vs linked list
  bench_matmul();    // run matrix multiply
  return 0;
}