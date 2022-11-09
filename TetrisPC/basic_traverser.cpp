#include "basic_traverser.h"

#include <string>

#include "definition.h"
#include "utils.h"

BasicTraverser::BasicTraverser() {
    hash_set = new HASH_SET<ull>;
    memset(used_count, 0, sizeof(used_count));
    memset(field, 0, sizeof(field));
    memset(col, 0, sizeof(col));
}
BasicTraverser::~BasicTraverser() {
}

void BasicTraverser::start() {
    for (int i = 0; i < 7; ++i) {
        used_count[i] = 1;
    }
    if (first_piece >= 0) ++used_count[first_piece];
    dfs(0, 0);
}

bool BasicTraverser::prune(int depth) {
    int empty_count = 0;
    std::bitset<10>* fp = field + depth * 4;
    std::bitset<10> seperated = (*fp | *fp >> 1) & (*(fp + 1) | *(fp + 1) >> 1) & (*(fp + 2) | *(fp + 2) >> 1) & (*(fp + 3) | *(fp + 3) >> 1);
    for (int i = 0; i < 9; ++i) {
        empty_count += 4 - col[depth][i];
        if ((col[depth][i] == 4 || seperated[i]) && (empty_count % 4)) return true;
    }
    return false;
}

double BasicTraverser::dfs(int depth, double alpha) {
    ProbContext pr;
    if (prune(depth)) return 1;
    int old_used_count[7];
    std::bitset<10> old_superposition = superposition;
    memcpy(old_used_count, used_count, sizeof(used_count));
    if ((depth + start_bag_idx) % 7 == 6) {
        int cnt = 0;
        for (int i = 0; i < 7; ++i) {
            if (used_count[i] == 2) {
                memset(used_count, 0, sizeof(used_count));
                used_count[i] = 1;
                cnt = 1;
                break;
            }
            if (used_count[i] == 1) ++cnt;
        }
        if (cnt > 1) {
            for (int i = 0; i < 7; ++i) {
                if (used_count[i] == 1) {
                    superposition[i] = 1;
                }
            }
        }
        for (int i = 0; i < 7; ++i) ++used_count[i];
    }

    for (int i = 0; i < 7; ++i) {
        if (!used_count[i]) continue;
        pieces[depth] = i;
        std::bitset<10> old_superposition = superposition;
        --used_count[i];
        bool is_collapse = superposition.to_ulong() && !used_count[i] && superposition[i];
        if (is_collapse) {
            for (int j = 0; j < 7; ++j) {
                if (j != i && superposition[j]) --used_count[j];
            }
            superposition = 0;
        }
        try_fit(i, field + depth * 4, depth, pr);
        if (is_collapse) {
            superposition = old_superposition;
            for (int j = 0; j < 7; ++j) {
                if (j != i && superposition[j]) ++used_count[j];
            }
        }
        ++used_count[i];
    }
    superposition = old_superposition;
    memcpy(used_count, old_used_count, sizeof(used_count));
    return 1;
}

void BasicTraverser::try_fit(int piece, std::bitset<10>* fp, int depth, ProbContext& pr) {
    ll h = get_field_hash_full(fp);
    std::bitset<64> bits;
    if (piece == 0) {  // I
        // vertical
        bits = h | h << 16 | h << 32 | h << 48;
        for (int i = 3; i < 13; ++i) {
            if (!bits[i + 3 * 16]) {
                fit(fp, i - 3, 0, 0, depth, pr);
#ifdef ONLINE_SOLVE
                if (equals(pr.prob, 1)) return;
#endif
            }
        }
        // horizontal
        bits = h | h >> 1 | h >> 2 | h >> 3 | (~h >> 16 & ~h >> 17 & ~h >> 18 & ~h >> 19) | ((h << 13 | h << 14 | h << 15 | h << 16 | ((h << 29 | h << 45) & (h << 30 | h << 46) & (h << 31 | h << 47) & (h << 32 | h << 48))) & (h << 16 | h << 32 | h << 48 | (~h << 17 & ~h << 18) | (~h << 13 & ~h << 14 & ~h << 15)) & (h << 13 | h << 29 | h << 45 | (~h << 11 & ~h << 12) | (~h << 14 & ~h << 15 & ~h << 16) | (~h << 42 & ~h << 43 & ~h << 44)));
        for (int i = 3; i < 10; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 1, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    } else if (piece == 1) {  // O
        bits = h | h << 15 | h << 16 | h >> 1 | (~h >> 16 & ~h >> 17) | ((h << 31 | h << 32 | h << 47 | h << 48) & (h << 1 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 14 | h << 30 | h << 31 | h << 46 | h << 47 | h >> 2) & (h << 1 | h << 17 | h << 18 | h << 2 | h << 33 | h << 34 | h << 49 | h << 50) & (h << 13 | h << 14 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 2 | h >> 3));
        for (int i = 3; i < 12; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 2, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    } else if (piece == 2) {  // T
                              // up
        bits = h | h << 15 | h >> 1 | h >> 2 | (~h >> 16 & ~h >> 17 & ~h >> 18) | ((h << 1 | h << 16 | h << 31 | h << 32 | h << 47 | h << 48) & (h << 14 | h << 30 | h << 31 | h << 46 | h << 47 | h >> 3) & (h << 14 | h << 30 | h << 31 | h << 46 | h << 47 | ~h << 13) & (h << 16 | h << 31 | h << 32 | h << 47 | h << 48 | ~h << 17) & (h << 1 | h << 16 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 13 | h << 14 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 3));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 3, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }

        // down
        bits = h << 14 | h << 15 | h << 16 | h >> 1 | (~h >> 17 & ~h >> 2) | ((h << 30 | h << 31 | h << 46 | h << 47) & (h << 31 | h << 32 | h << 47 | h << 48) & (h | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 13 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 2));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 4, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // left
        bits = h | h << 15 | h << 16 | h << 32 | (~h >> 1 & ~h >> 16) | ((h << 31 | h << 47 | h << 48) & (h << 17 | h << 33 | h << 48 | h << 49) & (h << 14 | h << 30 | h << 31 | h << 46 | h << 47 | h >> 1) & (h << 17 | h << 18 | h << 33 | h << 34 | h << 49 | h << 50));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 5, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // right
        bits = h << 15 | h << 16 | h << 31 | h >> 1 | (~h & ~h >> 17) | ((h << 32 | h << 47 | h << 48) & (h << 14 | h << 30 | h << 46 | h << 47) & (h | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 13 | h << 14 | h << 29 | h << 30 | h << 45 | h << 46));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 6, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    } else if (piece == 3) {  // S
                              // vertical
        bits = h | h << 15 | h << 16 | h << 31 | (~h >> 1 & ~h >> 16) | ((h << 32 | h << 47 | h << 48) & (h << 14 | h << 30 | h << 46 | h << 47 | h >> 1) & (h << 1 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 7, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // horizontal
        bits = h << 15 | h << 16 | h >> 1 | h >> 2 | (~h >> 17 & ~h >> 18) | ((h << 30 | h << 31 | h << 46 | h << 47) & (h | h << 31 | h << 32 | h << 47 | h << 48) & (h << 13 | h << 14 | h << 29 | h << 30 | h << 45 | h << 46) & (h | h << 1 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 8, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    } else if (piece == 4) {  // Z
                              // vertical
        bits = h << 15 | h << 16 | h << 32 | h >> 1 | (~h & ~h >> 17) | ((h << 31 | h << 47 | h << 48) & (h | h << 17 | h << 33 | h << 48 | h << 49) & (h << 14 | h << 30 | h << 31 | h << 46 | h << 47 | h >> 2));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 9, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // horizontal
        bits = h | h << 14 | h << 15 | h >> 1 | (~h >> 16 & ~h >> 17) | ((h << 31 | h << 32 | h << 47 | h << 48) & (h << 30 | h << 31 | h << 46 | h << 47 | h >> 2) & (h << 16 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 13 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 2 | h >> 3));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 10, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    } else if (piece == 5) {  // J
                              // up
        bits = h | h << 16 | h << 32 | h >> 1 | (~h >> 16 & ~h >> 17) | ((h << 15 | h << 31 | h << 47 | h << 48) & (h << 1 | h << 17 | h << 33 | h << 48 | h << 49) & (h << 14 | h << 15 | h << 30 | h << 31 | h << 46 | h << 47) & (h << 1 | h << 17 | h << 18 | h << 33 | h << 34 | h << 49 | h << 50));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 11, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // down
        bits = h << 15 | h << 31 | h << 32 | h >> 1 | ~h >> 17 | ((h << 47 | h << 48) & (h | h << 16 | h << 33 | h << 48 | h << 49) & (h << 14 | h << 30 | h << 46 | h << 47 | h >> 2));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 12, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // left
        bits = h | h << 14 | h << 15 | h << 16 | (~h >> 1 & ~h >> 16 & ~h >> 2) | ((h << 31 | h << 32 | h << 47 | h << 48 | h >> 1) & (h << 1 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 30 | h << 31 | h << 46 | h << 47 | h >> 1 | h >> 2) & (h << 1 | h << 17 | h << 18 | h << 33 | h << 34 | h << 49 | h << 50));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 13, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // right
        bits = h | h << 14 | h >> 1 | h >> 2 | (~h >> 16 & ~h >> 17 & ~h >> 18) | ((h << 15 | h << 16 | h << 31 | h << 32 | h << 47 | h << 48) & (h << 1 | h << 16 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 30 | h << 31 | h << 46 | h << 47 | (~h << 13 & ~h << 15 & ~h << 29)) & (h << 13 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 3 | (~h << 12 & ~h << 28)));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 14, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    } else {  // L
              // up
        bits = h | h << 15 | h << 31 | h >> 1 | (~h >> 16 & ~h >> 17) | ((h << 16 | h << 32 | h << 47 | h << 48) & (h << 14 | h << 30 | h << 46 | h << 47 | h >> 2) & (h << 16 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49) & (h << 13 | h << 14 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 2));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 15, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // down
        bits = h | h << 16 | h << 31 | h << 32 | ~h >> 16 | ((h << 47 | h << 48) & (h << 1 | h << 17 | h << 33 | h << 48 | h << 49) & (h << 15 | h << 30 | h << 46 | h << 47 | h >> 1));
        for (int i = 3; i < 12; ++i) {
            for (int j = 2; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 16, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // left
        bits = h | h << 16 | h >> 1 | h >> 2 | (~h >> 16 & ~h >> 17 & ~h >> 18) | ((h << 14 | h << 15 | h << 30 | h << 31 | h << 46 | h << 47) & (h << 13 | h << 14 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 3) & (h << 31 | h << 32 | h << 47 | h << 48 | (~h << 15 & ~h << 17 & ~h << 33)) & (h << 1 | h << 17 | h << 32 | h << 33 | h << 48 | h << 49 | (~h << 18 & ~h << 34)));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 17, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
        // right
        bits = h << 14 | h << 15 | h << 16 | h >> 2 | (~h & ~h >> 1 & ~h >> 18) | ((h << 30 | h << 31 | h << 46 | h << 47 | h >> 1) & (h | h << 31 | h << 32 | h << 47 | h << 48 | h >> 1) & (h << 13 | h << 29 | h << 30 | h << 45 | h << 46 | h >> 3));
        for (int i = 3; i < 11; ++i) {
            for (int j = 1; j < 4; ++j) {
                if (!bits[i + j * 16]) {
                    fit(fp, i - 3, 3 - j, 18, depth, pr);
#ifdef ONLINE_SOLVE
                    if (equals(pr.prob, 1)) return;
#endif
                }
            }
        }
    }
}

void BasicTraverser::fc() {
    if (hash_gen) {
        for (int i = 0; i < 9; ++i) {
            hash_set->insert(get_field_hash(field + i * 4));
        }
    }

#ifdef DEBUG_PRINT
    ++total;
    if (total % 1000000 == 0) {
        std::cout << first_piece << "  :  ";
        ll idx = 0;
        for (int i = 0; i < 10; ++i) {
            idx *= 19;
            idx += pieces_ori[i];
            std::cout << pieces[i];
        }
        std::cout << "    " << total << '\n';
    }
#endif
}

void BasicTraverser::fit(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
    ++func_calls[3][depth];
    ++depth_cnt[depth];
#endif
    pieces_ori[depth] = pid;
    for (int i = 0; i < 4; ++i) {
        int nx = x + PIECE_REPR[pid][i][0];
        (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = true;
#ifndef ONLINE_SOLVE
        ++col[depth][nx];
#endif
    }
    double prob = 1;
    if (depth < 9) {
        memcpy(fp + 4, fp, sizeof(*fp) * 4);
#ifndef ONLINE_SOLVE
        memcpy(col + depth + 1, col + depth, sizeof(col[0]));
#endif
        clearLines(fp + 4);
#ifdef GENERATE_ENDGAME
        if (depth == 4) memcpy(field5, fp + 4, sizeof(*fp) * 4);
        if (depth == 5) memcpy(field6, fp + 4, sizeof(*fp) * 4);
#endif
        prob = dfs(depth + 1, pr.prob);
    } else {
        fc();
    }

    for (int i = 0; i < 4; ++i) {
        int nx = x + PIECE_REPR[pid][i][0];
        (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = false;
#ifndef ONLINE_SOLVE
        --col[depth][nx];
#endif
    }

#ifdef DEBUG_PRINT
    if (depth == 0) std::cout << prob << ' ' << pid << ' ' << x << ' ' << y << '\n';
#endif
    if (prob > pr.prob) {
        pr = {prob, x, y, pid};
    }
}