#pragma once

#include "types/vector2/Vector2_t.hpp"

namespace ymd::foc{

class CaliMap{
protected:
    using Samples = std::vector<ymd::Vector2>;
    Samples samples_;
    
    void resize(const size_t size);
    void reserve(const size_t size);
public:
    CaliMap();
    void addSample(const ymd::Vector2 & vec);
};

}