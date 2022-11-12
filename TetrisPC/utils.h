#pragma once
#pragma warning(disable : 4996)

#include <bitset>
#include <string>
#include <vector>

#include "definition.h"

#define TIME_BENCHMARK std::chrono::time_point<std::chrono::system_clock> t_begin;
#define TIME_START t_begin = std::chrono::system_clock::now();
#define TIME_END std::cout << "Time spent: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t_begin).count() / 1000.0 << "s\n";

static inline bool equals(double a, double b) {
    return fabs(a - b) < 1e-7;
}

static inline ull get_field_hash(const std::bitset<10>* fp) {
    return (((fp->to_ullong() << 10) + (fp + 1)->to_ullong() << 10) + (fp + 2)->to_ullong() << 10) + (fp + 3)->to_ullong();
}

static inline ll get_field_hash_full(const std::bitset<10>* fp) {
    ll h = ((((fp->to_ullong() << 16) + (fp + 1)->to_ullong() << 16) + (fp + 2)->to_ullong() << 16) + (fp + 3)->to_ullong()) << 3;
    return h | 0b1110000000000111111000000000011111100000000001111110000000000111;
}

static inline void clearLines(std::bitset<10>* fp) {
    int c = 0;
    if (*fp == FULL) c += 1;
    if (*(fp + 1) == FULL) c += 2;
    if (*(fp + 2) == FULL) c += 4;
    if (*(fp + 3) == FULL) c += 8;
    if (c == 2) {
        swap(*(fp + 1), *fp);
    } else if (c == 4) {
        swap(*(fp + 2), *(fp + 1));
        swap(*(fp + 1), *fp);
    } else if (c == 5) {
        swap(*(fp + 2), *(fp + 1));
    } else if (c == 6) {
        swap(*(fp + 2), *fp);
    } else if (c == 8) {
        swap(*(fp + 3), *(fp + 2));
        swap(*(fp + 2), *(fp + 1));
        swap(*(fp + 1), *fp);
    } else if (c == 9) {
        swap(*(fp + 3), *(fp + 2));
        swap(*(fp + 2), *(fp + 1));
    } else if (c == 10) {
        swap(*(fp + 3), *(fp + 2));
        swap(*(fp + 2), *fp);
    } else if (c == 11) {
        swap(*(fp + 3), *(fp + 2));
    } else if (c == 12) {
        swap(*(fp + 2), *fp);
        swap(*(fp + 3), *(fp + 1));
    } else if (c == 13) {
        swap(*(fp + 3), *(fp + 1));
    } else if (c == 14) {
        swap(*(fp + 3), *fp);
    }
}

template <class T>
static void container_write(T& s, std::string filename) {
    std::vector<ull> v;
    for (auto& item : s) v.push_back(item);
    FILE* f = fopen(filename.c_str(), "wb");
    uint64_t size = v.size();
    fwrite(&size, 8, 1, f);
    fwrite(v.data(), sizeof(v[0]), size, f);
    fclose(f);
}

template <class T>
static void container_read(T& s, std::string filename) {
    s.clear();
    FILE* f = fopen(filename.c_str(), "rb");
    uint64_t size;
    fread(&size, 8, 1, f);
    std::vector<ull> v(size);
    fread(v.data(), sizeof(v[0]), size, f);
    fclose(f);
    for (auto& item : v) s.insert(item);
}