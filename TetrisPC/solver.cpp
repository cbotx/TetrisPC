#include "solver.h"

#include "utils.h"

Solver::Solver(std::string ff_fname, std::string es5_fname, std::string es6_fname) : feasible_fields(ff_fname),
                                                                                     endgame_state_d5(es5_fname),
                                                                                     endgame_state_d6(es6_fname),
                                                                                     hm_original(true) {
    aux_tr = nullptr;
    hash_map = new CONCURRENT_HASH_MAP<ull, bool>;
}

Solver::Solver(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr) : feasible_fields(ff_wr),
                                                                                                                endgame_state_d5(es5_wr),
                                                                                                                endgame_state_d6(es6_wr),
                                                                                                                hm_original(true) {
    aux_tr = nullptr;
    hash_map = new CONCURRENT_HASH_MAP<ull, bool>;
}

Solver::Solver(const HASH_SET_WRAPPER& ff_wr, const HASH_SET_WRAPPER& es5_wr, const HASH_SET_WRAPPER& es6_wr, const PARALLEL_HASH_MAP_P hm_p) : feasible_fields(ff_wr),
                                                                                                                                                endgame_state_d5(es5_wr),
                                                                                                                                                endgame_state_d6(es6_wr),
                                                                                                                                                hm_original(false) {
    aux_tr = nullptr;
    hash_map = hm_p;
}

Solver::~Solver() {
    if (hm_original) delete hash_map;
}

void Solver::clearHashMap() {
    hash_map->clear();
}

void Solver::setAuxTree(AuxiliaryTree* tr, auxNode* node) {
    aux_tr = tr;
    p = node;
}

void Solver::setState(int bag_idx, std::deque<int> pieces, std::bitset<7> bag_used, std::bitset<10> field[4], int depth) {
    this->bag_idx = bag_idx;
    this->v_pieces = pieces;
    this->bag_used[depth] = bag_used;
    init_field(field, depth);
    current_depth = depth;
    initial_depth = depth;
}

void Solver::setAlpha(double* p) {
    p_alpha = p;
}

ProbContext Solver::calculateProb(int selection, int x, int y, int ori, int depth) {
    ProbContext pr;
    if (depth < 5) action(selection, x, y, ori);
    if (selection == 1) std::swap(v_pieces[0], v_pieces[1]);
    v_pieces.pop_front();
    bag_idx = (bag_idx + 1) % 7;
    fit(field + depth * 4, x, y, ori, depth, pr);
    return pr;
}

ProbContext Solver::findBestMove() {
    ProbContext pr;
    if (current_depth < 5) {
        dfs_aux(current_depth, pr);
    } else {
        dfs2(current_depth, pr);
    }
    return pr;
}

bool Solver::action(int selection, int x, int y, int ori) {
    bool succ = false;
    auto& v = p->v[selection];
    for (auto& item : v) {
        if (item->op.x == x && item->op.y == y && item->op.ori == ori) {
            p = item;
            succ = true;
            break;
        }
    }
    if (!succ) std::cout << "BAD!!!!!!!!!!!!!!\n";
    return succ;
}

void Solver::constructTree() {
    aux_tr = new AuxiliaryTree(feasible_fields);
    aux_tr->constructTree(v_pieces);
    p = aux_tr->getRoot();
}

void Solver::destroyTree() {
    aux_tr->destroyTree();
}

inline ull Solver::get_state_hash() {
    ull hash = get_field_hash(field + 20);
    for (int i = 0; i < 6; ++i) {
        hash = (hash << 3) | v_pieces[i];
    }
    return hash;
}

void Solver::init_field(std::bitset<10> field[4], int depth) {
    for (int i = 0; i < 4; ++i) {
        this->field[depth * 4 + i] = field[i];
    }
    for (int i = 0; i < 10; ++i) {
        col[depth][i] = 0;
        for (int j = 0; j < 4; ++j) {
            if (field[j][i]) ++col[depth][i];
        }
    }
}

inline bool Solver::end_game_prune5(ull field_hash) {
    for (int i = 0; i < 6; ++i) {
        int piece_hash = 0;
        for (int j = 0; j < 6; ++j) {
            if (j != i) {
                piece_hash |= 1 << v_pieces[j];
            }
        }
        if (endgame_state_d5.contains((field_hash << 7) | piece_hash)) {
#ifdef DEBUG_PRINT
            ++not_prune5;
#endif
            return false;
        }
    }
#ifdef DEBUG_PRINT
    ++prune5;
#endif
    return true;
}

inline bool Solver::end_game_prune6(ull field_hash) {
    for (int i = 0; i < 5; ++i) {
        int piece_hash = 0;
        for (int j = 0; j < 5; ++j) {
            if (j != i) {
                piece_hash |= 1 << v_pieces[j];
            }
        }
        if (endgame_state_d5.contains((field_hash << 7) | piece_hash)) {
#ifdef DEBUG_PRINT
            ++not_prune6;
#endif
            return false;
        }
    }
#ifdef DEBUG_PRINT
    ++prune6;
#endif
    return true;
}

double Solver::dfs(int depth, double alpha) {
#ifdef DEBUG_PRINT
    ++func_calls[4][depth];
#endif
    if (!feasible_fields.contains(get_field_hash(field + depth * 4))) return 0;
    double prob = 0;
    int cnt = 0;
    int cnt2 = 0;
    if (depth + v_pieces.size() >= 11) {
        ProbContext pr;
        if (depth == 4) {
            dfs_aux(depth, pr);
            return pr.prob;
        }
        if (depth == 5 && end_game_prune5(get_field_hash(field + 20))) return 0;
        if (depth == 6 && end_game_prune6(get_field_hash(field + 24))) return 0;
        if (depth == 5) {
            ull hash = get_state_hash();

            bool prob = 0;
            bool is_contain = hash_map->if_contains(hash, [&prob](CONCURRENT_HASH_MAP<ull, bool>::value_type& item) { prob = item.second; });

            if (!is_contain) {
                dfs2(depth, pr);
                prob = pr.prob;
                hash_map->try_emplace_l(
                    hash, [](CONCURRENT_HASH_MAP<ull, bool>::value_type& item) {}, prob);
            }
            return prob;
        }
        dfs2(depth, pr);
        return pr.prob;
    }
    for (int i = 0; i < 7; ++i) {
        if (!bag_used[depth - 1][i]) ++cnt;
    }
    if (cnt == 0) {
        cnt = 7;
        bag_used[depth] = 0;
    } else {
        bag_used[depth] = bag_used[depth - 1];
    }
    for (int i = 0; i < 7; ++i) {
        if (!bag_used[depth][i]) {
            ProbContext pr;
            v_pieces.push_back(i);
            bag_used[depth][i] = 1;
            dfs_aux(depth, pr);
            bag_used[depth][i] = 0;
            v_pieces.pop_back();
            ++cnt2;
            if (depth == initial_depth + 1) {
                alpha = *p_alpha;
            }
            prob += pr.prob;
            if (prob + cnt - cnt2 < alpha * cnt) break;
        }
    }
    if (cnt > 0) prob /= cnt;
    return prob;
}

void Solver::dfs_aux(int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
    ++func_calls[0][depth];
#endif
    bag_idx = (bag_idx + 1) % 7;
    int p1 = v_pieces[0];
    v_pieces.pop_front();
    pieces[depth] = p1;

    auxNode* old_p = p;
    auto& v1 = p->v[0];
    for (auto& item : v1) {
        p = item;
        fit(field + depth * 4, item->op.x, item->op.y, item->op.ori, depth, pr);
#ifdef ONLINE_SOLVE
        if (equals(pr.prob, 1)) {
            v_pieces.push_front(p1);
            bag_idx = (bag_idx + 6) % 7;
            p = old_p;
            return;
        }
#endif
    }
    p = old_p;

    int p2 = v_pieces[0];
    v_pieces.pop_front();
    v_pieces.push_front(p1);
    pieces[depth] = p2;

    auto& v2 = p->v[1];
    for (auto& item : v2) {
        p = item;
        fit(field + depth * 4, item->op.x, item->op.y, item->op.ori, depth, pr);
#ifdef ONLINE_SOLVE
        if (equals(pr.prob, 1)) break;
#endif
    }
    p = old_p;

    v_pieces.pop_front();
    v_pieces.push_front(p2);
    v_pieces.push_front(p1);
    bag_idx = (bag_idx + 6) % 7;
}

void Solver::dfs2(int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
    ++func_calls[1][depth];
#endif
    bag_idx = (bag_idx + 1) % 7;
    int p1 = v_pieces[0];
    v_pieces.pop_front();
    pieces[depth] = p1;
    try_fit(p1, field + depth * 4, depth, pr);
#ifdef ONLINE_SOLVE
    if (equals(pr.prob, 1)) {
        v_pieces.push_front(p1);
        bag_idx = (bag_idx + 6) % 7;
        return;
    }
#endif
    int p2 = v_pieces[0];
    v_pieces.pop_front();
    v_pieces.push_front(p1);
    pieces[depth] = p2;
    try_fit(p2, field + depth * 4, depth, pr);
    v_pieces.pop_front();
    v_pieces.push_front(p2);
    v_pieces.push_front(p1);
    bag_idx = (bag_idx + 6) % 7;
}
