#pragma once

#include <deque>
#include <string>
#include <vector>

#include "basic_traverser.h"
#include "container_wrapper.h"

class EndgameGenerator : private BasicTraverser {
    using HASH_SET_WRAPPER = ContainerWrapper<HASH_SET<ull>, ull>;

   private:
    int current_depth = 0;
    int bag_idx = 0;
    std::deque<int> v_pieces;
    std::vector<int> prev_pieces;
    HASH_SET_WRAPPER feasible_fields, endgame_states_d5, endgame_states_d6;

   public:
    explicit EndgameGenerator(std::string ff_fname);

    virtual ~EndgameGenerator();

    void Generate(int first_bag_idx, std::string es5_fname, std::string es6_fname);

   private:
    void SetFirstPiece(int i, int bag_idx);

    inline ull GetEndgameHashD5() const;

    inline ull GetEndgameHashD6() const;

    bool Prune(int depth) override;

    void FullClear() override;

    void InitializeField(std::bitset<10> field[4], int depth);
};
