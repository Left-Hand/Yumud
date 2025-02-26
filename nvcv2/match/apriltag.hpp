#pragma once

#include "sys/debug/debug.hpp"

#include "types/image/image.hpp"

#include <array>
#include <cstdint>
#include <algorithm>
#include <tuple>

namespace ymd{

struct Apriltag{

};

class ApriltagDecoder{

};

class ApriltagRecognizer{
public:
    virtual void update(const Image<Grayscale> src, const Rect2i roi);
};


}
