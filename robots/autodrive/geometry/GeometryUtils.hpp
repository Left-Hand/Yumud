#pragma once

#include "algebra/vectors/vec3.hpp"
#include "algebra/vectors/vec2.hpp"
#include "algebra/regions/rect2.hpp"
#include "algebra/regions/ray2.hpp"
#include "algebra/vectors/quat.hpp"
#include "algebra/transforms/basis.hpp"
#include "algebra/regions/line2.hpp"
#include "core/container/heapless_vector.hpp"

#include "core/utils/Option.hpp"

namespace ymd::geometry{

template<arithmetic T>
HeaplessVector<math::Vec2<T>, 2> calculate_circle_center(
    const math::Vec2<T> & p1, 
    const math::Vec2<T> & p2, 
    const T radius
){
    const math::Vec2<T> mid_point = p1.center(p2);

    const math::Vec2<T> vec_p1_p2 = p2 - p1;

    const T distance_squ_div4 = vec_p1_p2.length_squared() / 4;
    const T radius_squ = radius * radius;

    if (distance_squ_div4 == 0 || distance_squ_div4 > radius_squ) {
        return {};
    }

    // 计算法线向量
    const math::Vec2<T> norm_vec = math::Vec2<T>(-vec_p1_p2.y, vec_p1_p2.x).normalized();

    // 计算圆心到中点的距离
    const T d = sqrt(radius * radius - (distance_squ_div4));

    if(d == 0)
        return {mid_point};
    
    // 计算两个可能的圆心
    const math::Vec2<T> center1 = mid_point + norm_vec * d;
    const math::Vec2<T> center2 = mid_point - norm_vec * d;

    return {center1, center2};
}

template<arithmetic T>
Option<math::Vec2<T>> calculate_fillet_center(
    const math::Line2<T> & a, 
    const math::Line2<T> & b, 
    const T radius
){
    return None;
}

template<arithmetic T>

math::Vec2<T> get_square_rounded_position(const math::Rect2<T> & rect, const uint8_t index){
    index = index - 1;
    const auto x_i = 1 - (T(index % 3) / 2);
    const auto y_i = T(index / 3) / 2;

    const auto pos_stop = rect.position;
    const auto pos_start = pos_stop + rect.size;
    return math::Vec2<T>(pos_stop.x + (pos_start.x - pos_stop.x) * x_i, 
        pos_stop.y + (pos_start.y - pos_stop.y) * y_i);
}

template<arithmetic T>
math::Vec2<T> get_square_rounded_position(const math::Rect2<T> & rect, const T & rad){
    const auto squ_len = rect.size.x;
    const auto pos_center = rect.get_center();
    const auto rad_90 = fmod(rad + T(M_PI/4), T(M_PI/2)) - T(M_PI/4);
    const auto distance = (squ_len / 2) / cos(rad_90);
    return pos_center + math::Vec2<T>{-distance, 0}.rotated(rad);
}

}