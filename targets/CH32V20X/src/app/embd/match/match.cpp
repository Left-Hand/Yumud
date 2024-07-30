#include "match.hpp"
#include "../data/autogen/digit_images.hpp"


void Matcher::init(){



}

int Matcher::number(const Image<Grayscale> & src, const Rect2i & roi){
    const auto tmp_size = roi.size;
    auto fault = src.clone(Rect2i(roi.position, tmp_size));
    auto fault_bina = make_bina_mirror(fault);

    for(size_t i = 0; i < N; i++){
        Image<Grayscale> tmp = Image<Grayscale>::load_from_buf(digit_images[i], tmp_size);
        result[i] = NVCV2::Match::template_match(fault, tmp);
    }

    uint maxi = 0;
    real_t maxp = 0;

    for(size_t i=0; i < result.size(); i++){
        if(result[i] > maxp) {
            maxi = i;
            maxp = result[i];
        }
    }

    return maxi;
}


int Matcher::number(const Image<Binary> & src, const Rect2i & roi){
    const auto tmp_size = roi.size;

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


int Matcher::april(const Image<Grayscale> &, const Rect2i & roi){
    return 0;
}

int Matcher::april(const Image<Binary> & src, const Rect2i & roi){
    // using Vertex = std::array<Vector2i, 4>;

    // Vertex vertex;

    // auto find_corner = [](const Image<Binary> & src, Vertex & vertex){
    //     auto update_vertex = 
    // }
    return 0;
}



real_t number_match(const Image<Grayscale> &src, const uint index){
    return real_t();
}


            // if(is_digit){


            //     auto char_pos = rect.get_center();
            //     const Vector2i tmp_size = {8, 12};
            //     const Rect2i clip_window = Rect2i::from_center(char_pos, tmp_size);
            //     auto clipped = img.clone(clip_window);


            //     auto tmp = Shape::x2(clipped);

            //     painter.setColor(RGB565::BLUE);
            //     painter.drawRoi(clip_window);

            //     auto result = matcher.number(tmp, Rect2i(Vector2i(0,0), tmp_size));

            //     plot_number(clip_window, result);

            //     Painter<Grayscale> pt;
            //     pt.bindImage(clipped);
            //     pt.drawString({0,0}, toString(result));

            //     trans.transmit(clipped,2);

            // }