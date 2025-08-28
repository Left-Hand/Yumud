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

        const Vec2<q16> v1 = Vec2<q16>::from_angle(self.angle_range.start);
        const Vec2<q16> v2 = Vec2<q16>::from_angle(self.angle_range.stop());

        const Vec2<q16> p1 = (v1).flip_y()* self.radius_range.stop;
        const Vec2<q16> p2 = (v2).flip_y() * self.radius_range.stop;

        const Vec2<q16> p3 = (v1).flip_y()* self.radius_range.start;
        const Vec2<q16> p4 = (v2).flip_y() * self.radius_range.start;

        const q16 x_min = x_reached_left ?     
            static_cast<q16>(-self.radius_range.stop) : MIN(p1.x, p2.x, p3.x, p4.x);
        const q16 x_max = x_reached_right ?    
            static_cast<q16>(self.radius_range.stop) : MAX(p1.x, p2.x, p3.x, p4.x);
        const q16 y_min = y_reached_top ?      
            static_cast<q16>(-self.radius_range.stop) : MIN(p1.y, p2.y, p3.y, p4.y);
        const q16 y_max = y_reached_bottom ?   
            static_cast<q16>(self.radius_range.stop) : MAX(p1.y, p2.y, p3.y, p4.y);

        return Rect2<T>(
            static_cast<T>(floor_int(x_min + self.center.x)), 
            static_cast<T>(floor_int(y_min + self.center.y)), 
            static_cast<T>(ceil_int(x_max - x_min)), 
            static_cast<T>(ceil_int(y_max - y_min))
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
        const auto v1 = Vec2<D>::from_angle(angle_range.start);
        const auto v2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            v1, v2, v2.is_counter_clockwise_to(v1)
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

        const q16 x_min = x_reached_left ?     static_cast<q16>(-self.radius) 
            : MIN(p1.x, p2.x);
        const q16 x_max = x_reached_right ?    static_cast<q16>(self.radius) 
            : MAX(p1.x, p2.x);
        const q16 y_min = y_reached_top ?      static_cast<q16>(-self.radius) 
            : MIN(p1.y, p2.y);
        const q16 y_max = y_reached_bottom ?   static_cast<q16>(self.radius) 
            : MAX(p1.y, p2.y);

        return Rect2<T>(
            static_cast<T>(floor_int(x_min + self.center.x)), 
            static_cast<T>(floor_int(y_min + self.center.y)), 
            static_cast<T>(ceil_int(x_max - x_min)), 
            static_cast<T>(ceil_int(y_max - y_min))
        ).merge(center);
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
        const auto v1 = Vec2<D>::from_angle(angle_range.start);
        const auto v2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            v1, v2, v2.is_counter_clockwise_to(v1)
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
        const auto v1 = Vec2<D>::from_angle(obj.angle_range.start);
        const auto v2 = Vec2<D>::from_angle(obj.angle_range.stop());
        return Self{
            .squ_inner_radius = square(obj.radius_range.start),
            .squ_outer_radius = square(obj.radius_range.stop),
            .start_norm_vec = v1,
            .stop_norm_vec = v2,
            .is_minor = v2.is_counter_clockwise_to(v1)
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
        const auto v1 = Vec2<D>::from_angle(obj.angle_range.start);
        const auto v2 = Vec2<D>::from_angle(obj.angle_range.stop());
        const bool is_minor = v2.is_counter_clockwise_to(v1);
    
        Rect2<T> bb = Rect2<T>::from_minimal_bounding_box({
            v1 * obj.radius_range.start,
            v1 * obj.radius_range.stop,
            v2 * obj.radius_range.start,
            v2 * obj.radius_range.stop
        });

        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {1,0}, v1, v2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {0,1}, v1, v2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {-1,0}, v1, v2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {0,-1}, v1, v2, is_minor);

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