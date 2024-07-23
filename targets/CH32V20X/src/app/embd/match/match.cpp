#include "match.hpp"
#include "../data/autogen/digit_images.hpp"


void Matcher::init(){



}

real_t Matcher::number(const Image<Grayscale> & src, const Rect2i & roi){
    for(size_t i = 0; i < N; i++){
        Image<Grayscale> tmp = Image<Grayscale>::load_from_buf(digit_images[i], Vector2i(28,28));
        result[i] = NVCV2::Match::template_match(src, tmp, roi.position);
    }
    // DEBUG_PRINTLN(result);
    for(const auto & item : result){
        DEBUGGER << item << ',';
    }
    DEBUGGER << "0\r\n";
    return 0;
}

real_t Matcher::april(const Image<Grayscale> &, const Rect2i & roi){
    return real_t();
}



real_t number_match(const Image<Grayscale> &src, const uint index){
    return real_t();
}

