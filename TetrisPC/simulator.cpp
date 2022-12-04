#include "simulator.h"

#include <utility>

#include "utils.h"

Simulator::Simulator() {
    g = new std::mt19937(0);
}
Simulator::~Simulator() {
    delete g;
}

void Simulator::Initialize() {
    memset(field, 0, sizeof(field));
    pieces.clear();
    //for (int x = 0; x < 3; ++x) {
    //    InsertNewBag();
    //}
    std::vector<int> x = { 4, 6, 3, 2, 0, 5, 1, 1, 3, 5, 2, 0, 4, 6, 0, 1, 2, 3, 4, 5, 6 };
    for (auto& item : x) pieces.push_back(item);
    idx = 0;
    pc_idx = 0;
}

void Simulator::SoftReset() {
    memset(field, 0, sizeof(field));
    pc_idx = idx % 7;
}

int Simulator::GetPCIndex() {
    return pc_idx;
}

void Simulator::GetState(int window_size, std::bitset<10>* fp, std::deque<int>& pieces, int& bag_idx, std::bitset<7>& bag_used) {
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

void Simulator::Action(int selection, int x, int y, int pid) {
    ShowField(x, y, pid);
    for (int i = 0; i < 4; ++i) {
        field[y + PIECE_REPR[pid][i][1]][x + PIECE_REPR[pid][i][0]] = true;
    }
    clear_lines(field);
    if (selection == 1) {
        std::swap(pieces[idx], pieces[idx + 1]);
    }
    ++idx;
    if (pieces.size() - idx <= 7) InsertNewBag();
    if (idx >= 7) RemoveOldBag();
}

void Simulator::ShowField(int x, int y, int pid) {
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
                    std::cout << " O";
                } else {
                    std::cout << "  ";
                }
            } else {
                std::cout << " =";
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void Simulator::InsertNewBag() {
    std::vector<int> bag;
    for (int i = 0; i < 7; ++i) bag.push_back(i);
    shuffle(bag.begin(), bag.end(), *g);
    for (auto& item : bag) pieces.push_back(item);
}

void Simulator::RemoveOldBag() {
    for (int i = 0; i < 7; ++i) pieces.pop_front();
    idx -= 7;
}
