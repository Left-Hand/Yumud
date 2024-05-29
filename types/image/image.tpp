

// template<typename ColorType>
// ColorType ImageWithData<ColorType>::operator()(const real_t & x, const real_t & y){
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