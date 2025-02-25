#pragma once

// Libs
#include "defines.h"

#define HCLAMP(val, _min, _max) (val <= _min) ? _min : (val >= _max) ? _max : val