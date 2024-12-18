#pragma once

#include "sys/core/platform.h"

namespace ymd{
struct Rng{
public:
    Rng(){};

    static void init();
    static int update();
};

}

#ifdef ENABLE_RNG
static inline Rng rng
#endif
