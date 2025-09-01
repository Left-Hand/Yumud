#include "match.hpp"
#include "data/autogen/digit_images.hpp"


using namespace ymd;
using namespace ymd::nvcv2;




DigitProbability match_numbers(
        const Image<Gray> & src, 
        const Rect2u & roi, 
        const real_t threshold
){
    const auto tmp_size = roi.size;
    auto fault = src.clone(Rect2u(roi.top_left, tmp_size));
    auto fault_bina = fault.mirror<Gray>();

    DigitProbability digit_p = {0};
    for(size_t i = 0; i < MAX_NUMBERS; i++){
        Image<Gray> tmp = Image<Gray>::from_buf(
            reinterpret_cast<const Gray *>(digit_images[i]), 
            tmp_size
        );
        digit_p[i] = ymd::nvcv2::match::template_match(fault, tmp);
    }
    return digit_p;

    // uint maxi = -1;
    // real_t maxp = threshold;

    // for(size_t i=0; i < digit_p.size(); i++){
    //     if(digit_p[i] > maxp) {
    //         maxi = i;
    //         maxp = digit_p[i];
    //     }
    // }

    // if(maxi < 0) return None;
    // else return Some(uint(maxi));
}


DigitProbability match_numbers(const Image<Binary> & src, const Rect2u & roi){
    const auto tmp_size = roi.size;

    DigitProbability digit_p;
    // const auto begin = clock::millis();
    for(size_t i = 0; i < MAX_NUMBERS; i++){
        Image<Binary> tmp = Image<Binary>::from_buf(
            reinterpret_cast<const Binary *>(digit_bina_images[i]), 
            tmp_size
        );
        ymd::nvcv2::pixels::inverse(tmp);
        digit_p[i] = ymd::nvcv2::match::template_match(src, tmp, roi.top_left);
    }
    return digit_p;
    // const auto elp = clock::millis() - begin;
    // for(const auto & item : digit_p){
    //     DEBUGGER << item << ',';
    // }
    // DEBUGGER << elp << "\r\n";
    // TODO();
    // return Some(0);
}


Option<uint> match_number(const Image<Gray> &, const Rect2u & roi){
    TODO();
    return Some(0);
}

Option<uint> match_number(const Image<Binary> & src, const Rect2u & roi){
    // using Vertex = std::array<Vec2i, 4>;

    // Vertex vertex;

    // auto find_corner = [](const Image<Binary> & src, Vertex & vertex){
    //     auto update_vertex = 
    // }
    TODO();
    return Some(0);
}



real_t number_match(const Image<Gray> &src, const uint index){
    TODO();
    return real_t();
}


            // if(is_digit){


            //     auto char_pos = rect.get_center();
            //     const Vec2i tmp_size = {8, 12};
            //     const Rect2u clip_window = Rect2u::from_center(char_pos, tmp_size);
            //     auto clipped = img.clone(clip_window);


            //     auto tmp = Shape::x2(clipped);

            //     painter.setColor(RGB565::BLUE);
            //     painter.drawRoi(clip_window);

            //     auto digit_p = matcher.number(tmp, Rect2u(Vec2i(0,0), tmp_size));

            //     plot_number(clip_window, digit_p);

            //     Painter<Gray> pt;
            //     pt.bindImage(clipped);
            //     pt.drawString({0,0}, toString(digit_p));

            //     trans.transmit(clipped,2);

            // }