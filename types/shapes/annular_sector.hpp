#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"
#include "core/utils/angle_range.hpp"

namespace ymd{

template<typename T, typename D>
struct AnnularSector;

template<typename T, typename D>
struct BoundingBoxOf<AnnularSector<T, D>>;


template<typename T, typename D>
struct AnnularSector final{
    static_assert(not std::is_integral_v<D>);

    Vec2<T> center;
    Range2<T> radius_range;
    AngleRange<D> angle_range;

    constexpr bool contains_angle(const Angle<D> angle) const {
        return angle_range.contains_angle(angle);
    }

    constexpr 
    Rect2<T> bounding_box() const {
        auto & self = *this;
        const bool x_reached_left = self.angle_range.
            contains_angle(Angle<D>::HALF_LAP);
        const bool x_reached_right = self.angle_range.
            contains_angle(Angle<D>::ZERO);
        const bool y_reached_top = self.angle_range.
            contains_angle(Angle<D>::QUARTER_LAP);
        const bool y_reached_bottom = self.angle_range.
            contains_angle(Angle<D>::NEG_QUARTER_LAP);

        const Vec2<q16> n1 = Vec2<q16>::from_angle(self.angle_range.start);
        const Vec2<q16> n2 = Vec2<q16>::from_angle(self.angle_range.stop());

        const Vec2<q16> p1 = (n1).flip_y() * self.radius_range.stop;
        const Vec2<q16> p2 = (n2).flip_y() * self.radius_range.stop;

        const Vec2<q16> p3 = (n1).flip_y() * self.radius_range.start;
        const Vec2<q16> p4 = (n2).flip_y() * self.radius_range.start;

        const T x_min = floor_cast<T>(self.center.x + (x_reached_left ?     
            static_cast<q16>(-self.radius_range.stop) : MIN(p1.x, p2.x, p3.x, p4.x)));
        const T y_min = floor_cast<T>(self.center.y + (y_reached_top ?      
            static_cast<q16>(-self.radius_range.stop) : MIN(p1.y, p2.y, p3.y, p4.y)));

        const T x_max = ceil_cast<T>(self.center.x + (x_reached_right ?    
            static_cast<q16>(self.radius_range.stop) : MAX(p1.x, p2.x, p3.x, p4.x)));
        const T y_max = ceil_cast<T>(self.center.y + (y_reached_bottom ?   
            static_cast<q16>(self.radius_range.stop) : MAX(p1.y, p2.y, p3.y, p4.y)));

        return Rect2<T>(
            Vec2<T>{
                x_min, y_min
            },

            Vec2<T>{
                static_cast<T>(x_max - x_min), 
                static_cast<T>(y_max - y_min)
            }
        );
    }

    __fast_inline constexpr bool contains_point(
        const Vec2<T>& p
    ) const{
        const auto offset = p - center;
        const auto p_r_squ = offset.length_squared();
        if(p_r_squ < square(radius_range.start)) return false;
        if(p_r_squ > square(radius_range.stop)) return false;
        return contains_angle(offset.angle());
    }

    __fast_inline constexpr bool contains_norm_dir(
        const Vec2<D> norm_dir_vec
    ) const {
        const auto n1 = Vec2<D>::from_angle(angle_range.start);
        const auto n2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            n1, n2, n2.is_counter_clockwise_to(n1)
        );
    }

    friend class BoundingBoxOf<AnnularSector<T, D>>;

private:
    static constexpr bool contains_angle_helper(
        const T angle,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        return contains_angle_helper(
            Vec2<D>::from_angle(angle), 
            start_norm_vec, 
            stop_norm_vec, 
            is_minor
        );
    }

    static constexpr bool contains_angle_helper(
        const Vec2<T> offset,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_minor) return b1 and b2;
        else return b1 or b2;
    }
};

template<typename T, typename D>
struct is_placed_t<AnnularSector<T, D>>:std::true_type{;};


template<typename T, typename D>
struct RoundedAnnularSector final{
    static_assert(not std::is_integral_v<D>);

    Vec2<T> center;
    Range2<T> radius_range;
    AngleRange<D> angle_range;

    constexpr 
    Rect2<T> bounding_box() const {
        auto & self = *this;
        const bool x_reached_left = self.angle_range.
            contains_angle(Angle<D>::HALF_LAP);
        const bool x_reached_right = self.angle_range.
            contains_angle(Angle<D>::ZERO);
        const bool y_reached_top = self.angle_range.
            contains_angle(Angle<D>::QUARTER_LAP);
        const bool y_reached_bottom = self.angle_range.
            contains_angle(Angle<D>::NEG_QUARTER_LAP);

        const Vec2<q16> n1 = Vec2<q16>::from_angle(self.angle_range.start);
        const Vec2<q16> n2 = Vec2<q16>::from_angle(self.angle_range.stop());

        const Vec2<q16> p1 = (n1).flip_y() * self.radius_range.stop;
        const Vec2<q16> p2 = (n2).flip_y() * self.radius_range.stop;

        const Vec2<q16> p3 = (n1).flip_y()* self.radius_range.start;
        const Vec2<q16> p4 = (n2).flip_y() * self.radius_range.start;

        const T x_min = floor_cast<T>(self.center.x + (x_reached_left ?     
            static_cast<q16>(-self.radius_range.stop) : MIN(p1.x, p2.x, p3.x, p4.x)));
        const T y_min = floor_cast<T>(self.center.y + (y_reached_top ?      
            static_cast<q16>(-self.radius_range.stop) : MIN(p1.y, p2.y, p3.y, p4.y)));

        const T x_max = ceil_cast<T>(self.center.x + (x_reached_right ?    
            static_cast<q16>(self.radius_range.stop) : MAX(p1.x, p2.x, p3.x, p4.x)));
        const T y_max = ceil_cast<T>(self.center.y + (y_reached_bottom ?   
            static_cast<q16>(self.radius_range.stop) : MAX(p1.y, p2.y, p3.y, p4.y)));

        return Rect2<T>(
            Vec2<T>{
                x_min, y_min
            },

            Vec2<T>{
                static_cast<T>(x_max - x_min), 
                static_cast<T>(y_max - y_min)
            }
        );
    }

    __fast_inline constexpr bool contains_norm_dir(
        const Vec2<D> norm_dir_vec
    ) const {
        const auto n1 = Vec2<D>::from_angle(angle_range.start);
        const auto n2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            n1, n2, n2.is_counter_clockwise_to(n1)
        );
    }

    friend class BoundingBoxOf<RoundedAnnularSector<T, D>>;

private:
    static constexpr bool contains_angle_helper(
        const T angle,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        return contains_angle_helper(
            Vec2<D>::from_angle(angle), 
            start_norm_vec, 
            stop_norm_vec, 
            is_minor
        );
    }

    static constexpr bool contains_angle_helper(
        const Vec2<T> offset,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_minor) return b1 and b2;
        else return b1 or b2;
    }

    static constexpr Rect2<T> bounding_box_of_square(const Vec2<T> pos, const T radius){
            const auto ret = Rect2<T>::from_uninitialied();

            ret.left_top.x = pos.x - radius;
            ret.left_top.y = pos.y - radius;
            ret.size.x = radius * 2;
            ret.size.y = radius * 2;

            return ret;
        };
};

template<typename T, typename D>
struct is_placed_t<RoundedAnnularSector<T, D>>:std::true_type{;};



template<typename T, typename D>
struct Sector final{
    static_assert(not std::is_integral_v<D>);

    Vec2<T> center;
    T radius;
    AngleRange<D> angle_range;

    constexpr bool contains_angle(const Angle<D> angle) const {
        return angle_range.contains_angle(angle);
    }

    constexpr 
    Rect2<T> bounding_box() const {
        auto & self = *this;

        const bool x_reached_left = 
            self.angle_range.contains_angle(Angle<D>::HALF_LAP);

        const bool x_reached_right = 
            self.angle_range.contains_angle(Angle<D>::ZERO);

        const bool y_reached_top = 
            self.angle_range.contains_angle(Angle<D>::QUARTER_LAP);

        const bool y_reached_bottom = 
            self.angle_range.contains_angle(Angle<D>::NEG_QUARTER_LAP);

        const Vec2<q16> p1 = Vec2<q16>::from_angle(self.angle_range.start)
            .flip_y() * self.radius;
        const Vec2<q16> p2 = Vec2<q16>::from_angle(self.angle_range.stop())
            .flip_y() * self.radius;

        const T x_min = floor_cast<T>(self.center.x + (x_reached_left ?     
            static_cast<q16>(-self.radius) : MIN(p1.x, p2.x, 0)));
        const T y_min = floor_cast<T>(self.center.y + (y_reached_top ?      
            static_cast<q16>(-self.radius) : MIN(p1.y, p2.y, 0)));

        const T x_max = ceil_cast<T>(self.center.x + (x_reached_right ?    
            static_cast<q16>(self.radius) : MAX(p1.x, p2.x, 0)));
        const T y_max = ceil_cast<T>(self.center.y + (y_reached_bottom ?   
            static_cast<q16>(self.radius) : MAX(p1.y, p2.y, 0)));

        return Rect2<T>(
            Vec2<T>{
                x_min, y_min
            },

            Vec2<T>{
                static_cast<T>(x_max - x_min), 
                static_cast<T>(y_max - y_min)
            }
        );
    }

    __fast_inline constexpr bool contains_point(
        const Vec2<T>& p
    ) const{
        const auto offset = p - center;
        const auto p_r_squ = offset.length_squared();
        if(p_r_squ > square(radius)) return false;
        return contains_angle(offset.angle());
    }

    __fast_inline constexpr bool contains_norm_dir(
        const Vec2<D> norm_dir_vec
    ) const {
        const auto n1 = Vec2<D>::from_angle(angle_range.start);
        const auto n2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            n1, n2, n2.is_counter_clockwise_to(n1)
        );
    }

    friend class BoundingBoxOf<Sector<T, D>>;

private:
    static constexpr bool contains_angle_helper(
        const T angle,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        return contains_angle_helper(
            Vec2<D>::from_angle(angle), 
            start_norm_vec, 
            stop_norm_vec, 
            is_minor
        );
    }

    static constexpr bool contains_angle_helper(
        const Vec2<T> offset,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_minor) return b1 and b2;
        else return b1 or b2;
    }
};

template<typename T, typename D>
struct is_placed_t<Sector<T, D>>:std::true_type{;};




template<typename T, typename D>
struct CacheOf<AnnularSector<T, D>, bool>{
    using Self = CacheOf<AnnularSector<T, D>, bool>;

    T squ_inner_radius;
    T squ_outer_radius;
    Vec2<T> start_norm_vec;
    Vec2<T> stop_norm_vec;
    bool is_minor;

    static constexpr Self from(const AnnularSector<T, D> & obj){
        const auto n1 = Vec2<D>::from_angle(obj.angle_range.start);
        const auto n2 = Vec2<D>::from_angle(obj.angle_range.stop());
        return Self{
            .squ_inner_radius = square(obj.radius_range.start),
            .squ_outer_radius = square(obj.radius_range.stop),
            .start_norm_vec = n1,
            .stop_norm_vec = n2,
            .is_minor = n2.is_counter_clockwise_to(n1)
        };

    } 
    __fast_inline constexpr uint8_t color_from_point(const Vec2<T> offset) const {
        return s_color_from_point(*this, offset);
    }
private:
    static constexpr uint8_t s_color_from_point(const Self & self, const Vec2<T> offset){
        const auto len_squ = offset.length_squared();

        if (len_squ < self.squ_inner_radius || 
            len_squ > self.squ_outer_radius) {
            return false;
        }

        return (self.is_minor 
            ? (offset.is_counter_clockwise_to(self.start_norm_vec) && 
            offset.is_clockwise_to(self.stop_norm_vec))
            : (offset.is_counter_clockwise_to(self.start_norm_vec) || 
            offset.is_clockwise_to(self.stop_norm_vec))) ? 0xff : 0;
    } 
};


template<typename T, typename D>
struct BoundingBoxOf<AnnularSector<T, D>> {
    static constexpr Rect2<T> bounding_box(const AnnularSector<T, D> & obj){
        const auto n1 = Vec2<D>::from_angle(obj.angle_range.start);
        const auto n2 = Vec2<D>::from_angle(obj.angle_range.stop());
        const bool is_minor = n2.is_counter_clockwise_to(n1);
    
        Rect2<T> bb = Rect2<T>::from_minimal_bounding_box({
            n1 * obj.radius_range.start,
            n1 * obj.radius_range.stop,
            n2 * obj.radius_range.start,
            n2 * obj.radius_range.stop
        });

        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {1,0}, n1, n2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {0,1}, n1, n2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {-1,0}, n1, n2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {0,-1}, n1, n2, is_minor);

        return bb;
    }
private:
    static constexpr bool contains_angle(
        const Vec2<T> offset,
        const Vec2<T> start_norm_vec,
        const Vec2<T> stop_norm_vec,
        const bool is_minor
    ){
        return AnnularSector<T, D>::contains_angle_helper(
            offset,
            start_norm_vec,
            stop_norm_vec,
            is_minor
        );
    };

    struct BoundingBoxMergeHelper{

        static constexpr void merge_if_contains_angle(
            Rect2<T> & box,
            const Vec2<T> offset,
            const Vec2<T> start_norm_vec,
            const Vec2<T> stop_norm_vec,
            const bool is_minor
        ){
            if(contains_angle(offset, start_norm_vec, stop_norm_vec, is_minor)){
                box = box.merge(offset);
            }
        }
    };

};

}