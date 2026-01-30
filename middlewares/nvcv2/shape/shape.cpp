#include "shape.hpp"
#include "middlewares/nvcv2/pixels/pixels.hpp"
#include "core/math/fastmath/sqrt_f.hpp"
#include "core/math/fastmath/sqrt_i.hpp"
#include "core/math/realmath.hpp"

namespace ymd::nvcv2::shape{

template<is_monochrome T>
static void clear_corners(Image<T> & dst){
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);
    for(uint16_t y = 0; y < h; y++)
        dst[{static_cast<uint16_t>(0), static_cast<uint16_t>(y)}] = T::black();
    for(uint16_t y = 0; y < h; y++)
        dst[{static_cast<uint16_t>(w-1u), static_cast<uint16_t>(y)}] = T::black();
    for(uint16_t x = 0; x < w; x++)
        dst[{static_cast<uint16_t>(x), static_cast<uint16_t>(0)}] = T::black();
    for(uint16_t x = 0; x < w; x++)
        dst[{static_cast<uint16_t>(x), static_cast<uint16_t>(h-1u)}] = T::black();
}


void convolution(
    Image<Gray> & dst, 
    const Image<Gray> & src, 
    const math::Matrix<int8_t, 3, 3> & core)
{
    const auto size = dst.size();
    const auto den = core.sum();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);
    for(uint16_t y = 1; y < h-1u; y++){
        for(uint16_t x = 1; x < w-1u; x++){
            int32_t pixel = 0;
            pixel += (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y - 1)}]	).to_u8() * core[0][0];
            pixel += (src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y - 1)}]		).to_u8() * core[0][1];
            pixel += (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y - 1)}]	).to_u8() * core[0][2];
            
            pixel += (src[{static_cast<uint16_t>(x-1), (y)}]		).to_u8() * core[1][0];
            pixel += (src[{static_cast<uint16_t>(x+0), (y)}]			).to_u8() * core[1][1];
            pixel += (src[{static_cast<uint16_t>(x+1), (y)}]		).to_u8() * core[1][2];
            
            pixel += (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 1)}] 	).to_u8() * core[2][0];
            pixel += (src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y + 1)}]		).to_u8() * core[2][1];
            pixel += (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 1)}]	).to_u8() * core[2][2];
            

            dst[{x, y}] = Gray::from_u8(CLAMP(ABS(pixel / den), 0, 255));
        }
    }
}


void gauss(Image<Gray> & dst, const Image<Gray> & src){
    clear_corners(dst);
    convolution(dst, src, cores::gauss);
}


void gauss5x5(Image<Gray> & dst, const Image<Gray> & src){
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);
    static constexpr size_t CORE_RADIUS = 2u;
    static constexpr size_t CORE_SUM = 256u;

    static constexpr size_t MAT_WIDTH = 2 * CORE_RADIUS + 1u;

    static constexpr uint8_t CORE[MAT_WIDTH][MAT_WIDTH] ={
        {1,     4,      6,      4,      1},
        {4,     16,     24,     16,     4},
        {6,     24,     36,     24,     6},
        {4,     16,     24,     16,     4},
        {1,     4,      6,      4,      1}
    };

    clear_corners(dst);
    for(uint16_t y = uint16_t(CORE_RADIUS); y < uint16_t(h - CORE_RADIUS); ++y){
        for(uint16_t x = uint16_t(CORE_RADIUS); x < uint16_t(w - CORE_RADIUS); ++x){
            uint32_t sum = 0;

            for(int dy = -int(CORE_RADIUS); dy <= int(CORE_RADIUS); ++dy){
                for(int dx = -int(CORE_RADIUS); dx <= int(CORE_RADIUS); ++dx){
                    sum += 
                        src[math::Vec2u16{uint16_t(x + dx), uint16_t(y + dy)}].to_u8()
                        * uint8_t(CORE[uint16_t(dy + CORE_RADIUS)][uint16_t(dx + CORE_RADIUS)]);
                }
            }
            
            dst[{x,y}] = Gray::from_u8(CLAMP(sum / CORE_SUM, 0, 255));
        }
    }
}

void gauss(Image<Gray> & src){
    auto temp = Image<Gray>(src.size());
    gauss(temp, src);
    pixels::copy(src, temp);
}

math::Vec2u16 find_most(const Image<Gray> & src, const Gray & tg_color,  const math::Vec2u16 & point, const math::Vec2u16 & vec){
    math::Vec2u16 now_point = point;
    math::Vec2u16 delta_point = math::Vec2u16(math::sign(vec.x), math::sign(vec.y));

    {
        while(true){
            if(not src.size().has_point(now_point)){
                return {0,0};//nothing
            }
            // DEBUG_PRINTLN(now_point, src[now_point]);

            if(src[now_point] == tg_color){
                break;
            }

            now_point += delta_point;
        }
    }

    auto eve = [](const math::Vec2u16 & _point, const math::Vec2u16 & _vec) -> size_t{
        return _point.dot(_vec);
    };

    size_t now_eve = eve(now_point, vec);
    while(true){
        math::Vec2u16 next_x_vec = now_point + math::Vec2u16(math::sign(vec.x), 0);
        math::Vec2u16 next_y_vec = now_point + math::Vec2u16(0, math::sign(vec.y));

        math::Vec2u16 * next_point = &now_point;
        now_eve = eve(now_point, vec);

        if(src[next_x_vec] == tg_color){
            auto x_eve = eve(next_x_vec, vec);
            if(x_eve > now_eve){
                next_point = &next_x_vec;
                now_eve = x_eve;
            }
        }

        if(src[next_y_vec] == tg_color){
            auto y_eve = eve(next_y_vec, vec);
            if(y_eve > now_eve){
                next_point = &next_y_vec;
                now_eve = y_eve;
            }
        }

        if(next_point == &now_point){
            return now_point;
        }

        now_point = *next_point;
    }
    // return now_point;
}


void sobel_xy(Image<Gray> & dst, const Image<Gray> & src){
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);
    {

        const auto & core = cores::sobel_x;
        for(uint16_t y = 1; y < h-1u; y++){
            for(uint16_t x = 1; x < w-1u; x++){
                size_t pixel = 0;
                pixel += (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y - 1)}]	).to_u8() * core[0][0];
                pixel += (src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y - 1)}]		).to_u8() * core[0][1];
                pixel += (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y - 1)}]	).to_u8() * core[0][2];
                
                pixel += (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y)}]		).to_u8() * core[1][0];
                pixel += (src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y)}]			).to_u8() * core[1][1];
                pixel += (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y)}]		).to_u8() * core[1][2];
                
                pixel += (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 1)}] 	).to_u8() * core[2][0];
                pixel += (src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y + 1)}]		).to_u8() * core[2][1];
                pixel += (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 1)}]	).to_u8() * core[2][2];
                
                dst[{x , y}] = Gray::from_u8(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }
    {
        const auto & core = cores::sobel_y;
        for(uint16_t y = 1; y < h-1u; y++){
            for(uint16_t x = 1; x < w-1u; x++){
                uint32_t pixel = 0;
                pixel += (src[{static_cast<uint16_t>(x-1) , static_cast<uint16_t>(y - 1) }]	).to_u8() * core[0][0];
                pixel += (src[{static_cast<uint16_t>(x+0) , static_cast<uint16_t>(y - 1) }]		).to_u8() * core[0][1];
                pixel += (src[{static_cast<uint16_t>(x+1) , static_cast<uint16_t>(y - 1) }]	).to_u8() * core[0][2];
                
                pixel += (src[{static_cast<uint16_t>(x-1) , static_cast<uint16_t>(y) }]		).to_u8() * core[1][0];
                pixel += (src[{static_cast<uint16_t>(x+0) , static_cast<uint16_t>(y) }]			).to_u8() * core[1][1];
                pixel += (src[{static_cast<uint16_t>(x+1) , static_cast<uint16_t>(y) }]		).to_u8() * core[1][2];
                
                pixel += (src[{static_cast<uint16_t>(x-1) , static_cast<uint16_t>(y + 1) }] 	).to_u8() * core[2][0];
                pixel += (src[{static_cast<uint16_t>(x+0) , static_cast<uint16_t>(y + 1) }]		).to_u8() * core[2][1];
                pixel += (src[{static_cast<uint16_t>(x+1) , static_cast<uint16_t>(y + 1) }]	).to_u8() * core[2][2];
                
                dst[{x, y}] = Gray::from_u8(MAX(
                    dst[{x, y}].to_u8(), CLAMP(ABS(pixel), 0, 255)
                ));
            }
        }
    }
}

void convolution(Image<Gray> & dst, const Image<Gray> & src, const size_t core[2][2]){
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);
    for(uint16_t y = 1; y < h-1u; y++){
        for(uint16_t x = 1; x < w-1u; x++){
            size_t pixel = 0;

            pixel += (src[{static_cast<uint16_t>(x-1),static_cast<uint16_t>(y - 1)}]).to_u8() * core[0][0];
            pixel += (src[{static_cast<uint16_t>(x+0),static_cast<uint16_t>(y - 1)}]).to_u8() * core[0][1];
            
            pixel += (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y)}]).to_u8() * core[1][0];
            pixel += (src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y)}]).to_u8() * core[1][1];
            
            dst[{x, y}] = Gray::from_u8(CLAMP(ABS(pixel), 0, 255));
        }
    }
}

void dilate(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        dilate(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);


    for(uint16_t y = 1; y < h-1u; y++){
        for(uint16_t x = 1; x < w-1u; x++){
            bool pixel = false;
            pixel |= src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y - 1)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y - 1)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y - 1)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 0)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y + 0)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 0)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 1)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y + 1)}].is_white();
            pixel |= src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 1)}].is_white();
            
            dst[{x, y}] = Binary::from_bool(pixel);
        }
    }
}

void dilate_xy(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        dilate_xy(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);


    {   
        const size_t y = 0;
        auto * p_dst = reinterpret_cast<uint8_t *>(&dst.head_ptr()[1]);
        auto * p_dst_end = reinterpret_cast<uint8_t *>(&dst.head_ptr()[w - 2]);

        auto * p_src = reinterpret_cast<const uint8_t *>(&src.head_ptr()[y * w + 1]);
        auto * p_src_next = reinterpret_cast<const uint8_t *>(&src.head_ptr()[(y+1) * w + 1]);
        while(p_dst < p_dst_end){
            *p_dst = *(p_src - 1) | *(p_src) | *(p_src + 1) | *(p_src_next); 

            p_dst++;
            p_src++;
            p_src_next++;
        }
    }

    for(size_t y = 1; y < h-1u; y++){
        auto * p_dst = reinterpret_cast<uint8_t *>(&dst.head_ptr()[y * w + 1]);
        auto * p_src_last = reinterpret_cast<const uint8_t *>(&src.head_ptr()[(y-1) * w + 1]);
        auto * p_src = reinterpret_cast<const uint8_t *>(&src.head_ptr()[y * w + 1]);
        auto * p_src_next = reinterpret_cast<const uint8_t *>(&src.head_ptr()[(y+1) * w + 1]);
        for(size_t x = 1; x < w-1u; x++){
            bool pixel = *(p_src - 1) | *(p_src) | *(p_src + 1); 
            if(!pixel){
                *p_dst = false;
            }else{
                pixel |= *(p_src_last) | *(p_src_next);
                *p_dst = pixel;
            }

            p_dst++;
            p_src_last++;
            p_src++;
            p_src_next++;
        
        }
    }

    {   
        const size_t y = h - 1;
        auto * p_dst = reinterpret_cast<uint8_t *>(&dst.head_ptr()[y * w + 1]);
        auto * p_dst_end = reinterpret_cast<uint8_t *>(&dst.head_ptr()[y * w + (w - 2)]);

        auto * p_src_last = reinterpret_cast<const uint8_t *>(&src.head_ptr()[(y-1) * w + 1]);
        auto * p_src = reinterpret_cast<const uint8_t *>(&src.head_ptr()[y * w + 1]);

        while(p_dst != p_dst_end){
            *p_dst = *(p_src - 1) | *(p_src) | *(p_src + 1) | *(p_src_last); 

            p_dst++;
            p_src++;
            p_src_last++;
        }
    }
}

void dilate_y(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        dilate_y(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);


    for(uint16_t y = 1; y < h-1u; y++){
        for(uint16_t x = 0; x < w; x++){
            bool pixel = false;

            pixel |= src[{x, static_cast<uint16_t>(y - 1)}].is_white();
            pixel |= src[{x, (y)}].is_white();
            pixel |= src[{x, static_cast<uint16_t>(y + 1)}].is_white();
            
            dst[{x, y}] = Binary::from_bool(pixel);
        }
    }
}

void erosion(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        erosion(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for(uint16_t y = 1; y < h-1u; y++){
        for(uint16_t x = 1; x < w-1u; x++){
            bool pixel = true;
            pixel &= src[{static_cast<uint16_t>(x-1),static_cast<uint16_t>(y - 1)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x+0),static_cast<uint16_t>(y - 1)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x+1),static_cast<uint16_t>(y - 1)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x-1),(y)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x+0),(y)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x+1),(y)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x-1),static_cast<uint16_t>(y + 1)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x+0),static_cast<uint16_t>(y + 1)}].is_white();
            pixel &= src[{static_cast<uint16_t>(x+1),static_cast<uint16_t>(y + 1)}].is_white();
            
            dst[{x, y}] = Binary::from_bool(pixel);
        }
    }
}

void erosion_x(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        erosion_x(dst, temp);
        return;
    }
    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for(size_t y = 0; y < h; y++){
        auto * p_src = reinterpret_cast<const uint8_t *>(&src.head_ptr()[y * w + 1]);
        auto * p_dst = reinterpret_cast<uint8_t *>(&dst.head_ptr()[y * w + 1]);
        uint8_t last_two = (*p_src - 1) & (*p_src);
        for (size_t x = 1; x < w - 1; ++x) {
            *p_dst = last_two & (*(p_src + 1));
            last_two = (*p_src) & (*(p_src + 1));

            // *p_dst= *(p_src - 1) & (*p_src) & (*(p_src + 1));
            
            p_src++;
            p_dst++; 
        }
    }
}

void dilate_x(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        dilate_x(dst, temp);
        return;
    }
    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for(size_t y = 0; y < h; y++){
        auto * p_src = reinterpret_cast<const uint8_t *>(&src.head_ptr()[y * w + 1]);
        auto * p_dst = reinterpret_cast<uint8_t *>(&dst.head_ptr()[y * w + 1]);
        uint8_t last_two = (*p_src - 1) & (*p_src);
        for (size_t x = 1; x < w - 1; ++x) {
            *p_dst = last_two | (*(p_src + 1));
            last_two = (*p_src) | (*(p_src + 1));

            p_src++;
            p_dst++; 
        }
    }
}

void erosion_y(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        erosion_y(dst, temp);
        return;
    }
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);

    for(uint16_t y = 1; y < h-1u; y++){
        for(uint16_t x = 0; x < w; x++){
            bool pixel = true;

            pixel &= src[{x , static_cast<uint16_t>(y - 1)}].is_white();
            pixel &= src[{x , (y)}].is_white();
            pixel &= src[{x , static_cast<uint16_t>(y + 1)}].is_white();
            
            dst[{x, y}] = Binary::from_bool(pixel);
        }
    }
}

void erosion_xy(Image<Binary> & dst, const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        erosion_xy(dst, temp);
        return;
    }
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);

    for(uint16_t y = 0; y < h; y++){
        for(uint16_t x = 1; x < w-1u; x++){
            bool pixel = true;
            pixel &= src[{x,    static_cast<uint16_t>(MAX((y - 1), 0)) }].is_white();
            pixel &= src[{x,    static_cast<uint16_t>((y)) }].is_white();
            pixel &= src[{x,    static_cast<uint16_t>((y)) }].is_white();
            pixel &= src[{x,    static_cast<uint16_t>((y)) }].is_white();
            pixel &= src[{x,    static_cast<uint16_t>(MIN(static_cast<uint16_t>(y + 1), h-1u)) }].is_white();
            
            dst[{x, y}] = Binary::from_bool(pixel);
        }
    }
}

auto x4(const Image<Gray> & src, const size_t m){
    Image<Gray> dst(src.size() / m);
    x4(dst, src, m);
    return dst;
}


Image<Gray> x2(const Image<Gray> & src){
    Image<Gray> dst(src.size() / 2);
    const auto size = dst.size();
    const uint16_t w = uint16_t(size.x);
    const uint16_t h = uint16_t(size.y);

    for(uint16_t y = 0; y < h; y++){
        for(uint16_t x = 0; x < w; x++){
            uint16_t sum = 0;

            for(size_t j = 0; j < 2; j++){
                for(size_t i = 0; i < 2; i++){
                    sum += src[{
                        static_cast<uint16_t>((x << 1) + i),
                        static_cast<uint16_t>((y << 1) + j)
                    }].to_u8();
                }
            }

            dst[{x,y}] = Gray::from_u8(sum >> 2);

            // dst[{x,y}] = src[{x << 1,y << 1}];
        }
    }
    return dst;
}

void anti_pepper_x(Image<Binary> & dst,const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        anti_pepper_x(dst, temp);
        return;
    }
    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for(size_t y = 0; y < h; y++){
        const auto * p_src = &src.head_ptr()[y * w + 1];
        auto * p_dst = &dst.head_ptr()[y * w + 1];
        uint8_t last_two = false;
        for (size_t x = 1; x < w - 2; ++x) {
            uint8_t next = (*(p_src + 1)).is_white();
            *p_dst = Binary::from_bool((last_two + next) > 1);
            last_two = uint8_t((*p_src).is_white()) + next;

            p_src++;
            p_dst++; 
        }
    }
}

void anti_pepper_y(Image<Binary> & dst,const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        anti_pepper_y(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);


    for(size_t x = 0; x < w - 1; ++x){
        auto * p_src = &src.head_ptr()[x];
        auto * p_dst = &dst.head_ptr()[x];
        uint8_t last_two = false;
        for (size_t y = 1; y < h - 2; ++y) {
            uint8_t next = (*(p_src + 1)).is_white();
            *p_dst = Binary::from_bool((last_two + next) > 1);
            last_two = uint8_t(((*p_src)).is_white()) + next;

            p_src += w;
            p_dst += w;
        }
    }
}

void anti_pepper(Image<Binary> & dst,const Image<Binary> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        anti_pepper(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for(size_t y = 0; y < h; y++){
        auto * p_src = &src.head_ptr()[y * w + 1];
        auto * p_dst = &dst.head_ptr()[y * w + 1];
        uint8_t last_two = true;
        for (size_t x = 1; x < w - 1; ++x) {
            uint8_t next = (*(p_src + 1)).is_white();
            *p_dst = Binary::from_bool((last_two + next) > 1);
            last_two = uint8_t(((*p_src)).is_white()) + next;

            p_src++;
            p_dst++; 
        }
    }
}


void XN(Image<Binary> dst, const Image<Binary> & src, const size_t m, const real_t percent){
    const auto size = src.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    size_t n = int(percent * m * m);

    for(size_t y = 0; y < h / m; y++){
        for(size_t x = 0; x < w / m; x++){
            math::Vec2u16 base = math::Vec2u16(x, y)* m;
            size_t pixel = 0;
            for(size_t j = 0; j < m; j++){
                for(size_t i = 0; i < m; i++){
                    math::Vec2u16 src_pos = base + math::Vec2u16(i, j);
                    pixel += src[src_pos].is_white();
                }
            math::Vec2u16 dst_pos = math::Vec2u16(x,y);
            dst[dst_pos] = Binary::from_bool(bool(pixel > n));
            }
        }
    }
}

#if 0
void zhang_suen(Image<Binary> & dst,const Image<Binary> & src){

    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        zhang_suen(temp, src);
        dst.clone(temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);
    auto temp = src.clone();
    dst.clone(src);
    
    uint8_t iter = 0;
    while (true) {

        bool is_end = true;

        for (size_t y = 0; y < h; ++y) {
            for (size_t x = 0; x < w; ++x) {
                const math::Vec2u16 p{x,y};
                // const Binary * p = &temp[x + y * w];
                
                if (temp[p] == 0) continue;
                
                Binary p1 = temp[p + Vec2i{0, -1}];
                Binary p2 = temp[p + Vec2i{1, -1}];
                Binary p3 = temp[p + Vec2i{1, 0}];
                Binary p4 = temp[p + Vec2i{1, 1}];
                Binary p5 = temp[p + Vec2i{0, 1}];
                Binary p6 = temp[p + Vec2i{-1, 1}];
                Binary p7 = temp[p + Vec2i{-1, 0}];
                Binary p8 = temp[p + Vec2i{-1, -1}];


                size_t A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                        (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                        (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                        (p8 == 0 && p1 == 1) + (p1 == 0 && p2 == 1);
                size_t B  = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
                size_t m1 = (iter % 2 == 0) ? (p1 * p3 * p5) : (p1 * p3 * p7);
                size_t m2 = (iter % 2 == 0) ? (p3 * p5 * p7) : (p1 * p5 * p7);
                
                if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0){
                    dst[x + y * w] = false;
                    is_end = false;
                }
            }
        }
        iter++;
        temp.clone(dst);
        if (is_end) break;
    }
    
    dst.clone(temp);

}

void zhang_suen2(Image<Binary> & dst,const Image<Binary> & src){

    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        zhang_suen2(temp, src);
        dst = std::move(temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);
    auto temp = src.clone();
    dst.clone(src);
    
    uint8_t iter = 0;
    while (true) {
        bool is_end = true;
        for (size_t y = 1; y < h - 1; ++y) {
            for (size_t x = 1; x < w - 1; ++x) {
                if(iter % 2 == 0){
                    if((x + y) % 2 != 0) continue;
                }else{
                    if((x + y) % 2 == 0) continue;
                }

                const math::Vec2u16 base = math::Vec2u16(x, y);
                bool p1 = temp[base];
                if (p1 == 0) continue;

                bool p2 = temp[base + math::Vec2u16(0, -1)];
                bool p3 = temp[base + math::Vec2u16(1, -1)];
                bool p4 = temp[base + math::Vec2u16(1, 0)];
                bool p5 = temp[base + math::Vec2u16(1, 1)];
                bool p6 = temp[base + math::Vec2u16(0, 1)];
                bool p7 = temp[base + math::Vec2u16(-1, 1)];
                bool p8 = temp[base + math::Vec2u16(-1, 0)];
                bool p9 = temp[base + math::Vec2u16(-1, -1)];

                size_t B  = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
                size_t C = ((!p2) && (p3 || p4)) + ((!p4) && (p5 || p6)) + ((!p6) && (p7 || p8)) + ((!p8) && (p9 || p2));
                size_t m1 = (iter % 2 == 0) ? (p2 * p4 * p6) : (p2 * p4 * p8);
                size_t m2 = (iter % 2 == 0) ? (p4 * p6 * p8) : (p2 * p6 * p8);
                
                if (C == 1 && (B >= 2 && B <= 7) && m1 == 0 && m2 == 0){
                    dst[x + y * w] = false;
                    is_end = false;
                }
            }
        }
        iter++;
        // memcpy(temp, dst, w * h * sizeof(Binary));
        temp.clone(dst);
        if (is_end) break;
    }
    
    dst.clone(temp);
}
#endif

void convo_roberts_x(Image<Gray> & dst, const Image<Gray> & src){
    if(src.is_shared_with(dst)){
        auto temp = src.clone();
        convo_roberts_x(dst, temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for (size_t y = 0; y < h; ++y) {
        const auto * p_src = &src.head_ptr()[y * w];
        auto * p_dst = &dst.head_ptr()[y * w];
        for (size_t x = 0; x < w - 1; ++x) {
            *p_dst = Gray::from_u8(ABS(p_src->to_u8() - (p_src + 1)->to_u8()));
            p_src++;
            p_dst++; 
        }
    }
}

void convo_roberts_xy(Image<Gray> & dst, const Image<Gray> & src){
    if(src.is_shared_with(dst)){
        auto temp = Image<Gray>(src.size());
        convo_roberts_xy(temp, src);
        dst = std::move(temp);
        return;
    }

    const auto size = dst.size();
    const size_t w = size_t(size.x);
    const size_t h = size_t(size.y);

    for (size_t y = 0; y < h - 1; ++y) {
        const auto * p_src = &src.head_ptr()[y * w];
        const auto * p_src2 = &src.head_ptr()[static_cast<uint16_t>(y + 1) * w];
        auto * p_dst = &dst.head_ptr()[y * w];
        for (size_t x = 0; x < w - 1; ++x) {
            *p_dst = Gray::from_u8(MAX(
                ABS(p_src->to_u8() - (p_src + 1)->to_u8()), 
                ABS(p_src->to_u8() - (p_src2)->to_u8())
            ));
            p_src++;
            p_dst++; 
            p_src2++;
        }
    }

    {
        size_t y = h - 1;
        auto * p_src = src.head_ptr() + y * w;
        auto * p_dst = dst.head_ptr() + y * w;
        for(size_t x = 0; x < w - 1; ++x) {
            *p_dst = Gray::from_u8(ABS(p_src->to_u8() - (p_src + 1)->to_u8()));
            p_src++;
            p_dst++; 
        }
    }
}


static constexpr Direction xy_to_dir(const int16_t x, const int16_t y){
    auto abs_x = ABS(x);
    auto abs_y = ABS(y);

    #define TWO_AND_HALF(x) ((x << 1) + (x >> 1))

    if(abs_y > TWO_AND_HALF(abs_x)){
        return Direction::U;
    }else if(abs_x > TWO_AND_HALF(abs_y)){
        return Direction::R;
    }else{
        if(x * y > 0){
            return Direction::UR;
        }else{
            return Direction::UL;
        }
    }

    __builtin_unreachable();

    #undef TWO_AND_HALF
} 

void canny(Image<Binary> &dst, const Image<Gray> &src, const math::Range2<uint16_t> & threshold){
    auto roi = math::Rect2u::from_size(src.size());

    const auto [low_thresh, high_thresh] = threshold;

    struct gvec_t{
        uint8_t g;
        Direction t;
    }__packed;

    auto gm = std::make_unique<gvec_t[]>(roi.area());

    const size_t w = roi.w();
    
    static constexpr size_t SHIFTS = 9;
    
    const uint8_t low_squ = math::square(low_thresh) >> SHIFTS;
    const uint8_t high_squ = math::square(high_thresh) >> SHIFTS;
    

    //2. Finding Image Gradients
    {
        for (size_t y = roi.y() + 1; y < roi.y() + roi.h() - 1u; y++) {
            for (size_t x = roi.x() + 1; x < roi.x() + roi.w() - 1u; x++) {
                int16_t vx = 0, vy = 0;

                //  1   0   -1
                //  2   0   -2
                //  1   0   -1

                vx =    (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y - 1)}]).to_u8()
                    -   (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y - 1)}]).to_u8()
                    +   ((src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 0)}]).to_u8() << 1)
                    -   ((src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 0)}]).to_u8() << 1)
                    +   (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 1)}]).to_u8()
                    -   (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 1)}]).to_u8();

                //  1   2   1
                //  0   0   0
                //  -1  2   -1

                vy =    (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y - 1)}]).to_u8()
                    +   ((src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y - 1)}]).to_u8() << 1)
                    +   (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y - 1)}]).to_u8()
                    -   (src[{static_cast<uint16_t>(x-1), static_cast<uint16_t>(y + 1)}]).to_u8()
                    -   ((src[{static_cast<uint16_t>(x+0), static_cast<uint16_t>(y + 1)}]).to_u8() << 1)
                    -   (src[{static_cast<uint16_t>(x+1), static_cast<uint16_t>(y + 1)}]).to_u8();

                // Find the direction and round angle to 0, 45, 90 or 135

                gm[w * y + x] = gvec_t{
                    .g = uint8_t((math::square(vx) + math::square(vy)) >> SHIFTS),
                    .t = xy_to_dir(vx, vy)};
            }
        }
    }

    // 3. Hysteresis Thresholding
    // 4. Non-maximum Suppression and output



    clear_corners(dst);
    for (size_t gy = 1; gy < size_t(roi.h())-1; gy++) {
        gvec_t * vc = &gm[gy * w];
        auto * dp = &dst.head_ptr()[gy * w];
        for (size_t gx = 1; gx < roi.w()-1u; gx++) {
            vc++;
            dp++;


            if (vc->g < low_squ) {
                // Not an edge
                *dp = Binary::black();
                continue;
                // Check if strong or weak edge
            } else if (vc->g >= high_squ){
                *dp = Binary::white();
            } else{
                if( gm[(gy - 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx-1))].g >= high_squ ||
                    gm[(gy - 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx+0))].g >= high_squ ||
                    gm[(gy - 1) * size_t(roi.w()) + static_cast<uint16_t>(gx + 1)].g >= high_squ ||
                    gm[(gy + 0) * size_t(roi.w()) + (static_cast<uint16_t>(gx-1))].g >= high_squ ||
                    gm[(gy + 0) * size_t(roi.w()) + static_cast<uint16_t>(gx + 1)].g >= high_squ ||
                    gm[(gy + 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx-1))].g >= high_squ ||
                    gm[(gy + 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx+0))].g >= high_squ ||
                    gm[(gy + 1) * size_t(roi.w()) + static_cast<uint16_t>(gx + 1)].g >= high_squ)
                {

                    *dp = Binary::white();
                } else {
                    // Not an edge
                    *dp = Binary::black();
                    continue;
                }
            }

            auto [va, vb] = [&](const Direction dir) -> std::tuple<gvec_t, gvec_t>{
                switch (dir) {
                    case Direction::R: {
                        return{
                            gm[(gy + 0) * size_t(roi.w()) + (static_cast<uint16_t>(gx-1))],
                            gm[(gy + 0) * size_t(roi.w()) + static_cast<uint16_t>(gx + 1)]
                        };
                    }

                    case Direction::UR: {
                        return{
                            gm[(gy + 1) * size_t(roi.w()) + static_cast<uint16_t>(gx + 1)],
                            gm[(gy - 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx-1))]
                        };
                    }

                    case Direction::U: {
                        return{
                            gm[(gy + 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx+0))],
                            gm[(gy - 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx+0))]
                        };
                    }

                    case Direction::UL: {
                        return{
                            gm[(gy + 1) * size_t(roi.w()) + (static_cast<uint16_t>(gx-1))],
                            gm[(gy - 1) * size_t(roi.w()) + static_cast<uint16_t>(gx + 1)]
                        };
                    }
                    default:
                    __builtin_unreachable();

                }
            }(vc->t);

            if (((vc->g < va.g) || (vc->g < vb.g))) {
                *dp = Binary::black();
            }
        }
    }
}

void eye(Image<Gray> &dst, const Image<Gray> &src){
    PANIC("todo");
    // using vec_t = Vec2<int8_t>;
    // #define math::square(x) (x * x)
    // static constexpr size_t SHIFTS = 3;

    // // sizeof(vec_t);
    // auto roi = src.size().to_rect();
    // auto gm = std::make_unique<vec_t[]>(roi.get_area());
    
    // const size_t w = size_t(roi.w());

    // //2. Finding Image Gradients
    // {
    //     for (size_t y = size_t(roi.y()) + 1; y < size_t(roi.y()) + size_t(roi.h()) - 1u; y++) {
    //         for (size_t x = size_t(roi.x()) + 1; x < size_t(roi.x()) + size_t(roi.w()) - 1u; x++) {
    //             int16_t vx = 0, vy = 0;

    //             //  1   0   -1
    //             //  2   0   -2
    //             //  1   0   -1

    //             vx = uint8_t(src[(y - 1) * w + static_cast<uint16_t>(x-1)])
    //                 - uint8_t(src[(y - 1) * w + x + 1])
    //                 + uint8_t(uint8_t(src[static_cast<uint16_t>(y + 0) * w + static_cast<uint16_t>(x-1)]) << 1)
    //                 - uint8_t(uint8_t(src[static_cast<uint16_t>(y + 0) * w + x + 1]) << 1)
    //                 + uint8_t(src[static_cast<uint16_t>(y + 1) * w + static_cast<uint16_t>(x-1)])
    //                 - uint8_t(src[static_cast<uint16_t>(y + 1) * w + x + 1]);

    //             //  1   2   1
    //             //  0   0   0
    //             //  -1  2   -1

    //             vy = uint8_t(src[(y - 1) * w + static_cast<uint16_t>(x-1)])
    //                 + uint8_t(uint8_t(src[(y - 1) * w + static_cast<uint16_t>(x+0)]) << 1)
    //                 + uint8_t(src[(y - 1) * w + x + 1])
    //                 - uint8_t(src[static_cast<uint16_t>(y + 1) * w + static_cast<uint16_t>(x-1)])
    //                 - uint8_t(uint8_t(src[static_cast<uint16_t>(y + 1) * w + static_cast<uint16_t>(x+0)]) << 1)
    //                 - uint8_t(src[static_cast<uint16_t>(y + 1) * w + x + 1]);

    //             // Find the direction and round angle to 0, 45, 90 or 135

    //             gm[w * y + x] = vec_t{
    //                 int8_t(vx >> SHIFTS), 
    //                 int8_t(vy >> SHIFTS)};
    //         }
    //     }
    // }

    // clear_corners(dst);

    // [[maybe_unused]] static constexpr size_t WINDOW_HALF_SIZE = 4;


    // using template_t = std::array<
    //     std::array<vec_t, WINDOW_HALF_SIZE * 2 + 1>, 
    // WINDOW_HALF_SIZE * 2 + 1>;

    // auto generate_template = []() -> template_t{
    //     template_t ret ;
    //     for(size_t y = -WINDOW_HALF_SIZE; y <= WINDOW_HALF_SIZE; y++){
    //         for(size_t x = -WINDOW_HALF_SIZE; x <= WINDOW_HALF_SIZE; x++){
    //             real_t rad = atan2(real_t(y), real_t(x));
    //             const auto [s, c] = sincos(rad);
    //             vec_t vec = vec_t{int8_t(s), int8_t(c)};
    //             // vec_t vec = vec_t{scale, 0};
    //             ret[y + WINDOW_HALF_SIZE][x + WINDOW_HALF_SIZE] = vec;
    //         }
    //     }
    //     return ret;
    // };

    // auto temp = generate_template();

    // for (size_t gy = WINDOW_HALF_SIZE; gy < size_t(roi.h()) - WINDOW_HALF_SIZE; gy++) {
    //     vec_t * vc = &gm[gy * w];
    //     auto * dp = &dst[gy * w];
    //     for (size_t gx = WINDOW_HALF_SIZE; gx < size_t(roi.w()) - WINDOW_HALF_SIZE; gx++) {
    //         vc++;
    //         dp++;

    //         // *dp = fast_sqrt_i<uint16_t>((vc->x * vc->x + vc->y * vc->y));

    //         // size_t x_sum = 0;
    //         // size_t y_sum = 0;
    //         size_t sum = 0;
    //         for(size_t y = -WINDOW_HALF_SIZE; y <= WINDOW_HALF_SIZE; y++){
    //             for(size_t x = -WINDOW_HALF_SIZE; x <= WINDOW_HALF_SIZE; x++){
    //                 const auto & vec = gm[(gy + y) * w + (gx + x)];
    //                 const auto & tvec = temp[y + WINDOW_HALF_SIZE][x + WINDOW_HALF_SIZE];
    //                 // x_sum += ABS(vec.x * tvec.x);
    //                 // y_sum += ABS(vec.y * tvec.y);
    //                 // sum += temp[3][3].length_squared();
    //                 // sum += tvec.length_squared();
    //                 sum += vec.x * tvec.x + vec.y * tvec.y;
    //             }
    //         }
    //         // size_t sum = fast_sqrt_i<int>(math::square(x_sum) + math::square(y_sum));
    //         // size_t sum = fast_sqrt_i<int>(math::square(x_sum) + math::square(y_sum));
    //         // size_t sum = MAX(x_sum, y_sum);
    //         // size_t sum =  x_sum * x_sum + y_sum * y_sum;
    //         *dp = Gray((ABS(sum) / ((WINDOW_HALF_SIZE * 2 + 1) * (WINDOW_HALF_SIZE * 2 + 1))) >> 4); 
    //     }
    // }
}

void adaptive_threshold(Image<Gray> & dst, const Image<Gray> & src) {
    if(dst.is_shared_with(src)){
        auto temp = Image<Gray>(src.size());
        adaptive_threshold(temp, src);
        dst = std::move(temp);
        return;
    }

    const auto [w,h] = math::Vec2u16{
        MIN(src.size().x, dst.size().x),
        MIN(src.size().y, dst.size().y),
    };

    static constexpr uint16_t WINDOW_HALF_SIZE = 3;
    static constexpr uint16_t LEAST_POINTS = 7;

    for(uint16_t y = WINDOW_HALF_SIZE; y < h - WINDOW_HALF_SIZE - 1u; y++){
        for(uint16_t x = WINDOW_HALF_SIZE; x < w - WINDOW_HALF_SIZE - 1u; x++){

            std::array<uint8_t, LEAST_POINTS> min_values;
            std::fill(min_values.begin(), min_values.end(), 255);
            for(uint16_t i=y-WINDOW_HALF_SIZE;i<=y+WINDOW_HALF_SIZE;i++){
                for(uint16_t j=x-WINDOW_HALF_SIZE;j<=x+WINDOW_HALF_SIZE;j++){
                    auto now_value = src[{j,i}].to_u8();
                    auto it = std::find_if(min_values.begin(), min_values.end(), 
                    [now_value](const uint8_t val){
                        return val > now_value;
                    });
                    if (it != min_values.end()) {
                        *it = now_value; // Replace the found value with the now value
                    }
                }
            }

            const auto ave = std::accumulate(min_values.begin(), min_values.end(), 0)/LEAST_POINTS;
            const auto raw = src[{x,y}];

            auto RELU = [](uint8_t _x) -> uint8_t {return (_x) > 0 ? (_x) : (0);};
            dst[{x,y}] = Gray::from_u8(CLAMP(RELU(
                raw.to_u8() - uint8_t(ave) - 30) * 8, 0, 255));
        }
    }
}
}