#ifndef __NVCV2_HPP__

#define __NVCV2_HPP__

#include "sys/platform.h"

#include "types/color/color_t.hpp"
#include "types/image/image.hpp"
#include "types/image/packed_image.hpp"
#include "types/image/painter.hpp"

#include "thirdparty/sstl/include/sstl/vector.h" 
#include "thirdparty/sstl/include/sstl/function.h" 


#include <vector>
#include <map>
#include <list>
#include <unordered_set>

namespace NVCV2{
    enum TB{
        TOP = false,
        BOTTOM = true
    };

    enum LR{
        LEFT = false,
        RIGHT = true
    };

    enum CornerType{
        ACORNER = -1,
        ALL = 0,
        VCORNER = 1
    };


    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };


    class Seed{
        public:            

            // Vector2i get_next_dir(const bool ccw){
            //     warp_dir_index(dir + )
            //     return dirs[dir_index];
            // }

            uint8_t warp_dir_index(const int8_t & _dir) const{
                if(_dir > 7){
                    return 0;
                }else if(_dir < 0){
                    return 7;
                }
                return _dir;
            }

            Vector2i forward(){
                m_jounrey ++;
                return run_pos += dirs[dir_index];
            }

            Vector2i backward(){
                m_jounrey--;
                return run_pos -= dirs[dir_index];
            }

            Vector2i drop() const{
                return run_pos + dirs[warp_dir_index(dir_index)];
            }

            uint8_t jounrey() const{
                return m_jounrey;
            }

            void reset(){
                run_pos = default_pos;
                m_jounrey = 0;
            }

            void move_to(const Vector2i& pos){
                run_pos = pos;
            }

            void spin(){
                dir_index = warp_dir_index(dir_index + (cw? -1 : 1));
            }

            void spin(const bool & not_inverse){
                dir_index = warp_dir_index(dir_index + (cw ^ (!not_inverse) ? -1 : 1));
            }

            bool is_cw()const{
                return cw;
            }

            Seed(const Vector2i & pos, const Direction & index = Direction::L, const bool _cw = true):default_pos(pos), dir_index((uint8_t)index), run_pos(pos), cw(_cw){;}

            // Seed(const Vector2i & pos)
            auto & operator = (const Vector2i & pos){
                run_pos = pos;
                m_jounrey = 0;
                return *this;
            }

            operator Vector2i()const{
                return run_pos;
                // m_jounrey = 0;
                // return *this;
            }

        protected:
            Vector2i default_pos;
            uint8_t dir_index;
            Vector2i run_pos;
            uint8_t m_jounrey = 0;
            const bool cw;

            static constexpr std::array<Vector2i, 8>dirs = {
                Vector2i{1,0},
                Vector2i{1,-1}, 
                Vector2i{0, -1}, 
                Vector2i{-1,-1},
                Vector2i{-1,0},
                Vector2i{-1,1},
                Vector2i{0,1},
                Vector2i{1,1}
            };
    };

    using Coast = sstl::vector<Vector2i, 80>;
    using Boundary = std::map<int, int>;
    using Pile = std::pair<int, Rangei>;
    using Piles = std::map<int, Rangei>;
    using Point = Vector2i; 
    using Points = sstl::vector<Point, 64>;
    using Ranges = sstl::vector<Rangei, 64>;

    using Segment = std::pair<Point, Point>;
    using Corner = std::pair<CornerType,Vector2i>;
    using Corners = sstl::vector<Corner, 8>;
    using Coasts = sstl::vector<Coast, 4>;

    using Histogram = std::array<int, 256>;
}




#endif