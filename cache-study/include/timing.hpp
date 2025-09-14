#pragma once
#include <chrono>    // give access to clock
#include <algorithm> // gives access to min function

// tiny reusable function to time any callable (a lambda or function) reliably. It should
// warm up once (so cahces/JIT-like effects don't skew the first run)
// Run the code several times
// Return a stable number (min or median ms)

// a function template: can accept any callable type (lambda, function pointer, functor);
template <class F>
double time_ms(F &&f, int iters = 7)
{
  using clk = std::chrono::steady_clock;
  // use a monotonic clock so that it wont jump backward like systemt time

  // warm up once to prime caches/allocators
  // This primes the CPU caches and branch predictors, allocators etc
  // without this first run may be artifically slow
  f();
  double best = 1e300;
  // start with a huge worst case time in ms
  // we will replace it with the min observered run time
  for (int i = 0; i < iters; ++i) // loop iters time (default = 7)
  {
    auto t0 = clk::now();                                                   // start timestamp
    f();                                                                    // run code we are benchmarking
    auto t1 = clk::now();                                                   // end timestampt
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count(); // compute duration in ms as a double
    best = std::min(best, ms);                                              // keep track of best
  }
  return best;
}