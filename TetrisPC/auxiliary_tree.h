#pragma once

#include <deque>

#include "basic_traverser.h"
#include "container_wrapper.h"
#include "definition.h"

class AuxiliaryTree : private BasicTraverser {
    using HASH_SET_WRAPPER = ContainerWrapper<HASH_SET<ull>, ull>;

   private:
    auxNode* root = nullptr;
    auxNode* p = nullptr;
    std::deque<int> v_pieces;
    opData ops[5]{0};
    int selections[5]{0};
    auxNode* path[6]{0};
    int lowest = 0;
    HASH_SET_WRAPPER feasible_fields;

   public:
    explicit AuxiliaryTree(const HASH_SET_WRAPPER& ff_wr);

    auxNode* getRoot();

    void constructTree(std::deque<int>& v_pieces);

    void destroyTree();

    virtual double dfs(int depth, double alpha);

    virtual void fit(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr);

   private:
    void destroy_node(auxNode* node);

    void initialize();
};