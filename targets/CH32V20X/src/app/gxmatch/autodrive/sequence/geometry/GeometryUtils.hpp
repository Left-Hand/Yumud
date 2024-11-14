#pragma once

#include "types/vector3/Vector3_t.hpp"
#include "types/vector2/Vector2_t.hpp"
#include "types/ray2d/Ray2D_t.hpp"

namespace geometry {

template<typename T>
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

}