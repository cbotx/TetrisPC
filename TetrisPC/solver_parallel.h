#pragma once
#include "solver.h"
#include "solver_task_generator.h"

class SolverParallel : private Solver {
   private:
    SolverTaskGenerator finder_d1;

   public:
    explicit SolverParallel(std::string ff_fname, std::string es5_fname, std::string es6_fname);

    virtual ~SolverParallel();

    virtual ProbContext findBestMove();
};