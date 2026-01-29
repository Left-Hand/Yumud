#pragma once

#include "core/debug/debug.hpp"
#include "primitive/image/image.hpp"
#include "middlewares/nvcv2/nvcv2.hpp"
#include <array>
#include <cstdint>
#include <algorithm>
#include <tuple>

namespace ymd::nvcv2{

struct Apriltag{

};

class ApriltagDecoder{

};

class ApriltagRecognizerIntf{
public:
    using Vertexs = std::array<math::Vec2<real_t>, 4>;
};


}
