#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"

namespace ymd{



template<typename T>
struct AnnularSector;

template<typename T>
struct BoundingBoxOf<AnnularSector<T>>;

template<typename T>
struct AnnularSector final{

    T inner_radius;
    T outer_radius;
    
    Range2<T> angle_range;

    constexpr bool contains_angle(const T angle) const {
        return angle_range.contains(angle);
    }

    constexpr Rect2<T> get_bounding_box() const {
        const bool x_reached_left = contains_angle(T(PI));
        const bool x_reached_right = contains_angle(T(0));
        const bool y_reached_top = contains_angle(T(PI/2));
        const bool y_reached_bottom = contains_angle(T(-PI/2));

        const auto p1 = Vec2<T>::from_idenity_rotation(angle_range.start) * outer_radius;
        const auto p2 = Vec2<T>::from_idenity_rotation(angle_range.stop) * outer_radius;

        const auto x_min = x_reached_left ? (-outer_radius) : MIN(p1.x, p2.x);
        const auto x_max = x_reached_right ? (outer_radius) : MAX(p1.x, p2.x);
        const auto y_min = y_reached_top ? (-outer_radius) : MIN(p1.y, p2.y);
        const auto y_max = y_reached_bottom ? (outer_radius) : MAX(p1.y, p2.y);

        return Rect2<T>(x_min, x_max, y_min, y_max);
    }

    __fast_inline constexpr bool has_point(
        const Vec2<T>& p
    ) const{
        const auto p_r_squ = p.length_squared();
        if(p_r_squ < square(inner_radius)) return false;
        if(p_r_squ > square(outer_radius)) return false;
        return has_angle(p.angle());
    }

    __fast_inline constexpr bool has_angle(
        const T angle)
    const {
        return has_angle(Vec2<T>::from_idenity_rotation(angle));
    }

    __fast_inline constexpr bool has_angle(
        const Vec2<T> offset)
    const {
        const auto v1 = Vec2<T>::from_idenity_rotation(angle_range.start);
        const auto v2 = Vec2<T>::from_idenity_rotation(angle_range.stop);
        return has_angle_helper(
            offset,
            v1, v2, v2.is_count_clockwise_to(v1)
        );
    }

    friend class BoundingBoxOf<AnnularSector<T>>;

private:
    static constexpr bool has_angle_helper(
        const T angle,
        const Vec2<T> start_norm_vec,
        const Vec2<T> stop_norm_vec,
        const bool is_close
    ){
        return has_angle_helper(
            Vec2<T>::from_idenity_rotation(angle), 
            start_norm_vec, stop_norm_vec, 
            is_close
        );
    }

    static constexpr bool has_angle_helper(
        const Vec2<T> offset,
        const Vec2<T> start_norm_vec,
        const Vec2<T> stop_norm_vec,
        const bool is_close
    ){
        const auto b1 = offset.is_count_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_close) return b1 and b2;
        else return b1 or b2;
    }
};


template<typename T>
struct CacheOf<AnnularSector<T>, bool>{
    using Self = CacheOf<AnnularSector<T>, bool>;

    T squ_inner_radius;
    T squ_outer_radius;
    Vec2<T> start_norm_vec;
    Vec2<T> stop_norm_vec;
    bool is_close;

    static constexpr Self from(const AnnularSector<T> & obj){
        const auto v1 = Vec2<T>::from_idenity_rotation(obj.angle_range.start);
        const auto v2 = Vec2<T>::from_idenity_rotation(obj.angle_range.stop);
        return Self{
            .squ_inner_radius = square(obj.inner_radius),
            .squ_outer_radius = square(obj.outer_radius),
            .start_norm_vec = v1,
            .stop_norm_vec = v2,
            .is_close = v2.is_count_clockwise_to(v1)
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

        return (self.is_close 
            ? (offset.is_count_clockwise_to(self.start_norm_vec) && 
            offset.is_clockwise_to(self.stop_norm_vec))
            : (offset.is_count_clockwise_to(self.start_norm_vec) || 
            offset.is_clockwise_to(self.stop_norm_vec))) ? 0xff : 0;
    } 
};


template<typename T>
struct BoundingBoxOf<AnnularSector<T>> {
    static constexpr Rect2<T> to_bounding_box(const AnnularSector<T> & obj){
        const auto v1 = Vec2<T>::from_idenity_rotation(obj.angle_range.start);
        const auto v2 = Vec2<T>::from_idenity_rotation(obj.angle_range.stop);
        const bool is_close = v2.is_count_clockwise_to(v1);
    
        Rect2<T> bb = Rect2<T>::from_minimal_bounding_box({
            v1 * obj.inner_radius,
            v1 * obj.outer_radius,
            v2 * obj.inner_radius,
            v2 * obj.outer_radius
        });

        BoundingBoxMergeHelper::merge_if_has_angle(bb, {1,0}, v1, v2, is_close);
        BoundingBoxMergeHelper::merge_if_has_angle(bb, {0,1}, v1, v2, is_close);
        BoundingBoxMergeHelper::merge_if_has_angle(bb, {-1,0}, v1, v2, is_close);
        BoundingBoxMergeHelper::merge_if_has_angle(bb, {0,-1}, v1, v2, is_close);

        return bb;
    }
private:
    static constexpr bool has_angle(
        const Vec2<T> offset,
        const Vec2<T> start_norm_vec,
        const Vec2<T> stop_norm_vec,
        const bool is_close
    ){
        return AnnularSector<T>::has_angle_helper(
            offset,
            start_norm_vec,
            stop_norm_vec,
            is_close
        );
    };

    struct BoundingBoxMergeHelper{

        static constexpr void merge_if_has_angle(
            Rect2<T> & box,
            const Vec2<T> offset,
            const Vec2<T> start_norm_vec,
            const Vec2<T> stop_norm_vec,
            const bool is_close
        ){
            if(has_angle(offset, start_norm_vec, stop_norm_vec, is_close)){
                box = box.merge(offset);
            }
        }
    };

};

}