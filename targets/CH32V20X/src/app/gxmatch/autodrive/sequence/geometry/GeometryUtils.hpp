#pragma once

#include "types/vector3/Vector3_t.hpp"
#include "types/vector2/Vector2_t.hpp"
#include "types/rect2/rect2_t.hpp"
#include "types/ray2d/Ray2D_t.hpp"
#include "types/quat/Quat_t.hpp"
#include "types/basis/Basis_t.hpp"
#include "types/line2d/Line2D_t.hpp"

namespace geometry {

template<arithmetic T>
std::tuple<std::optional<ymd::Vector2_t<T>>, std::optional<ymd::Vector2_t<T>>> calculate_circle_center(const ymd::Vector2_t<T> & p1, const ymd::Vector2_t<T> & p2, const T radius){
    const ymd::Vector2_t<T> mid_point = p1.center(p2);

    const ymd::Vector2_t<T> vec_p1_p2 = p2 - p1;

    const T distance_squ_div4 = vec_p1_p2.length_squared()/4;
    const T radius_squ = radius * radius;

    if (distance_squ_div4 == 0 || distance_squ_div4 > radius_squ) {
        return {std::nullopt, std::nullopt};
    }

    // 计算法线向量
    const ymd::Vector2_t<T> normal_vector = ymd::Vector2_t<T>(-vec_p1_p2.y, vec_p1_p2.x).normalized();

    // 计算圆心到中点的距离
    const T d = sqrt(radius * radius - (distance_squ_div4));

    // 计算两个可能的圆心
    const ymd::Vector2_t<T> center1 = mid_point + normal_vector * d;
    const ymd::Vector2_t<T> center2 = mid_point - normal_vector * d;

    // 返回其中一个圆心（这里返回第一个）
    return {center1, center2};
}

template<arithmetic T>
std::optional<ymd::Vector2_t<T>> calculate_fillet_center(const ymd::Line2D_t<T> & a, const ymd::Line2D_t<T> & b, const T & radius){
    return std::nullopt;
}


// scexpr T squ_len = 96;
// scexpr Vector2 pos_begin = {111, 46};
// scexpr Vector2 pos_end = pos_begin + Vector2{squ_len,squ_len};
// scexpr Vector2 pos_center = Vector2(pos_begin) + Vector2(squ_len / 2, squ_len / 2);
// scexpr Vector2 pos_pending = Vector2(pos_center) - Vector2(80, 0);


template<arithmetic T>

ymd::Vector2_t<T> get_square_rounded_position(const ymd::Rect2_t<T> & rect, const uint8_t index){
    index = index - 1;
    const auto x_i = 1 - (T(index % 3) / 2);
    const auto y_i = T(index / 3) / 2;

    const auto pos_begin = rect.position;
    const auto pos_end = pos_begin + rect.size;
    return ymd::Vector2_t<T>(pos_begin.x + (pos_end.x - pos_begin.x) * x_i, pos_begin.y + (pos_end.y - pos_begin.y) * y_i);
}

template<arithmetic T>
ymd::Vector2_t<T> get_square_rounded_position(const ymd::Rect2_t<T> & rect, const T & rad){
    const auto squ_len = rect.size.x;
    const auto pos_center = rect.get_center();
    const auto rad_90 = fmod(rad + pi_4, pi_2) - pi_4;
    const auto distance = (squ_len / 2) / cos(rad_90);
    return pos_center + ymd::Vector2_t<T>{-distance, 0}.rotated(rad);
}

}