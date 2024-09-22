#include "shape.hpp"
#include "../pixels/pixels.hpp"
#include "../dsp/fastmath/sqrt.hpp"
#include "../dsp/fastmath/square.hpp"


namespace NVCV2::Shape{
    using Vector2 = Vector2_t<real_t>;
    using Vector2i = Vector2_t<int>;

    static void clear_corners(ImageWritable<monochrome auto> & dst){
        auto size = dst.get_size();
        static constexpr uint8_t targ_v = 0;
        for(int y = 0; y < size.y; y++) dst[{0, y}] = targ_v;
        for(int y = 0; y < size.y; y++) dst[{size.x-1, y}] = targ_v;
        for(int x = 0; x < size.x; x++) dst[{x, 0}] = targ_v;
        for(int x = 0; x < size.x; x++) dst[{x, size.y-1}] = targ_v;
    }


    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int core[3][3], const int div){
        auto size = dst.get_size();
        for(int y = 1; y < size.y-1; y++){
            for(int x = 1; x < size.x-1; x++){
                int pixel = 0;
                pixel += src[x - 1 + (y - 1) * size.x]	* core[0][0];
                pixel += src[x +  (y - 1) * size.x]		* core[0][1];
                pixel += src[x + 1 + (y - 1) * size.x]	* core[0][2];
                
                pixel += src[x - 1 + (y) * size.x]		* core[1][0];
                pixel += src[x +  (y) * size.x]			* core[1][1];
                pixel += src[x + 1 + (y) * size.x]		* core[1][2];
                
                pixel += src[x - 1 + (y + 1) * size.x] 	* core[2][0];
                pixel += src[x +  (y + 1) * size.x]		* core[2][1];
                pixel += src[x + 1 + (y + 1) * size.x]	* core[2][2];
                
                if(div != 1) pixel /= div;

                dst[x + y * size.x] = Grayscale(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }


    void gauss(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        clear_corners(dst);
        convolution(dst, src, Cores::gauss, 10);
    }


    void gauss5x5(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        auto size = dst.get_size();
        static constexpr auto core_radius = 2;
        static constexpr auto core_sum = 256;
    
        static constexpr uint8_t core[5][5] ={
            {1,     4,      6,      4,      1},
            {4,     16,     24,     16,     4},
            {6,     24,     36,     24,     6},
            {4,     16,     24,     16,     4},
            {1,     4,      6,      4,      1}
        };

        clear_corners(dst);
        for(int y = core_radius; y < size.y - core_radius; ++y){
            for(int x = core_radius; x < size.x - core_radius; ++x){
                uint32_t sum = 0;

                for(int dy = -core_radius; dy <= core_radius; ++dy){
                    for(int dx = -core_radius; dx <= core_radius; ++dx){
                        sum += src[Vector2i{x + dx, y + dy}] * core[dy + core_radius][dx + core_radius];
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

    Vector2i find_most(const Image<Grayscale> & src, const Grayscale & tg_color,  const Vector2i & point, const Vector2i & vec){
        Vector2i current_point = point;
        Vector2i delta_point = Vector2i(sign(vec.x), sign(vec.y));

        {
            while(true){
                if(not src.has_point(current_point)){
                    return {0,0};//nothing
                }
                // DEBUG_PRINTLN(current_point, src[current_point]);

                if(src[current_point] == tg_color){
                    break;
                }

                current_point += delta_point;
            }
        }

        auto eve = [](const Vector2i & _point, const Vector2i & _vec) -> int{
            return _point.dot(_vec);
        };

        int current_eve = eve(current_point, vec);
        while(true){
            Vector2i next_x_vec = current_point + Vector2i(sign(vec.x), 0);
            Vector2i next_y_vec = current_point + Vector2i(0, sign(vec.y));

            Vector2i * next_point = &current_point;
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
        auto size = dst.get_size();
        {
            const auto & core = Cores::sobel_x;
            for(int y = 1; y < size.y-1; y++){
                for(int x = 1; x < size.x-1; x++){
                    int pixel = 0;
                    pixel += src[x - 1 + (y - 1) * size.x]	* core[0][0];
                    pixel += src[x +  (y - 1) * size.x]		* core[0][1];
                    pixel += src[x + 1 + (y - 1) * size.x]	* core[0][2];
                    
                    pixel += src[x - 1 + (y) * size.x]		* core[1][0];
                    pixel += src[x +  (y) * size.x]			* core[1][1];
                    pixel += src[x + 1 + (y) * size.x]		* core[1][2];
                    
                    pixel += src[x - 1 + (y + 1) * size.x] 	* core[2][0];
                    pixel += src[x +  (y + 1) * size.x]		* core[2][1];
                    pixel += src[x + 1 + (y + 1) * size.x]	* core[2][2];
                    
                    dst[x + y * size.x] = Grayscale(CLAMP(ABS(pixel), 0, 255));
                }
            }
        }
        {
            const auto & core = Cores::sobel_y;
            for(int y = 1; y < size.y-1; y++){
                for(int x = 1; x < size.x-1; x++){
                    int pixel = 0;
                    pixel += src[x - 1 + (y - 1) * size.x]	* core[0][0];
                    pixel += src[x +  (y - 1) * size.x]		* core[0][1];
                    pixel += src[x + 1 + (y - 1) * size.x]	* core[0][2];
                    
                    pixel += src[x - 1 + (y) * size.x]		* core[1][0];
                    pixel += src[x +  (y) * size.x]			* core[1][1];
                    pixel += src[x + 1 + (y) * size.x]		* core[1][2];
                    
                    pixel += src[x - 1 + (y + 1) * size.x] 	* core[2][0];
                    pixel += src[x +  (y + 1) * size.x]		* core[2][1];
                    pixel += src[x + 1 + (y + 1) * size.x]	* core[2][2];
                    
                    dst[x + y * size.x] = std::max((uint8_t)dst[x + y *size.x], (uint8_t)CLAMP(ABS(pixel), 0, 255));
                }
            }
        }
    }

    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int core[2][2]){
        auto size = dst.get_size();
        for(int y = 1; y < size.y-1; y++){
            for(int x = 1; x < size.x-1; x++){
                int pixel = 0;

                pixel += src[x - 1 + (y - 1) * size.x]	* core[0][0];
                pixel += src[x +  (y - 1) * size.x]		* core[0][1];
                
                pixel += src[x - 1 + (y) * size.x]		* core[1][0];
                pixel += src[x +  (y) * size.x]			* core[1][1];
                
                dst[x + y * size.x] = Grayscale(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }

    void dilate(Image<Binary> & dst, const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            dilate(dst, temp);
            return;
        }

        for(int y = 1; y < size.y-1; y++){
            for(int x = 1; x < size.x-1; x++){
                bool pixel = false;
                pixel |= src[x - 1 + (y - 1) * size.x];
                pixel |= src[x +  (y - 1) * size.x];
                pixel |= src[x + 1 + (y - 1) * size.x];
                pixel |= src[x - 1 + (y) * size.x];
                pixel |= src[x +  (y) * size.x];
                pixel |= src[x + 1 + (y) * size.x];
                pixel |= src[x - 1 + (y + 1) * size.x];
                pixel |= src[x +  (y + 1) * size.x];
                pixel |= src[x + 1 + (y + 1) * size.x];
                
                dst[x + y * size.x] = pixel;
            }
        }
    }

    void dilate_xy(Image<Binary> & dst, const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            dilate_xy(dst, temp);
            return;
        }

        {   
            const int y = 0;
            auto * p_dst = (uint8_t *)&dst[1];
            auto * p_dst_end = (uint8_t *)&dst[size.x - 2];

            auto * p_src = (uint8_t *)&src[y * size.x + 1];
            auto * p_src_next = (uint8_t *)&src[(y+1) * size.x + 1];
            while(p_dst < p_dst_end){
                *p_dst = *(p_src - 1) | *(p_src) | *(p_src + 1) | *(p_src_next); 

                p_dst++;
                p_src++;
                p_src_next++;
            }
        }

        for(int y = 1; y < size.y-1; y++){
            auto * p_dst = (uint8_t *)&dst[y * size.x + 1];
            auto * p_src_last = (uint8_t *)&src[(y-1) * size.x + 1];
            auto * p_src = (uint8_t *)&src[y * size.x + 1];
            auto * p_src_next = (uint8_t *)&src[(y+1) * size.x + 1];
            for(int x = 1; x < size.x-1; x++){
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
            const int y = size.y - 1;
            auto * p_dst = (uint8_t *)&dst[y * size.x + 1];
            auto * p_dst_end = (uint8_t *)&dst[y * size.x + (size.x - 2)];

            auto * p_src_last = (uint8_t *)&src[(y-1) * size.x + 1];
            auto * p_src = (uint8_t *)&src[y * size.x + 1];

            while(p_dst != p_dst_end){
                *p_dst = *(p_src - 1) | *(p_src) | *(p_src + 1) | *(p_src_last); 

                p_dst++;
                p_src++;
                p_src_last++;
            }
        }
    }

    void dilate_y(Image<Binary> & dst, const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            dilate_y(dst, temp);
            return;
        }

        for(int y = 1; y < size.y-1; y++){
            for(int x = 0; x < size.x; x++){
                bool pixel = false;

                pixel |= src[x +  (y - 1) * size.x];
                pixel |= src[x +  (y) * size.x];
                pixel |= src[x +  (y + 1) * size.x];
                
                dst[x + y * size.x] = pixel;
            }
        }
    }

    void erosion(Image<Binary> & dst, const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            erosion(dst, temp);
            return;
        }

        for(int y = 1; y < size.y-1; y++){
            // auto * p_src = (uint8_t *)&src[y * size.x + 1];
            // auto * p_dst_last = (uint8_t *)&dst[(y-1) * size.x + 1];
            // auto * p_dst = (uint8_t *)&dst[y * size.x + 1];
            // auto * p_dst_next = (uint8_t *)&dst[(y+1) * size.x + 1];
            
            for(int x = 1; x < size.x-1; x++){
                bool pixel = true;
                pixel &= src[x - 1 + (y - 1) * size.x];
                pixel &= src[x +  (y - 1) * size.x];
                pixel &= src[x + 1 + (y - 1) * size.x];
                pixel &= src[x - 1 + (y) * size.x];
                pixel &= src[x +  (y) * size.x];
                pixel &= src[x + 1 + (y) * size.x];
                pixel &= src[x - 1 + (y + 1) * size.x];
                pixel &= src[x +  (y + 1) * size.x];
                pixel &= src[x + 1 + (y + 1) * size.x];
                
                dst[x + y * size.x] = pixel;
            }
        }
    }

    void erosion_x(Image<Binary> & dst, Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            erosion_x(dst, temp);
            return;
        }

        for(int y = 0; y < size.y; y++){
            auto * p_src = (uint8_t *)&src[y * size.x + 1];
            auto * p_dst = (uint8_t *)&dst[y * size.x + 1];
            uint8_t last_two = (*p_src - 1) & (*p_src);
            for (int x = 1; x < size.x - 1; ++x) {
                *p_dst = last_two & (*(p_src + 1));
                last_two = (*p_src) & (*(p_src + 1));

                // *p_dst= *(p_src - 1) & (*p_src) & (*(p_src + 1));
                
                p_src++;
                p_dst++; 
            }
        }
    }

    void dilate_x(Image<Binary> & dst, Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            dilate_x(dst, temp);
            return;
        }

        for(int y = 0; y < size.y; y++){
            auto * p_src = (uint8_t *)&src[y * size.x + 1];
            auto * p_dst = (uint8_t *)&dst[y * size.x + 1];
            uint8_t last_two = (*p_src - 1) & (*p_src);
            for (int x = 1; x < size.x - 1; ++x) {
                *p_dst = last_two | (*(p_src + 1));
                last_two = (*p_src) | (*(p_src + 1));

                p_src++;
                p_dst++; 
            }
        }
    }

    void erosion_y(Image<Binary> & dst, const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            erosion_y(dst, temp);
            return;
        }

        for(int y = 1; y < size.y-1; y++){
            for(int x = 0; x < size.x; x++){
                bool pixel = true;

                pixel &= src[x +  (y - 1) * size.x];
                pixel &= src[x +  (y) * size.x];
                pixel &= src[x +  (y + 1) * size.x];
                
                dst[x + y * size.x] = pixel;
            }
        }
    }

    void erosion_xy(Image<Binary> & dst, const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            erosion_xy(dst, temp);
            return;
        }

        for(int y = 0; y < size.y; y++){
            for(int x = 1; x < size.x-1; x++){
                bool pixel = true;
                pixel &= src[x +  MAX((y - 1), 0) * size.x];
                pixel &= src[x+  (y) * size.x];
                pixel &= src[x +  (y) * size.x];
                pixel &= src[x +  (y) * size.x];
                pixel &= src[x +  MIN((y + 1), size.y-1) * size.x];
                
                dst[x + y * size.x] = pixel;
            }
        }
    }

    auto x4(const ImageReadable<Grayscale> & src, const int m){
        Image<Grayscale> dst(src.get_size() / m);
        x4(dst, src, m);
        return dst;
    }


    Image<Grayscale> x2(const Image<Grayscale> & src){
        Image<Grayscale> dst(src.get_size() / 2);
        const auto size = dst.get_size();
        for(int y = 0; y < size.y; y++){
            for(int x = 0; x < size.x; x++){
                uint16_t sum = 0;

                for(int j = 0; j < 2; j++){
                    for(int i = 0; i < 2; i++){
                        sum += src[{(x << 1) + i,(y << 1) + j}];
                    }
                }

                dst[{x,y}] = sum / 4;

                // dst[{x,y}] = src[{x << 1,y << 1}];
            }
        }
        return dst;
    }

    void anti_pepper_x(Image<Binary> & dst,const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            anti_pepper_x(dst, temp);
            return;
        }

        for(int y = 0; y < size.y; y++){
            const auto * p_src = &src[y * size.x + 1];
            auto * p_dst = &dst[y * size.x + 1];
            uint8_t last_two = false;
            for (int x = 1; x < size.x - 2; ++x) {
                uint8_t next = bool(*(p_src + 1));
                *p_dst = ((last_two + next) > 1);
                last_two = uint8_t(bool((*p_src))) + next;

                p_src++;
                p_dst++; 
            }
        }
    }

    void anti_pepper_y(Image<Binary> & dst,const Image<Binary> & src){
        auto size = dst.get_size();

        if(src == dst){
            auto temp = src.clone();
            anti_pepper_y(dst, temp);
            return;
        }

        for(int x = 0; x < size.x - 1; ++x){
            auto * p_src = &src[x];
            auto * p_dst = &dst[x];
            uint8_t last_two = false;
            for (int y = 1; y < size.y - 2; ++y) {
                uint8_t next = bool(*(p_src + 1));
                *p_dst = ((last_two + next) > 1);
                last_two = uint8_t(bool((*p_src))) + next;

                p_src += size.x;
                p_dst += size.x;
            }
        }
    }

    void anti_pepper(Image<Binary> & dst,const Image<Binary> & src){
        auto size = dst.get_size();
        if(src == dst){
            auto temp = src.clone();
            anti_pepper(dst, temp);
            return;
        }

        for(int y = 0; y < size.y; y++){
            auto * p_src = &src[y * size.x + 1];
            auto * p_dst = &dst[y * size.x + 1];
            uint8_t last_two = true;
            for (int x = 1; x < size.x - 1; ++x) {
                uint8_t next = bool(*(p_src + 1));
                *p_dst = Binary((last_two + next) > 1);
                last_two = uint8_t(bool((*p_src))) + next;

                p_src++;
                p_dst++; 
            }
        }
    }


    void XN(Image<Binary> dst, const Image<Binary> & src, const int & m, const real_t & percent){
        auto size = src.get_size();
        int n = int(m * m * percent);

        for(int y = 0; y < size.y / m; y++){
            for(int x = 0; x < size.x / m; x++){
                Vector2i base = Vector2i(x, y)* m;
                int pixel = 0;
                for(int j = 0; j < m; j++){
                    for(int i = 0; i < m; i++){
                        Vector2i src_pos = base + Vector2i(i, j);
                        pixel += bool(src[src_pos]);
                    }
                Vector2i dst_pos = Vector2i(x,y);
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

        auto size = dst.get_size();
        auto temp = src.clone();
        dst.clone(src);
        
        uint8_t iter = 0;
        while (true) {

            bool is_end = true;

            for (int y = 0; y < size.y; ++y) {
                for (int x = 0; x < size.x; ++x) {
                    const Vector2i p{x,y};
                    // const Binary * p = &temp[x + y * size.x];
                    
                    if (temp[p] == 0) continue;
                    
                    Binary p1 = temp[p + Vector2i{0, -1}];
                    Binary p2 = temp[p + Vector2i{1, -1}];
                    Binary p3 = temp[p + Vector2i{1, 0}];
                    Binary p4 = temp[p + Vector2i{1, 1}];
                    Binary p5 = temp[p + Vector2i{0, 1}];
                    Binary p6 = temp[p + Vector2i{-1, 1}];
                    Binary p7 = temp[p + Vector2i{-1, 0}];
                    Binary p8 = temp[p + Vector2i{-1, -1}];


                    int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                            (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                            (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                            (p8 == 0 && p1 == 1) + (p1 == 0 && p2 == 1);
                    int B  = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
                    int m1 = (iter % 2 == 0) ? (p1 * p3 * p5) : (p1 * p3 * p7);
                    int m2 = (iter % 2 == 0) ? (p3 * p5 * p7) : (p1 * p5 * p7);
                    
                    if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0){
                        dst[x + y * size.x] = false;
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

        auto size = dst.get_size();
        auto temp = src.clone();
        dst.clone(src);
        
        uint8_t iter = 0;
        while (true) {
            bool is_end = true;
            for (int y = 1; y < size.y - 1; ++y) {
                for (int x = 1; x < size.x - 1; ++x) {
                    if(iter % 2 == 0){
                        if((x + y) % 2 != 0) continue;
                    }else{
                        if((x + y) % 2 == 0) continue;
                    }

                    const Vector2i base = Vector2i(x, y);
                    bool p1 = temp[base];
                    if (p1 == 0) continue;

                    bool p2 = temp[base + Vector2i(0, -1)];
                    bool p3 = temp[base + Vector2i(1, -1)];
                    bool p4 = temp[base + Vector2i(1, 0)];
                    bool p5 = temp[base + Vector2i(1, 1)];
                    bool p6 = temp[base + Vector2i(0, 1)];
                    bool p7 = temp[base + Vector2i(-1, 1)];
                    bool p8 = temp[base + Vector2i(-1, 0)];
                    bool p9 = temp[base + Vector2i(-1, -1)];

                    int B  = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
                    int C = ((!p2) && (p3 || p4)) + ((!p4) && (p5 || p6)) + ((!p6) && (p7 || p8)) + ((!p8) && (p9 || p2));
                    int m1 = (iter % 2 == 0) ? (p2 * p4 * p6) : (p2 * p4 * p8);
                    int m2 = (iter % 2 == 0) ? (p4 * p6 * p8) : (p2 * p6 * p8);
                    
                    if (C == 1 && (B >= 2 && B <= 7) && m1 == 0 && m2 == 0){
                        dst[x + y * size.x] = false;
                        is_end = false;
                    }
                }
            }
            iter++;
            // memcpy(temp, dst, size.x * size.y * sizeof(Binary));
            temp.clone(dst);
            if (is_end) break;
        }
        
        dst.clone(temp);

    }

    void convo_roberts_x(Image<Grayscale> & dst, Image<Grayscale> & src){
        if(src == dst){
            auto temp = src.clone();
            convo_roberts_x(dst, temp);
            return;
        }

        auto size = dst.get_size();
    
        for (int y = 0; y < size.y; ++y) {
            auto * p_src = &src[y * size.x];
            auto * p_dst = &dst[y * size.x];
            for (int x = 0; x < size.x - 1; ++x) {
                *p_dst = ABS(*p_src - *(p_src + 1));
                p_src++;
                p_dst++; 
            }
        }
    }

    void convo_roberts_xy(Image<Grayscale> & dst, Image<Grayscale> & src){
        if(src == dst){
            auto temp = src.space();
            convo_roberts_xy(temp, src);
            dst.clone(temp);
            return;
        }

        auto size = dst.get_size();
    
        for (int y = 0; y < size.y - 1; ++y) {
            auto * p_src = &src[y * size.x];
            auto * p_src2 = &src[(y + 1) * size.x];
            auto * p_dst = &dst[y * size.x];
            for (int x = 0; x < size.x - 1; ++x) {
                *p_dst = MAX(ABS(*p_src - *(p_src + 1)), ABS(*p_src - *(p_src2))) ;
                p_src++;
                p_dst++; 
                p_src2++;
            }
        }

        {
            int y = size.y - 1;
            auto * p_src = &src[y * size.x];
            auto * p_dst = &dst[y * size.x];
            for(int x = 0; x < size.x - 1; ++x) {
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

    void canny(Image<Binary> &dst, const Image<Grayscale> &src, const Range_t<uint16_t> & threshold){
        auto roi = src.get_view();

        const auto [low_thresh, high_thresh] = threshold;

        enum class gcmp_t:uint8_t{
            LESS,
            MED,
            MORE
        };
        
        struct gvec_t{
            uint8_t g;
            Direction t;
        }__packed;

        auto gm = new gvec_t[int(roi)];

        const int w = roi.w;

        #define FAST_SQUARE(x) (x * x)
        #define FAST_SQRT(x) ((uint16_t)fast_sqrt_i((uint16_t)x))
        
        scexpr uint shift_bits = 9;
        
        const uint8_t low_squ = FAST_SQUARE(low_thresh) >> shift_bits;
        const uint8_t high_squ = FAST_SQUARE(high_thresh) >> shift_bits;
        

        //2. Finding Image Gradients
        {
            for (int y = roi.y + 1; y < roi.y + roi.h - 1; y++) {
                for (int x = roi.x + 1; x < roi.x + roi.w - 1; x++) {
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
                        .g = uint8_t((FAST_SQUARE(vx) + FAST_SQUARE(vy)) >> shift_bits),
                        .t = xy_to_dir(vx, vy)};
                }
            }
        }

        // 3. Hysteresis Thresholding
        // 4. Non-maximum Suppression and output

        gvec_t *va = nullptr, *vb = nullptr;

        clear_corners(dst);

        for (int gy = 1; gy < roi.h-1; gy++) {
            gvec_t * vc = &gm[gy * w];
            auto * dp = &dst[gy * w];
            for (int gx = 1; gx < roi.w-1; gx++) {
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
                    if( gm[(gy - 1) * roi.w + (gx - 1)].g >= high_squ ||
                        gm[(gy - 1) * roi.w + (gx + 0)].g >= high_squ ||
                        gm[(gy - 1) * roi.w + (gx + 1)].g >= high_squ ||
                        gm[(gy + 0) * roi.w + (gx - 1)].g >= high_squ ||
                        gm[(gy + 0) * roi.w + (gx + 1)].g >= high_squ ||
                        gm[(gy + 1) * roi.w + (gx - 1)].g >= high_squ ||
                        gm[(gy + 1) * roi.w + (gx + 0)].g >= high_squ ||
                        gm[(gy + 1) * roi.w + (gx + 1)].g >= high_squ)
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
                        default:
                        case Direction::R: {
                            va = &gm[(gy + 0) * roi.w + (gx - 1)];
                            vb = &gm[(gy + 0) * roi.w + (gx + 1)];
                            break;
                        }

                        case Direction::UR: {
                            va = &gm[(gy + 1) * roi.w + (gx + 1)];
                            vb = &gm[(gy - 1) * roi.w + (gx - 1)];
                            break;
                        }

                        case Direction::U: {
                            va = &gm[(gy + 1) * roi.w + (gx + 0)];
                            vb = &gm[(gy - 1) * roi.w + (gx + 0)];
                            break;
                        }

                        case Direction::UL: {
                            va = &gm[(gy + 1) * roi.w + (gx - 1)];
                            vb = &gm[(gy - 1) * roi.w + (gx + 1)];
                            break;
                        }
                    }

                    if (((vc->g < va->g) || (vc->g < vb->g))) {
                        *dp = 0;
                    }
                }
            }
        }
        #undef FAST_SQRT
        #undef FAST_SQUARE8

        delete gm;
    }

    void eye(Image<Binary> &dst, const Image<Grayscale> &src, const Range_t<uint16_t> & threshold){
        auto roi = src.get_view();

        const auto [low_thresh, high_thresh] = threshold;

        struct gvec_t{
            uint16_t g:13;
            Direction t:3;
        }__packed;

        auto gm = new gvec_t[int(roi)];

        const int w = roi.w;

        #define FAST_SQUARE(x) (x * x)
        #define FAST_SQRT(x) ((uint16_t)fast_sqrt_i((uint16_t)x))


        //2. Finding Image Gradients
        {
            for (int y = roi.y + 1; y < roi.y + roi.h - 1; y++) {
                for (int x = roi.x + 1; x < roi.x + roi.w - 1; x++) {
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
                        FAST_SQRT(FAST_SQUARE(vx) + FAST_SQUARE(vy)),
                        xy_to_dir(vx, vy)};
                }
            }
        }

        // 3. Hysteresis Thresholding
        // 4. Non-maximum Suppression and output

        gvec_t *va = nullptr, *vb = nullptr;

        clear_corners(dst);

        for (int gy = 1; gy < roi.h-1; gy++) {
            gvec_t * vc = &gm[gy * w];
            auto * dp = &dst[gy * w];
            for (int gx = 1; gx < roi.w-1; gx++) {
                vc++;
                dp++;

                if (vc->g < low_thresh) {
                    // Not an edge
                    *dp = 0;
                    continue;
                    // Check if strong or weak edge
                } else if (vc->g >= high_thresh){
                    *dp = 255;
                } else{
                    if( gm[(gy - 1) * roi.w + (gx - 1)].g >= high_thresh ||
                        gm[(gy - 1) * roi.w + (gx + 0)].g >= high_thresh ||
                        gm[(gy - 1) * roi.w + (gx + 1)].g >= high_thresh ||
                        gm[(gy + 0) * roi.w + (gx - 1)].g >= high_thresh ||
                        gm[(gy + 0) * roi.w + (gx + 1)].g >= high_thresh ||
                        gm[(gy + 1) * roi.w + (gx - 1)].g >= high_thresh ||
                        gm[(gy + 1) * roi.w + (gx + 0)].g >= high_thresh ||
                        gm[(gy + 1) * roi.w + (gx + 1)].g >= high_thresh)
                    {

                        *dp = 255;
                    } else {
                        // Not an edge
                        *dp = 0;
                        continue;
                    }
                }

                if(true){
                    switch (vc->t) {
                        default:
                        case Direction::R: {
                            va = &gm[(gy + 0) * roi.w + (gx - 1)];
                            vb = &gm[(gy + 0) * roi.w + (gx + 1)];
                            break;
                        }

                        case Direction::UR: {
                            va = &gm[(gy + 1) * roi.w + (gx + 1)];
                            vb = &gm[(gy - 1) * roi.w + (gx - 1)];
                            break;
                        }

                        case Direction::U: {
                            va = &gm[(gy + 1) * roi.w + (gx + 0)];
                            vb = &gm[(gy - 1) * roi.w + (gx + 0)];
                            break;
                        }

                        case Direction::UL: {
                            va = &gm[(gy + 1) * roi.w + (gx - 1)];
                            vb = &gm[(gy - 1) * roi.w + (gx + 1)];
                            break;
                        }
                    }

                    if (((vc->g < va->g) || (vc->g < vb->g))) {
                        *dp = 0;
                    }
                }
            }
        }
        #undef FAST_SQRT
        #undef FAST_SQUARE8

        delete gm;
    }

    void adaptive_threshold(Image<Grayscale> & dst, const Image<Grayscale> & src) {
        if(dst == src){
            auto temp = dst.space();
            adaptive_threshold(temp, src);
            dst.clone(temp);
            return;
        }
    
        const auto size = (Rect2i(Vector2i(), dst.get_size()).intersection(Rect2i(Vector2i(), src.get_size()))).size;

        static constexpr int wid = 3;
        static constexpr int least_size = 7;

        for(int y = wid; y < size.y - wid - 1; y++){
            for(int x = wid; x < size.x - wid - 1; x++){

                std::array<uint8_t, least_size> min_values;
                std::fill(min_values.begin(), min_values.end(), 255);
                for(int i=y-wid;i<=y+wid;i++){
                    for(int j=x-wid;j<=x+wid;j++){
                        auto current_value = uint8_t(src[{j,i}]);
                        auto it = std::find_if(min_values.begin(), min_values.end(), [current_value](const uint8_t val){
                            return val > current_value;
                        });
                        if (it != min_values.end()) {
                            *it = current_value; // Replace the found value with the current value
                        }
                    }
                }

                auto ave = std::accumulate(min_values.begin(), min_values.end(), 0)/least_size;
                auto raw = src[{x,y}];

                #define RELU(x) ((x) > 0 ? (x) : 0)

                dst[{x,y}] = CLAMP(RELU(raw - ave - 30) * 8, 0, 255);

                #undef RELU
            }
        }
    }
}