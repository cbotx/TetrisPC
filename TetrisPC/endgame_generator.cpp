#include "endgame_generator.h"

#include <string>
#include <vector>

#include "utils.h"

EndgameGenerator::EndgameGenerator(std::string ff_fname)
    : BasicTraverser(),
      feasible_fields(ff_fname) {
}

EndgameGenerator::~EndgameGenerator() {
}

void EndgameGenerator::SetFirstPiece(int i, int bag_idx) {
    first_piece = i;
    start_bag_idx = bag_idx;
}

void EndgameGenerator::Generate(int first_bag_idx, std::string es5_fname, std::string es6_fname) {
    for (int i = 0; i < 7; ++i) {
        SetFirstPiece(i, first_bag_idx);
        Start();
    }
    endgame_states_d5.WriteToFile(es5_fname);
    endgame_states_d6.WriteToFile(es6_fname);
}

bool EndgameGenerator::Prune(int depth) {
    return !feasible_fields.contains(get_field_hash(field + depth * 4));
}

ull EndgameGenerator::GetEndgameHashD5() const {
    ull hash = get_field_hash(field5);
    hash <<= 7;
    uint32_t piece_hash = 0;
    for (int i = 5; i < 10; ++i) {
        piece_hash |= 1 << pieces[i];
    }
    return hash | piece_hash;
}

ull EndgameGenerator::GetEndgameHashD6() const {
    ull hash = get_field_hash(field6);
    hash <<= 7;
    uint32_t piece_hash = 0;
    for (int i = 6; i < 10; ++i) {
        piece_hash |= 1 << pieces[i];
    }
    return hash | piece_hash;
}

void EndgameGenerator::FullClear() {
    endgame_states_d5.insert(GetEndgameHashD5());
    endgame_states_d6.insert(GetEndgameHashD6());
}

void EndgameGenerator::InitializeField(std::bitset<10> field[4], int depth) {
    for (int i = 0; i < 4; ++i) {
        this->field[depth * 4 + i] = field[i];
    }
}
