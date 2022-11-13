#pragma once

#include "solver_divider.h"

#include "utils.h"

SolverDivider::SolverDivider(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr)
    : Solver(ff_wr, es5_wr, es6_wr) {}

void SolverDivider::Reset() {
    v_possible_drops.clear();
}

const std::vector<std::array<int, 7>>& SolverDivider::GetPossibleDrops() {
    return v_possible_drops;
}

void SolverDivider::DfsFitPiece(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
    pieces_ori[depth] = pid;
    for (int i = 0; i < 4; ++i) {
        int nx = x + PIECE_REPR[pid][i][0];
        (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = true;
    }
    std::bitset<10> fp_new[4];
    memcpy(fp_new, fp, sizeof(*fp) * 4);
    clear_lines(fp_new);
    std::array<int, 7> drop_data = {fp_new[0].to_ulong(), fp_new[1].to_ulong(), fp_new[2].to_ulong(), fp_new[3].to_ulong(), x, y, pid};
    v_possible_drops.push_back(drop_data);

    for (int i = 0; i < 4; ++i) {
        int nx = x + PIECE_REPR[pid][i][0];
        (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = false;
    }
}
