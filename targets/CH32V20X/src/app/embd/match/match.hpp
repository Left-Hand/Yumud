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
    static constexpr size_t N = 10;
    std::array<real_t, N> result;
public:
    void init();
    real_t number(const Image<Grayscale> &, const Rect2i & roi);
    real_t number(const Image<Binary> &, const Rect2i & roi);
    real_t april(const Image<Grayscale> &, const Rect2i & roi);
    real_t april(const Image<Binary> &, const Rect2i & roi);
};


real_t number_match(const Image<Grayscale> & src, const uint index);