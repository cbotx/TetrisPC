#include "solver_parallel.h"
#include "dll_interface.h"

#ifdef _MSC_VER
#  include <intrin.h>
#  define __builtin_popcount __popcnt
#endif

static bool halt;
static SolverParallel* solvers[7];

void __stdcall __halt__() {
    halt = true;
}

void __stdcall __core_init__(const char path[]) {
    std::string path_str(path);
#pragma omp parallel for
    for (int i = 0; i < 7; ++i) {
        if (solvers[i]) {
            solvers[i]->DestroyTree();
            delete solvers[i];
        }
        solvers[i] = new SolverParallel(
            path_str + "field_hash" + std::to_string(i) + ".dat",
            path_str + "hash_end_game5_" + std::to_string(i) + ".dat",
            path_str + "hash_end_game6_" + std::to_string(i) + ".dat"
        );
    }
}

void __stdcall __destroy__() {
    for (int i = 0; i < 7; ++i) {
        if (solvers[i]) {
            solvers[i]->DestroyTree();
            delete solvers[i];
            solvers[i] = nullptr;
        }
    }
}

void __stdcall __calculate_best_move__(int pc_idx,
                                     const int pieces[],
                                     const int bag_used,
                                     const int field[],
                                     int depth,
                                     int& ori,
                                     int& x,
                                     int& y) {
    static const int id_mapping[7] = { 4, 6, 1, 3, 0, 5, 2 };
    std::bitset<7> bs = bag_used;
    std::bitset<7> nbs = 0;
    int _pieces[7];
    for (int i = 0; i < 7; ++i) {
        if (bs[i]) nbs[id_mapping[i]] = 1;
        _pieces[i] = id_mapping[pieces[i]];
    }


    std::deque<int> v_pieces;
    for (int i = 0; i < 7; ++i) v_pieces.push_back(_pieces[i]);

    std::bitset<10> fields[4];
    // Add cleared lines
    int blocks = 0;
    for (int i = 0; i < 4; ++i) blocks += __builtin_popcount(field[i]);
    int cleared_lines = (depth * 4 - blocks) / 10;
    for (int i = cleared_lines; i < 4; ++i) fields[i] = field[i - cleared_lines];
    for (int i = 0; i < cleared_lines; ++i) fields[i] = 0b1111111111;

    int bag_idx = (pc_idx + depth) % 7;
    solvers[pc_idx]->SetState(bag_idx, v_pieces, nbs.to_ulong(), fields, depth);
    if (depth == 0) solvers[pc_idx]->ConstructTree();
    ProbContext pr = solvers[pc_idx]->FindBestMove();
    ori = pr.ori;
    x = 9 - pr.x - PIECE_OFFSET[ori][0];
    y = pr.y + PIECE_OFFSET[ori][1] - cleared_lines;

    // Post process
    if (pr.ori < 0) {
        return;
    }
    int selection = 0;
    if (PIECEMAP[pr.ori] == _pieces[1]) selection = 1;
    if (depth < 5) solvers[pc_idx]->Action(selection, pr.x, pr.y, pr.ori);
    if (depth == 9) solvers[pc_idx]->DestroyTree();
}
