#pragma once

#include "types/vector2/vector2.hpp"
#include "core/math/real.hpp"

namespace ymd::foc{

class CaliMap{
protected:
    using Samples = std::vector<Vector2_t<real_t>>;
    Samples samples_;
    
    void resize(const size_t size);
    void reserve(const size_t size);
public:
    CaliMap();
    void addSample(const Vector2_t<real_t> & vec);
};

}