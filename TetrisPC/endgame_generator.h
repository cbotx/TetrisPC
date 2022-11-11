#pragma once

#include <deque>

#include "basic_traverser.h"
#include "container_wrapper.h"

class EndGameGenerator : private BasicTraverser {
   private:
    int current_depth = 0;
    int bag_idx = 0;
    std::deque<int> v_pieces;
    std::vector<int> prev_pieces;
    ContainerWrapper<HASH_SET<ull>, ull> feasible_fields, endgame_state_d5, endgame_state_d6;

   public:
    EndGameGenerator() : BasicTraverser() {
    }

    void setState(int bag_idx, std::deque<int> pieces, std::vector<int> prev_pieces, std::bitset<10> field[4], int depth);

   private:
    inline ull get_end_game_hash5();

    inline ull get_end_game_hash6();

    virtual bool prune(int depth);

    virtual void fc();

    void init_field(std::bitset<10> field[4], int depth);
};