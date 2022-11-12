#pragma once
#include <array>
#include <vector>

#include "solver.h"

class SolverTaskGenerator : private Solver {
    friend class SolverParallel;
    using HASH_SET_WRAPPER = ContainerWrapper<HASH_SET<ull>, ull>;

   private:
    std::vector<std::array<int, 7>> v_possible_drops;

   public:
    SolverTaskGenerator(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr);

    void reset();

    const std::vector<std::array<int, 7>>& getPossibleDrops();

   private:
    virtual void fit(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr);
};