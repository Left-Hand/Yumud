
#include "../nvcv2/flood_fill.hpp"
#include "../nvcv2/geometry/geometry.hpp"
#include "../nvcv2/pixels/pixels.hpp"
#include "../nvcv2/shape/shape.hpp"

#include <vector>
#include <map>
#include <list>
#include <unordered_set>

#include "elements.hpp"

using namespace ymd::nvcv2;

namespace SMC{
    enum class AlignMode:uint8_t{
        LEFT,
        RIGHT,
        BOTH,
        UPPER
    };


    using Boundry = std::map<int, int>;
    using Pile = std::pair<int, Rangei>;
    using Piles = std::map<int, Rangei>;
    using Point = Vector2i; 
    using ymd::nvcv2::Shape::Seed;
    using Segment = std::pair<const Point & ,const Point &>;


    scexpr int max_item_size = 64;
    scexpr int max_ranges_size = 16;

    using CoastItem = Vector2_t<uint8_t>;
    using Points = sstl::vector<Vector2_t<int16_t>, max_item_size>;
    using Coast = sstl::vector<CoastItem, max_item_size>;
    using Coasts = sstl::vector<Coast, 4>;
    using Ranges = sstl::vector<Range_t<int16_t>, max_ranges_size>;

    enum class CornerType:uint8_t{
        NONE,
        AC,
        VC,
        ALL
    };

    struct Corner{
    // protected:
    public:
        CornerType type;
        CoastItem point;
        CoastItem orientation = CoastItem();//TODO
    public:
        Corner(const CornerType _type, const CoastItem & _point):type(_type), point(_point){;}
        operator CoastItem () const { return point; }
        operator Point () const { return point; }

        // auto type() const { return m_type; }
        // auto point() const { return m_point; }
    };


    using Corners = sstl::vector<Corner, 8>;

    class CoastFinder{
    protected:
        using m_Image = ImageReadable<Binary>;

        const m_Image & src;
        Seed m_seed;
        uint32_t step_limit = max_item_size;

    public:
        CoastFinder(const m_Image & image, const Seed & seed): src(image), m_seed(seed){;}

        Coast find(){
            Coast coast;
            Seed seed = m_seed;
            seed.reset();

            enum class SubStatus{
                APPROCH,
                TRACK
            };

            SubStatus status = SubStatus::APPROCH;
            
            //定义上方的最高点 当回落超过阈值时停止寻找

            scexpr auto fallback_threshold = 3;
            auto min_y = Vector2i(m_seed).y;

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

                    case SubStatus::TRACK:{
                            scexpr uint8_t spin_limit = 7;
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
                                coast.push_back(Vector2i(seed));
                            }
                            break;
                        }
                    }

                auto now_y = Vector2i(seed).y;
                min_y = std::min(min_y, now_y);
                if(now_y - min_y > fallback_threshold){
                    break;
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
            return (uint8_t)src[pos];
        }

        bool is_edge(const Vector2i & pos, const Vector2i next_pos){
            // return (uint8_t)src(next_pos) - (uint8_t)src(pos) > (uint8_t)(Grayscale)(edge_threshold);
            // return ((uint8_t)src(next_pos) > positive_threshold) && ((uint8_t)src(pos) < positive_threshold);

            //本点为赛道 但下一点为堤岸
            return (bool(src[next_pos]) == true) && 
                    (bool(src[pos]) == false); 
        }

        bool is_edge(const Seed & seed){
            return is_edge(seed, seed.drop());
        }
    };


    int get_x_edges(const ImageReadable<Binary> & src, const int y);
    int get_x_edges(const ImageReadable<Grayscale> & src, const int y);

    std::tuple<Point, Rangei> get_entry(const ImageReadable<Binary> &, const Vector2i &, const AlignMode);
    Piles get_x_piles(const ImageReadable<Binary> & src, const Point);
    Rangei get_h_range(const ImageReadable<Binary> & src, const Vector2i & pos);
    Rangei get_side_range(const ImageReadable<Binary> & src, const int y, const int minimal_length, const AlignMode);

    namespace PileUtils{
        bool invalidity(const Pile & pile, const Rangei & valid_width);
        bool invalidity(const Piles & piles, const Rangei & valid_width);
    }
    namespace CoastUtils{
        bool is_self_intersection(const Coast & coast);
        
        bool is_single(const Coast &, const LR, const int fall_back = 6);

        int sigle_sign(const Coast &);
    
        Rect2i bounding_box(const Coast & coast);

        Piles ypiles(const Coast & coast);

        Piles xpiles(const Coast & coast);
    
        Point which_in_window(const Coast & coast, const Rect2i & window);

        Point which_in_window(const Coast & coast, const Vector2i & window_size);

        bool is_ccw(const Coast & coast, const bool);

        scexpr real_t default_corner_threshold = -0.4;

        Corners search_corners(const Coast & coast, const CornerType default_ct = CornerType::ALL, const real_t threshold = default_corner_threshold);

        // Points a_points(const Coast & coast, const real_t threshold = default_corner_threshold);//120 deg

        // Points v_points(const Coast & coast, const real_t threshold = default_corner_threshold);//120 deg

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

    namespace CornerUtils{
        const Corner * find_corner(const Corners & corners, const size_t from_index = 0, const CornerType ct = CornerType::NONE);
        const Corner * find_a(const Corners & corners, const size_t from_index = 0);
        const Corner * find_v(const Corners & corners, const size_t from_index = 0);
        size_t cnt_a(const Corners & corners, const size_t from_index = 0);
        size_t cnt_v(const Corners & corners, const size_t from_index = 0);
        Coast a_points(const Corners & corners);
        Coast v_points(const Corners & corners);
    };

    struct Circle{
            Vector2 pos;
            real_t r;
    };

    Circle calculate_cicular(const Vector2 &, const Vector2 &, const Vector2 &);

    Circle calculate_cicular(const Coast &, const int, const int);

    namespace WorldUtils{
        scexpr real_t scale = 0.014;
        scexpr int blind_rows = 15;
    
        __fast_inline Vector2 position(const Point & point){
            Vector2i centered = {point.x - 94, (-point.y + 45) + blind_rows};
            return centered * scale;
        }


        __fast_inline Vector2 displacement(const Point & p1, const Point & p2 = {0,0}){
            return ((p2 - p1) * scale);
        }
        __fast_inline real_t distance(const Point & p1, const Point & p2 = {0,0}){
            return displacement(p1, p2).length();
        }

        template<arithmetic T>
        __fast_inline real_t distance(const T & val){
            return val * scale;
        }

        __fast_inline real_t pixels(const real_t l){
            return l / scale;
        }
        
        __fast_inline real_t pixels(const Vector2 & p1, const Vector2 & p2 = {0,0}){
            return pixels((p2 - p1).length());
        }

    }
};

using SMC::Corner;
using SMC::Corners;
using SMC::CornerType;
using SMC::AlignMode;

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::Corner & corner){
    using namespace SMC;
    return os << '(' << corner.type << ',' << corner.point << ')';
}

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::CornerType type){
    using namespace SMC;
    switch(type){
        case CornerType::AC: return os << 'A';break;
        case CornerType::VC: return os << 'V';break;
        case CornerType::ALL: return os << 'W';break;
        default: return os << '?';break;
    };
}

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::AlignMode mode){
    using namespace SMC;
    switch(mode){
        case AlignMode::LEFT: return os << 'l';break;
        case AlignMode::RIGHT: return os << 'r';break;
        case AlignMode::BOTH: return os << 'b';break;
        default: return os << '?';break;
    };
}