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
#include "sys/constants/enums.hpp"
#include "sys/constants/concepts.hpp"
#include "sys/constants/units.hpp"

#include <type_traits>
#include <limits>
#endif
//#endregion
