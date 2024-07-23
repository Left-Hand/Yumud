#include "match.hpp"
#include "../data/autogen/digit_images.hpp"


void Matcher::init(){



}

real_t Matcher::number(const Image<Grayscale> & src, const Rect2i & roi){
    const auto tmp_size = Vector2i(28,28);
    auto fault = src.clone(Rect2i(roi.position, tmp_size));
    auto fault_bina = make_bina_mirror(fault);
    // NVCV2::Pixels::ostu(fault_bina, fault);

    auto begin = millis();
    for(size_t i = 0; i < N; i++){
        Image<Grayscale> tmp = Image<Grayscale>::load_from_buf(digit_images[i], tmp_size);
        result[i] = NVCV2::Match::template_match(fault, tmp);
    }
    // DEBUG_PRINTLN(result);
    auto elp = millis() - begin;
    for(const auto & item : result){
        DEBUGGER << item << ',';
    }
    DEBUGGER << elp << "\r\n";
    return 0;
}


real_t Matcher::number(const Image<Binary> & src, const Rect2i & roi){
    const auto tmp_size = Vector2i(28,28);

    auto begin = millis();
    for(size_t i = 0; i < N; i++){
        Image<Binary> tmp = Image<Binary>::load_from_buf(digit_bina_images[i], tmp_size);
        NVCV2::Pixels::inverse(tmp);
        result[i] = NVCV2::Match::template_match(src, tmp, roi.position);
    }

    auto elp = millis() - begin;
    for(const auto & item : result){
        DEBUGGER << item << ',';
    }
    DEBUGGER << elp << "\r\n";
    return 0;
}


real_t Matcher::april(const Image<Grayscale> &, const Rect2i & roi){
    return 0;
}

real_t Matcher::april(const Image<Binary> & src, const Rect2i & roi){
    using Vertex = std::array<Vector2i, 4>;

    // Vertex vertex;

    // auto find_corner = [](const Image<Binary> & src, Vertex & vertex){
    //     auto update_vertex = 
    // }
    return 0;
}



real_t number_match(const Image<Grayscale> &src, const uint index){
    return real_t();
}

