#pragma once
#include <vector>

// compact, contiguous row major matrix container
// we keep it as a flat vector to control layout explicitly and avoid
// accidently padding/stride issues you can get with vector<vector<double>>

using Mat = std::vector<double>;

// row major accessor helpers
inline double &at(Mat &M, int n, int r, int c) { return M[(size_t)r * n + c]; };
inline double atc(const Mat &M, int n, int r, int c) { return M[(size_t)r * n + c]; };

// compute BT = transpose(B)
// we expose this in the ehader becuase its small and may be inlined by the compiler
// but its also fine to keep the declaration here and define it in the .cpp (as we do)
Mat transpose(const Mat &B, int n);

// two matmul kernels with identical arithmetic count (n^3 multiplies/adds)
// differing only in memory access patterns
//
// 1) mm_bad inner loop reads b by color (stride n in row major) =>poor locality
// 2) mm_good: inner loop reads BT by row (stride-1) after transposing B

// both return the elapseed time in ms for the computation
double mm_bad(const Mat &A, const Mat &B, Mat &C, int n); // column stride for B (bad)
double mm_good(const Mat &A, const Mat &BT, Mat &C, int n);