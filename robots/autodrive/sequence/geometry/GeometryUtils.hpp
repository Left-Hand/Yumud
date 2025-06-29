#pragma once

#include "types/vectors/vector3/vector3.hpp"
#include "types/vectors/vector2/vector2.hpp"
#include "types/regions/rect2/rect2.hpp"
#include "types/regions/ray2/Ray2.hpp"
#include "types/vectors/quat/Quat.hpp"
#include "types/transforms/basis/Basis.hpp"
#include "types/regions/line2/Line2.hpp"

#include "core/utils/Option.hpp"

namespace ymd::geometry{

template<arithmetic T>
std::tuple<Option<Vector2<T>>, Option<Vector2<T>>> calculate_circle_center(
    const Vector2<T> & p1, 
    const Vector2<T> & p2, 
    const T radius
){
    const Vector2<T> mid_point = p1.center(p2);

    const Vector2<T> vec_p1_p2 = p2 - p1;

    const T distance_squ_div4 = vec_p1_p2.length_squared()/4;
    const T radius_squ = radius * radius;

    if (distance_squ_div4 == 0 || distance_squ_div4 > radius_squ) {
        return {std::nullopt, std::nullopt};
    }

    // 计算法线向量
    const Vector2<T> normal_vector = Vector2<T>(-vec_p1_p2.y, vec_p1_p2.x).normalized();

    // 计算圆心到中点的距离
    const T d = sqrt(radius * radius - (distance_squ_div4));

    // 计算两个可能的圆心
    const Vector2<T> center1 = mid_point + normal_vector * d;
    const Vector2<T> center2 = mid_point - normal_vector * d;

    // 返回其中一个圆心（这里返回第一个）
    return {Some(center1), Some(center2)};
}

template<arithmetic T>
Option<Vector2<T>> calculate_fillet_center(
    const ymd::Line2<T> & a, 
    const ymd::Line2<T> & b, 
    const T radius
){
    return None;
}


// scexpr T squ_len = 96;
// scexpr Vector2 pos_begin = {111, 46};
// scexpr Vector2 pos_end = pos_begin + Vector2{squ_len,squ_len};
// scexpr Vector2 pos_center = Vector2(pos_begin) + Vector2(squ_len / 2, squ_len / 2);
// scexpr Vector2 pos_pending = Vector2(pos_center) - Vector2(80, 0);


template<arithmetic T>

Vector2<T> get_square_rounded_position(const Rect2<T> & rect, const uint8_t index){
    index = index - 1;
    const auto x_i = 1 - (T(index % 3) / 2);
    const auto y_i = T(index / 3) / 2;

    const auto pos_begin = rect.position;
    const auto pos_end = pos_begin + rect.size;
    return Vector2<T>(pos_begin.x + (pos_end.x - pos_begin.x) * x_i, 
        pos_begin.y + (pos_end.y - pos_begin.y) * y_i);
}

template<arithmetic T>
Vector2<T> get_square_rounded_position(const Rect2<T> & rect, const T & rad){
    const auto squ_len = rect.size.x;
    const auto pos_center = rect.get_center();
    const auto rad_90 = fmod(rad + T(PI/4), T(PI/2)) - T(PI/4);
    const auto distance = (squ_len / 2) / cos(rad_90);
    return pos_center + Vector2<T>{-distance, 0}.rotated(rad);
}

}