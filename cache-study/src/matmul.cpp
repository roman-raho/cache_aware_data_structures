#include "matmul.hpp"
#include <algorithm>
#include <random>
#include <chrono>

// volatile sink prevents the compiler from proving the result is unused and deleteing the loops  (DCE). Touching C[0] and assingin to sink d pins the work
static volatile double sink_d = 0.0;

// computer BT(j,i). O(n^2) work and memory traffic
// cost is small relative to O(n^3) matmul for moderate/large n, and it unlcoks
// contiguous access (stride-1) in the inner multiply loop
Mat transpose(const Mat &B, int n)
{
  Mat BT((size_t)n * n);
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      at(BT, n, j, i) = atc(B, n, i, j);
    }
  }
  return BT;
}

// naive row*column numptiply: C = A * B
// layour row major for all matrices
// loop order i-j-k means the inner loop walks down a column of B: B[k][j].
// In row major, consecutive elements fo a columsn are n apart in memory (stride-n)
// so we get poor spatial locality and weak prefetching for B.
//
// returns wall time in ms for the kernel
double mm_bad(const Mat &A, const Mat &B, Mat &C, int n)
{
  // always overwrite C to a know sate so timing is comparable run to run
  std::fill(C.begin(), C.end(), 0.0);

  auto t0 = std::chrono::steady_clock::now();

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
    {
      double s = 0.0;
      // access pattern
      // A[i,k] -> stride 1 (good)
      // B[k,j] -> stride-n (bad) <-- main source of stalls
      for (int k = 0; k < n; ++k)
        s += atc(A, n, i, k) * atc(B, n, k, j);
      at(C, n, i, j) = s;
    }

  auto t1 = std::chrono::steady_clock::now();
  double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

  // touch result to keep compiler honest
  sink_d = C[0];
  return ms;
}

// Transpose aided multiple C = A * B where we pass BT = transpose(B)
// we keep the same i-j-k loop order, but now the inner loop uses BT[j,k],
// which is row-wise in row-major => stride -1 for both A and BT
//
// returns wall time in ms for the kernel
double mm_good(const Mat &A, const Mat &BT, Mat &C, int n)
{
  std::fill(C.begin(), C.end(), 0.0);

  auto t0 = std::chrono::steady_clock::now();

  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      double s = 0.0;
      // access pattern
      // A the same
      // BT stride-1 (improved)
      for (int k = 0; k < n; ++k)
      {
        s += atc(A, n, i, k) * atc(BT, n, j, k);
      }
      at(C, n, i, j) = s;
    }
  }

  auto t1 = std::chrono::steady_clock::now();
  double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

  sink_d = C[0];
  return ms;
}
