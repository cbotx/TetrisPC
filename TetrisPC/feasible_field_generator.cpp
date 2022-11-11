#include "feasible_field_generator.h"

#include "utils.h"

void FeasibleFieldGenerator::fc() {
    for (int i = 0; i < 9; ++i) {
        feasible_fields.insert(get_field_hash(field + i * 4));
    }
}

void FeasibleFieldGenerator::setFirstPiece(int i, int bag_idx) {
    first_piece = i;
    start_bag_idx = bag_idx;
}