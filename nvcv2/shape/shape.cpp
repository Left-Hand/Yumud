#include "shape.hpp"
#include "nvcv2/pixels/pixels.hpp"
#include "dsp/fastmath/sqrt.hpp"
#include "dsp/fastmath/square.hpp"
#include "core/math/realmath.hpp"

namespace ymd::nvcv2::Shape{

    static void clear_corners(ImageWritable<is_monochrome auto> & dst){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        scexpr uint8_t targ_v = 0;
        for(size_t y = 0; y < h; y++) dst[{0, y}] = targ_v;
        for(size_t y = 0; y < h; y++) dst[{w-1u, y}] = targ_v;
        for(size_t x = 0; x < w; x++) dst[{x, 0}] = targ_v;
        for(size_t x = 0; x < w; x++) dst[{x, h-1u}] = targ_v;
    }


    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const size_t core[3][3], const size_t div){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        for(size_t y = 1; y < h-1u; y++){
            for(size_t x = 1; x < w-1u; x++){
                size_t pixel = 0;
                pixel += src[x - 1 + (y - 1) * w]	* core[0][0];
                pixel += src[x +  (y - 1) * w]		* core[0][1];
                pixel += src[x + 1 + (y - 1) * w]	* core[0][2];
                
                pixel += src[x - 1 + (y) * w]		* core[1][0];
                pixel += src[x +  (y) * w]			* core[1][1];
                pixel += src[x + 1 + (y) * w]		* core[1][2];
                
                pixel += src[x - 1 + (y + 1) * w] 	* core[2][0];
                pixel += src[x +  (y + 1) * w]		* core[2][1];
                pixel += src[x + 1 + (y + 1) * w]	* core[2][2];
                
                if(div != 1) pixel /= div;

                dst[x + y * w] = Grayscale(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }


    void gauss(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        clear_corners(dst);
        convolution(dst, src, Cores::gauss, 10);
    }


    void gauss5x5(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        scexpr auto core_radius = 2u;
        scexpr auto core_sum = 256u;
    
        scexpr uint8_t core[5][5] ={
            {1,     4,      6,      4,      1},
            {4,     16,     24,     16,     4},
            {6,     24,     36,     24,     6},
            {4,     16,     24,     16,     4},
            {1,     4,      6,      4,      1}
        };

        clear_corners(dst);
        for(size_t y = core_radius; y < h - core_radius; ++y){
            for(size_t x = core_radius; x < w - core_radius; ++x){
                uint32_t sum = 0;

                for(int dy = -core_radius; dy <= int(core_radius); ++dy){
                    for(int dx = -core_radius; dx <= int(core_radius); ++dx){
                        sum += src[Vector2u{size_t(x + dx), size_t(y + dy)}] 
                            * core[size_t(dy + core_radius)][size_t(dx + core_radius)];
                    }
                }
                
                dst[{x,y}] = Grayscale(CLAMP(sum / core_sum, 0, 255));
            }
        }
    }

    void gauss(Image<Grayscale> & src){
        auto temp = src.space();
        gauss(temp, src);
        Pixels::copy(src, temp);
    }

    Vector2u find_most(const Image<Grayscale> & src, const Grayscale & tg_color,  const Vector2u & point, const Vector2u & vec){
        Vector2u current_point = point;
        Vector2u delta_point = Vector2u(sign(vec.x), sign(vec.y));

        {
            while(true){
                if(not src.size().has_point(current_point)){
                    return {0,0};//nothing
                }
                // DEBUG_PRINTLN(current_point, src[current_point]);

                if(src[current_point] == tg_color){
                    break;
                }

                current_point += delta_point;
            }
        }

        auto eve = [](const Vector2u & _point, const Vector2u & _vec) -> int{
            return _point.dot(_vec);
        };

        int current_eve = eve(current_point, vec);
        while(true){
            Vector2u next_x_vec = current_point + Vector2u(sign(vec.x), 0);
            Vector2u next_y_vec = current_point + Vector2u(0, sign(vec.y));

            Vector2u * next_point = &current_point;
            current_eve = eve(current_point, vec);

            if(src[next_x_vec] == tg_color){
                auto x_eve = eve(next_x_vec, vec);
                if(x_eve > current_eve){
                    next_point = &next_x_vec;
                    current_eve = x_eve;
                }
            }

            if(src[next_y_vec] == tg_color){
                auto y_eve = eve(next_y_vec, vec);
                if(y_eve > current_eve){
                    next_point = &next_y_vec;
                    current_eve = y_eve;
                }
            }

            if(next_point == &current_point){
                return current_point;
            }

            current_point = *next_point;
        }
        // return current_point;
    }
    void sobel_xy(Image<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        {

            const auto & core = Cores::sobel_x;
            for(size_t y = 1; y < h-1u; y++){
                for(size_t x = 1; x < w-1u; x++){
                    size_t pixel = 0;
                    pixel += src[x - 1 + (y - 1) * w]	* core[0][0];
                    pixel += src[x +  (y - 1) * w]		* core[0][1];
                    pixel += src[x + 1 + (y - 1) * w]	* core[0][2];
                    
                    pixel += src[x - 1 + (y) * w]		* core[1][0];
                    pixel += src[x +  (y) * w]			* core[1][1];
                    pixel += src[x + 1 + (y) * w]		* core[1][2];
                    
                    pixel += src[x - 1 + (y + 1) * w] 	* core[2][0];
                    pixel += src[x +  (y + 1) * w]		* core[2][1];
                    pixel += src[x + 1 + (y + 1) * w]	* core[2][2];
                    
                    dst[x + y * w] = Grayscale(CLAMP(ABS(pixel), 0, 255));
                }
            }
        }
        {
            const auto & core = Cores::sobel_y;
            for(size_t y = 1; y < h-1u; y++){
                for(size_t x = 1; x < w-1u; x++){
                    size_t pixel = 0;
                    pixel += src[x - 1 + (y - 1) * w]	* core[0][0];
                    pixel += src[x +  (y - 1) * w]		* core[0][1];
                    pixel += src[x + 1 + (y - 1) * w]	* core[0][2];
                    
                    pixel += src[x - 1 + (y) * w]		* core[1][0];
                    pixel += src[x +  (y) * w]			* core[1][1];
                    pixel += src[x + 1 + (y) * w]		* core[1][2];
                    
                    pixel += src[x - 1 + (y + 1) * w] 	* core[2][0];
                    pixel += src[x +  (y + 1) * w]		* core[2][1];
                    pixel += src[x + 1 + (y + 1) * w]	* core[2][2];
                    
                    dst[x + y * w] = std::max((uint8_t)dst[x + y *w], (uint8_t)CLAMP(ABS(pixel), 0, 255));
                }
            }
        }
    }

    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const size_t core[2][2]){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        for(size_t y = 1; y < h-1u; y++){
            for(size_t x = 1; x < w-1u; x++){
                size_t pixel = 0;

                pixel += src[x - 1 + (y - 1) * w]	* core[0][0];
                pixel += src[x +  (y - 1) * w]		* core[0][1];
                
                pixel += src[x - 1 + (y) * w]		* core[1][0];
                pixel += src[x +  (y) * w]			* core[1][1];
                
                dst[x + y * w] = Grayscale(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }

    void dilate(Image<Binary> & dst, const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            dilate(dst, temp);
            return;
        }

        for(size_t y = 1; y < h-1u; y++){
            for(size_t x = 1; x < w-1u; x++){
                bool pixel = false;
                pixel |= src[x - 1 + (y - 1) * w];
                pixel |= src[x +  (y - 1) * w];
                pixel |= src[x + 1 + (y - 1) * w];
                pixel |= src[x - 1 + (y) * w];
                pixel |= src[x +  (y) * w];
                pixel |= src[x + 1 + (y) * w];
                pixel |= src[x - 1 + (y + 1) * w];
                pixel |= src[x +  (y + 1) * w];
                pixel |= src[x + 1 + (y + 1) * w];
                
                dst[x + y * w] = pixel;
            }
        }
    }

    void dilate_xy(Image<Binary> & dst, const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            dilate_xy(dst, temp);
            return;
        }

        {   
            const size_t y = 0;
            auto * p_dst = reinterpret_cast<uint8_t *>(&dst[1]);
            auto * p_dst_end = reinterpret_cast<uint8_t *>(&dst[w - 2]);

            auto * p_src = reinterpret_cast<const uint8_t *>(&src[y * w + 1]);
            auto * p_src_next = reinterpret_cast<const uint8_t *>(&src[(y+1) * w + 1]);
            while(p_dst < p_dst_end){
                *p_dst = *(p_src - 1) | *(p_src) | *(p_src + 1) | *(p_src_next); 

                p_dst++;
                p_src++;
                p_src_next++;
            }
        }

        for(size_t y = 1; y < h-1u; y++){
            auto * p_dst = reinterpret_cast<uint8_t *>(&dst[y * w + 1]);
            auto * p_src_last = reinterpret_cast<const uint8_t *>(&src[(y-1) * w + 1]);
            auto * p_src = reinterpret_cast<const uint8_t *>(&src[y * w + 1]);
            auto * p_src_next = reinterpret_cast<const uint8_t *>(&src[(y+1) * w + 1]);
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
            auto * p_dst = reinterpret_cast<uint8_t *>(&dst[y * w + 1]);
            auto * p_dst_end = reinterpret_cast<uint8_t *>(&dst[y * w + (w - 2)]);

            auto * p_src_last = reinterpret_cast<const uint8_t *>(&src[(y-1) * w + 1]);
            auto * p_src = reinterpret_cast<const uint8_t *>(&src[y * w + 1]);

            while(p_dst != p_dst_end){
                *p_dst = *(p_src - 1) | *(p_src) | *(p_src + 1) | *(p_src_last); 

                p_dst++;
                p_src++;
                p_src_last++;
            }
        }
    }

    void dilate_y(Image<Binary> & dst, const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            dilate_y(dst, temp);
            return;
        }

        for(size_t y = 1; y < h-1u; y++){
            for(size_t x = 0; x < w; x++){
                bool pixel = false;

                pixel |= src[x +  (y - 1) * w];
                pixel |= src[x +  (y) * w];
                pixel |= src[x +  (y + 1) * w];
                
                dst[x + y * w] = pixel;
            }
        }
    }

    void erosion(Image<Binary> & dst, const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            erosion(dst, temp);
            return;
        }

        for(size_t y = 1; y < h-1u; y++){
            // auto * p_src = reinterpret_cast<uint8_t *>&src[y * w + 1];
            // auto * p_dst_last = reinterpret_cast<uint8_t *>&dst[(y-1) * w + 1];
            // auto * p_dst = reinterpret_cast<uint8_t *>&dst[y * w + 1];
            // auto * p_dst_next = reinterpret_cast<uint8_t *>&dst[(y+1) * w + 1];
            
            for(size_t x = 1; x < w-1u; x++){
                bool pixel = true;
                pixel &= src[x - 1 + (y - 1) * w];
                pixel &= src[x +  (y - 1) * w];
                pixel &= src[x + 1 + (y - 1) * w];
                pixel &= src[x - 1 + (y) * w];
                pixel &= src[x +  (y) * w];
                pixel &= src[x + 1 + (y) * w];
                pixel &= src[x - 1 + (y + 1) * w];
                pixel &= src[x +  (y + 1) * w];
                pixel &= src[x + 1 + (y + 1) * w];
                
                dst[x + y * w] = pixel;
            }
        }
    }

    void erosion_x(Image<Binary> & dst, const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            erosion_x(dst, temp);
            return;
        }

        for(size_t y = 0; y < h; y++){
            auto * p_src = reinterpret_cast<const uint8_t *>(&src[y * w + 1]);
            auto * p_dst = reinterpret_cast<uint8_t *>(&dst[y * w + 1]);
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
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            dilate_x(dst, temp);
            return;
        }

        for(size_t y = 0; y < h; y++){
            auto * p_src = reinterpret_cast<const uint8_t *>(&src[y * w + 1]);
            auto * p_dst = reinterpret_cast<uint8_t *>(&dst[y * w + 1]);
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
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            erosion_y(dst, temp);
            return;
        }

        for(size_t y = 1; y < h-1u; y++){
            for(size_t x = 0; x < w; x++){
                bool pixel = true;

                pixel &= src[x +  (y - 1) * w];
                pixel &= src[x +  (y) * w];
                pixel &= src[x +  (y + 1) * w];
                
                dst[x + y * w] = pixel;
            }
        }
    }

    void erosion_xy(Image<Binary> & dst, const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            erosion_xy(dst, temp);
            return;
        }

        for(size_t y = 0; y < h; y++){
            for(size_t x = 1; x < w-1u; x++){
                bool pixel = true;
                pixel &= src[x +  MAX((y - 1), 0) * w];
                pixel &= src[x+  (y) * w];
                pixel &= src[x +  (y) * w];
                pixel &= src[x +  (y) * w];
                pixel &= src[x +  MIN((y + 1), h-1u) * w];
                
                dst[x + y * w] = pixel;
            }
        }
    }

    auto x4(const ImageReadable<Grayscale> & src, const size_t m){
        Image<Grayscale> dst(src.size() / m);
        x4(dst, src, m);
        return dst;
    }


    Image<Grayscale> x2(const Image<Grayscale> & src){
        Image<Grayscale> dst(src.size() / 2);
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);

        for(size_t y = 0; y < h; y++){
            for(size_t x = 0; x < w; x++){
                uint16_t sum = 0;

                for(size_t j = 0; j < 2; j++){
                    for(size_t i = 0; i < 2; i++){
                        sum += src[{(x << 1) + i,(y << 1) + j}];
                    }
                }

                dst[{x,y}] = sum >> 2;

                // dst[{x,y}] = src[{x << 1,y << 1}];
            }
        }
        return dst;
    }

    void anti_pepper_x(Image<Binary> & dst,const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            anti_pepper_x(dst, temp);
            return;
        }

        for(size_t y = 0; y < h; y++){
            const auto * p_src = &src[y * w + 1];
            auto * p_dst = &dst[y * w + 1];
            uint8_t last_two = false;
            for (size_t x = 1; x < w - 2; ++x) {
                uint8_t next = bool(*(p_src + 1));
                *p_dst = ((last_two + next) > 1);
                last_two = uint8_t(bool((*p_src))) + next;

                p_src++;
                p_dst++; 
            }
        }
    }

    void anti_pepper_y(Image<Binary> & dst,const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);

        if(src == dst){
            auto temp = src.clone();
            anti_pepper_y(dst, temp);
            return;
        }

        for(size_t x = 0; x < w - 1; ++x){
            auto * p_src = &src[x];
            auto * p_dst = &dst[x];
            uint8_t last_two = false;
            for (size_t y = 1; y < h - 2; ++y) {
                uint8_t next = bool(*(p_src + 1));
                *p_dst = ((last_two + next) > 1);
                last_two = uint8_t(bool((*p_src))) + next;

                p_src += w;
                p_dst += w;
            }
        }
    }

    void anti_pepper(Image<Binary> & dst,const Image<Binary> & src){
        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        if(src == dst){
            auto temp = src.clone();
            anti_pepper(dst, temp);
            return;
        }

        for(size_t y = 0; y < h; y++){
            auto * p_src = &src[y * w + 1];
            auto * p_dst = &dst[y * w + 1];
            uint8_t last_two = true;
            for (size_t x = 1; x < w - 1; ++x) {
                uint8_t next = bool(*(p_src + 1));
                *p_dst = Binary((last_two + next) > 1);
                last_two = uint8_t(bool((*p_src))) + next;

                p_src++;
                p_dst++; 
            }
        }
    }


    void XN(Image<Binary> dst, const Image<Binary> & src, const size_t m, const real_t percent){
        const auto size = src.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);

        size_t n = int(percent * m * m);

        for(size_t y = 0; y < h / m; y++){
            for(size_t x = 0; x < w / m; x++){
                Vector2u base = Vector2u(x, y)* m;
                size_t pixel = 0;
                for(size_t j = 0; j < m; j++){
                    for(size_t i = 0; i < m; i++){
                        Vector2u src_pos = base + Vector2u(i, j);
                        pixel += bool(src[src_pos]);
                    }
                Vector2u dst_pos = Vector2u(x,y);
                dst[dst_pos] = Binary(bool(pixel > n));
                }
            }
        }
    }

    void zhang_suen(Image<Binary> & dst,const Image<Binary> & src){

        if(src == dst){
            auto temp = src.clone();
            zhang_suen(temp, src);
            dst.clone(temp);
            return;
        }

        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
        auto temp = src.clone();
        dst.clone(src);
        
        uint8_t iter = 0;
        while (true) {

            bool is_end = true;

            for (size_t y = 0; y < h; ++y) {
                for (size_t x = 0; x < w; ++x) {
                    const Vector2u p{x,y};
                    // const Binary * p = &temp[x + y * w];
                    
                    if (temp[p] == 0) continue;
                    
                    Binary p1 = temp[p + Vector2i{0, -1}];
                    Binary p2 = temp[p + Vector2i{1, -1}];
                    Binary p3 = temp[p + Vector2i{1, 0}];
                    Binary p4 = temp[p + Vector2i{1, 1}];
                    Binary p5 = temp[p + Vector2i{0, 1}];
                    Binary p6 = temp[p + Vector2i{-1, 1}];
                    Binary p7 = temp[p + Vector2i{-1, 0}];
                    Binary p8 = temp[p + Vector2i{-1, -1}];


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

        if(src == dst){
            auto temp = src.clone();
            zhang_suen2(temp, src);
            dst.clone(temp);
            return;
        }

        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
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

                    const Vector2u base = Vector2u(x, y);
                    bool p1 = temp[base];
                    if (p1 == 0) continue;

                    bool p2 = temp[base + Vector2u(0, -1)];
                    bool p3 = temp[base + Vector2u(1, -1)];
                    bool p4 = temp[base + Vector2u(1, 0)];
                    bool p5 = temp[base + Vector2u(1, 1)];
                    bool p6 = temp[base + Vector2u(0, 1)];
                    bool p7 = temp[base + Vector2u(-1, 1)];
                    bool p8 = temp[base + Vector2u(-1, 0)];
                    bool p9 = temp[base + Vector2u(-1, -1)];

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

    void convo_roberts_x(Image<Grayscale> & dst, const Image<Grayscale> & src){
        if(src == dst){
            auto temp = src.clone();
            convo_roberts_x(dst, temp);
            return;
        }

        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
    
        for (size_t y = 0; y < h; ++y) {
            const auto * p_src = &src[y * w];
            auto * p_dst = &dst[y * w];
            for (size_t x = 0; x < w - 1; ++x) {
                *p_dst = ABS(*p_src - *(p_src + 1));
                p_src++;
                p_dst++; 
            }
        }
    }

    void convo_roberts_xy(Image<Grayscale> & dst, const Image<Grayscale> & src){
        if(src == dst){
            auto temp = src.space();
            convo_roberts_xy(temp, src);
            dst.clone(temp);
            return;
        }

        const auto size = dst.size();
        const auto w = size_t(size.x);
        const auto h = size_t(size.y);
    
        for (size_t y = 0; y < h - 1; ++y) {
            const auto * p_src = &src[y * w];
            const auto * p_src2 = &src[(y + 1) * w];
            auto * p_dst = &dst[y * w];
            for (size_t x = 0; x < w - 1; ++x) {
                *p_dst = MAX(ABS(*p_src - *(p_src + 1)), ABS(*p_src - *(p_src2))) ;
                p_src++;
                p_dst++; 
                p_src2++;
            }
        }

        {
            size_t y = h - 1;
            auto * p_src = &src[y * w];
            auto * p_dst = &dst[y * w];
            for(size_t x = 0; x < w - 1; ++x) {
                *p_dst = ABS(*p_src - *(p_src + 1));
                p_src++;
                p_dst++; 
            }
        }
    }


    static __fast_inline Direction xy_to_dir(const int16_t x, const int16_t y){
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

        #undef TWO_AND_HALF
    } 

    void canny(Image<Binary> &dst, const Image<Grayscale> &src, const Range2<uint16_t> & threshold){
        auto roi = src.size().to_rect();

        const auto [low_thresh, high_thresh] = threshold;

        struct gvec_t{
            uint8_t g;
            Direction t;
        }__packed;

        auto gm = std::make_unique<gvec_t[]>(roi.get_area());

        const auto w = roi.w();
        
        scexpr size_t shift_bits = 9;
        
        const uint8_t low_squ = square(low_thresh) >> shift_bits;
        const uint8_t high_squ = square(high_thresh) >> shift_bits;
        

        //2. Finding Image Gradients
        {
            for (size_t y = roi.y() + 1; y < roi.y() + roi.h() - 1u; y++) {
                for (size_t x = roi.x() + 1; x < roi.x() + roi.w() - 1u; x++) {
                    int16_t vx = 0, vy = 0;

                    //  1   0   -1
                    //  2   0   -2
                    //  1   0   -1

                    vx = int(src[(y - 1) * w + x - 1])
                        - int(src[(y - 1) * w + x + 1])
                        + int(src[(y + 0) * w + x - 1] << 1)
                        - int(src[(y + 0) * w + x + 1] << 1)
                        + int(src[(y + 1) * w + x - 1])
                        - int(src[(y + 1) * w + x + 1]);

                    //  1   2   1
                    //  0   0   0
                    //  -1  2   -1

                    vy = int(src[(y - 1) * w + x - 1])
                        + int(src[(y - 1) * w + x + 0] << 1)
                        + int(src[(y - 1) * w + x + 1])
                        - int(src[(y + 1) * w + x - 1])
                        - int(src[(y + 1) * w + x + 0] << 1)
                        - int(src[(y + 1) * w + x + 1]);

                    // Find the direction and round angle to 0, 45, 90 or 135

                    gm[w * y + x] = gvec_t{
                        .g = uint8_t((square(vx) + square(vy)) >> shift_bits),
                        .t = xy_to_dir(vx, vy)};
                }
            }
        }

        // 3. Hysteresis Thresholding
        // 4. Non-maximum Suppression and output

        gvec_t *va = nullptr, *vb = nullptr;

        clear_corners(dst);
        for (size_t gy = 1; gy < size_t(roi.h())-1; gy++) {
            gvec_t * vc = &gm[gy * w];
            auto * dp = &dst[gy * w];
            for (size_t gx = 1; gx < roi.w()-1u; gx++) {
                vc++;
                dp++;


                if (vc->g < low_squ) {
                    // Not an edge
                    *dp = 0;
                    continue;
                    // Check if strong or weak edge
                } else if (vc->g >= high_squ){
                    *dp = 255;
                } else{
                    if( gm[(gy - 1) * size_t(roi.w()) + (gx - 1)].g >= high_squ ||
                        gm[(gy - 1) * size_t(roi.w()) + (gx + 0)].g >= high_squ ||
                        gm[(gy - 1) * size_t(roi.w()) + (gx + 1)].g >= high_squ ||
                        gm[(gy + 0) * size_t(roi.w()) + (gx - 1)].g >= high_squ ||
                        gm[(gy + 0) * size_t(roi.w()) + (gx + 1)].g >= high_squ ||
                        gm[(gy + 1) * size_t(roi.w()) + (gx - 1)].g >= high_squ ||
                        gm[(gy + 1) * size_t(roi.w()) + (gx + 0)].g >= high_squ ||
                        gm[(gy + 1) * size_t(roi.w()) + (gx + 1)].g >= high_squ)
                    {

                        *dp = 255;
                    } else {
                        // Not an edge
                        *dp = 0;
                        continue;
                    }
                }

                if(true){
                    switch (Direction(vc->t)) {
                        default: __builtin_unreachable();
                        case Direction::R: {
                            va = &gm[(gy + 0) * size_t(roi.w()) + (gx - 1)];
                            vb = &gm[(gy + 0) * size_t(roi.w()) + (gx + 1)];
                            break;
                        }

                        case Direction::UR: {
                            va = &gm[(gy + 1) * size_t(roi.w()) + (gx + 1)];
                            vb = &gm[(gy - 1) * size_t(roi.w()) + (gx - 1)];
                            break;
                        }

                        case Direction::U: {
                            va = &gm[(gy + 1) * size_t(roi.w()) + (gx + 0)];
                            vb = &gm[(gy - 1) * size_t(roi.w()) + (gx + 0)];
                            break;
                        }

                        case Direction::UL: {
                            va = &gm[(gy + 1) * size_t(roi.w()) + (gx - 1)];
                            vb = &gm[(gy - 1) * size_t(roi.w()) + (gx + 1)];
                            break;
                        }
                    }

                    if (((vc->g < va->g) || (vc->g < vb->g))) {
                        *dp = 0;
                    }
                }
            }
        }
    }

    void eye(Image<Grayscale> &dst, const Image<Grayscale> &src){

        using vec_t = Vector2<int8_t>;
        #define square(x) (x * x)
        scexpr size_t shift_bits = 3;
    
        // sizeof(vec_t);
        auto roi = src.size().to_rect();
        auto gm = std::make_unique<vec_t[]>(roi.get_area());
        
        const auto w = size_t(roi.w());

        //2. Finding Image Gradients
        {
            for (size_t y = size_t(roi.y()) + 1; y < size_t(roi.y()) + size_t(roi.h()) - 1u; y++) {
                for (size_t x = size_t(roi.x()) + 1; x < size_t(roi.x()) + size_t(roi.w()) - 1u; x++) {
                    int16_t vx = 0, vy = 0;

                    //  1   0   -1
                    //  2   0   -2
                    //  1   0   -1

                    vx = int(src[(y - 1) * w + x - 1])
                        - int(src[(y - 1) * w + x + 1])
                        + int(src[(y + 0) * w + x - 1] << 1)
                        - int(src[(y + 0) * w + x + 1] << 1)
                        + int(src[(y + 1) * w + x - 1])
                        - int(src[(y + 1) * w + x + 1]);

                    //  1   2   1
                    //  0   0   0
                    //  -1  2   -1

                    vy = int(src[(y - 1) * w + x - 1])
                        + int(src[(y - 1) * w + x + 0] << 1)
                        + int(src[(y - 1) * w + x + 1])
                        - int(src[(y + 1) * w + x - 1])
                        - int(src[(y + 1) * w + x + 0] << 1)
                        - int(src[(y + 1) * w + x + 1]);

                    // Find the direction and round angle to 0, 45, 90 or 135

                    gm[w * y + x] = vec_t{
                        int8_t(vx >> shift_bits), 
                        int8_t(vy >> shift_bits)};
                }
            }
        }

        clear_corners(dst);

        scexpr size_t WINDOW_HALF_SIZE = 4;

        using template_t = std::array<std::array<vec_t, WINDOW_HALF_SIZE * 2 + 1>, WINDOW_HALF_SIZE * 2 + 1>;

        auto generate_template = []() -> template_t{
            template_t ret;
            for(size_t y = -WINDOW_HALF_SIZE; y <= WINDOW_HALF_SIZE; y++){
                for(size_t x = -WINDOW_HALF_SIZE; x <= WINDOW_HALF_SIZE; x++){
                    real_t rad = atan2(real_t(y), real_t(x));
                    const auto [s, c] = sincos(rad);
                    vec_t vec = vec_t{int8_t(s), int8_t(c)};
                    // vec_t vec = vec_t{scale, 0};
                    ret[y + WINDOW_HALF_SIZE][x + WINDOW_HALF_SIZE] = vec;
                }
            }
            return ret;
        };

        auto temp = generate_template();

        for (size_t gy = WINDOW_HALF_SIZE; gy < size_t(roi.h()) - WINDOW_HALF_SIZE; gy++) {
            vec_t * vc = &gm[gy * w];
            auto * dp = &dst[gy * w];
            for (size_t gx = WINDOW_HALF_SIZE; gx < size_t(roi.w()) - WINDOW_HALF_SIZE; gx++) {
                vc++;
                dp++;

                // *dp = fast_sqrt_i<uint16_t>((vc->x * vc->x + vc->y * vc->y));

                // size_t x_sum = 0;
                // size_t y_sum = 0;
                size_t sum = 0;
                for(size_t y = -WINDOW_HALF_SIZE; y <= WINDOW_HALF_SIZE; y++){
                    for(size_t x = -WINDOW_HALF_SIZE; x <= WINDOW_HALF_SIZE; x++){
                        const auto & vec = gm[(gy + y) * w + (gx + x)];
                        const auto & tvec = temp[y + WINDOW_HALF_SIZE][x + WINDOW_HALF_SIZE];
                        // x_sum += ABS(vec.x * tvec.x);
                        // y_sum += ABS(vec.y * tvec.y);
                        // sum += temp[3][3].length_squared();
                        // sum += tvec.length_squared();
                        sum += vec.x * tvec.x + vec.y * tvec.y;
                    }
                }
                // size_t sum = fast_sqrt_i<int>(square(x_sum) + square(y_sum));
                // size_t sum = fast_sqrt_i<int>(square(x_sum) + square(y_sum));
                // size_t sum = MAX(x_sum, y_sum);
                // size_t sum =  x_sum * x_sum + y_sum * y_sum;
                *dp = (ABS(sum) / ((WINDOW_HALF_SIZE * 2 + 1) * (WINDOW_HALF_SIZE * 2 + 1))) >> 4; 
            }
        }
    }

    void adaptive_threshold(Image<Grayscale> & dst, const Image<Grayscale> & src) {
        if(dst == src){
            auto temp = dst.space();
            adaptive_threshold(temp, src);
            dst.clone(temp);
            return;
        }
    
        const auto [w,h] = Vector2u{
            MIN(src.size().x, dst.size().x),
            MIN(src.size().y, dst.size().y),
        };

        static constexpr size_t WINDOW_HALF_SIZE = 3;
        static constexpr size_t LEAST_POINTS = 7;

        for(size_t y = WINDOW_HALF_SIZE; y < h - WINDOW_HALF_SIZE - 1u; y++){
            for(size_t x = WINDOW_HALF_SIZE; x < w - WINDOW_HALF_SIZE - 1u; x++){

                std::array<uint8_t, LEAST_POINTS> min_values;
                std::fill(min_values.begin(), min_values.end(), 255);
                for(size_t i=y-WINDOW_HALF_SIZE;i<=y+WINDOW_HALF_SIZE;i++){
                    for(size_t j=x-WINDOW_HALF_SIZE;j<=x+WINDOW_HALF_SIZE;j++){
                        auto current_value = uint8_t(src[{j,i}]);
                        auto it = std::find_if(min_values.begin(), min_values.end(), [current_value](const uint8_t val){
                            return val > current_value;
                        });
                        if (it != min_values.end()) {
                            *it = current_value; // Replace the found value with the current value
                        }
                    }
                }

                const auto ave = std::accumulate(min_values.begin(), min_values.end(), 0)/LEAST_POINTS;
                const auto raw = src[{x,y}];

                auto RELU = [](uint8_t _x) -> uint8_t {return (_x) > 0 ? (_x) : (0);};
                dst[{x,y}] = CLAMP(RELU(raw - ave - 30) * 8, 0, 255);
            }
        }
    }
}