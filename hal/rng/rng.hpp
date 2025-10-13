#pragma once

#include "core/platform.hpp"

namespace ymd::hal{
struct Rng{
public:
    Rng(){};

    void init();
    int32_t update();
private:
    bool inited_ = false;
};

#ifdef ENABLE_RNG
extern Rng rng;
#endif

}

