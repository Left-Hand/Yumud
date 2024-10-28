#pragma once

#include "../../nvcv2/pixels/pixels.hpp"
#include "../../nvcv2/shape/shape.hpp"
#include "../../nvcv2/geometry/geometry.hpp"
#include "../../nvcv2/two_pass.hpp"
#include "../../nvcv2/flood_fill.hpp"
#include "../../nvcv2/mnist/mnist.hpp"
#include "../../nvcv2/template/template.hpp"

class Matcher{
protected:
    scexpr size_t N = 10;
    std::array<real_t, N> result;
public:
    void init();
    int number(const Image<Grayscale> &, const Rect2i & roi, const real_t threshold = real_t(0.0));
    int number(const Image<Binary> &, const Rect2i & roi);
    int april(const Image<Grayscale> &, const Rect2i & roi);
    int april(const Image<Binary> &, const Rect2i & roi);
};


real_t number_match(const Image<Grayscale> & src, const uint index);