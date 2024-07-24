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


// template<typename ColorType>
// ColorType Image<ColorType>::operator()(const real_t & x, const real_t & y){
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