#pragma once

#include "types/vectors/vector3.hpp"
#include "types/vectors/vector2.hpp"
#include "types/regions/rect2.hpp"
#include "types/regions/ray2.hpp"
#include "types/vectors/quat.hpp"
#include "types/transforms/basis.hpp"
#include "types/regions/line2.hpp"
#include "core/container/inline_vector.hpp"

#include "core/utils/Option.hpp"

namespace ymd::geometry{

template<arithmetic T>
HeaplessVector<Vec2<T>, 2> calculate_circle_center(
    const Vec2<T> & p1, 
    const Vec2<T> & p2, 
    const T radius
){
    const Vec2<T> mid_point = p1.center(p2);

    const Vec2<T> vec_p1_p2 = p2 - p1;

    const T distance_squ_div4 = vec_p1_p2.length_squared() / 4;
    const T radius_squ = radius * radius;

    if (distance_squ_div4 == 0 || distance_squ_div4 > radius_squ) {
        return {};
    }

    // 计算法线向量
    const Vec2<T> norm_vec = Vec2<T>(-vec_p1_p2.y, vec_p1_p2.x).normalized();

    // 计算圆心到中点的距离
    const T d = sqrt(radius * radius - (distance_squ_div4));

    if(d == 0)
        return {mid_point};
    
    // 计算两个可能的圆心
    const Vec2<T> center1 = mid_point + norm_vec * d;
    const Vec2<T> center2 = mid_point - norm_vec * d;

    return {center1, center2};
}

template<arithmetic T>
Option<Vec2<T>> calculate_fillet_center(
    const ymd::Line2<T> & a, 
    const ymd::Line2<T> & b, 
    const T radius
){
    return None;
}

template<arithmetic T>

Vec2<T> get_square_rounded_position(const Rect2<T> & rect, const uint8_t index){
    index = index - 1;
    const auto x_i = 1 - (T(index % 3) / 2);
    const auto y_i = T(index / 3) / 2;

    const auto pos_begin = rect.position;
    const auto pos_end = pos_begin + rect.size;
    return Vec2<T>(pos_begin.x + (pos_end.x - pos_begin.x) * x_i, 
        pos_begin.y + (pos_end.y - pos_begin.y) * y_i);
}

template<arithmetic T>
Vec2<T> get_square_rounded_position(const Rect2<T> & rect, const T & rad){
    const auto squ_len = rect.size.x;
    const auto pos_center = rect.get_center();
    const auto rad_90 = fmod(rad + T(PI/4), T(PI/2)) - T(PI/4);
    const auto distance = (squ_len / 2) / cos(rad_90);
    return pos_center + Vec2<T>{-distance, 0}.rotated(rad);
}

}