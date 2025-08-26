#pragma once

#include "core/utils/angle_range.hpp"

#include "types/regions/range2.hpp"
#include "types/shapes/Circle2.hpp"

namespace ymd{

template<arithmetic T>
struct Arc2{
public:
	Vec2<T> center;
	T radius;
	AngleRange<T> angle_range;


    // 最佳推荐：清晰且专业
    static constexpr Arc2 from_chord_and_central_angle(
        const Vec2<T>& chord_start,
        const Vec2<T>& chord_stop,
        T central_angle
    ) {
        // 计算弦的中点
        const Vec2<T> chord_midpoint = (chord_start + chord_stop) * T(0.5);
        
        // 计算弦的方向向量和长度
        const Vec2<T> chord_vector = chord_stop - chord_start;
        const T chord_length = chord_vector.length();
        
        // 计算弦的垂直平分线方向
        const Vec2<T> perpendicular = Vec2<T>(-chord_vector.y, chord_vector.x).normalized();
        
        // 计算圆弧半径
        const T radius = chord_length / (T(2) * std::sin(central_angle * T(0.5)));
        
        // 计算圆心到弦中点的距离
        const T distance_to_chord = std::sqrt(radius * radius - chord_length * chord_length * T(0.25));
        
        const Vec2<T> center1 = chord_midpoint + perpendicular * distance_to_chord;
        
        // 计算从圆心到起点和终点的向量
        const Vec2<T> to_start1 = chord_start - center1;
        const Vec2<T> to_end1 = chord_stop - center1;

        // 计算角度范围
        T start_angle1 = std::atan2(to_start1.y, to_start1.x);
        T end_angle1 = std::atan2(to_end1.y, to_end1.x);
        T angle_diff1 = normalize_angle(end_angle1 - start_angle1);
        
        return Arc2(center1, radius, {start_angle1, end_angle1});
    }
    
	__fast_inline constexpr bool intersects(const Arc2<T> & other) const {
		//FIXME

		return (center - other.center).length_squared() <= square(radius + other.radius);
	}

	__fast_inline constexpr bool tangent(const Arc2<T> & other) const {

		//FIXME
		return (center - other.center).length_squared() == square(radius + other.radius);		
	}
};


__inline OutputStream & operator <<(OutputStream & os, const Arc2<auto> & arc){
	const auto splt = os.splitter();
    return os << '(' << arc.center << splt << arc.radius << splt << arc.angle_range << ')';
}


template<typename T>
struct is_placed_t<Arc2<T>>:std::true_type{};

}