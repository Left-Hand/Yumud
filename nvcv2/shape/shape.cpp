#include "shape.hpp"
#include "../pixels/pixels.hpp"


namespace NVCV2::Shape{


    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int core[3][3]){
        auto size = dst.get_size();
        for(int y = 1; y < size.y-1; y++){
            for(int x = 1; x < size.x-1; x++){
                int pixel = 0;
                pixel += src(x - 1 + (y - 1) * size.x)	* core[0][0];
                pixel += src(x +  (y - 1) * size.x)		* core[0][1];
                pixel += src(x + 1 + (y - 1) * size.x)	* core[0][2];
                
                pixel += src(x - 1 + (y) * size.x)		* core[1][0];
                pixel += src(x +  (y) * size.x)			* core[1][1];
                pixel += src(x + 1 + (y) * size.x)		* core[1][2];
                
                pixel += src(x - 1 + (y + 1) * size.x) 	* core[2][0];
                pixel += src(x +  (y + 1) * size.x)		* core[2][1];
                pixel += src(x + 1 + (y + 1) * size.x)	* core[2][2];
                
                dst[x + y * size.x] = Grayscale(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }

    static void clear_corners(ImageWritable<Grayscale> & dst){
        auto size = dst.get_size();
        static constexpr Grayscale targ_v = 255;
        for(int y = 0; y < size.y; y++) dst[{0, y}] = targ_v;
        for(int y = 0; y < size.y; y++) dst[{size.x-1, y}] = targ_v;
        for(int x = 0; x < size.x; x++) dst[{x, 0}] = targ_v;
        for(int x = 0; x < size.x; x++) dst[{x, size.y-1}] = targ_v;
    }

    void gauss(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        auto size = dst.get_size();
        const auto & core = Cores::gauss;
        clear_corners(dst);
        for(int y = 1; y < size.y-1; y++){
            for(int x = 1; x < size.x-1; x++){
                int pixel = 0;
                pixel += src(x - 1 + (y - 1) * size.x)	* core[0][0];
                pixel += src(x +  (y - 1) * size.x)		* core[0][1];
                pixel += src(x + 1 + (y - 1) * size.x)	* core[0][2];
                
                pixel += src(x - 1 + (y) * size.x)		* core[1][0];
                pixel += src(x +  (y) * size.x)			* core[1][1];
                pixel += src(x + 1 + (y) * size.x)		* core[1][2];
                
                pixel += src(x - 1 + (y + 1) * size.x) 	* core[2][0];
                pixel += src(x +  (y + 1) * size.x)		* core[2][1];
                pixel += src(x + 1 + (y + 1) * size.x)	* core[2][2];

                pixel /= 10;
                
                dst[x + y * size.x] = Grayscale(CLAMP(ABS(pixel), 0, 255));
            }
        }
    }

    void gauss(Image<Grayscale> src){
        auto temp = src.space();
        gauss(temp, src);
        Pixels::copy(src, temp);
    }

    void sobel_xy(Image<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        auto size = dst.get_size();
        {
            const auto & core = Cores::sobel_x;
            for(int y = 1; y < size.y-1; y++){
                for(int x = 1; x < size.x-1; x++){
                    int pixel = 0;
                    pixel += src(x - 1 + (y - 1) * size.x)	* core[0][0];
                    pixel += src(x +  (y - 1) * size.x)		* core[0][1];
                    pixel += src(x + 1 + (y - 1) * size.x)	* core[0][2];
                    
                    pixel += src(x - 1 + (y) * size.x)		* core[1][0];
                    pixel += src(x +  (y) * size.x)			* core[1][1];
                    pixel += src(x + 1 + (y) * size.x)		* core[1][2];
                    
                    pixel += src(x - 1 + (y + 1) * size.x) 	* core[2][0];
                    pixel += src(x +  (y + 1) * size.x)		* core[2][1];
                    pixel += src(x + 1 + (y + 1) * size.x)	* core[2][2];
                    
                    dst[x + y * size.x] = Grayscale(CLAMP(ABS(pixel), 0, 255));
                }
            }
        }
        {
            const auto & core = Cores::sobel_y;
            for(int y = 1; y < size.y-1; y++){
                for(int x = 1; x < size.x-1; x++){
                    int pixel = 0;
                    pixel += src(x - 1 + (y - 1) * size.x)	* core[0][0];
                    pixel += src(x +  (y - 1) * size.x)		* core[0][1];
                    pixel += src(x + 1 + (y - 1) * size.x)	* core[0][2];
                    
                    pixel += src(x - 1 + (y) * size.x)		* core[1][0];
                    pixel += src(x +  (y) * size.x)			* core[1][1];
                    pixel += src(x + 1 + (y) * size.x)		* core[1][2];
                    
                    pixel += src(x - 1 + (y + 1) * size.x) 	* core[2][0];
                    pixel += src(x +  (y + 1) * size.x)		* core[2][1];
                    pixel += src(x + 1 + (y + 1) * size.x)	* core[2][2];
                    
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

                pixel += src(x - 1 + (y - 1) * size.x)	* core[0][0];
                pixel += src(x +  (y - 1) * size.x)		* core[0][1];
                
                pixel += src(x - 1 + (y) * size.x)		* core[1][0];
                pixel += src(x +  (y) * size.x)			* core[1][1];
                
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
                pixel |= src(x - 1 + (y - 1) * size.x);
                pixel |= src(x +  (y - 1) * size.x);
                pixel |= src(x + 1 + (y - 1) * size.x);
                pixel |= src(x - 1 + (y) * size.x);
                pixel |= src(x +  (y) * size.x);
                pixel |= src(x + 1 + (y) * size.x);
                pixel |= src(x - 1 + (y + 1) * size.x);
                pixel |= src(x +  (y + 1) * size.x);
                pixel |= src(x + 1 + (y + 1) * size.x);
                
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

                pixel |= src(x +  (y - 1) * size.x);
                pixel |= src(x +  (y) * size.x);
                pixel |= src(x +  (y + 1) * size.x);
                
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
                pixel &= src(x - 1 + (y - 1) * size.x);
                pixel &= src(x +  (y - 1) * size.x);
                pixel &= src(x + 1 + (y - 1) * size.x);
                pixel &= src(x - 1 + (y) * size.x);
                pixel &= src(x +  (y) * size.x);
                pixel &= src(x + 1 + (y) * size.x);
                pixel &= src(x - 1 + (y + 1) * size.x);
                pixel &= src(x +  (y + 1) * size.x);
                pixel &= src(x + 1 + (y + 1) * size.x);
                
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

                pixel &= src(x +  (y - 1) * size.x);
                pixel &= src(x +  (y) * size.x);
                pixel &= src(x +  (y + 1) * size.x);
                
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
                pixel &= src(x +  MAX((y - 1), 0) * size.x);
                pixel &= src(x+  (y) * size.x);
                pixel &= src(x +  (y) * size.x);
                pixel &= src(x +  (y) * size.x);
                pixel &= src(x +  MIN((y + 1), size.y-1) * size.x);
                
                dst[x + y * size.x] = pixel;
            }
        }
    }

    auto x4(const ImageReadable<Grayscale> & src, const int m){
        Image<Grayscale> dst(src.size / m);
        x4(dst, src, m);
        return dst;
    }


    Image<Grayscale> x2(const Image<Grayscale> & src){
        Image<Grayscale> dst(src.size / 2);
        const auto size = dst.size;
        for(int y = 0; y < size.y; y++){
            for(int x = 0; x < size.x; x++){
                dst[{x,y}] = src[{x << 1,y << 1}];
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
            auto * p_src = &src[y * size.x + 1];
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
                *p_dst = ((last_two + next) > 1);
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
                    
                    if (temp(p) == 0) continue;
                    
                    Binary p1 = temp(p + Vector2i{0, -1});
                    Binary p2 = temp(p + Vector2i{1, -1});
                    Binary p3 = temp(p + Vector2i{1, 0});
                    Binary p4 = temp(p + Vector2i{1, 1});
                    Binary p5 = temp(p + Vector2i{0, 1});
                    Binary p6 = temp(p + Vector2i{-1, 1});
                    Binary p7 = temp(p + Vector2i{-1, 0});
                    Binary p8 = temp(p + Vector2i{-1, -1});


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
                    bool p1 = temp(base);
                    if (p1 == 0) continue;

                    bool p2 = temp(base + Vector2i(0, -1));
                    bool p3 = temp(base + Vector2i(1, -1));
                    bool p4 = temp(base + Vector2i(1, 0));
                    bool p5 = temp(base + Vector2i(1, 1));
                    bool p6 = temp(base + Vector2i(0, 1));
                    bool p7 = temp(base + Vector2i(-1, 1));
                    bool p8 = temp(base + Vector2i(-1, 0));
                    bool p9 = temp(base + Vector2i(-1, -1));

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

}