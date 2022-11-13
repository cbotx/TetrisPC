#pragma once

#include <algorithm>
#include <bitset>
#include <deque>
#include <random>
#include <vector>

#include "definition.h"

class Simulator {
   private:
    std::bitset<10> field[4];
    std::deque<int> pieces;
    int idx = 0;
    int pc_idx = 0;
    std::mt19937* g;

   public:
    Simulator();
    virtual ~Simulator();

    void Initialize();

    void SoftReset();

    int GetPCIndex();

    void GetState(int window_size, std::bitset<10>* fp, std::deque<int>& pieces, int& bag_idx, std::bitset<7>& bag_used);

    void Action(int selection, int x, int y, int pid);

    void ShowField(int x, int y, int pid);

   private:
    void InsertNewBag();

    void RemoveOldBag();
};
