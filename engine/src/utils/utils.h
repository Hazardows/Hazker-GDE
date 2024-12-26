#pragma once

// Libs
#include <stdlib.h>
#include "defines.h"
#include "utils/hstring.h"

#define clamp(val, _min, _max) (val <= _min) ? _min : (val >= _max) ? _max : val