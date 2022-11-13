#pragma once

#include <deque>
#include <string>

#include "auxiliary_tree.h"
#include "basic_traverser.h"
#include "container_wrapper.h"
class Solver : private BasicTraverser {
    friend class SolverDivider;
    friend class SolverParallel;
    using HASH_SET_WRAPPER = ContainerWrapper<HASH_SET<ull>, ull>;
    using PARALLEL_HASH_MAP_P = CONCURRENT_HASH_MAP<ull, bool>*;

   private:
    int initial_depth = 0;
    int current_depth = 0;
    int bag_idx = 0;
    std::deque<int> v_pieces;
    std::bitset<7> bag_used[10];

    AuxiliaryTree* aux_tr;
    auxNode* p = nullptr;

    PARALLEL_HASH_MAP_P hash_map;
    bool hm_original;
    HASH_SET_WRAPPER feasible_fields, endgame_state_d5, endgame_state_d6;

    double* p_alpha = nullptr;

   public:
    Solver(std::string ff_fname, std::string es5_fname, std::string es6_fname);

    Solver(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr);

    Solver(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr, const PARALLEL_HASH_MAP_P hm_p);

    virtual ~Solver();

    void ClearHashMap();

    void SetAuxiliaryTree(AuxiliaryTree* tr, auxNode* node);

    void SetState(int bag_idx, std::deque<int> pieces, std::bitset<7> bag_used, std::bitset<10> field[4], int depth);

    void SetAlpha(double* p);

    ProbContext CalculateProb(int selection, int x, int y, int ori, int depth);

    virtual ProbContext FindBestMove();

    bool Action(int selection, int x, int y, int ori);

    void ConstructTree();

    void DestroyTree();

   private:
    inline ull GetStateHash();

    void InitializeField(std::bitset<10> field[4], int depth);

    inline bool EndgamePruneD5(ull field_hash);

    inline bool EndgamePruneD6(ull field_hash);

    double DfsGeneratePiece(int depth, double alpha) override;

    void DfsChoosePieceLazyFit(int depth, ProbContext& pr);

    void DfsChoosePiece(int depth, ProbContext& pr);
};
