#pragma once

#include "types/float/bf16.hpp"

namespace CANProtocolUtils{
    using E = bf16;
    using E_2 = std::tuple<E, E>;
    using E_3 = std::tuple<E, E, E>;
    using E_4 = std::tuple<E, E, E, E>;
}