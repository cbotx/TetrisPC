#pragma once

#include <vector>

#include "definition.h"

using namespace std;

struct auxNode {
	opData op;
	vector<auxNode*> v[2];
};