#pragma once

#include <vector>

namespace ymd::tmp{
template<typename T, template<typename> typename Allocator>
using std_vector_t = std::vector<T, Allocator<T>>;
}