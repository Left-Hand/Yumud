template<typename ColorType>
ColorType ImageReadable<ColorType>::operator()(const Vector2 & pos){
    Vector2i pos_i = Vector2i(int(pos.x), int(pos.y));
    if(!this->area.has_point(pos_i)) return ColorType();
    Vector2 pos_frac = pos - Vector2(pos);
    if constexpr(std::is_same_v<ColorType, Grayscale>){
        if(pos_frac.x){
            uint16_t x_u16;
            uni_to_u16(pos_frac.x, x_u16);
            uint32_t color_up = x_u16 * operator()(pos_i) + (~x_u16) * operator()(pos_i + Vector2i(1, 0));

            if(!pos_frac.y){
                return color_up >> 16;
            }else{
                uint32_t color_dn = x_u16 * operator()(pos_i + Vector2i(0, 1)) + (~x_u16) * operator()(pos_i + Vector2i(1, 1));
                uint16_t y_u16;
                uni_to_u16(pos_frac.y, y_u16);
                return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
            }
        }else{
            if(pos_frac.y){
                uint16_t y_u16;
                uni_to_u16(pos_frac.y, y_u16);
                return (y_u16 * operator()(pos_i) + (~y_u16) * operator()(pos_i + Vector2i(0, 1))) >> 16;
            }else{
                return operator()(pos_i);
            }
        }
    }else{
        return (ColorType)LERP(
            pos_frac.y,
            LERP(pos_frac.x, operator()(pos_i), operator()(pos_i + Vector2i(1, 0))),
            LERP(pos_frac.x, operator()(pos_i + Vector2i(0, 1)), operator()(pos_i + Vector2i(1, 1))));
    }
}

// template<typename ColorType>
// void Image<ColorType>::shade(PixelShaderCallback callback, const Rect2i & _shade_area){
//     Rect2i shade_area = _shade_area.intersection(area);
//     if(!area) return;
//     size_t buflen = shade_area.size.x;
//     ColorType * buf = new ColorType[buflen];

//     setarea_unsafe(shade_area);
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

//     setarea_unsafe(shade_area);
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