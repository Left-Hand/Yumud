#pragma once

#include "nvcv2/pixels/pixels.hpp"
#include "nvcv2/shape/shape.hpp"
#include "nvcv2/geometry/geometry.hpp"
// #include "nvcv2/seprate/two_pass.hpp"
// #include "nvcv2/flood_fill.hpp"
#include "nvcv2/mnist/mnist.hpp"
#include "nvcv2/template/template.hpp"


namespace ymd::nvcv2{
class Matcher{
protected:
    scexpr size_t N = 10;
    std::array<real_t, N> result;
public:
    void init();
    int number(const Image<Grayscale> &, const Rect2u & roi, const real_t threshold = real_t(0.0));
    int number(const Image<Binary> &, const Rect2u & roi);
    int april(const Image<Grayscale> &, const Rect2u & roi);
    int april(const Image<Binary> &, const Rect2u & roi);
};


real_t number_match(const Image<Grayscale> & src, const uint index);

}