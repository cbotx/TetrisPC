#include "auxiliary_tree.h"

#include "utils.h"

AuxiliaryTree::AuxiliaryTree(const HASH_SET_WRAPPER& ff_wr)
    : BasicTraverser(),
      feasible_fields(ff_wr) {
    initialize();
}
auxNode* AuxiliaryTree::getRoot() {
    return root;
}

void AuxiliaryTree::constructTree(std::deque<int>& v_pieces) {
    this->v_pieces = v_pieces;
    dfs(0, 0);
}

void AuxiliaryTree::destroyTree() {
    destroy_node(root);
    initialize();
}

double AuxiliaryTree::dfs(int depth, double alpha) {
    if (!feasible_fields.contains(get_field_hash(field + depth * 4))) return 0;
    ProbContext pr;

    int p1 = v_pieces[0];
    v_pieces.pop_front();
    pieces[depth] = p1;
    selections[depth] = 0;
    try_fit(p1, field + depth * 4, depth, pr);
    int p2 = v_pieces[0];
    v_pieces.pop_front();
    v_pieces.push_front(p1);
    pieces[depth] = p2;
    selections[depth] = 1;
    try_fit(p2, field + depth * 4, depth, pr);
    v_pieces.pop_front();
    v_pieces.push_front(p2);
    v_pieces.push_front(p1);
    return 1;
}

void AuxiliaryTree::fit(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
    pieces_ori[depth] = pid;
    opData opdata = {pid, x, y};
    ops[depth] = opdata;

    for (int i = 0; i < 4; ++i) {
        int nx = x + PIECE_REPR[pid][i][0];
        (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = true;
    }
    if (depth <= 4) {
        memcpy(fp + 4, fp, sizeof(*fp) * 4);
        clearLines(fp + 4);
        if (depth < 4) {
            dfs(depth + 1, pr.prob);
            lowest = std::min(lowest, depth);
        } else {
            if (!feasible_fields.contains(get_field_hash(fp + 4))) {
                lowest = std::min(lowest, depth);
            } else {
                // construct auxiliary tree
                p = path[lowest];
                for (int i = lowest; i < 5; ++i) {
                    auxNode* new_n = new auxNode{ops[i]};
                    p->v[selections[i]].push_back(new_n);
                    path[i + 1] = new_n;
                    p = new_n;
                }
                lowest = 4;
            }
        }
    }

    for (int i = 0; i < 4; ++i) {
        int nx = x + PIECE_REPR[pid][i][0];
        (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = false;
    }
}

void AuxiliaryTree::destroy_node(auxNode* node) {
    for (int i = 0; i < 2; ++i) {
        for (auto& item : node->v[i]) destroy_node(item);
    }
    delete node;
}
void AuxiliaryTree::initialize() {
    root = new auxNode();
    p = root;
    path[0] = root;
}