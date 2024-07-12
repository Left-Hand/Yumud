#pragma once

#include "../nvcv2/nvcv2.hpp"



namespace NVCV2::Shape{
    namespace Cores{
        const int roberts_x[2][2] = {{-1, 0}, {0, 1}};
        const int roberts_y[2][2] = {{0, -1}, {1, 0}};
        const int prewiit_x[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
        const int prewiit_y[3][3] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
        const int sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
        const int sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
        const int scharr_x[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};
        const int scharr_y[3][3] = {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}};
        const int laplacian_4[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
        const int laplacian_8[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
        const int gauss[3][3] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}};
    }


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

    void gauss(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        auto size = dst.get_size();
        const auto & core = Cores::gauss;
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

    void gauss(ImageWithData<Grayscale, Grayscale> src){
        auto temp = src.space();
        gauss(temp, src);
        Pixels::copy(src, temp);
    }
    void sobel_x(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::sobel_x);}
    void sobel_y(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::sobel_y);}
    void scharr_x(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::scharr_x);}
    void scharr_y(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::scharr_y);}

    void sobel_xy(ImageWithData<Grayscale, Grayscale> & dst, const ImageReadable<Grayscale> & src){
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

    void dilate(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
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

    void dilate_xy(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
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

    void dilate_y(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
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

    void erosion(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
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

    void erosion_x(ImageWithData<Binary, Binary> & dst, ImageWithData<Binary, Binary> & src){
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

    void dilate_x(ImageWithData<Binary, Binary> & dst, ImageWithData<Binary, Binary> & src){
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

    void erosion_y(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
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

    void erosion_xy(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
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

    void x4(ImageWritable<bool> & dst, const ImageReadable<bool> & src){
        constexpr int m = 4;
        auto size = Rect2i{{}, src.get_size()}.intersection(Rect2i{{}, dst.get_size() * m});

        for(int y = 0; y < size.y / m; y++){
            for(int x = 0; x < size.x / m; x++){
                Vector2i base = Vector2i(x, y)* m;
                bool pixel = false;
                for(int j = 0; j < m; j++){
                    for(int i = 0; i < m; i++){
                        Vector2i src_pos = base + Vector2i(i, j);
                        pixel |= bool(src[src_pos]);
                    }
                }
                dst[{x,y}] = pixel;
            }
        }
    }

    void x4(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int m = 2){
        // constexpr int m = 4;
        auto size = (Rect2i{{}, src.get_size()}.intersection(Rect2i{{}, dst.get_size() * m})).size;

        for(int y = 0; y < size.y / m; y++){
            for(int x = 0; x < size.x / m; x++){
                Vector2i base = Vector2i(x, y)* m;
                uint16_t pixel = 0;
                for(int j = 0; j < m; j++){
                    for(int i = 0; i < m; i++){
                        Vector2i src_pos = base + Vector2i(i, j);
                        pixel += uint8_t(src(src_pos));
                    }
                }
                dst[{x,y}] = Grayscale(~uint8_t(pixel / (m * m)));
            }
        }
    }

    auto x4(const ImageReadable<Grayscale> & src, const int m = 2){
        ImageWithData<Grayscale, Grayscale> dst(src.size / m);
        x4(dst, src, m);
        return dst;
    }

    void x2(ImageWithData<Grayscale, Grayscale> & dst, const ImageWithData<Grayscale, Grayscale> & src){
        const auto size = dst.size;
        for(int y = 0; y < size.y; y++){
            for(int x = 0; x < size.x; x++){
                dst[{x,y}] = src[{x << 1,y << 1}];
            }
        }
    }

    auto x2(const ImageWithData<Grayscale, Grayscale> & src){
        ImageWithData<Grayscale, Grayscale> dst(src.size / 2);
        x2(dst, src);
        return dst;
    }

    void XN(ImageWithData<Binary, Binary> dst, const ImageWithData<Binary, Binary> & src, const int & m, const real_t & percent){
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

    void zhang_suen(ImageWithData<Binary, Binary> & dst,const ImageWithData<Binary, Binary> & src){

        if(src == dst){
            auto temp = src.clone();
            zhang_suen(temp, src);
            dst.copy_from(temp);
            return;
        }

        // Binary * temp = new Binary[size.x * size.y];
        auto size = dst.get_size();
        auto temp = src.clone();
        dst.copy_from(src);
        // memcpy(dst, src, size.x * size.y * sizeof(Binary));
        
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
            // memcpy(temp, dst, size.x * size.y * sizeof(Binary));
            temp.copy_from(dst);
            if (is_end) break;
        }
        
        // memcpy(dst, temp, size.x * size.y);
        dst.copy_from(temp);

    }

    void zhang_suen2(ImageWithData<Binary, Binary> & dst,const ImageWithData<Binary, Binary> & src){

        if(src == dst){
            auto temp = src.clone();
            zhang_suen2(temp, src);
            dst.copy_from(temp);
            return;
        }

        auto size = dst.get_size();
        auto temp = src.clone();
        dst.copy_from(src);
        
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
            temp.copy_from(dst);
            if (is_end) break;
        }
        
        // memcpy(dst, temp, size.x * size.y);
        dst.copy_from(temp);

    }

    void dilate(ImageWithData<Binary, Binary> & src){
        dilate(src, src);
    }

    void dilate_xy(ImageWithData<Binary, Binary> & src){
        dilate_xy(src, src);
    }


    void erosion(ImageWithData<Binary, Binary> & src){
        erosion(src, src);
    }

    void erosion_xy(ImageWithData<Binary, Binary> & src){
        erosion_xy(src, src);
    }

    void zhang_suen(ImageWithData<Binary, Binary> & src){
        zhang_suen(src, src);
    }

    void zhang_suen2(ImageWithData<Binary, Binary> & src){
        zhang_suen2(src, src);
    }

    void erosion_x(ImageWithData<Binary, Binary> & src){
        erosion_x(src, src);
    }
    
    void erosion_y(ImageWithData<Binary, Binary> & src){
        erosion_y(src, src);
    }

    void dilate_x(ImageWithData<Binary, Binary> & src){
        dilate_x(src, src);
    }


    void dilate_y(ImageWithData<Binary, Binary> & src){
        dilate_y(src, src);
    }

    void morph_open(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
        ImageWithData<Binary, Binary> temp(dst.get_size());        
        erosion(temp, src);
        dilate(dst, temp);
    }

    void morph_close(ImageWithData<Binary, Binary> & dst, const ImageWithData<Binary, Binary> & src){
        ImageWithData<Binary, Binary> temp(dst.get_size());        
        dilate(temp, src);
        erosion(dst, temp);
    }

    void morph_close(ImageWithData<Binary, Binary> & src){
        morph_close(src, src);
    }

    void morph_open(ImageWithData<Binary, Binary> & src){
        morph_open(src, src);
    }

    void convo_roberts_x(ImageWithData<Grayscale, Grayscale> & dst, ImageWithData<Grayscale, Grayscale> & src){
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

    void convo_roberts_xy(ImageWithData<Grayscale, Grayscale> & dst, ImageWithData<Grayscale, Grayscale> & src){
        if(src == dst){
            auto temp = src.clone();
            convo_roberts_xy(dst, temp);
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

    void convo_roberts_y(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        convolution(dst, src, Cores::roberts_y);
    }

    class CoastFinder{
    protected:
        using m_Image = ImageReadable<Binary>;

        const m_Image & src;
        Seed m_seed;
        uint32_t step_limit = 64;

    public:
        using Coast = sstl::vector<Vector2i, 80>;
        CoastFinder(const m_Image & image, const Seed & seed): src(image), m_seed(seed){;}

        Coast find(){
            Coast coast;
            Seed seed = m_seed;
            Vector2i p0;
            seed.reset();

            enum class SubStatus{
                APPROCH,
                TRACK
            };

            SubStatus status = SubStatus::APPROCH;

            while(seed.jounrey() < step_limit && src.has_point(seed)){
                switch(status){
                    case SubStatus::APPROCH:
                        seed.forward();
                        if(!src.has_point(seed)) break;

                        if(is_positive(seed)){
                            seed.backward();
                            
                            status = SubStatus::TRACK;
                        }
                        break;

                    case SubStatus::TRACK:
                        {
                            // if(Vector2i(seed) == p0) break;
                            static constexpr uint8_t spin_limit = 7;
                            int spins = 0;
                            if(is_edge(seed)){
                                while(is_edge(seed) && (spins < spin_limit)){
                                    seed.spin(true);
                                    spins++;
                                }
                            }else{
                                while((!is_edge(seed)) && (spins < spin_limit)){
                                    seed.spin(false);
                                    spins++;
                                }
                                seed.spin(true);
                            }

                            seed.forward();
                            if(src.has_point(seed)){
                                // if(coast.size() == 0) p0 = Vector2i(seed);
                                coast.push_back(seed);
                            }
                            break;
                        }
                    }
            }
            Coast ret;

            for(const auto & item : coast){
                if(src.has_point(item)){
                    ret.push_back(item);
                }else{
                    DEBUG_LOG("invalid point", item);
                }
            }

            return ret;
        }

        bool is_positive(const Vector2i & pos){
            return (uint8_t)src(pos);
        }

        bool is_edge(const Vector2i & pos, const Vector2i next_pos){
            // return (uint8_t)src(next_pos) - (uint8_t)src(pos) > (uint8_t)(Grayscale)(edge_threshold);
            return (!!src[next_pos]) && (!src[pos]); 
            // return ((uint8_t)src(next_pos) > positive_threshold) && ((uint8_t)src(pos) < positive_threshold);
        }

        bool is_edge(const Seed & seed){
            return is_edge(seed, seed.drop());
        }
    };

    
}


// using namespace NVCV2;

// void grayscaleimg_to_binary(const Grayscale * src, Binary * dst, const Vector2i & size, const Grayscale & threshold){
// 	size_t cnt = 0;
// 	for(int y = 0; y < size.y; y++){
// 		for(int x = 0; x < size.x; x++){
// 			dst[cnt] = src[cnt] > threshold;
// 			cnt++;
// 		}
// 	}
// }
