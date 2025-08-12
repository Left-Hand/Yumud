#pragma once

#include "../nvcv2/nvcv2.hpp"
#include "types/matrix/matrix_static.hpp"

#include <algorithm>
#include <bits/stl_numeric.h>

namespace ymd::nvcv2::shape{
    namespace cores{
        static constexpr auto roberts_x      = Matrix<int8_t, 2, 2>{-1, 0, 0, 1};
        static constexpr auto roberts_y      = Matrix<int8_t, 2, 2>{0, -1, 1, 0};
        static constexpr auto prewiit_x      = Matrix<int8_t, 3, 3>{-1, 0, 1, -1, 0, 1, -1, 0, 1};
        static constexpr auto prewiit_y      = Matrix<int8_t, 3, 3>{-1, -1, -1, 0, 0, 0, 1, 1, 1};
        static constexpr auto sobel_x        = Matrix<int8_t, 3, 3>{-1, 0, 1, -2, 0, 2, -1, 0, 1};
        static constexpr auto sobel_y        = Matrix<int8_t, 3, 3>{-1, -2, -1, 0, 0, 0, 1, 2, 1};
        static constexpr auto scharr_x       = Matrix<int8_t, 3, 3>{-3, 0, 3, -10, 0, 10, -3, 0, 3};
        static constexpr auto scharr_y       = Matrix<int8_t, 3, 3>{-3, -10, -3, 0, 0, 0, 3, 10, 3};
        static constexpr auto laplacian_4    = Matrix<int8_t, 3, 3>{0, -1, 0, -1, 4, -1, 0, -1, 0};
        static constexpr auto laplacian_8    = Matrix<int8_t, 3, 3>{-1, -1, -1, -1, 8, -1, -1, -1, -1};
        static constexpr auto gauss          = Matrix<int8_t, 3, 3>{1, 1, 1, 1, 2, 1, 1, 1, 1};
    }


    void convolution(
        __restrict Image<Gray> & dst, 
        __restrict const Image<Gray> & src, 
        const Matrix<int8_t, 3, 3> & core, 
        const int div = 1);

    void convolution(
        Image<Gray> & dst, 
        const Image<Gray> & src, 
        const Matrix<int8_t, 2, 2> & core);

    void gauss(Image<Gray> & dst, const Image<Gray> & src);
    void gauss(Image<Gray> &src);
    void gauss5x5(Image<Gray> & dst, const Image<Gray> & src);

    Vec2i find_most(
        const Image<Gray> & src, 
        const Gray & tg_color, 
        const Vec2i & point, 
        const Vec2i & vec);
    __inline void sobel_x(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::sobel_x);}
    __inline void sobel_y(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::sobel_y);}
    __inline void scharr_x(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::scharr_x);}
    __inline void scharr_y(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::scharr_y);}

    void sobel_xy(Image<Gray> & dst, const Image<Gray> & src);



    void dilate(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void dilate_xy(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void dilate_y(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void erosion(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void erosion_x(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void dilate_x(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void erosion_y(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void erosion_xy(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src);

    void x4(Image<Binary> & dst, const Image<Binary> & src);

    void x4(Image<Gray> & dst, const Image<Gray> & src, const int m = 2);

    auto x4(const Image<Gray> & src, const int m = 2);

    Image<Gray> x2(const Image<Gray> & src);

    void XN(
        __restrict Image<Binary> dst,
        __restrict const Image<Binary> & src, 
        const int m, 
        const real_t percent);

    void zhang_suen(Image<Binary> & dst,const Image<Binary> & src);

    void zhang_suen2(Image<Binary> & dst,const Image<Binary> & src);

    void anti_pepper_x(Image<Binary> & dst,const Image<Binary> & src);

    void anti_pepper_y(Image<Binary> & dst,const Image<Binary> & src);

    void anti_pepper(Image<Binary> & dst,const Image<Binary> & src);
    __inline void dilate(Image<Binary> & src){
        dilate(src, src);
    }

    __inline void dilate_xy(Image<Binary> & src){
        dilate_xy(src, src);
    }


    __inline void erosion(Image<Binary> & src){
        erosion(src, src);
    }

    __inline void erosion_xy(Image<Binary> & src){
        erosion_xy(src, src);
    }

    __inline void zhang_suen(Image<Binary> & src){
        zhang_suen(src, src);
    }

    __inline void zhang_suen2(Image<Binary> & src){
        zhang_suen2(src, src);
    }

    __inline void erosion_x(Image<Binary> & src){
        erosion_x(src, src);
    }
    
    __inline void erosion_y(Image<Binary> & src){
        erosion_y(src, src);
    }

    __inline void dilate_x(Image<Binary> & src){
        dilate_x(src, src);
    }


    __inline void dilate_y(Image<Binary> & src){
        dilate_y(src, src);
    }

    __inline void morph_open(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src){
        Image<Binary> temp(dst.size());        
        erosion(temp, src);
        dilate(dst, temp);
    }

    __inline void morph_close(__restrict Image<Binary> & dst,__restrict const Image<Binary> & src){
        Image<Binary> temp(dst.size());        
        dilate(temp, src);
        erosion(dst, temp);
    }

    __inline void morph_close(Image<Binary> & src){
        morph_close(src, src);
    }

    __inline void morph_open(Image<Binary> & src){
        morph_open(src, src);
    }

    void convo_roberts_x(__restrict Image<Gray> & dst,__restrict const Image<Gray> & src);

    void convo_roberts_xy(__restrict Image<Gray> & dst,__restrict const Image<Gray> & src);

    void adaptive_threshold(__restrict Image<Gray> & dst,__restrict const Image<Gray> & src);
    __inline void convo_roberts_y(__restrict Image<Gray> & dst,__restrict const Image<Gray> & src){
        convolution(dst, src, cores::roberts_y);
    }


    void canny(Image<Binary> & dst,const Image<Gray> & src, const Range2<uint16_t> & threshold);
    void eye(Image<Gray> & dst, const Image<Gray> & src);
    class Seed{
        public:            

            uint8_t warp_dir_index(const int8_t & _dir) const{
                if(_dir > 7){
                    return 0;
                }else if(_dir < 0){
                    return 7;
                }
                return _dir;
            }

            Vec2i forward(){
                m_jounrey ++;
                return run_pos += dirs[dir_index];
            }

            Vec2i backward(){
                m_jounrey--;
                return run_pos -= dirs[dir_index];
            }

            Vec2i drop() const{
                return run_pos + dirs[warp_dir_index(dir_index)];
            }

            uint8_t jounrey() const{
                return m_jounrey;
            }

            void reset(){
                run_pos = default_pos;
                m_jounrey = 0;
            }

            void move_to(const Vec2i& pos){
                run_pos = pos;
            }

            void spin(){
                dir_index = warp_dir_index(dir_index + (cw? -1 : 1));
            }

            void spin(const bool not_inverse){
                dir_index = warp_dir_index(dir_index + (cw ^ (!not_inverse) ? -1 : 1));
            }

            bool is_cw()const{
                return cw;
            }

            Seed(const Vec2i & pos, const Direction & index = Direction::L, const bool _cw = true):default_pos(pos), dir_index((uint8_t)index), run_pos(pos), cw(_cw){;}

            // Seed(const Vec2i & pos)
            auto & operator = (const Vec2i & pos){
                run_pos = pos;
                m_jounrey = 0;
                return *this;
            }

            operator Vec2i()const{
                return run_pos;
                // m_jounrey = 0;
                // return *this;
            }

        protected:
            Vec2i default_pos;
            uint8_t dir_index;
            Vec2i run_pos;
            uint8_t m_jounrey = 0;
            const bool cw;

            static constexpr std::array<Vec2i, 8>dirs = {
                Vec2i{1,0},
                Vec2i{1,-1}, 
                Vec2i{0, -1}, 
                Vec2i{-1,-1},
                Vec2i{-1,0},
                Vec2i{-1,1},
                Vec2i{0,1},
                Vec2i{1,1}
            };
    };
    


    
}
