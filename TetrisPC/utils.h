#pragma once

#include "definition.h"

#define TIME_BENCHMARK std::chrono::time_point<std::chrono::system_clock> t_begin;
#define TIME_START t_begin = std::chrono::system_clock::now();
#define TIME_END std::cout << "Time spent: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t_begin).count() / 1000.0 << "s\n";

static inline bool equals(double a, double b) {
    return fabs(a - b) < 1e-7;
}