#pragma once

#include "middlewares/nvcv2/pixels/pixels.hpp"
#include "middlewares/nvcv2/shape/shape.hpp"
#include "middlewares/nvcv2/geometry/geometry.hpp"
#include "middlewares/nvcv2/mnist/mnist.hpp"
#include "middlewares/nvcv2/match/template_matching.hpp"


namespace ymd::nvcv2{

static constexpr size_t MAX_NUMBERS = 10;
using DigitProbability = std::array<real_t, MAX_NUMBERS>;

DigitProbability match_numbers(
    const Image<Gray> & img, 
    const Rect2u & roi, 
    const real_t threshold = real_t(0.0));

DigitProbability match_numbers(
    const Image<Binary> & img, 
    const Rect2u & roi);

Option<uint> match_april(const Image<Gray> & img, const Rect2u & roi);
Option<uint> match_april(const Image<Binary> & img, const Rect2u & roi);


}