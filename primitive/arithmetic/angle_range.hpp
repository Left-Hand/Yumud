#pragma once

#include "angle.hpp"

namespace ymd{
// https://zhuanlan.zhihu.com/p/576032998

// 锐角（acute angle）：大于0°且小于90°的角叫做锐角。

// 直角（right angle）：等于90°的角叫做直角。

// 钝角（obtuse angle）：大于90°且小于180°的角叫做钝角。

// 优角（major angle）：大于180°且小于360°叫优角。

// 劣角（minor angle）：大于0°且小于180°叫做劣角。

// 平角（straight angle）：等于180°的角叫做平角。

// 零角（zero angle）：等于0°的角。

// 周角（round angle）：等于360°的角叫做周角。

// 负角（negative angle）：按照顺时针方向旋转而成的角叫做负角。

// 正角（positive angle）：逆时针旋转的角为正角。

template<typename T>
struct AngleRange{
    Angle<T> start;
    Angle<T> interval;

    static constexpr AngleRange from_uninitialized(){
        return AngleRange();
    }

    static constexpr AngleRange from_start_and_stop(
        Angle<T> _start, Angle<T> _stop
    ){
        return AngleRange{_start, _stop - _start};
    }


    static constexpr AngleRange from_start_and_interval(
        Angle<T> _start, Angle<T> _interval
    ){
        return AngleRange{_start, _interval};
    }

    constexpr std::tuple<Angle<T>, Angle<T>> start_and_stop() const {
        return {start, start + interval};
    }

    constexpr Angle<T> stop() const {
        return start + interval;
    }

    constexpr bool is_clockwise() const {
        return interval.is_positive();
    }

    constexpr bool is_counter_clockwise() const {
        return interval.is_negative();
    }

    constexpr AngleRange<T> swapped() const {
        return {start - interval, -interval};
    }

    constexpr AngleRange<T> clockwised() const {
        if(is_clockwise()) return *this;
        return swapped();
    }

    constexpr AngleRange<T> counter_clockwised() const {
        if(is_counter_clockwise()) return *this;
        return swapped();
    }


    constexpr AngleRange<T> normalize_counter_clockwised() const {
        if(is_counter_clockwise()) {
            // Already counter-clockwise, just normalize the start angle
            Angle<T> normalized_start = start.normalized();
            return {normalized_start, interval};
        }
        
        // Convert clockwise to counter-clockwise
        // For counter-clockwise representation of a clockwise range,
        // we need to adjust the start angle
        Angle<T> new_start = (start + interval).normalized();
        Angle<T> new_interval = -interval;
        
        // Ensure the new start is within [0, 1) range
        Angle<T> normalized_start = new_start.normalized();
        
        return {normalized_start, new_interval};
    }

    constexpr bool is_major() const {
        return interval.abs() > Angle<T>::HALF_LAP;
    }

    constexpr bool is_minor() const {
        return interval.abs() < Angle<T>::HALF_LAP;
    }

    constexpr bool is_straight() const {
        return interval.abs() == Angle<T>::HALF_LAP;
    }

    constexpr bool is_full() const {
        return interval.abs() >= Angle<T>::LAP;
    }

    constexpr Angle<T> sweep() const {
        return interval.abs();
    }

    constexpr Angle<T> bisector() const {
        return start + interval / T(2);
    }

    // 改进的包含性检查 - 考虑逆时针为正的约定
    constexpr bool contains_angle(Angle<T> angle) const {
        // Normalize all angles to [0, 1) range for comparison
        Angle<T> norm_start = start.normalized();
        Angle<T> norm_stop = stop().normalized();
        Angle<T> norm_angle = angle.normalized();
        
        if (interval.is_positive()) {
            // Counter-clockwise direction (positive interval)
            if (norm_start < norm_stop) {
                // Normal case: start < stop (e.g., 30° -> 120°)
                return norm_angle >= norm_start && norm_angle < norm_stop;
            } else {
                // Wrap-around case: start > stop (e.g., 350° -> 30°)
                return norm_angle >= norm_start || norm_angle < norm_stop;
            }
        } else {
            // Clockwise direction (negative interval)
            if (norm_start > norm_stop) {
                // Normal case: start > stop (e.g., 120° -> 30° clockwise)
                return norm_angle <= norm_start && norm_angle > norm_stop;
            } else {
                // Wrap-around case: start < stop (e.g., 30° -> 350° clockwise)
                return norm_angle <= norm_start || norm_angle > norm_stop;
            }
        }
    }

    // 检查是否包含另一个范围
    constexpr bool contains_range(const AngleRange& other) const {
        auto norm_this = normalize_counter_clockwised();
        auto norm_other = other.normalize_counter_clockwised();
        
        if (norm_this.start <= norm_this.stop()) {
            if (norm_other.start <= norm_other.stop()) {
                // 两个范围都不跨360°
                return norm_other.start >= norm_this.start && 
                        norm_other.stop() <= norm_this.stop();
            } else {
                // 其他范围跨360°，当前范围不跨
                return false;
            }
        } else {
            if (norm_other.start <= norm_other.stop()) {
                // 当前范围跨360°，其他范围不跨
                return (norm_other.start >= norm_this.start || 
                        norm_other.stop() <= norm_this.stop());
            } else {
                // 两个范围都跨360°
                return norm_other.start >= norm_this.start && 
                        norm_other.stop() <= norm_this.stop();
            }
        }
    }

    // 检查两个范围是否重叠
    constexpr bool overlaps_with(const AngleRange& other) const {
        auto norm_this = normalize_counter_clockwised();
        auto norm_other = other.normalize_counter_clockwised();
        
        // 简化重叠检测
        return norm_this.contains_angle(norm_other.start) ||
            norm_this.contains_angle(norm_other.stop()) ||
            norm_other.contains_angle(norm_this.start) ||
            norm_other.contains_angle(norm_this.stop());
    }

    // 获取重叠部分（返回逆时针方向的范围）
    constexpr Option<AngleRange> intersection_with(const AngleRange& other) const {
        if (!overlaps(other)) {
            return None;
        }
        
        auto norm_this = normalize_counter_clockwised();
        auto norm_other = other.normalize_counter_clockwised();
        
        Angle<T> overlap_start = std::max(norm_this.start, norm_other.start);
        Angle<T> overlap_stop = std::min(norm_this.stop(), norm_other.stop());
        
        if (overlap_start > overlap_stop) {
            // 处理跨360°的重叠
            overlap_stop = overlap_stop + Angle<T>::FULL_LAP;
        }
        
        return Some(from_start_and_stop(overlap_start, overlap_stop));
    }

    // 将角度限制在此范围内（保持原始方向）
    constexpr Angle<T> clamp_angle(Angle<T> angle) const {
        if (contains_angle(angle)) return angle;
        
        auto norm_range = normalize_counter_clockwised();
        Angle<T> norm_angle = angle.normalized();
        
        // 计算到两个边界的距离（考虑圆的周期性）
        Angle<T> dist_to_start = (norm_angle - norm_range.start).normalized();
        Angle<T> dist_to_stop = (norm_range.stop() - norm_angle).normalized();
        
        // 返回较近的边界
        return (dist_to_start < dist_to_stop) ? norm_range.start : norm_range.stop();
    }

    // 运算符重载
    constexpr bool operator==(const AngleRange& other) const {
        return start == other.start && interval == other.interval;
    }

    constexpr bool operator!=(const AngleRange& other) const {
        return !(*this == other);
    }

    // 流输出支持
    friend OutputStream & operator<<(OutputStream & os, const AngleRange & self) {
        return os << os.brackets<'('>() << self.start << "->" 
            << self.stop() << os.brackets<')'>(); 
    }

private:
    constexpr AngleRange() = default;

    constexpr AngleRange(const Angle<T> _start, const Angle<T> _interval):
        start(_start), interval(_interval){;}
};

}