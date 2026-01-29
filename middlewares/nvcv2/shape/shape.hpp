#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"
#include "core/math/matrix/static_matrix.hpp"

#include <algorithm>
#include <bits/stl_numeric.h>

namespace ymd::nvcv2::shape{
    namespace cores{
        static constexpr auto roberts_x      = math::Matrix<int8_t, 2, 2>{
            -1, 0, 0, 1};
        static constexpr auto roberts_y      = math::Matrix<int8_t, 2, 2>{
            0, -1, 1, 0};
        static constexpr auto prewiit_x      = math::Matrix<int8_t, 3, 3>{
            -1, 0, 1, -1, 0, 1, -1, 0, 1};
        static constexpr auto prewiit_y      = math::Matrix<int8_t, 3, 3>{
            -1, -1, -1, 0, 0, 0, 1, 1, 1};
        static constexpr auto sobel_x        = math::Matrix<int8_t, 3, 3>{
            -1, 0, 1, -2, 0, 2, -1, 0, 1};
        static constexpr auto sobel_y        = math::Matrix<int8_t, 3, 3>{
            -1, -2, -1, 0, 0, 0, 1, 2, 1};
        static constexpr auto scharr_x       = math::Matrix<int8_t, 3, 3>{
            -3, 0, 3, -10, 0, 10, -3, 0, 3};
        static constexpr auto scharr_y       = math::Matrix<int8_t, 3, 3>{
            -3, -10, -3, 0, 0, 0, 3, 10, 3};
        static constexpr auto laplacian_4    = math::Matrix<int8_t, 3, 3>{
            0, -1, 0, -1, 4, -1, 0, -1, 0};
        static constexpr auto laplacian_8    = math::Matrix<int8_t, 3, 3>{
            -1, -1, -1, -1, 8, -1, -1, -1, -1};
        static constexpr auto gauss          = math::Matrix<int8_t, 3, 3>{
            1, 1, 1, 1, 2, 1, 1, 1, 1};
    }


    void convolution(
        Image<Gray> & dst, 
        const Image<Gray> & src, 
        const math::Matrix<int8_t, 3, 3> & core);

    void convolution(
        Image<Gray> & dst, 
        const Image<Gray> & src, 
        const math::Matrix<int8_t, 2, 2> & core);

    void gauss(Image<Gray> & dst, const Image<Gray> & src);
    void gauss(Image<Gray> &src);
    void gauss5x5(Image<Gray> & dst, const Image<Gray> & src);

    math::Vec2i find_most(
        const Image<Gray> & src, 
        const Gray & tg_color, 
        const math::Vec2i & point, 
        const math::Vec2i & vec);
    __inline void sobel_x(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::sobel_x);}
    __inline void sobel_y(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::sobel_y);}
    __inline void scharr_x(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::scharr_x);}
    __inline void scharr_y(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::scharr_y);}

    void sobel_xy(Image<Gray> & dst, const Image<Gray> & src);



    void dilate(Image<Binary> & dst, const Image<Binary> & src);

    void dilate_xy(Image<Binary> & dst, const Image<Binary> & src);

    void dilate_y(Image<Binary> & dst, const Image<Binary> & src);

    void erosion(Image<Binary> & dst, const Image<Binary> & src);

    void erosion_x(Image<Binary> & dst, const Image<Binary> & src);

    void dilate_x(Image<Binary> & dst, const Image<Binary> & src);

    void erosion_y(Image<Binary> & dst, const Image<Binary> & src);

    void erosion_xy(Image<Binary> & dst, const Image<Binary> & src);

    void x4(Image<Binary> & dst, const Image<Binary> & src);

    void x4(Image<Gray> & dst, const Image<Gray> & src, const size_t m = 2);

    auto x4(const Image<Gray> & src, const size_t m = 2);

    Image<Gray> x2(const Image<Gray> & src);

    void XN(
        Image<Binary> dst,
        const Image<Binary> & src, 
        const size_t m, 
        const real_t percent);

    void zhang_suen(Image<Binary> & dst, const Image<Binary> & src);

    void zhang_suen2(Image<Binary> & dst, const Image<Binary> & src);

    void anti_pepper_x(Image<Binary> & dst, const Image<Binary> & src);

    void anti_pepper_y(Image<Binary> & dst, const Image<Binary> & src);

    void anti_pepper(Image<Binary> & dst, const Image<Binary> & src);
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

    __inline void morph_open(Image<Binary> & dst, const Image<Binary> & src){
        Image<Binary> temp(dst.size());        
        erosion(temp, src);
        dilate(dst, temp);
    }

    __inline void morph_close(Image<Binary> & dst, const Image<Binary> & src){
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

    void convo_roberts_x(Image<Gray> & dst, const Image<Gray> & src);

    void convo_roberts_xy(Image<Gray> & dst, const Image<Gray> & src);

    void adaptive_threshold(Image<Gray> & dst, const Image<Gray> & src);
    __inline void convo_roberts_y(Image<Gray> & dst, const Image<Gray> & src){
        convolution(dst, src, cores::roberts_y);
    }


    void canny(Image<Binary> & dst, const Image<Gray> & src, const math::Range2<uint16_t> & threshold);
    void eye(Image<Gray> & dst, const Image<Gray> & src);


    struct Seed final{
        public:            

            uint8_t warp_dir_index(const int8_t & _dir) const{
                if(_dir > 7){
                    return 0;
                }else if(_dir < 0){
                    return 7;
                }
                return _dir;
            }

            math::Vec2i forward(){
                jounrey_ ++;
                return now_position_ += dirs[dir_index_];
            }

            math::Vec2i backward(){
                jounrey_--;
                return now_position_ -= dirs[dir_index_];
            }

            math::Vec2i drop() const{
                return now_position_ + dirs[warp_dir_index(dir_index_)];
            }

            uint8_t jounrey() const{
                return jounrey_;
            }

            void reset(){
                now_position_ = default_position_;
                jounrey_ = 0;
            }

            void move_to(const math::Vec2i& pos){
                now_position_ = pos;
            }

            void spin(){
                dir_index_ = warp_dir_index(dir_index_ + (is_cw_? -1 : 1));
            }

            void spin(const bool not_inverse){
                dir_index_ = warp_dir_index(dir_index_ + (is_cw_ ^ (!not_inverse) ? -1 : 1));
            }

            bool is_cw()const{
                return is_cw_;
            }

            Seed(const math::Vec2i & pos, const Direction & index = Direction::L, const bool _cw = true):
                default_position_(pos), 
                now_position_(pos), 
                dir_index_((uint8_t)index), 
                is_cw_(_cw)
                {;}

            // Seed(const math::Vec2i & pos)
            auto & operator = (const math::Vec2i & pos){
                now_position_ = pos;
                jounrey_ = 0;
                return *this;
            }

            operator math::Vec2i()const{
                return now_position_;
                // jounrey_ = 0;
                // return *this;
            }

        private:
            math::Vec2i default_position_;
            math::Vec2i now_position_;
            uint8_t dir_index_;
            uint8_t jounrey_ = 0;
            const bool is_cw_;

            static constexpr std::array<math::Vec2i, 8>dirs = {
                math::Vec2i{1,0},
                math::Vec2i{1,-1}, 
                math::Vec2i{0, -1}, 
                math::Vec2i{-1,-1},
                math::Vec2i{-1,0},
                math::Vec2i{-1,1},
                math::Vec2i{0,1},
                math::Vec2i{1,1}
            };
    };
    


    
}
