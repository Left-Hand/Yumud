#pragma once

#include "sys/core/platform.h"
#include "types/buffer/buffer.hpp"
#include "thirdparty/sstl/include/sstl/vector.h"

#include <bits/ios_base.h>
#include <iomanip>

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>

#include <utility>

enum class SpecToken {
    Space,
    NoSpace,
    Comma,
    CommaWithSpace,
    Tab,
    End,

    Bin,
    Oct,
    Dec,
    Hex,

    Eps1,
    Eps2,
    Eps3,
    Eps4,
    Eps5,
    Eps6
};

class BasicStream{

};