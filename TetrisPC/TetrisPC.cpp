#include <assert.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "definition.h"
#include "endgame_generator.h"
#include "feasible_field_generator.h"
#include "simulator.h"
#include "solver_parallel.h"
#include "utils.h"

bool debug_switch = false;

int depth_cnt[11] = {0};
int prune5 = 0;
int prune6 = 0;
int not_prune5 = 0;
int not_prune6 = 0;
int func_calls[5][10];

class PCFinder {
   private:
    int used_count[7];
    std::bitset<10> superposition = 0;
    int start_bag_idx = 0;

   protected:
    std::bitset<10> field[40];
    std::bitset<10> field5[4];
    std::bitset<10> field6[4];
    int col[10][10];
    int pieces[10];
    int pieces_ori[10];
    int total = 0;

    int first_piece = -1;

    HASH_SET<ull>* hash_set;
    bool hash_gen = false;

   public:
    PCFinder() {
        hash_set = new HASH_SET<ull>;
        memset(used_count, 0, sizeof(used_count));
        memset(field, 0, sizeof(field));
        memset(col, 0, sizeof(col));
    }
    virtual ~PCFinder() {
    }

    PCFinder(const PCFinder&) = default;
    PCFinder& operator=(const PCFinder&) = default;

    void loadHashSet(std::string filename) {
        container_read(*hash_set, filename);
        hash_set->insert(0);
#ifdef DEBUG_PRINT
        printf("%s size : %ull\n", filename.c_str(), hash_set->size());
#endif
    }

    void setHashSet(HASH_SET<ull>* hs) {
        hash_set = hs;
    }

    void SetFirstPiece(int i, int bag_idx) {
        first_piece = i;
        start_bag_idx = bag_idx;
    }

    void setHashMode(bool gen) {
        hash_gen = gen;
    }

    void Start() {
        for (int i = 0; i < 7; ++i) {
            used_count[i] = 1;
        }
        if (first_piece >= 0) ++used_count[first_piece];
        DfsGeneratePiece(0, 0);
    }

    const HASH_SET<ull>* getHashSet() {
        return hash_set;
    }

   protected:
    void ShowField(int depth) {
        for (int i = 3; i >= 0; --i) {
            for (int j = 9; j >= 0; --j) {
                std::cout << (field[depth * 4 + i][j] ? " 0" : "  ");
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    virtual bool Prune(int depth) {
        int empty_count = 0;
        std::bitset<10>* fp = field + depth * 4;
        std::bitset<10> seperated = (*fp | *fp >> 1) & (*(fp + 1) | *(fp + 1) >> 1) & (*(fp + 2) | *(fp + 2) >> 1) & (*(fp + 3) | *(fp + 3) >> 1);
        for (int i = 0; i < 9; ++i) {
            empty_count += 4 - col[depth][i];
            if ((col[depth][i] == 4 || seperated[i]) && (empty_count % 4)) return true;
        }
        return false;
    }

    inline ull get_field_hash(std::bitset<10>* fp) {
        return (((fp->to_ullong() << 10) + (fp + 1)->to_ullong() << 10) + (fp + 2)->to_ullong() << 10) + (fp + 3)->to_ullong();
    }

    inline ll get_field_hash_full(std::bitset<10>* fp) {
        ll h = ((((fp->to_ullong() << 16) + (fp + 1)->to_ullong() << 16) + (fp + 2)->to_ullong() << 16) + (fp + 3)->to_ullong()) << 3;
        return h | 0b1110000000000111111000000000011111100000000001111110000000000111;
    }

    virtual double DfsGeneratePiece(int depth, double alpha) {
        ProbContext pr;
        if (Prune(depth)) return 1;
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
            DfsTryFitPiece(i, field + depth * 4, depth, pr);
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

    virtual double dfs_old(int depth, double alpha) {
        ProbContext pr;
        if (depth == 0 && first_piece >= 0) {
            pieces[0] = first_piece;
            --used_count[first_piece];
            DfsTryFitPiece(first_piece, field, 0, pr);
            return 1;
        }
        if (Prune(depth)) return 1;
        if (depth == 6) {
            for (int i = 0; i < 7; ++i) ++used_count[i];
        }

        for (int i = 0; i < 7; ++i) {
            if (!used_count[i]) continue;
            pieces[depth] = i;
            --used_count[i];
            DfsTryFitPiece(i, field + depth * 4, depth, pr);
            ++used_count[i];
        }
        if (depth == 6) {
            for (int i = 0; i < 7; ++i) --used_count[i];
        }
        return 1;
    }

    void DfsTryFitPiece(int piece, std::bitset<10>* fp, int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
        ++func_calls[2][depth];
#endif
        ll h = get_field_hash_full(fp);
        std::bitset<64> bits;
        if (piece == 0) {  // I
            // vertical
            bits = h | h << 16 | h << 32 | h << 48;
            for (int i = 3; i < 13; ++i) {
                if (!bits[i + 3 * 16]) {
                    DfsFitPiece(fp, i - 3, 0, 0, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 1, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 2, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 3, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 4, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 5, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 6, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 7, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 8, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 9, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 10, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 11, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 12, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 13, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 14, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 15, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 16, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 17, depth, pr);
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
                        DfsFitPiece(fp, i - 3, 3 - j, 18, depth, pr);
#ifdef ONLINE_SOLVE
                        if (equals(pr.prob, 1)) return;
#endif
                    }
                }
            }
        }
    }

    virtual void FullClear() {
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

    virtual void DfsFitPiece(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
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
            clear_lines(fp + 4);
#ifdef GENERATE_ENDGAME
            if (depth == 4) memcpy(field5, fp + 4, sizeof(*fp) * 4);
            if (depth == 5) memcpy(field6, fp + 4, sizeof(*fp) * 4);
#endif
            prob = DfsGeneratePiece(depth + 1, pr.prob);
        } else {
            FullClear();
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

    inline void clear_lines(std::bitset<10>* fp) {
        int c = 0;
        if (*fp == FULL) c += 1;
        if (*(fp + 1) == FULL) c += 2;
        if (*(fp + 2) == FULL) c += 4;
        if (*(fp + 3) == FULL) c += 8;
        if (c == 2) {
            swap(*(fp + 1), *fp);
        } else if (c == 4) {
            swap(*(fp + 2), *(fp + 1));
            swap(*(fp + 1), *fp);
        } else if (c == 5) {
            swap(*(fp + 2), *(fp + 1));
        } else if (c == 6) {
            swap(*(fp + 2), *fp);
        } else if (c == 8) {
            swap(*(fp + 3), *(fp + 2));
            swap(*(fp + 2), *(fp + 1));
            swap(*(fp + 1), *fp);
        } else if (c == 9) {
            swap(*(fp + 3), *(fp + 2));
            swap(*(fp + 2), *(fp + 1));
        } else if (c == 10) {
            swap(*(fp + 3), *(fp + 2));
            swap(*(fp + 2), *fp);
        } else if (c == 11) {
            swap(*(fp + 3), *(fp + 2));
        } else if (c == 12) {
            swap(*(fp + 2), *fp);
            swap(*(fp + 3), *(fp + 1));
        } else if (c == 13) {
            swap(*(fp + 3), *(fp + 1));
        } else if (c == 14) {
            swap(*(fp + 3), *fp);
        }
    }
};

class endGameGenerator : public PCFinder {
   protected:
    int current_depth = 0;
    int bag_idx = 0;
    std::deque<int> v_pieces;
    std::vector<int> prev_pieces;
    HASH_SET<ull> hash_end_game5, hash_end_game6;

   public:
    endGameGenerator() : PCFinder() {
    }

    void saveHashEndGame(std::string hash5, std::string hash6) {
        container_write(hash_end_game5, hash5);
        container_write(hash_end_game6, hash6);
    }

    void getHashEndGame(HASH_SET<ull>*& hash5, HASH_SET<ull>*& hash6) {
        hash5 = &hash_end_game5;
        hash6 = &hash_end_game6;
    }

    void SetState(int bag_idx, std::deque<int> pieces, std::vector<int> prev_pieces, std::bitset<10> field[4], int depth) {
        this->bag_idx = bag_idx;
        this->v_pieces = pieces;
        this->prev_pieces = prev_pieces;
        InitializeField(field, depth);
        current_depth = depth;
    }

   protected:
    virtual bool Prune(int depth) {
        return hash_set->find(get_field_hash(field + depth * 4)) == hash_set->end();
    }

    inline ull GetEndgameHashD5() {
        ull hash = get_field_hash(field5);
        hash <<= 7;
        uint32_t piece_hash = 0;
        for (int i = 5; i < 10; ++i) {
            piece_hash |= 1 << pieces[i];
        }
        return hash | piece_hash;
    }

    inline ull GetEndgameHashD6() {
        ull hash = get_field_hash(field6);
        hash <<= 7;
        uint32_t piece_hash = 0;
        for (int i = 6; i < 10; ++i) {
            piece_hash |= 1 << pieces[i];
        }
        return hash | piece_hash;
    }

    virtual void FullClear() {
#ifdef DEBUG_PRINT
        ++total;
        if (total % 10000000 == 0) {
            std::cout << total << ' ' << hash_end_game5.size() << ' ' << hash_end_game6.size() << '\n';
        }
#endif
        hash_end_game5.insert(GetEndgameHashD5());
        hash_end_game6.insert(GetEndgameHashD6());
    }

    void InitializeField(std::bitset<10> field[4], int depth) {
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
};

class auxTree : public PCFinder {
   protected:
    auxNode* root = nullptr;
    auxNode* p = nullptr;
    std::deque<int> v_pieces;
    opData ops[5]{0};
    int selections[5]{0};
    auxNode* path[6]{0};
    int lowest = 0;
    HASH_SET<ull>* hash_set_r;

   public:
    explicit auxTree(HASH_SET<ull>* s) : PCFinder(), hash_set_r(s) {
        Initialize();
    }
    auxNode* getRoot() {
        return root;
    }

    void ConstructTree(std::deque<int>& v_pieces) {
        this->v_pieces = v_pieces;
        DfsGeneratePiece(0, 0);
    }

    void DestroyTree() {
        DestroyNode(root);
        Initialize();
    }

    virtual double DfsGeneratePiece(int depth, double alpha) {
        if (hash_set_r->find(get_field_hash(field + depth * 4)) == hash_set_r->end()) return 0;
        ProbContext pr;

        int p1 = v_pieces[0];
        v_pieces.pop_front();
        pieces[depth] = p1;
        selections[depth] = 0;
        DfsTryFitPiece(p1, field + depth * 4, depth, pr);
        int p2 = v_pieces[0];
        v_pieces.pop_front();
        v_pieces.push_front(p1);
        pieces[depth] = p2;
        selections[depth] = 1;
        DfsTryFitPiece(p2, field + depth * 4, depth, pr);
        v_pieces.pop_front();
        v_pieces.push_front(p2);
        v_pieces.push_front(p1);
        return 1;
    }

    virtual void DfsFitPiece(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
        pieces_ori[depth] = pid;
        opData opdata = {pid, x, y};
        ops[depth] = opdata;

        for (int i = 0; i < 4; ++i) {
            int nx = x + PIECE_REPR[pid][i][0];
            (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = true;
        }
        if (depth <= 4) {
            memcpy(fp + 4, fp, sizeof(*fp) * 4);
            fp += 4;
            int c = 0;
            if (*fp == FULL) c += 1;
            if (*(fp + 1) == FULL) c += 2;
            if (*(fp + 2) == FULL) c += 4;
            if (*(fp + 3) == FULL) c += 8;
            if (c == 2) {
                swap(*(fp + 1), *fp);
            } else if (c == 4) {
                swap(*(fp + 2), *(fp + 1));
                swap(*(fp + 1), *fp);
            } else if (c == 5) {
                swap(*(fp + 2), *(fp + 1));
            } else if (c == 6) {
                swap(*(fp + 2), *fp);
            } else if (c == 8) {
                swap(*(fp + 3), *(fp + 2));
                swap(*(fp + 2), *(fp + 1));
                swap(*(fp + 1), *fp);
            } else if (c == 9) {
                swap(*(fp + 3), *(fp + 2));
                swap(*(fp + 2), *(fp + 1));
            } else if (c == 10) {
                swap(*(fp + 3), *(fp + 2));
                swap(*(fp + 2), *fp);
            } else if (c == 11) {
                swap(*(fp + 3), *(fp + 2));
            } else if (c == 12) {
                swap(*(fp + 2), *fp);
                swap(*(fp + 3), *(fp + 1));
            } else if (c == 13) {
                swap(*(fp + 3), *(fp + 1));
            } else if (c == 14) {
                swap(*(fp + 3), *fp);
            }
            if (depth < 4) {
                DfsGeneratePiece(depth + 1, pr.prob);
                lowest = std::min(lowest, depth);
            } else {
                if (hash_set_r->find(get_field_hash(fp)) == hash_set_r->end()) {
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
            fp -= 4;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = x + PIECE_REPR[pid][i][0];
            (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = false;
        }
    }

   private:
    void DestroyNode(auxNode* node) {
        for (int i = 0; i < 2; ++i) {
            for (auto& item : node->v[i]) DestroyNode(item);
        }
        delete node;
    }
    void Initialize() {
        root = new auxNode();
        p = root;
        path[0] = root;
    }
};

class OnlinePCFinder : public PCFinder {
   protected:
    int initial_depth = 0;
    int current_depth = 0;
    int bag_idx = 0;
    std::deque<int> v_pieces;
    std::bitset<7> bag_used[10];

    auxTree* aux_tr;
    auxNode* p = nullptr;

    CONCURRENT_HASH_MAP<ull, bool>* hash_map;
    HASH_SET<ull>*hash_end_game5, *hash_end_game6;

    double* p_alpha = nullptr;

   public:
    OnlinePCFinder() : PCFinder() {
        aux_tr = nullptr;
        hash_map = new CONCURRENT_HASH_MAP<ull, bool>;
        hash_end_game5 = nullptr;
        hash_end_game6 = nullptr;
    }

    virtual ~OnlinePCFinder() {
    }

    void ClearHashMap() {
        hash_map->clear();
    }

    void setHashSets(CONCURRENT_HASH_MAP<ull, bool>* hsmp, HASH_SET<ull>* hseg5, HASH_SET<ull>* hseg6) {
        hash_map = hsmp;
        hash_end_game5 = hseg5;
        hash_end_game6 = hseg6;
    }

    void SetAuxiliaryTree(auxTree* tr, auxNode* node) {
        aux_tr = tr;
        p = node;
    }

    void SetState(int bag_idx, std::deque<int> pieces, std::bitset<7> bag_used, std::bitset<10> field[4], int depth) {
        this->bag_idx = bag_idx;
        this->v_pieces = pieces;
        this->bag_used[depth] = bag_used;
        InitializeField(field, depth);
        current_depth = depth;
        initial_depth = depth;
    }

    void SetAlpha(double* p) {
        p_alpha = p;
    }

    ProbContext CalculateProb(int selection, int x, int y, int ori, int depth) {
        ProbContext pr;
        if (depth < 5) Action(selection, x, y, ori);
        if (selection == 1) std::swap(v_pieces[0], v_pieces[1]);
        v_pieces.pop_front();
        bag_idx = (bag_idx + 1) % 7;
        DfsFitPiece(field + depth * 4, x, y, ori, depth, pr);
        return pr;
    }

    virtual ProbContext FindBestMove() {
        ProbContext pr;
        if (current_depth < 5) {
            DfsChoosePieceLazyFit(current_depth, pr);
        } else {
            DfsChoosePiece(current_depth, pr);
        }
        return pr;
    }

    bool Action(int selection, int x, int y, int ori) {
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

    void ConstructTree() {
        aux_tr = new auxTree(hash_set);
        aux_tr->ConstructTree(v_pieces);
        p = aux_tr->getRoot();
    }

    void DestroyTree() {
        aux_tr->DestroyTree();
    }

    void loadEndGameHashSet(std::string hash5, std::string hash6) {
        hash_end_game5 = new HASH_SET<ull>;
        hash_end_game6 = new HASH_SET<ull>;
        container_read(*hash_end_game5, hash5);
        container_read(*hash_end_game6, hash6);

#ifdef DEBUG_PRINT
        printf("%s size : %ull\n", hash5.c_str(), hash_end_game5->size());
        printf("%s size : %ull\n", hash6.c_str(), hash_end_game6->size());
#endif
    }

   protected:
    inline ull GetStateHash() {
        ull hash = get_field_hash(field + 20);
        for (int i = 0; i < 6; ++i) {
            hash = (hash << 3) | v_pieces[i];
        }
        return hash;
    }

    void InitializeField(std::bitset<10> field[4], int depth) {
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

    inline bool EndgamePruneD5(ull field_hash) {
        for (int i = 0; i < 6; ++i) {
            int piece_hash = 0;
            for (int j = 0; j < 6; ++j) {
                if (j != i) {
                    piece_hash |= 1 << v_pieces[j];
                }
            }
            if (hash_end_game5->find((field_hash << 7) | piece_hash) != hash_end_game5->end()) {
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

    inline bool EndgamePruneD6(ull field_hash) {
        for (int i = 0; i < 5; ++i) {
            int piece_hash = 0;
            for (int j = 0; j < 5; ++j) {
                if (j != i) {
                    piece_hash |= 1 << v_pieces[j];
                }
            }
            if (hash_end_game6->find((field_hash << 7) | piece_hash) != hash_end_game6->end()) {
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

    virtual double DfsGeneratePiece(int depth, double alpha) {
#ifdef DEBUG_PRINT
        ++func_calls[4][depth];
#endif
        if (hash_set->find(get_field_hash(field + depth * 4)) == hash_set->end()) return 0;
        double prob = 0;
        int cnt = 0;
        int cnt2 = 0;
        if (depth + v_pieces.size() >= 11) {
            ProbContext pr;
            if (depth == 4) {
                DfsChoosePieceLazyFit(depth, pr);
                return pr.prob;
            }
            if (depth == 5 && EndgamePruneD5(get_field_hash(field + 20))) return 0;
            if (depth == 6 && EndgamePruneD6(get_field_hash(field + 24))) return 0;
            if (depth == 5) {
                ull hash = GetStateHash();

                bool prob = 0;
                bool is_contain = hash_map->if_contains(hash, [&prob](CONCURRENT_HASH_MAP<ull, bool>::value_type& item) { prob = item.second; });

                if (!is_contain) {
                    DfsChoosePiece(depth, pr);
                    prob = pr.prob;
                    hash_map->try_emplace_l(
                        hash, [](CONCURRENT_HASH_MAP<ull, bool>::value_type& item) {}, prob);
                }
                return prob;
            }
            DfsChoosePiece(depth, pr);
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
                DfsChoosePieceLazyFit(depth, pr);
                bag_used[depth][i] = 0;
                v_pieces.pop_back();
                ++cnt2;
                if (depth == initial_depth + 1) {
                    alpha = *p_alpha;
                }
                if (false) {
                    if (depth <= 3) {
                        if (debug_switch) {
                            if (pr.ori >= 0) {
                                for (int k = 0; k < depth; ++k) std::cout << "   ";
                                for (auto& item : v_pieces) std::cout << PIECENAME[item];
                                std::cout << " | ";
                                std::cout << PIECENAME[i] << ' ' << pr.prob << ' ' << pr.ori << "=========\n";
                            }
                        }
                        if (depth == 3 && PIECENAME[i] == 'I' && pr.ori == 13 && pr.prob == 1) {
                            if (PIECENAME[v_pieces[0]] == 'J' && PIECENAME[v_pieces[5]] == 'S') {
                                std::cout << p->op.ori << ' ' << p->op.x << ' ' << p->op.y << ' ';
                                for (int i = 0; i <= depth; ++i) std::cout << bag_used[i] << ' ';
                                std::cout << "\n**************************\n";
                            }
                        }
                    }
                }
                prob += pr.prob;
                if (prob + cnt - cnt2 < alpha * cnt) break;
            }
        }
        if (cnt > 0) prob /= cnt;
        return prob;
    }

    void DfsChoosePieceLazyFit(int depth, ProbContext& pr) {
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
            DfsFitPiece(field + depth * 4, item->op.x, item->op.y, item->op.ori, depth, pr);
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
            DfsFitPiece(field + depth * 4, item->op.x, item->op.y, item->op.ori, depth, pr);
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

    void DfsChoosePiece(int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
        ++func_calls[1][depth];
#endif
        bag_idx = (bag_idx + 1) % 7;
        int p1 = v_pieces[0];
        v_pieces.pop_front();
        pieces[depth] = p1;
        DfsTryFitPiece(p1, field + depth * 4, depth, pr);
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
        DfsTryFitPiece(p2, field + depth * 4, depth, pr);
        v_pieces.pop_front();
        v_pieces.push_front(p2);
        v_pieces.push_front(p1);
        bag_idx = (bag_idx + 6) % 7;
    }
};

class OnlinePCFinderDepthOne : public OnlinePCFinder {
   private:
    std::vector<std::array<int, 7>> v_possible_drops;

   public:
    OnlinePCFinderDepthOne() : OnlinePCFinder() {}

    void Reset() {
        v_possible_drops.clear();
    }

    const std::vector<std::array<int, 7>>& GetPossibleDrops() {
        return v_possible_drops;
    }

   private:
    virtual void DfsFitPiece(std::bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
        pieces_ori[depth] = pid;
        for (int i = 0; i < 4; ++i) {
            int nx = x + PIECE_REPR[pid][i][0];
            (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = true;
        }
        std::bitset<10> fp_new[4];
        memcpy(fp_new, fp, sizeof(*fp) * 4);
        clear_lines(fp_new);
        std::array<int, 7> drop_data = {fp_new[0].to_ulong(), fp_new[1].to_ulong(), fp_new[2].to_ulong(), fp_new[3].to_ulong(), x, y, pid};
        v_possible_drops.push_back(drop_data);

        for (int i = 0; i < 4; ++i) {
            int nx = x + PIECE_REPR[pid][i][0];
            (*(fp + y + PIECE_REPR[pid][i][1]))[nx] = false;
        }
    }
};

class OnlinePCFinderParallel : public OnlinePCFinder {
   private:
    OnlinePCFinderDepthOne finder_d1;

   public:
    explicit OnlinePCFinderParallel() : OnlinePCFinder() {
    }
    virtual ~OnlinePCFinderParallel() {
        if (hash_set) delete hash_set;
        if (hash_map) delete hash_map;
        if (hash_end_game5) delete hash_end_game5;
        if (hash_end_game6) delete hash_end_game6;
    }

    virtual ProbContext FindBestMove() {
        ProbContext pr;
        finder_d1.Reset();
        finder_d1.setHashSet(hash_set);
        finder_d1.setHashSets(hash_map, hash_end_game5, hash_end_game6);
        finder_d1.SetAuxiliaryTree(aux_tr, p);
        finder_d1.SetState(bag_idx, v_pieces, bag_used[current_depth], field + current_depth * 4, current_depth);
        finder_d1.FindBestMove();
        std::vector<std::array<int, 7>> v_possible_drops = finder_d1.GetPossibleDrops();
        int v_size = v_possible_drops.size();
        std::vector<double> v_pr(v_size);
        double* p_alpha = new double(0);
#pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < v_size; ++i) {
            const auto& item = v_possible_drops[i];
            OnlinePCFinder finder;
            finder.setHashSet(hash_set);
            finder.setHashSets(hash_map, hash_end_game5, hash_end_game6);
            finder.SetAuxiliaryTree(aux_tr, p);
            finder.SetState(bag_idx, v_pieces, bag_used[current_depth], field + current_depth * 4, current_depth);
            finder.SetAlpha(p_alpha);
            int selection = (PIECEMAP[item[6]] == v_pieces[0] ? 0 : 1);
            ProbContext pr = finder.CalculateProb(selection, item[4], item[5], item[6], current_depth);
            v_pr[i] = pr.prob;
#pragma omp critical
            *p_alpha = std::max(*p_alpha, pr.prob);
        }
        delete p_alpha;
        ClearHashMap();
        ProbContext result;
        for (int i = 0; i < v_size; ++i) {
            if (v_pr[i] > result.prob) {
                auto& item = v_possible_drops[i];
                result = {v_pr[i], item[4], item[5], item[6]};
            }
        }
        return result;
    }
};

void benchmark() {
    PCFinder finder;
    finder.SetFirstPiece(3, 3);
    finder.Start();
}

void generate_hash(int first_bag_idx) {
    std::vector<PCFinder> v_finder(7);
#pragma omp parallel for
    for (int i = 0; i < 7; ++i) {
        v_finder[i].SetFirstPiece(i, first_bag_idx);
        v_finder[i].setHashMode(true);
        v_finder[i].Start();
    }
    HASH_SET<ull> aggr_set;
    for (int i = 0; i < 7; ++i) {
        const HASH_SET<ull>* hs = v_finder[i].getHashSet();
        std::cout << hs->size() << ' ';
        container_write(*hs, "field_hash" + std::to_string(first_bag_idx) + std::to_string(i) + ".dat");
        for (auto& item : *hs) {
            aggr_set.insert(item);
        }
        delete hs;
    }
    std::cout << aggr_set.size() << '\n';
    container_write(aggr_set, "field_hash" + std::to_string(first_bag_idx) + ".dat");
}

void solve() {
    OnlinePCFinder finder;
    Simulator simulator;
    finder.loadHashSet("field_hash.dat");
    finder.loadEndGameHashSet("hash_end_game5.dat", "hash_end_game6.dat");
    simulator.Initialize();
    for (int i = 0; i < 9; ++i) {
        std::deque<int> pieces;
        std::bitset<7> bag_used;
        int bag_idx;
        std::bitset<10> field[4];
        simulator.GetState(WINDOW_SIZE, field, pieces, bag_idx, bag_used);
        finder.SetState(bag_idx, pieces, bag_used, field, i);
        if (i == 0) finder.ConstructTree();
        ProbContext pr = finder.FindBestMove();
        for (auto& item : pieces) {
            std::cout << PIECENAME[item];
        }
        std::cout << '\n';
        printf("%.2f%% Winning | Piece Code %d | x=%d y=%d\n", pr.prob * 100, pr.ori, pr.x, pr.y);
        if (pr.ori < 0) {
            std::cout << "FAILED\n";
            break;
        }
        int selection = 0;
        if (PIECEMAP[pr.ori] == pieces[1]) selection = 1;
        simulator.Action(selection, pr.x, pr.y, pr.ori);
        if (i < 5) finder.Action(selection, pr.x, pr.y, pr.ori);

#ifdef DEBUG_PRINT
        std::cout << "aux\tdfs2\ttryfit\tfit\tdfs\n";
        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 5; ++y) std::cout << func_calls[y][x] << '\t';
            std::cout << '\n';
        }
        for (int x = 0; x < 10; ++x) std::cout << depth_cnt[x] << ' ';
        std::cout << '\n';
        std::cout << "PRUNE5: " << prune5 << " / " << not_prune5 << '\n';
        std::cout << "PRUNE6: " << prune6 << " / " << not_prune6 << '\n';
#endif
    }
}

void solve_test() {
    OnlinePCFinder finder;
    Simulator simulator;
    finder.loadHashSet("field_hash.dat");
    simulator.Initialize();
    std::deque<int> pieces = {6, 0, 5, 3, 2, 0, 1};
    std::bitset<7> bag_used = 0b1111000;
    int bag_idx = 3;
    std::bitset<10> field[4] = {120, 252, 68, 0};

    finder.SetState(bag_idx, pieces, bag_used, field, 3);
    ProbContext pr = finder.FindBestMove();
    for (auto& item : pieces) {
        std::cout << PIECENAME[item];
    }
    std::cout << '\n';
    printf("%.2f%% Winning | Piece Code %d | x=%d y=%d\n", pr.prob * 100, pr.ori, pr.x, pr.y);
    if (pr.ori < 0) {
        std::cout << "FAILED\n";
    }
}

void generate_endgame(int first_bag_idx) {
    HASH_SET<ull> aggr_set5, aggr_set6;
    HASH_SET<ull>*p_set5, *p_set6;
    endGameGenerator generator;
    generator.loadHashSet("field_hash" + std::to_string(first_bag_idx) + ".dat");
    for (int i = 0; i < 7; ++i) {
        generator.SetFirstPiece(i, first_bag_idx);
        generator.Start();
        generator.getHashEndGame(p_set5, p_set6);
        for (auto& item : *p_set5) aggr_set5.insert(item);
        for (auto& item : *p_set6) aggr_set6.insert(item);
        p_set5->clear();
        p_set6->clear();
    }
    container_write(aggr_set5, "hash_end_game5_" + std::to_string(first_bag_idx) + ".dat");
    container_write(aggr_set6, "hash_end_game6_" + std::to_string(first_bag_idx) + ".dat");
}

void refactor_generate_endgame(int first_bag_idx) {
    EndgameGenerator generator("field_hash" + std::to_string(first_bag_idx) + ".dat");
    generator.Generate(first_bag_idx, "hash_end_game5_" + std::to_string(first_bag_idx) + ".dat", "hash_end_game6_" + std::to_string(first_bag_idx) + ".dat");
}

void refactor_solve_parallel(int total_tests) {
    TIME_BENCHMARK
    SolverParallel* solvers[7];
    Simulator simulator;

#pragma omp parallel for
    for (int i = 0; i < 7; ++i) {
        solvers[i] = new SolverParallel("field_hash" + std::to_string(i) + ".dat", "hash_end_game5_" + std::to_string(i) + ".dat", "hash_end_game6_" + std::to_string(i) + ".dat");
    }
    simulator.Initialize();
    int failed_tests = 0;
    TIME_START;
    for (int x = 0; x < total_tests; ++x) {
        int pc_idx = simulator.GetPCIndex();
        for (int i = 0; i < 10; ++i) {
            std::deque<int> pieces;
            std::bitset<7> bag_used;
            int bag_idx;
            std::bitset<10> field[4];
            simulator.GetState(WINDOW_SIZE, field, pieces, bag_idx, bag_used);
            solvers[pc_idx]->SetState(bag_idx, pieces, bag_used, field, i);
            if (i == 0) solvers[pc_idx]->ConstructTree();
            ProbContext pr = solvers[pc_idx]->FindBestMove();
#ifdef DEBUG_PRINT
            for (auto& item : pieces) {
                std::cout << PIECENAME[item];
            }
            std::cout << '\n';
            std::printf("%.2f%% Winning | Piece Code %d | x=%d y=%d\n", pr.prob * 100, pr.ori, pr.x, pr.y);
#endif
            if (pr.ori < 0) {
                std::cout << "FAILED\n";
                ++failed_tests;
                break;
            }
            int selection = 0;
            if (PIECEMAP[pr.ori] == pieces[1]) selection = 1;
            simulator.Action(selection, pr.x, pr.y, pr.ori);
            if (i < 5) solvers[pc_idx]->Action(selection, pr.x, pr.y, pr.ori);
        }
        solvers[pc_idx]->DestroyTree();
        simulator.SoftReset();
    }
    TIME_END;
    std::cout << "\n========= Summary =========\n";
    std::printf("Success rate: %.2f%% (%d/%d)", 100.0 * (total_tests - failed_tests) / total_tests, total_tests - failed_tests, total_tests);
    std::cout << "\n===========================\n";
}

int main() {
#ifdef GENERATE_HASH
    for (int i = 0; i < 7; ++i) {
        generate_hash(i);
    }
#endif

#ifdef GENERATE_ENDGAME
    // #pragma omp parallel for
    for (int i = 0; i < 7; ++i) {
        generate_endgame(i);
    }
#endif

#ifdef ONLINE_SOLVE
    refactor_solve_parallel(2);
#endif
}
