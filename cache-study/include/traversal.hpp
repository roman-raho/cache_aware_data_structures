#pragma once
#include <vector>
#include <cstddef>

// A simple singly-linked list node
// Pointer based layout is intentionally cache unfriendly
struct Node
{
  int val;
  Node *next;
};

// Contiguous container alias. vector<int> stores elemtsn back-to-back in memory
using Vec = std::vector<int>;

// build a linked list with n nodes
// if randomise = true then we deliveratly shuffle node addresses to destroy locality
Node *make_linked(std::size_t n, bool randomise);

// Free the whole list (avoid leaks in long runs)
void free_list(Node *head);

// Do a simple O(n) sum over the vector (stride-1, great cache locality)
long long traverse_vector(const Vec &a);

// Do a simple O(n) sum over the linked list (pointer chasing, poor locality)
long long traverse_linked(const Node *head);