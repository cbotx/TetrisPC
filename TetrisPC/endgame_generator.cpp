#include "endgame_generator.h"

#include "utils.h"

EndGameGenerator::EndGameGenerator(const HASH_SET_WRAPPER& ff_wr)
    : BasicTraverser(),
      feasible_fields(ff_wr) {
}

EndGameGenerator::~EndGameGenerator() {
}

void EndGameGenerator::setState(int bag_idx, std::deque<int> pieces, std::vector<int> prev_pieces, std::bitset<10> field[4], int depth) {
    this->bag_idx = bag_idx;
    this->v_pieces = pieces;
    this->prev_pieces = prev_pieces;
    init_field(field, depth);
    current_depth = depth;
}

bool EndGameGenerator::prune(int depth) {
    return feasible_fields.contains(get_field_hash(field + depth * 4));
}

ull EndGameGenerator::get_end_game_hash5() const {
    ull hash = get_field_hash(field5);
    hash <<= 7;
    uint32_t piece_hash = 0;
    for (int i = 5; i < 10; ++i) {
        piece_hash |= 1 << pieces[i];
    }
    return hash | piece_hash;
}

ull EndGameGenerator::get_end_game_hash6() const {
    ull hash = get_field_hash(field6);
    hash <<= 7;
    uint32_t piece_hash = 0;
    for (int i = 6; i < 10; ++i) {
        piece_hash |= 1 << pieces[i];
    }
    return hash | piece_hash;
}

void EndGameGenerator::fc() {
#ifdef DEBUG_PRINT
    ++total;
    if (total % 10000000 == 0) {
        std::cout << total << ' ' << hash_end_game5.size() << ' ' << hash_end_game6.size() << '\n';
    }
#endif
    endgame_states_d5.insert(get_end_game_hash5());
    endgame_states_d6.insert(get_end_game_hash6());
}

void EndGameGenerator::init_field(std::bitset<10> field[4], int depth) {
    for (int i = 0; i < 4; ++i) {
        this->field[depth * 4 + i] = field[i];
    }
}