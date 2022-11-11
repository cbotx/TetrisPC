#pragma once

#include "basic_traverser.h"
#include "container_wrapper.h"
#include "definition.h"

class FeasibleFieldGenerator : private BasicTraverser {
   public:
    FeasibleFieldGenerator() = default;

    void setFirstPiece(int i, int bag_idx);

   private:
    virtual void fc();

   private:
    ContainerWrapper<HASH_SET<ull>, ull> feasible_fields;
};