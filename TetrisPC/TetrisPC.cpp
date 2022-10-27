#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <assert.h>

#include "gtl/phmap.hpp"

#include "writer.h"
#include "definition.h"
#include "utils.h"
#include "simulator.h"
#include "auxtree.h"

using namespace std;

// #define GENERATE_HASH
#define ONLINE_SOLVE
// #define GENERATE_ENDGAME

// #define DEBUG_PRINT

int depth_cnt[11] = { 0 };
int hit = 0;
int not_hit = 0;
int can_fc = 0;
int cant_fc = 0;
int prune5 = 0;
int prune6 = 0;
int not_prune5 = 0;
int not_prune6 = 0;
int func_calls[5][10];

template<class T>
using HASH_SET = gtl::flat_hash_set<T>;

template<class T, class V>
using HASH_MAP = gtl::flat_hash_map<T, V>;

template<class T, class V>
using CONCURRENT_HASH_MAP = gtl::parallel_flat_hash_map<T, V>;

struct ProbContext {
	double prob = 0;
	int x = 0;
	int y = 0;
	int ori = -1;
};

class PCFinder {
protected:
	int used_count[7];
	bitset<10> field[40];
	bitset<10> field5[4];
	bitset<10> field6[4];
	int col[10][10];
	int pieces[10];
	int pieces_ori[10];
	int total = 0;

	int first_piece = -1;

	bool is_output = false;
	Writer writer;

	deque<opData> q;
	int go_back_cnt = 0;

	HASH_SET<ull> *hash_set;
	bool hash_gen = false;

public:
	PCFinder() {
		hash_set = new HASH_SET<ull>;
		memset(used_count, 0, sizeof(used_count));
		memset(field, 0, sizeof(field));
		memset(col, 0, sizeof(col));
	}
	virtual ~PCFinder() {
		delete hash_set;
	}

	PCFinder(const PCFinder&) = default;
	PCFinder& operator=(const PCFinder&) = default;
	
	void loadHashSet(string filename) {
		HashIO hio;
		hio.read(*hash_set, filename);
		hash_set->insert(0);
#ifdef DEBUG_PRINT
		printf("%s size : %ull\n", filename.c_str(), hash_set.size());
#endif
	}

	void setHashSet(HASH_SET<ull>* hs) {
		hash_set = hs;
	}

	void setOutputFile(string filename) {
		writer.openFile(filename);
		is_output = true;
	}

	void setFirstPiece(int i) {
		first_piece = i;
	}
	void setHashMode(bool gen) {
		hash_gen = gen;
	}

	void start() {
		for (int i = 0; i < 7; ++i) {
			++used_count[i];
		}
		dfs(0, 0);
		writer.closeFile();
	}
	const HASH_SET<ull>* getHashSet() {
		return hash_set;
	}

protected:

	inline bitset<10> getb(bitset<10>* fp, int layer, int pid) {
		return *(fp + layer + PIECE_REPR[pid][0][1]) >> PIECE_REPR[pid][0][0]
			| *(fp + layer + PIECE_REPR[pid][1][1]) >> PIECE_REPR[pid][1][0]
			| *(fp + layer + PIECE_REPR[pid][2][1]) >> PIECE_REPR[pid][2][0]
			| *(fp + layer + PIECE_REPR[pid][3][1]) >> PIECE_REPR[pid][3][0];
	}

	void show_field(int depth) {
		for (int i = 3; i >= 0; --i) {
			for (int j = 9; j >= 0; --j) {
				cout << (field[depth * 4 + i][j] ? " 0" : "  ");
			}
			cout << '\n';
		}
		cout << endl;
	}

	virtual bool prune(int depth) {
		int empty_count = 0;
		for (int i = 0; i < 9; ++i) {
			empty_count += 4 - col[depth][i];
			if (col[depth][i] == 4 && (empty_count % 4)) return true;
		}
		return false;
	}

	inline ull get_field_hash(bitset<10>* fp) {
		return (((fp->to_ullong() << 10) + (fp + 1)->to_ullong() << 10) + (fp + 2)->to_ullong() << 10) + (fp + 3)->to_ullong();
	}

	inline ll get_field_hash_full(bitset<10>* fp) {
		ll h = ((((fp->to_ullong() << 16) + (fp + 1)->to_ullong() << 16) + (fp + 2)->to_ullong() << 16) + (fp + 3)->to_ullong()) << 3;
		return h | 0b1110000000000111111000000000011111100000000001111110000000000111;
	}

	virtual double dfs(int depth, double alpha) {
		ProbContext pr;
		if (depth == 0 && first_piece >= 0) {
			pieces[0] = first_piece;
			--used_count[first_piece];
			try_fit(first_piece, field, 0, pr);
			return 1;
		}
		if (prune(depth)) return 1;
		if (depth == 6) {
			for (int i = 0; i < 7; ++i) ++used_count[i];
		}
		
		for (int i = 0; i < 7; ++i) {
			if (!used_count[i]) continue;
			pieces[depth] = i;
			--used_count[i];
			try_fit(i, field + depth * 4, depth, pr);
			++used_count[i];
		}
		if (depth == 6) {
			for (int i = 0; i < 7; ++i) --used_count[i];
		}
		return 1;
	}


	void try_fit(int piece, bitset<10>* fp, int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
		++func_calls[2][depth];
#endif
		ll h = get_field_hash_full(fp);
		bitset<64> bits;
		if (piece == 0) { // I
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
		}
		else if (piece == 1) { // O
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
		}
		else if (piece == 2) { // T
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
		}
		else if (piece == 3) { // S
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
		}
		else if (piece == 4) { // Z
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
		}
		else if (piece == 5) { // J
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
		}
		else { // L
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

	virtual void fc() {
		if (hash_gen) {
			for (int i = 0; i < 9; ++i) {
				hash_set->insert(get_field_hash(field + i * 4));
			}
		}
		if (is_output) {
			writeQueue();
		}


#ifdef DEBUG_PRINT
		++total;
		if (total % 10000 == 0) {
			cout << first_piece << "  :  ";
			ll idx = 0;
			for (int i = 0; i < 10; ++i) {
				idx *= 19;
				idx += pieces_ori[i];
				cout << pieces[i];
			}
			cout << "    " << total << "   " << not_hit << ' ' << hit << '\n';
		}
#endif
	}

	virtual void fit(bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
#ifdef DEBUG_PRINT
		++func_calls[3][depth];
		++depth_cnt[depth];
#endif
		pieces_ori[depth] = pid;
		if (is_output) {
			opData opdata = { pid, x, y };
			q.push_back(opdata);
		}
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
			fp += 4;
			int c = 0;
			if (*fp == FULL) c += 1;
			if (*(fp + 1) == FULL) c += 2;
			if (*(fp + 2) == FULL) c += 4;
			if (*(fp + 3) == FULL) c += 8;
			if (c == 2) {
				swap(*(fp + 1), *fp);
			}
			else if (c == 4) {
				swap(*(fp + 2), *(fp + 1));
				swap(*(fp + 1), *fp);
			}
			else if (c == 5) {
				swap(*(fp + 2), *(fp + 1));
			}
			else if (c == 6) {
				swap(*(fp + 2), *fp);
			}
			else if (c == 8) {
				swap(*(fp + 3), *(fp + 2));
				swap(*(fp + 2), *(fp + 1));
				swap(*(fp + 1), *fp);
			}
			else if (c == 9) {
				swap(*(fp + 3), *(fp + 2));
				swap(*(fp + 2), *(fp + 1));
			}
			else if (c == 10) {
				swap(*(fp + 3), *(fp + 2));
				swap(*(fp + 2), *fp);
			}
			else if (c == 11) {
				swap(*(fp + 3), *(fp + 2));
			}
			else if (c == 12) {
				swap(*(fp + 2), *fp);
				swap(*(fp + 3), *(fp + 1));
			}
			else if (c == 13) {
				swap(*(fp + 3), *(fp + 1));
			}
			else if (c == 14) {
				swap(*(fp + 3), *fp);
			}
#ifdef GENERATE_ENDGAME
			if (depth == 4) memcpy(field5, fp, sizeof(*fp) * 4);
			if (depth == 5) memcpy(field6, fp, sizeof(*fp) * 4);
#endif  
			prob = dfs(depth + 1, pr.prob);
			fp -= 4;
		}
		else {
			fc();
		}

		for (int i = 0; i < 4; ++i) {
			int nx = x + PIECE_REPR[pid][i][0];
			(*(fp + y + PIECE_REPR[pid][i][1]))[nx] = false;
#ifndef ONLINE_SOLVE
			--col[depth][nx];
#endif
		}
		if (is_output) {
			if (q.empty()) {
				++go_back_cnt;
			}
			else {
				q.pop_back();
			}
		}

#ifdef DEBUG_PRINT
		if (depth == 0) cout << prob << ' ' << pid << ' ' << x << ' ' << y << '\n';
#endif
		if (prob > pr.prob) {
			pr = { prob, x, y, pid };
		}
	}

	void writeQueue() {
		if (!is_output) {
			go_back_cnt = 0;
			q.clear();
			return;
		}
		if (go_back_cnt > 0) {
			writer.writeCount(go_back_cnt);
			go_back_cnt = 0;
		}
		while (!q.empty()) {
			writer.writeMove(q.front());
			q.pop_front();
		}
		writer.writeCount(255);
	}
};

class endGameGenerator : public PCFinder {
protected:
	int current_depth = 0;
	int bag_idx = 0;
	deque<int> v_pieces;
	vector<int> prev_pieces;
	HASH_SET<ull> hash_end_game5, hash_end_game6;
public:
	endGameGenerator() : PCFinder() {
		
	}

	void saveHashEndGame(string hash5, string hash6) {
		HashIO hio;
		hio.write(hash_end_game5, hash5);
		hio.write(hash_end_game6, hash6);
	}

	void setState(int bag_idx, deque<int> pieces, vector<int> prev_pieces, bitset<10> field[4], int depth) {
		this->bag_idx = bag_idx;
		this->v_pieces = pieces;
		this->prev_pieces = prev_pieces;
		init_field(field, depth);
		current_depth = depth;
	}

protected:
	virtual bool prune(int depth) {
		return hash_set->find(get_field_hash(field + depth * 4)) == hash_set->end();
	}

	inline ull get_end_game_hash5() {
		ull hash = get_field_hash(field5);
		hash <<= 7;
		uint32_t piece_hash = 0;
		for (int i = 5; i < 10; ++i) {
			piece_hash |= 1 << pieces[i];
		}
		return hash | piece_hash;
	}

	inline ull get_end_game_hash6() {
		ull hash = get_field_hash(field6);
		hash <<= 7;
		uint32_t piece_hash = 0;
		for (int i = 6; i < 10; ++i) {
			piece_hash |= 1 << pieces[i];
		}
		return hash | piece_hash;
	}
	
	virtual void fc() {
		++total;
		if (total % 100000 == 0) cout << total << ' ' << hash_end_game5.size() << ' ' << hash_end_game6.size() << '\n';
		hash_end_game5.insert(get_end_game_hash5());
		hash_end_game6.insert(get_end_game_hash6());
	}

	void init_field(bitset<10> field[4], int depth) {
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


	void dfs2(int depth) {
		ProbContext pr;
		bag_idx = (bag_idx + 1) % 7;
		int p1 = v_pieces[0];
		v_pieces.pop_front();
		pieces[depth] = p1;
		prev_pieces.push_back(p1);
		try_fit(p1, field + depth * 4, depth, pr);
		int p2 = v_pieces[0];
		v_pieces.pop_front();
		v_pieces.push_front(p1);
		pieces[depth] = p2;
		prev_pieces[prev_pieces.size() - 1] = p2;
		try_fit(p2, field + depth * 4, depth, pr);
		prev_pieces.pop_back();
		v_pieces.pop_front();
		v_pieces.push_front(p2);
		v_pieces.push_front(p1);
		bag_idx = (bag_idx + 6) % 7;
	}

};

class auxTree : public PCFinder {
protected:
	auxNode* root = nullptr;
	auxNode* p = nullptr;
	deque<int> v_pieces;
	opData ops[5]{ 0 };
	int selections[5]{ 0 };
	auxNode* path[6]{ 0 };
	int lowest = 0;
	HASH_SET<ull>* hash_set_r;
public:
	explicit auxTree(HASH_SET<ull>* s) : PCFinder(), hash_set_r(s) {
		root = new auxNode();
		p = root;
		path[0] = root;
	}
	auxNode* getRoot() {
		return root;
	}
	void constructTree(deque<int>& v_pieces) {
		this->v_pieces = v_pieces;
		dfs(0, 0);
	}
	virtual double dfs(int depth, double alpha) {
		if (hash_set_r->find(get_field_hash(field + depth * 4)) == hash_set_r->end()) return 0;
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

	virtual void fit(bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
		pieces_ori[depth] = pid;
		opData opdata = { pid, x, y };
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
			}
			else if (c == 4) {
				swap(*(fp + 2), *(fp + 1));
				swap(*(fp + 1), *fp);
			}
			else if (c == 5) {
				swap(*(fp + 2), *(fp + 1));
			}
			else if (c == 6) {
				swap(*(fp + 2), *fp);
			}
			else if (c == 8) {
				swap(*(fp + 3), *(fp + 2));
				swap(*(fp + 2), *(fp + 1));
				swap(*(fp + 1), *fp);
			}
			else if (c == 9) {
				swap(*(fp + 3), *(fp + 2));
				swap(*(fp + 2), *(fp + 1));
			}
			else if (c == 10) {
				swap(*(fp + 3), *(fp + 2));
				swap(*(fp + 2), *fp);
			}
			else if (c == 11) {
				swap(*(fp + 3), *(fp + 2));
			}
			else if (c == 12) {
				swap(*(fp + 2), *fp);
				swap(*(fp + 3), *(fp + 1));
			}
			else if (c == 13) {
				swap(*(fp + 3), *(fp + 1));
			}
			else if (c == 14) {
				swap(*(fp + 3), *fp);
			}
			if (depth < 4) {
				dfs(depth + 1, pr.prob);
				lowest = min(lowest, depth);
			}
			else {
				if (hash_set_r->find(get_field_hash(fp)) == hash_set_r->end()) {
					lowest = min(lowest, depth);
				}
				else {
					// construct auxiliary tree
					p = path[lowest];
					for (int i = lowest; i < 5; ++i) {
						auxNode* new_n = new auxNode{ ops[i] };
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

};


class OnlinePCFinder : public PCFinder {
protected:
	int current_depth = 0;
	int bag_idx = 0;
	deque<int> v_pieces;
	bitset<7> bag_used[10];

	auxTree* aux_tr;
	auxNode* p = nullptr;

	CONCURRENT_HASH_MAP<ull, bool> *hash_map;
	HASH_SET<ull> *hash_end_game5, *hash_end_game6;

public:
	OnlinePCFinder() : PCFinder() {
		hash_map = new CONCURRENT_HASH_MAP<ull, bool>;
	}

	virtual ~OnlinePCFinder() {
		delete hash_map;
		delete hash_end_game5;
		delete hash_end_game6;
	}

	void setHashSets(CONCURRENT_HASH_MAP<ull, bool> *hsmp, HASH_SET<ull> *hseg5, HASH_SET<ull> *hseg6) {
		hash_map = hsmp;
		hash_end_game5 = hseg5;
		hash_end_game6 = hseg6;
	}

	void setAuxTree(auxTree* tr) {
		aux_tr = tr;
	}

	void setState(int bag_idx, deque<int> pieces, bitset<7> bag_used, bitset<10> field[4], int depth) {
		this->bag_idx = bag_idx;
		this->v_pieces = pieces;
		this->bag_used[depth] = bag_used;
		init_field(field, depth);
		current_depth = depth;
	}


	ProbContext calculateProb(int selection, int x, int y, int ori, int depth) {
		ProbContext pr;
		if (depth < 5) action(selection, x, y, ori);
		if (selection == 1) swap(v_pieces[0], v_pieces[1]);
		v_pieces.pop_front();
		bag_idx = (bag_idx + 1) % 7;
		fit(field + depth * 4, x, y, ori, depth, pr);
		return pr;
	}

	virtual ProbContext findBestMove() {
		ProbContext pr;
		if (current_depth < 5) {
			dfs_aux(current_depth, pr);
		}
		else {
			dfs2(current_depth, pr);
		}
		return pr;
	}

	bool action(int selection, int x, int y, int ori) {
		bool succ = false;
		auto& v = p->v[selection];
		for (auto& item : v) {
			if (item->op.x == x && item->op.y == y && item->op.ori == ori) {
				p = item;
				succ = true;
				break;
			}
		}
		return succ;
	}

	

	void constructTree() {
		aux_tr = new auxTree(hash_set);
		aux_tr->constructTree(v_pieces);
		p = aux_tr->getRoot();
	}

	void loadEndGameHashSet(string hash5, string hash6) {
		HashIO hio;
		hash_end_game5 = new HASH_SET<ull>;
		hash_end_game6 = new HASH_SET<ull>;
		hio.read(*hash_end_game5, hash5);
		hio.read(*hash_end_game6, hash6);

#ifdef DEBUG_PRINT
		printf("%s size : %ull\n", hash5.c_str(), hash_end_game5.size());
		printf("%s size : %ull\n", hash6.c_str(), hash_end_game6.size());
#endif
	}

protected:
	inline ull get_state_hash() {
		ull hash = get_field_hash(field + 20);
		for (int i = 0; i < 6; ++i) {
			hash = (hash << 3) | v_pieces[i];
		}
		return hash;
	}

	void init_field(bitset<10> field[4], int depth) {
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

	inline bool end_game_prune5(ull field_hash) {
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

	inline bool end_game_prune6(ull field_hash) {
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

	virtual double dfs(int depth, double alpha) {
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
				dfs_aux(depth, pr);
				return pr.prob;
			}
			if (depth == 5 && end_game_prune5(get_field_hash(field + 20))) return 0;
			if (depth == 6 && end_game_prune6(get_field_hash(field + 24))) return 0;
			if (depth == 5) {
				ull hash = get_state_hash();
				if (hash_map->find(hash) == hash_map->end()) {
					dfs2(depth, pr);
					(*hash_map)[hash] = pr.prob;
				}
				else {
					return (*hash_map)[hash];
				}
			}
			dfs2(depth, pr);
			return pr.prob;
		}
		for (int i = 0; i < 7; ++i) {
			if (!bag_used[depth][i]) ++cnt;
		}
		if (cnt == 0) {
			cnt = 7;
			bag_used[depth] = 0;
		}
		bag_used[depth + 1] = bag_used[depth];
		for (int i = 0; i < 7; ++i) {
			if (!bag_used[depth][i]) {
				ProbContext pr;
				v_pieces.push_back(i);
				bag_used[depth + 1][i] = 1;
				dfs_aux(depth, pr);
				bag_used[depth + 1][i] = 0;
				v_pieces.pop_back();
				++cnt2;
				prob += pr.prob;
				if (prob + cnt - cnt2 < alpha * cnt) break;
			}
		}
		if (cnt > 0) prob /= cnt;
		return prob;
	}

	void dfs_aux(int depth, ProbContext& pr) {
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

	void dfs2(int depth, ProbContext& pr) {
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
};

class OnlinePCFinderDepthOne : public OnlinePCFinder {
private:
	vector<array<int, 7>> v_possible_drops;

public:
	OnlinePCFinderDepthOne() : OnlinePCFinder() {}

	const vector<array<int, 7>>& getPossibleDrops() {
		return v_possible_drops;
	}

private:
	virtual void fit(bitset<10>* fp, int x, int y, int pid, int depth, ProbContext& pr) {
		pieces_ori[depth] = pid;
		if (is_output) {
			opData opdata = { pid, x, y };
			q.push_back(opdata);
		}
		for (int i = 0; i < 4; ++i) {
			int nx = x + PIECE_REPR[pid][i][0];
			(*(fp + y + PIECE_REPR[pid][i][1]))[nx] = true;
		}
		bitset<10> fp_new[4];
		memcpy(fp_new, fp, sizeof(*fp) * 4);
		fp = fp_new;
		int c = 0;
		if (*fp == FULL) c += 1;
		if (*(fp + 1) == FULL) c += 2;
		if (*(fp + 2) == FULL) c += 4;
		if (*(fp + 3) == FULL) c += 8;
		if (c == 2) {
			swap(*(fp + 1), *fp);
		}
		else if (c == 4) {
			swap(*(fp + 2), *(fp + 1));
			swap(*(fp + 1), *fp);
		}
		else if (c == 5) {
			swap(*(fp + 2), *(fp + 1));
		}
		else if (c == 6) {
			swap(*(fp + 2), *fp);
		}
		else if (c == 8) {
			swap(*(fp + 3), *(fp + 2));
			swap(*(fp + 2), *(fp + 1));
			swap(*(fp + 1), *fp);
		}
		else if (c == 9) {
			swap(*(fp + 3), *(fp + 2));
			swap(*(fp + 2), *(fp + 1));
		}
		else if (c == 10) {
			swap(*(fp + 3), *(fp + 2));
			swap(*(fp + 2), *fp);
		}
		else if (c == 11) {
			swap(*(fp + 3), *(fp + 2));
		}
		else if (c == 12) {
			swap(*(fp + 2), *fp);
			swap(*(fp + 3), *(fp + 1));
		}
		else if (c == 13) {
			swap(*(fp + 3), *(fp + 1));
		}
		else if (c == 14) {
			swap(*(fp + 3), *fp);
		}
		array<int, 7> drop_data = { fp_new[0].to_ulong(), fp_new[1].to_ulong(), fp_new[2].to_ulong(), fp_new[3].to_ulong(), x, y, pid };
		v_possible_drops.push_back(drop_data);
	}

};

class OnlinePCFinderParallel : public OnlinePCFinder {
private:
	OnlinePCFinderDepthOne finder_d1;
public:
	explicit OnlinePCFinderParallel() : OnlinePCFinder() {
		
	}

	virtual ProbContext findBestMove() {
		ProbContext pr;
		finder_d1.setHashSet(hash_set);
		finder_d1.setHashSets(hash_map, hash_end_game5, hash_end_game6);
		finder_d1.setAuxTree(aux_tr);
		finder_d1.setState(bag_idx, v_pieces, bag_used[current_depth], field, current_depth);
		finder_d1.findBestMove();
		vector<array<int, 7>> v_possible_drops = finder_d1.getPossibleDrops();
		vector<double> v_pr(v_possible_drops.size());
#pragma omp parallel for schedule (dynamic, 1)
		for (int i = 0; i < v_possible_drops.size(); ++i) {
			const auto& item = v_possible_drops[i];
			OnlinePCFinder finder;
			finder.setHashSet(hash_set);
			finder.setHashSets(hash_map, hash_end_game5, hash_end_game6);
			finder.setAuxTree(aux_tr);
			bitset<10> field_new[4] = { item[0], item[1], item[2], item[3] };
			finder.setState(bag_idx, v_pieces, bag_used[current_depth], field_new, current_depth);
			int selection = (PIECEMAP[item[6]] == v_pieces[0] ? 0 : 1);
			ProbContext pr = finder.calculateProb(selection, item[4], item[5], item[6], current_depth);
			v_pr[i] = pr.prob;
		}
		ProbContext result;
		for (int i = 0; i < v_pr.size(); ++i) {
			if (v_pr[i] > result.prob) {
				auto& item = v_possible_drops[i];
				result = { v_pr[i], item[4], item[5], item[6] };
			}
		}
		return result;
	}
private:
	

};




void benchmark() {
	PCFinder finder;
	finder.setFirstPiece(3);
	finder.start();
}

void generate_solution() {
	vector<PCFinder> v_finder(7);
#pragma omp parallel for
	for (int i = 0; i < 7; ++i) {
		v_finder[i].setOutputFile(to_string(i) + ".dat");
		v_finder[i].setFirstPiece(i);
		v_finder[i].start();
	}
}

void generate_hash() {
	vector<PCFinder> v_finder(7);
#pragma omp parallel for
	for (int i = 0; i < 7; ++i) {
		v_finder[i].setFirstPiece(i);
		v_finder[i].setHashMode(true);
		v_finder[i].start();
	}
	HASH_SET<ull> aggr_set;
	for (int i = 0; i < 7; ++i) {
		const HASH_SET<ull>* hs = v_finder[i].getHashSet();
		for (auto& item : *hs) {
			aggr_set.insert(item);
		}
	}
	HashIO hio;
	hio.write(aggr_set, "field_hash.dat");
}

void solve() {
	OnlinePCFinder finder;
	Simulator simulator;
	finder.loadHashSet("field_hash.dat");
	finder.loadEndGameHashSet("hash_end_game5.dat", "hash_end_game6.dat");
	simulator.initialize();
	for (int i = 0; i < 9; ++i) {
		deque<int> pieces;
		bitset<7> bag_used;
		int bag_idx;
		bitset<10> field[4];
		simulator.getState(WINDOW_SIZE, field, pieces, bag_idx, bag_used);
		finder.setState(bag_idx, pieces, bag_used, field, i);
		if (i == 0) finder.constructTree();
		ProbContext pr = finder.findBestMove();
		for (auto& item : pieces) {
			cout << PIECENAME[item];
		}
		cout << '\n';
		printf("%.2f%% Winning | Piece Code %d | x=%d y=%d\n", pr.prob * 100, pr.ori, pr.x, pr.y);
		if (pr.ori < 0) {
			cout << "FAILED\n";
			break;
		}
		int selection = 0;
		if (PIECEMAP[pr.ori] == pieces[1]) selection = 1;
		simulator.action(selection, pr.x, pr.y, pr.ori);
		if (i < 5) finder.action(selection, pr.x, pr.y, pr.ori);

#ifdef DEBUG_PRINT
		cout << "aux\tdfs2\ttryfit\tfit\tdfs\n";
		for (int x = 0; x < 10; ++x) {
			for (int y = 0; y < 5; ++y) cout << func_calls[y][x] << '\t'; cout << '\n';
		}
		for (int x = 0; x < 10; ++x) cout << depth_cnt[x] << ' '; cout << '\n';
		cout << "PRUNE5: " << prune5 << " / " << not_prune5 << '\n';
		cout << "PRUNE6: " << prune6 << " / " << not_prune6 << '\n';
#endif

	}
	
}

void solve_parallel() {
	OnlinePCFinderParallel finder_parallel;
	Simulator simulator;

	finder_parallel.loadHashSet("field_hash.dat");
	finder_parallel.loadEndGameHashSet("hash_end_game5.dat", "hash_end_game6.dat");
	simulator.initialize();

	for (int i = 0; i < 9; ++i) {
		deque<int> pieces;
		bitset<7> bag_used;
		int bag_idx;
		bitset<10> field[4];
		simulator.getState(WINDOW_SIZE, field, pieces, bag_idx, bag_used);
		finder_parallel.setState(bag_idx, pieces, bag_used, field, i);
		if (i == 0) finder_parallel.constructTree();
		ProbContext pr = finder_parallel.findBestMove();
		for (auto& item : pieces) {
			cout << PIECENAME[item];
		}
		cout << '\n';
		printf("%.2f%% Winning | Piece Code %d | x=%d y=%d\n", pr.prob * 100, pr.ori, pr.x, pr.y);
		if (pr.ori < 0) {
			cout << "FAILED\n";
			break;
		}
		int selection = 0;
		if (PIECEMAP[pr.ori] == pieces[1]) selection = 1;
		simulator.action(selection, pr.x, pr.y, pr.ori);
		if (i < 5) finder_parallel.action(selection, pr.x, pr.y, pr.ori);
	}
}

void solve_test() {
	OnlinePCFinder finder;
	Simulator simulator;
	finder.loadHashSet("field_hash.dat");
	simulator.initialize();
	deque<int> pieces = { 6, 0, 5,3,2,0, 1 };
	bitset<7> bag_used = 0b1111000;
	int bag_idx = 3;
	bitset<10> field[4] = { 120, 252, 68, 0 };

	finder.setState(bag_idx, pieces, bag_used, field, 3);
	ProbContext pr = finder.findBestMove();
	for (auto& item : pieces) {
		cout << PIECENAME[item];
	}
	cout << '\n';
	printf("%.2f%% Winning | Piece Code %d | x=%d y=%d\n", pr.prob * 100, pr.ori, pr.x, pr.y);
	if (pr.ori < 0) {
		cout << "FAILED\n";
	}
}


void generate_endgame() {
	endGameGenerator generator;
	generator.loadHashSet("field_hash.dat");
	generator.start();
	generator.saveHashEndGame("hash_end_game5.dat", "hash_end_game6.dat");
}

int main() {
#ifdef GENERATE_HASH
	generate_hash();
#endif

#ifdef ONLINE_SOLVE
	solve();
#endif

#ifdef GENERATE_ENDGAME
	generate_endgame();
#endif
}



