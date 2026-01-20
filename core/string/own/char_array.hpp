#pragma once

#include "core/string/view/string_view.hpp"
#include "core/stream/ostream.hpp"

#include "core/utils/Result.hpp"


namespace ymd{

template<size_t N>
using CharArray = std::array<char, N>;
}