#pragma once
#include <string>

#include "solver.h"
#include "solver_divider.h"
class SolverParallel : public Solver {
   private:
    SolverDivider finder_d1;

   public:
    explicit SolverParallel(std::string ff_fname, std::string es5_fname, std::string es6_fname);

    virtual ~SolverParallel();

    ProbContext FindBestMove() override;
};
