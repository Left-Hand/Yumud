#ifndef __SHADERS_HPP__

#define __SHADERS_HPP__

// namespace Shaders{
// __fast_inline RGB565 ShaderP(const Vec2i & pos){
//     static int cnt = 0;
//     cnt++;
//     return pos.x + pos.y + cnt;
// }

// __fast_inline RGB565 ShaderUV(const Vec2 & UV){
//     if((UV - Vec2(0.5, 0.5)).length_squared() <= iq16(0.25)){
//         return RGB565::RED;
//     }else{
//         return RGB565::BLUE;
//     }
// }

// __fast_inline RGB565 Mandelbrot(const Vec2 & UV){
//     Complex c(lerp(UV.x, iq16(0.5), iq16(-1.5)), lerp(UV.y, iq16(-1), iq16(1)));
//     Complex z;
//     uint8_t count = 0;

//     while ((z < iq16(4)) && (count < 23))
//     {
//         z = z*z + c;
//         count = count + 1;
//     }
//     return count * 100;
// }
// };


#endif