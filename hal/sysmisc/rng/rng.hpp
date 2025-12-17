#pragma once

#include "core/platform.hpp"

namespace ymd::hal{
struct [[nodiscard]] Rng{
public:
    Rng(){};

    void init();
    [[nodiscard]] int32_t update();
private:
    bool inited_ = false;
};

#ifdef ENABLE_RNG
extern Rng rng;
#endif

}

