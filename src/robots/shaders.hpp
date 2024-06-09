#ifndef __SHADERS_HPP__

#define __SHADERS_HPP__

// namespace Shaders{
// __fast_inline RGB565 ShaderP(const Vector2i & pos){
//     static int cnt = 0;
//     cnt++;
//     return pos.x + pos.y + cnt;
// }

// __fast_inline RGB565 ShaderUV(const Vector2 & UV){
//     if((UV - Vector2(0.5, 0.5)).length_squared() <= real_t(0.25)){
//         return RGB565::RED;
//     }else{
//         return RGB565::BLUE;
//     }
// }

// __fast_inline RGB565 Mandelbrot(const Vector2 & UV){
//     Complex c(lerp(UV.x, real_t(0.5), real_t(-1.5)), lerp(UV.y, real_t(-1), real_t(1)));
//     Complex z;
//     uint8_t count = 0;

//     while ((z < real_t(4)) && (count < 23))
//     {
//         z = z*z + c;
//         count = count + 1;
//     }
//     return count * 100;
// }
// };


#endif