#pragma once

#include "core/platform.hpp"

namespace ymd::hal{
struct Rng{
public:
    Rng(){};

    void init();
    int update();
private:
    bool inited_ = false;
};

#ifdef ENABLE_RNG
extern Rng rng;
#endif

}

