#pragma once

#include <vector>

#include "definition.h"

using namespace std;

struct auxNode {
	opData op = { -1,0,0 };
	vector<auxNode*> v[2];
};