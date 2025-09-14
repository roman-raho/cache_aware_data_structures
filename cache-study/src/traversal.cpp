#include "traversal.hpp"

#include <numeric>   // std::iota
#include <random>    // std::mt19937, std::shuffle
#include <algorithm> //std::shuffle

// sink prevents the compiler from "optimsing away" the work.
// Writing the sum here convices -03 that the loop has effects
static volatile long long sink_ll = 0;

// Build n nodes and link them. If randomise = true we shuffle the node pointers
// before wiring "next" so successive nodes are far apart in memory
Node *make_linked(std::size_t n, bool randomise)
{
  std::vector<Node *> pool;
  pool.reserve(n);

  // Allocate nodes. Each "new" can land anywhere on the heap -> fragmentation.
  for (std::size_t i = 0; i < n; ++i)
  {
    // (i & 1023) just keeps values small; any deterministic value is fine
    pool.push_back(new Node{int(i & 1023), nullptr});
  }

  // enforce bad locality by shuffling the addresses
  if (randomise)
  {
    std::mt19937 rng(123); // fixed seed for reproducibilty
    std::shuffle(pool.begin(), pool.end(), rng);
  }

  // wire next pointers in pool order.
  for (std::size_t i = 1; i < n; ++i)
  {
    pool[i - 1]->next = pool[i];
  }

  // return head (nullptr if n==0)
  return n ? pool[0] : nullptr;
}

// standard loop to delete the list and avoid memory leaks
void free_list(Node *h)
{
  while (h)
  {
    Node *t = h->next;
    delete h;
    h = t;
  }
}

// Sume over contiguous memory: stride-1 means excellent spatial locality
// hardware prefetches will fetch future cache lines proactively
long long traverse_vector(const Vec &a)
{
  long long s = 0;
  for (std::size_t i = 0; i < a.size(); ++i)
  {
    s += a[i];
  }
  sink_ll = s; // stop all dead code elimination
  return s;
}

// sum over a singly linked list: every step follows a pointer to an arbitrary address
// This defeats spatial locality and can cause frequent cache/TLB misses.
long long traverse_linked(const Node *head)
{
  long long s = 0;
  for (auto p = head; p; p = p->next)
  {
    s += p->val;
  }
  sink_ll = s;
  return s;
}
