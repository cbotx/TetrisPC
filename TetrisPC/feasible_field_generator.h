#pragma once

#include "basic_traverser.h"
#include "container_wrapper.h"
#include "definition.h"

class FeasibleFieldGenerator : private BasicTraverser {
    using HASH_SET_WRAPPER = ContainerWrapper<HASH_SET<ull>, ull>;

   public:
    FeasibleFieldGenerator() = default;

    void SetFirstPiece(int i, int bag_idx);

   private:
    void FullClear() override;

   private:
    HASH_SET_WRAPPER feasible_fields;
};
