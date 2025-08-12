#pragma once

#include "types/regions/rect2/Rect2.hpp"

namespace ymd{

template<typename T>
struct AnnularSector final{

    T inner_radius;
    T outer_radius;
    
    T start_rad;
    T stop_rad;

    constexpr bool does_contains_rad(const T orientation) const {
        return IN_RANGE(orientation, start_rad, stop_rad);
    }

    constexpr Rect2<T> get_bounding_box() const {
        const bool x_reached_left = does_contains_rad(T(PI));
        const bool x_reached_right = does_contains_rad(T(0));
        const bool y_reached_top = does_contains_rad(T(PI/2));
        const bool y_reached_bottom = does_contains_rad(T(-PI/2));

        const auto p1 = Vector2<T>::from_idenity_rotation(start_rad) * outer_radius;
        const auto p2 = Vector2<T>::from_idenity_rotation(stop_rad) * outer_radius;

        const auto x_min = x_reached_left ? (-outer_radius) : MIN(p1.x, p2.x);
        const auto x_max = x_reached_right ? (outer_radius) : MAX(p1.x, p2.x);
        const auto y_min = y_reached_top ? (-outer_radius) : MIN(p1.y, p2.y);
        const auto y_max = y_reached_bottom ? (outer_radius) : MAX(p1.y, p2.y);

        return Rect2<T>(x_min, x_max, y_min, y_max);
    }

    struct alignas(4) Cache{
        T squ_inner_radius;
        T squ_outer_radius;
        Vector2<T> start_norm_vec;
        Vector2<T> stop_norm_vec;
        bool is_close;


        __fast_inline constexpr uint8_t color_from_point(const Vector2<T> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        static constexpr uint8_t s_color_from_point(const Cache & self, const Vector2<T> offset){
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

    constexpr auto to_cache() const {
        const auto v1 = Vector2<T>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2<T>::from_idenity_rotation(stop_rad);
        return Cache{
            .squ_inner_radius = square(inner_radius),
            .squ_outer_radius = square(outer_radius),
            .start_norm_vec = v1,
            .stop_norm_vec = v2,
            .is_close = v2.is_count_clockwise_to(v1)
        };
    }

    constexpr auto to_bounding_box() const {
        const auto v1 = Vector2<T>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2<T>::from_idenity_rotation(stop_rad);
        const bool is_close = v2.is_count_clockwise_to(v1);
        Rect2<T> bb = Rect2<T>::from_minimal_bounding_box({
            v1 * inner_radius,
            v1 * outer_radius,
            v2 * inner_radius,
            v2 * outer_radius
        });

        BoundingBoxMergeHelper::merge_if_has_radian(bb, {1,0}, v1, v2, is_close);
        BoundingBoxMergeHelper::merge_if_has_radian(bb, {0,1}, v1, v2, is_close);
        BoundingBoxMergeHelper::merge_if_has_radian(bb, {-1,0}, v1, v2, is_close);
        BoundingBoxMergeHelper::merge_if_has_radian(bb, {0,-1}, v1, v2, is_close);

        return bb;
    }

    __fast_inline constexpr bool has_radian(
        const T radian)
    const {
        return has_radian(Vector2<T>::from_idenity_rotation(radian));
    }

    __fast_inline constexpr bool has_radian(
        const Vector2<T> offset)
    const {
        const auto v1 = Vector2<T>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2<T>::from_idenity_rotation(stop_rad);
        return BoundingBoxMergeHelper::has_radian(
            offset,
            v1, v2, v2.is_count_clockwise_to(v1)
        );
    }
private:

    struct BoundingBoxMergeHelper{
        static constexpr bool has_radian(
            const T radian,
            const Vector2<T> start_norm_vec,
            const Vector2<T> stop_norm_vec,
            const bool is_close
        ){
            return has_radian(
                Vector2<T>::from_idenity_rotation(radian), 
                start_norm_vec, stop_norm_vec, 
                is_close
            );
        }

        static constexpr bool has_radian(
            const Vector2<T> offset,
            const Vector2<T> start_norm_vec,
            const Vector2<T> stop_norm_vec,
            const bool is_close
        ){
            const auto b1 = offset.is_count_clockwise_to(start_norm_vec);
            const auto b2 = offset.is_clockwise_to(stop_norm_vec);
            if(is_close) return b1 and b2;
            else return b1 or b2;
        }

        static constexpr void merge_if_has_radian(
            Rect2<T> & box,
            const Vector2<T> offset,
            const Vector2<T> start_norm_vec,
            const Vector2<T> stop_norm_vec,
            const bool is_close
        ){
            if(has_radian(offset, start_norm_vec, stop_norm_vec, is_close)){
                box = box.merge(offset);
            }
        }
    };

};

}