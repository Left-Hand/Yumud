#include "numbers_matching.hpp"
#include "data/autogen/digit_images.hpp"


using namespace ymd;
using namespace ymd::nvcv2;


DigitsProbability match_numbers(
        const Image<Gray> & src, 
        const math::Rect2u16 & roi, 
        const iq16 threshold
){
    const auto tmp_size = roi.size;
    auto ret = src.clone(math::Rect2u16(roi.top_left, tmp_size));
    auto ret_bina = ret.copy_as<Gray>();

    DigitsProbability digit_p = {0};
    for(size_t i = 0; i < MAX_NUMBERS; i++){
        Image<Gray> tmp = Image<Gray>::clone_from_buf(
            reinterpret_cast<const Gray *>(digit_images[i]), 
            tmp_size
        );
        digit_p[i] = ymd::nvcv2::match::template_match(ret, tmp);
    }
    return digit_p;

}


DigitsProbability match_numbers(const Image<Binary> & src, const math::Rect2u16 & roi){
    const auto tmp_size = roi.size;

    DigitsProbability digit_p;
    for(size_t i = 0; i < MAX_NUMBERS; i++){
        Image<Binary> tmp = Image<Binary>::clone_from_buf(
            reinterpret_cast<const Binary *>(digit_bina_images[i]), 
            tmp_size
        );
        pixels::bitwise_inverse(tmp);
        digit_p[i] = match::template_match(src, tmp, roi.top_left);
    }
    return digit_p;
}


Option<uint> match_number(const Image<Gray> &, const math::Rect2u16 & roi){
    TODO();
    return Some(0);
}

Option<uint> match_number(const Image<Binary> & src, const math::Rect2u16 & roi){
    // using Vertex = std::array<Vec2i, 4>;

    // Vertex vertex;

    // auto find_corner = [](const Image<Binary> & src, Vertex & vertex){
    //     auto update_vertex = 
    // }
    TODO();
    return Some(0);
}



iq16 number_match(const Image<Gray> &src, const uint index){
    TODO();
    return iq16();
}
