namespace ymd{


template<typename ColorType>
Grayscale Image<ColorType>::mean(const Rect2i & roi) const{
    return this->sum(roi) / (int(roi));
}


template<typename ColorType>
uint64_t Image<ColorType>::sum(const Rect2i & roi) const{
    uint64_t _sum = 0;
    const Range_t<uint> x_range = roi.get_x_range();
    const Range_t<uint> y_range = roi.get_y_range();

    for(uint j = y_range.from; j < y_range.to; ++j){
        const auto * ptr = &(this->operator[](Vector2i{x_range.from, j}));
        for(uint i = 0; i < x_range.length(); ++i){
            _sum += uint8_t(ptr[i]);
        }
    }
    return _sum;
}

template<typename ColorType>
void Image<ColorType>::load(const uint8_t * buf, const Vector2i & _size){
    this->size = _size;
    this->data = std::make_shared<ColorType[]>(_size.x * _size.y);
    // *this = Image<ColorType>{_size};
    auto area = this->size.x * this->size.y;
    memcpy((uint8_t *)this->data.get(), buf, area);
}


template<typename ColorType>
Grayscale Image<ColorType>::bilinear_interpol(const Vector2 & pos) const {
    Vector2i pos_i = {int(pos.x), int(pos.y)};
    // return img(pos_i);
    if(!this->has_point(pos_i) || !this->has_point(pos_i + Vector2i{1,1})) return Grayscale();
    Vector2 pos_frac = {frac(pos.x), frac(pos.y)};
    
    if(pos_frac.x){
        // uint16_t x_u16;
        // uni_to_u16(pos_frac.x, x_u16);
        // uint8_t color_up = x_u16 * uint8_t(img(pos_i)) >> 16;
        // color_up += ((~x_u16) * uint8_t(img(pos_i + Vector2i(1, 0))) >> 16);

        // return Grayscale(color_up);
        // if(!pos_frac.y){
        //     return color_up >> 16;
        //     // return img(pos_i);
        // }else{
        //     uint32_t color_dn = (uint16_t)x_u16 * (uint8_t)img(pos_i + Vector2i(0, 1)) + (~x_u16) * img(pos_i + Vector2i(1, 1));
        //     uint16_t y_u16;
        //     uni_to_u16(pos_frac.y, y_u16);
        //     return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
        // }

                    // uint16_t x_u16;
        // uni_to_u16(pos_frac.x, x_u16);
        // int c1 =  int(img(pos_i));
        // int c2 = int(img(pos_i + Vector2i(1, 0)));
        // return int((real_t(1)-pos_frac.x) * c1 + pos_frac.x * c2);
        int color_up = int(LERP(pos_frac.x, int(this->operator[](pos_i)), int(this->operator[](pos_i + Vector2i(1, 0)))));
        // return color_up;
        if(!pos_frac.y){
            return color_up;
        }else{
            // uint32_t color_dn = x_u16 * img(pos_i + Vector2i(0, 1)) + (~x_u16) * img(pos_i + Vector2i(1, 1));
            int color_dn = int(LERP(pos_frac.x, int(this->operator[](pos_i + Vector2i(0, 1))), int(this->operator[](pos_i + Vector2i(1, 1)))));
            // uint16_t y_u16;
            // uni_to_u16(pos_frac.y, y_u16);
            // return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
            return int(LERP(pos_frac.y, color_up, color_dn));
        }
    }else{
        // if(pos_frac.y){
        //     // uint16_t y_u16;
        //     // uni_to_u16(pos_frac.y, y_u16);
        //     // return (y_u16 * img(pos_i) + (~y_u16) * img(pos_i + Vector2i(0, 1))) >> 16;
        //     return LERP(pos_frac.y, img(pos_i), img(pos_i + Vector2i(0, 1)));
        // }else{
            return this->operator[](pos_i);
        // }
    }
    // return (ColorType)LERP(
    //         pos_frac.y,
    //         LERP(pos_frac.x, operator()(pos_i), operator()(pos_i + Vector2i(1, 0))),
    //         LERP(pos_frac.x, operator()(pos_i + Vector2i(0, 1)), operator()(pos_i + Vector2i(1, 1))));
}


// template<typename ColorType>
// ColorType Image<ColorType>::operator()(const real_t x, const real_t y){
//         return this -> operator()(Vector2(x,y));
// }

// template<typename ColorType>
// void Image<ColorType>::shade(PixelShaderCallback callback, const Rect2i & _shade_area){
//     Rect2i shade_area = _shade_area.intersection(area);
//     if(!area) return;
//     size_t buflen = shade_area.size.x;
//     ColorType * buf = new ColorType[buflen];

//     setArea_Unsafe(shade_area);
//     for(int y = 0; y < shade_area.size.y; y++){
//         for(int x = 0; x < shade_area.size.x; x++){
//             buf[x] = (callback(Vector2i(x, y)));
//         }
//         putPixels(buf, buflen);
//     }

//     delete[] buf;
// }


// template<typename ColorType>
// void Image<ColorType>::shade(UVShaderCallback callback, const Rect2i & _shade_area){
//     Rect2i shade_area = _shade_area.intersection(area);
//     if(!area) return;
//     size_t buflen = shade_area.size.x;
//     ColorType * buf = new ColorType[buflen];

//     setArea_Unsafe(shade_area);
//     real_t fx, fy;
//     real_t dx = real_t(1) / shade_area.size.x;
//     real_t dy = real_t(1) / shade_area.size.y;

//     for(int y = 0; y < shade_area.size.y; y++){
//         fx = real_t(0);
//         for(int x = 0; x < shade_area.size.x; x++){
//             buf[x] = (callback(Vector2(fx, fy)));
//             fx += dx;
//         }
//         putPixels(buf, buflen);
//         fy += dy;
//     }

//     delete[] buf;
// }

}