#pragma once

#include <bitset>

#include "definition.h"

class BasicTraverser {
    friend class FeasibleFieldGenerator;
    friend class EndgameGenerator;
    friend class AuxiliaryTree;
    friend class Solver;
    friend class SolverDivider;
    friend class SolverParallel;

   public:
    BasicTraverser();

    virtual ~BasicTraverser();

    BasicTraverser(const BasicTraverser&) = default;

    BasicTraverser& operator=(const BasicTraverser&) = default;

    void Start();

   private:
    virtual bool Prune(int depth);

    virtual double DfsGeneratePiece(int depth, double alpha);

    virtual void DfsTryFitPiece(int piece, std::bitset<10>* fp, int depth, ProbContext& pr);

    virtual void FullClear();

    virtual void DfsFitPiece(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr);

   private:
    int used_count[7];
    std::bitset<10> superposition = 0;

    int col[10][10];
    int pieces_ori[10];
    int total = 0;

    int start_bag_idx = 0;
    int first_piece = -1;
    std::bitset<10> field[40];
    std::bitset<10> field5[4];
    std::bitset<10> field6[4];
    int pieces[10];
};
