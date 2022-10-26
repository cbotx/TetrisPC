#pragma once

#include "definition.h"

static inline bool equals(double a, double b) {
	return fabs(a - b) < 1e-7;
}