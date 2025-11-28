#pragma once

#include "core/string/string_view.hpp"
#include "core/stream/ostream.hpp"

#include "core/utils/Result.hpp"


namespace ymd{

template<size_t N>
struct CharArray{



private:
    std::array<char, N> chars_;
};
}