#pragma once

#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "targets/CH32V20X/src/defines/defs.h"

// #include "sdk.h"
#include "isr.h"
#include "arch.h"


//#region cpp-only
#ifdef __cplusplus
#include "core/constants/enums.hpp"
#include "core/constants/concepts.hpp"
#include "core/constants/units.hpp"

#include <type_traits>
#include <limits>
#endif
//#endregion
