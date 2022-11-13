#pragma once

#include <iostream>
#include <vector>

#include "gtl/phmap.hpp"

// #define GENERATE_HASH
#define ONLINE_SOLVE
// #define GENERATE_ENDGAME

// #define DEBUG_PRINT

typedef int64_t ll;
typedef uint64_t ull;

template <class T>
using HASH_SET = gtl::flat_hash_set<T>;

template <class T, class V>
using HASH_MAP = gtl::flat_hash_map<T, V>;

template <class T>
using CONCURRENT_HASH_SET = gtl::parallel_flat_hash_set<
    T,
    gtl::priv::hash_default_hash<T>,
    gtl::priv::hash_default_eq<T>,
    gtl::priv::Allocator<T>,
    4, std::mutex>;

template <class T, class V>
using CONCURRENT_HASH_MAP = gtl::parallel_flat_hash_map<
    T,
    V,
    gtl::priv::hash_default_hash<T>,
    gtl::priv::hash_default_eq<T>,
    gtl::priv::Allocator<gtl::priv::Pair<const T, V>>,
    4, std::mutex>;

struct opData {
    uint16_t ori : 5;
    uint16_t x : 4;
    uint16_t y : 2;
};

union unionOpData {
    opData op_data;
    char data[2];
};

struct ProbContext {
    double prob = 0;
    int x = 0;
    int y = 0;
    int ori = -1;
};

struct auxNode {
    opData op;
    std::vector<auxNode*> v[2];
};

constexpr int WINDOW_SIZE = 7;
constexpr int FULL = 1023;
constexpr char PIECENAME[7] = {'I', 'O', 'T', 'S', 'Z', 'J', 'L'};
constexpr int PIECEMAP[19] = {0, 0, 1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6};
constexpr int PIECE_REPR[19][4][2] = {
    {{0, 0}, {0, 1}, {0, 2}, {0, 3}},  // Iv
    {{0, 0}, {1, 0}, {2, 0}, {3, 0}},  // Ih
    {{0, 0}, {0, 1}, {1, 0}, {1, 1}},  // O
    {{0, 0}, {1, 0}, {2, 0}, {1, 1}},  // Tu
    {{1, 0}, {0, 1}, {1, 1}, {2, 1}},  // Td
    {{0, 0}, {0, 1}, {1, 1}, {0, 2}},  // Tl
    {{1, 0}, {0, 1}, {1, 1}, {1, 2}},  // Tr
    {{0, 0}, {0, 1}, {1, 1}, {1, 2}},  // Sv
    {{1, 0}, {2, 0}, {0, 1}, {1, 1}},  // Sh
    {{1, 0}, {0, 1}, {1, 1}, {0, 2}},  // Zv
    {{0, 0}, {1, 0}, {1, 1}, {2, 1}},  // Zh
    {{0, 0}, {1, 0}, {0, 1}, {0, 2}},  // Ju
    {{1, 0}, {1, 1}, {0, 2}, {1, 2}},  // Jd
    {{0, 0}, {0, 1}, {1, 1}, {2, 1}},  // Jl
    {{0, 0}, {1, 0}, {2, 0}, {2, 1}},  // Jr
    {{0, 0}, {1, 0}, {1, 1}, {1, 2}},  // Lu
    {{0, 0}, {0, 1}, {0, 2}, {1, 2}},  // Ld
    {{0, 0}, {1, 0}, {2, 0}, {0, 1}},  // Ll
    {{2, 0}, {0, 1}, {1, 1}, {2, 1}}   // Lr
};
