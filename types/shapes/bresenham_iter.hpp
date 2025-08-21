#pragma once

#include "core/platform.hpp"
#include "types/vectors/vector2.hpp"

namespace ymd {

template<typename T>
struct MajorMinor {
    T major;
    T minor;
    
    constexpr MajorMinor() = default;
    constexpr MajorMinor(const T& _major, const T& _minor) : major(_major), minor(_minor) {}
    
    constexpr MajorMinor(const MajorMinor&) = default;
    constexpr MajorMinor& operator=(const MajorMinor&) = default;
};

struct BresenhamCache {
    int32_t error_threshold;
    MajorMinor<int32_t> error_step;
    MajorMinor<Vec2<int32_t>> position_step;
    
    
    template<typename T>
    constexpr BresenhamCache(const Vec2<T>& start, const Vec2<T>& end) {
        Vec2<int32_t> delta = Vec2<int32_t>(end.x - start.x, end.y - start.y);
        
        Vec2<int32_t> direction(
            delta.x >= 0 ? 1 : -1,
            delta.y >= 0 ? 1 : -1
        );
        
        delta = Vec2<int32_t>(std::abs(delta.x), std::abs(delta.y));
        
        // Determine major and minor directions
        if (delta.y >= delta.x) {
            error_threshold = delta.y;
            error_step = MajorMinor<int32_t>(2 * delta.x, 2 * delta.y);
            position_step = MajorMinor<Vec2<int32_t>>(
                Vec2<int32_t>(0, direction.y),
                Vec2<int32_t>(direction.x, 0)
            );
        } else {
            error_threshold = delta.x;
            error_step = MajorMinor<int32_t>(2 * delta.y, 2 * delta.x);
            position_step = MajorMinor<Vec2<int32_t>>(
                Vec2<int32_t>(direction.x, 0),
                Vec2<int32_t>(0, direction.y)
            );
        }
    }
private:
    constexpr bool mirror_extra_points() const {
        if (position_step.major.x != 0) {
            return position_step.major.x == position_step.minor.y;
        } else {
            return position_step.major.y == -position_step.minor.x;
        }
    }

    constexpr bool increase_error(int32_t & error) const {
        error += error_step.major;
        if (error > error_threshold) {
            error -= error_step.minor;
            return true;
        }
        return false;
    }
    
    constexpr bool decrease_error(int32_t & error) const {
        error -= error_step.major;
        if (error <= -error_threshold) {
            error += error_step.minor;
            return true;
        }
        return false;
    }
    
};

enum class BresenhamPointType:uint8_t {
    Normal,
    Extra
};

template<typename T>
struct BresenhamPoint {
    BresenhamPointType type;
    Vec2<T> point;
    
    
    static constexpr BresenhamPoint from_normal(const Vec2<T>& point) {
        return BresenhamPoint(BresenhamPointType::Normal, point);
    }
    
    static constexpr BresenhamPoint from_extra(const Vec2<T>& point) {
        return BresenhamPoint(BresenhamPointType::Extra, point);
    }
};

template<typename T>
struct BresenhamIter {
public:
    Vec2<T> point;
    int32_t error;
    
    explicit constexpr BresenhamIter(const Vec2<T>& start_point) : point(start_point), error(0) {}
    
    constexpr BresenhamIter(const Vec2<T>& start_point, int32_t initial_error) 
        : point(start_point), error(initial_error) {}
    
    constexpr Vec2<T> next(const BresenhamCache & parameters) {
        if (error > parameters.error_threshold) {
            point += parameters.position_step.minor;
            error -= parameters.error_step.minor;
        }
        
        Vec2<T> ret = point;
        
        point += parameters.position_step.major;
        error += parameters.error_step.major;
        
        return ret;
    }
    
    constexpr BresenhamPoint<T> next_all(const BresenhamCache & parameters) {
        Vec2<T> current_point = point;
        
        if (error > parameters.error_threshold) {
            point += parameters.position_step.minor;
            error -= parameters.error_step.minor;
            
            // Note: mirror_extra_points is private in the original, 
            // so this logic would need to be adapted
            return BresenhamPoint<T>::from_extra(current_point);
        } else {
            point += parameters.position_step.major;
            error += parameters.error_step.major;
            
            return BresenhamPoint<T>::from_normal(current_point);
        }
    }
    
    constexpr BresenhamPoint<T> previous_all(const BresenhamCache& parameters) {
        Vec2<T> current_point = point;
        
        if (error <= -parameters.error_threshold) {
            point -= parameters.position_step.minor;
            error += parameters.error_step.minor;
            
            return BresenhamPoint<T>::from_extra(current_point);
        } else {
            point -= parameters.position_step.major;
            error -= parameters.error_step.major;
            
            return BresenhamPoint<T>::from_normal(current_point);
        }
    }
};

} // namespace ymd