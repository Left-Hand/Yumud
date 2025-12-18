#pragma once

#include "core/utils/Errno.hpp"

namespace ymd::robots::kinematics::prelude{
enum class Error_Kind:uint8_t{
    OutOfRange
};

DEF_ERROR_WITH_KIND(Error, Error_Kind)


}