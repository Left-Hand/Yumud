#pragma once

#include "core/platform.hpp"

namespace ymd::hal{
struct Rng{
public:
    Rng(){};

    static void init();
    static int update();
};

}

#ifdef ENABLE_RNG
extern Rng rng
#endif
