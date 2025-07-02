#pragma once

#include "core/debug/debug.hpp"
#include "types/image/image.hpp"
#include "nvcv2/nvcv2.hpp"
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
    using Vertexs = std::array<Vector2<real_t>, 4>;
    virtual void update(const Image<Gray> src, const Rect2u roi);
};


}
