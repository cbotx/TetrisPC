#pragma once
#include <array>
#include <vector>

#include "solver.h"

class SolverDivider : private Solver {
    friend class SolverParallel;
    using HASH_SET_WRAPPER = ContainerWrapper<HASH_SET<ull>, ull>;

   private:
    std::vector<std::array<int, 7>> v_possible_drops;

   public:
    SolverDivider(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr);

    void Reset();

    const std::vector<std::array<int, 7>>& GetPossibleDrops();

   private:
    void DfsFitPiece(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) override;
};
