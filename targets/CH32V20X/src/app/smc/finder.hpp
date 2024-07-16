#ifndef __FINER_HPP__
#define __FINER_HPP__

#include "../../types/image/image.hpp"

#include "../nvcv2/flood_fill.hpp"
#include "../nvcv2/geometry/geometry.hpp"
#include "../nvcv2/pixels/pixels.hpp"
#include "../nvcv2/shape/shape.hpp"
#include "../nvcv2/template.hpp"

#include <vector>
#include <map>
#include <list>
#include <unordered_set>

using namespace NVCV2;

namespace SMC{
    using Coast = sstl::vector<Vector2i, 80>;
    using Boundry = std::map<int, int>;
    using Pile = std::pair<int, Rangei>;
    using Piles = std::map<int, Rangei>;
    using Point = Vector2i; 
    using Points = sstl::vector<Point, 64>;
    using Ranges = sstl::vector<Rangei, 64>;
    using NVCV2::Shape::Seed;
    using Segment = std::pair<Point, Point>;


    enum CornerType{
        ACORNER = -1,
        ALL = 0,
        VCORNER = 1
    };

    using Corner = std::pair<CornerType,const Vector2i &>;
    using Corners = sstl::vector<Corner, 8>;
    using Coasts = sstl::vector<Coast, 4>;


    std::tuple<Point, Rangei> get_entry(const ImageReadable<Binary> &, const Vector2i &, const LR);
    Piles get_x_piles(const ImageReadable<Binary> & src, Seed seed);
    // bool get_cross_point(const Point &p1,const Point &p2,const Point &q1,const Point &q2,Point & out);
    Rangei get_h_range(const ImageReadable<Binary> & src, const Vector2i & pos);
    Rangei get_side_range(const ImageReadable<Binary> & src, const int y, const int minimal_length, const LR);

    namespace PileUtils{
        bool invalidity(const Pile & pile, const Rangei & valid_width);
        bool invalidity(const Piles & piles, const Rangei & valid_width);
    }
    namespace CoastUtils{
        bool is_self_intersection(const Coast & coast);
        
        Rect2i bounding_box(const Coast & coast);

        Piles ypiles(const Coast & coast);

        Piles xpiles(const Coast & coast);
    
        Point which_in_window(const Coast & coast, const Rect2i & window);

        Point which_in_window(const Coast & coast, const Vector2i & window_size);

        Corners corners(const Coast & coast, const real_t threshold, const CornerType default_ct);

        Points acorners(const Coast & coast, const real_t threshold = real_t(-0.2));

        Points vcorners(const Coast & coast, const real_t threshold = real_t(-0.2));

        Coast trim(const Coast & coast, const Vector2i & window_size);
        Coast form(const ImageReadable<Binary> &, const Vector2i &, const LR);
        Coast constrain(const Coast & coast, const Rect2i & rect);
        Coast shrink(const Coast & line, const real_t width, const Vector2i & window_size);

        Coast douglas_peucker(const Coast & line, const real_t epsilon);

        __inline bool dir_until(const Coast & coast, const Vector2i & point,const Vector2i & dir){
            if(coast.size() < 2) return false;

            bool ret = true;
            for(auto it = coast.begin(); it != std::prev(coast.end()); ++it){
                auto currpoint = *it;
                auto nextpoint = *std::next(it);
                if((nextpoint - currpoint).sign() != dir.sign()) ret = false;
                if(nextpoint == point) break;
                return ret;
            }
            return false;
        }

        __inline bool ul_until(const Coast & coast, const Vector2i & point){
            return dir_until(coast, point, Vector2i(-1, -1));
        }

        __inline bool ur_until(const Coast & coast, const Vector2i & point){
            return dir_until(coast, point, Vector2i(1, -1));
        }

        __inline bool dl_until(const Coast & coast, const Vector2i & point){
            return dir_until(coast, point, Vector2i(-1, 1));
        }

        __inline bool dr_until(const Coast & coast, const Vector2i & point){
            return dir_until(coast, point, Vector2i(1, 1));
        }
    }


    namespace SegmentUtils{
        Segment constrain(const Segment &, const Rect2i &);
        Segment shift(const Segment &, const Point &);
        Vector2i vec(const Segment &);
    };

    namespace BoundryUtils{
        Boundry mean(const Boundry & left, const Boundry &);
        Coast to_coast(const Boundry & bound);
    };


    struct Circle{
            Vector2 pos;
            real_t r;
    };

    Circle calculate_cicular(const Vector2 &, const Vector2 &, const Vector2 &);

    Circle calculate_cicular(const Coast &, const int, const int);

    namespace WorldUtils{
        static constexpr uint8_t blind_rows = 15;
        static constexpr real_t scale = 0.02;
    
        __inline Vector2 position(const Point & point){
            Point centered = {point.x - 94, (-point.y + 45) + blind_rows};
            return centered * scale;
        }

        __inline real_t distance(const Point & p1, const Point & p2){
            return ((p2 - p1) * scale).length();
        }

        auto displacement(const Vector2_t<auto> & diff){
            return diff * scale;
        }
    }



};

#endif