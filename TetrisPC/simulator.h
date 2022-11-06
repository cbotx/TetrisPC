#pragma once

#include <algorithm>
#include <bitset>
#include <deque>
#include <random>
#include <vector>

#include "definition.h"

using namespace std;

class Simulator {
   protected:
    bitset<10> field[4];
    deque<int> pieces;
    int idx = 0;
    int pc_idx = 0;
    mt19937* g;

   public:
    explicit Simulator() {
        g = new mt19937(0);
    }
    virtual ~Simulator() {
        delete g;
    }

    void initialize() {
        memset(field, 0, sizeof(field));
        pieces.clear();
        for (int x = 0; x < 3; ++x) {
            add_new_bag();
        }
        idx = 0;
        pc_idx = 0;
    }

    void softReset() {
        memset(field, 0, sizeof(field));
        pc_idx = idx % 7;
    }

    int getPCIndex() {
        return pc_idx;
    }

    void getState(int window_size, bitset<10>* fp, deque<int>& pieces, int& bag_idx, bitset<7>& bag_used) {
        for (int i = 0; i < 4; ++i) {
            *(fp + i) = field[i];
        }
        pieces.clear();
        for (int i = 0; i < window_size; ++i) {
            if (idx + i >= this->pieces.size()) break;
            pieces.push_back(this->pieces[idx + i]);
        }
        bag_used = 0b1111111;
        for (int i = idx + window_size; i < ((idx + window_size) / 7 + 1) * 7; ++i) {
            bag_used[this->pieces[i]] = 0;
        }
        bag_idx = idx % 7;
    }

    void action(int selection, int x, int y, int pid) {
        show_field(x, y, pid);
        for (int i = 0; i < 4; ++i) {
            field[y + PIECE_REPR[pid][i][1]][x + PIECE_REPR[pid][i][0]] = true;
        }
        clear_rows(field);
        if (selection == 1) {
            swap(pieces[idx], pieces[idx + 1]);
        }
        ++idx;
        if (pieces.size() - idx <= 7) add_new_bag();
        if (idx >= 7) recycle_old_bag();
    }

    void show_field(int x, int y, int pid) {
        for (int i = 3; i >= 0; --i) {
            for (int j = 9; j >= 0; --j) {
                if (field[i][j] == 0) {
                    bool flag = false;
                    for (int k = 0; k < 4; ++k) {
                        if (j == x + PIECE_REPR[pid][k][0] && i == y + PIECE_REPR[pid][k][1]) {
                            flag = true;
                            break;
                        }
                    }
                    if (flag) {
                        cout << " O";
                    } else {
                        cout << "  ";
                    }
                } else {
                    cout << " =";
                }
            }
            cout << '\n';
        }
        cout << endl;
    }

   private:
    void add_new_bag() {
        vector<int> bag;
        for (int i = 0; i < 7; ++i) bag.push_back(i);
        shuffle(bag.begin(), bag.end(), *g);
        for (auto& item : bag) pieces.push_back(item);
    }

    void recycle_old_bag() {
        for (int i = 0; i < 7; ++i) pieces.pop_front();
        idx -= 7;
    }

    void clear_rows(bitset<10>* fp) {
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