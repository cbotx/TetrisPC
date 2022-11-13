#include "solver_parallel.h"

#include <algorithm>
#include <string>
#include <vector>

SolverParallel::SolverParallel(std::string ff_fname, std::string es5_fname, std::string es6_fname)
    : Solver(ff_fname, es5_fname, es6_fname),
      finder_d1(feasible_fields, endgame_state_d5, endgame_state_d6) {
}
SolverParallel::~SolverParallel() {
}

ProbContext SolverParallel::FindBestMove() {
    ProbContext pr;
    finder_d1.Reset();
    finder_d1.SetAuxiliaryTree(aux_tr, p);
    finder_d1.SetState(bag_idx, v_pieces, bag_used[current_depth], field + current_depth * 4, current_depth);
    finder_d1.FindBestMove();
    std::vector<std::array<int, 7>> v_possible_drops = finder_d1.GetPossibleDrops();
    int v_size = v_possible_drops.size();
    std::vector<double> v_pr(v_size);
    double* p_alpha = new double(0);
#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < v_size; ++i) {
        const auto& item = v_possible_drops[i];
        Solver finder(feasible_fields, endgame_state_d5, endgame_state_d6, hash_map);
        finder.SetAuxiliaryTree(aux_tr, p);
        finder.SetState(bag_idx, v_pieces, bag_used[current_depth], field + current_depth * 4, current_depth);
        finder.SetAlpha(p_alpha);
        int selection = (PIECEMAP[item[6]] == v_pieces[0] ? 0 : 1);
        ProbContext pr = finder.CalculateProb(selection, item[4], item[5], item[6], current_depth);
        v_pr[i] = pr.prob;
#pragma omp critical
        *p_alpha = std::max(*p_alpha, pr.prob);
    }
    delete p_alpha;
    ClearHashMap();
    ProbContext result;
    for (int i = 0; i < v_size; ++i) {
        if (v_pr[i] > result.prob) {
            auto& item = v_possible_drops[i];
            result = {v_pr[i], item[4], item[5], item[6]};
        }
    }
    return result;
}
