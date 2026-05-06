#pragma once

#include "vec3.hpp"
#include "polar.hpp"

namespace ymd::math{

template<typename T>
struct [[nodiscard]] SphericalCoordinates{
    using Self = SphericalCoordinates;
    T distance;
    Angular<T> azimuth;
    Angular<T> elevation;

    static constexpr Self zero(){
        return Self{
            .distance = static_cast<T>(0),
            .azimuth = Angular<T>::ZERO,
            .elevation = Angular<T>::ZERO
        };
    }

    [[nodiscard]] math::Vec3<T> constexpr to_vec3() const noexcept {
        const auto [azimuth_sine, azimuth_cosine] = azimuth.sincos();
        const auto [elevation_sine, elevation_cosine] = elevation.sincos();
        return math::Vec3<T>{
            distance * static_cast<T>(azimuth_cosine) * static_cast<T>(elevation_cosine),
            distance * static_cast<T>(azimuth_sine) * static_cast<T>(elevation_cosine),
            distance * static_cast<T>(elevation_sine)
        };
    }

    // 可以添加反向转换
    static constexpr Self from_vec3(const math::Vec3<T> & vec) {
        const T length_squared = vec.length_squared();
        if (length_squared == static_cast<T>(0)) [[unlikely]]
            return zero();
        const T length = std::sqrt(length_squared);
        const T azimuth_rad = std::atan2(vec.y, vec.x);
        const T elevation_rad = std::asin(vec.z / length);
        
        return Self{
            .distance = length,
            .azimuth = Angular<T>::from_radians(azimuth_rad),
            .elevation = Angular<T>::from_radians(elevation_rad)
        };
    }

    [[nodiscard]] math::Vec3<T> constexpr to_vec3_downplaced() const noexcept {
        const auto [x,y,z] = to_vec3();
        return math::Vec3{x, -y, z};
    }

    // [[nodiscard]] Polar<T> constexpr to_polar() const noexcept {
    //     const auto elevation_cosine = elevation.cos();
    //     return Polar<T>(distance * elevation_cosine, azimuth);
    // }

    [[nodiscard]] std::tuple<Polar<T>, T> constexpr to_polar_and_height() const noexcept {
        const auto [elevation_sine, elevation_cosine] = elevation.sincos();
        return std::make_tuple(
            Polar<T>(distance * elevation_cosine, azimuth), 
            distance * elevation_sine
        );
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self) noexcept {
        return os    
            << os.field("distance")(self.distance) << os.splitter()
            << os.field("azimuth")(self.azimuth.to_radians()) << os.splitter()
            << os.field("elevation")(self.elevation.to_radians())
        ;
    }
};

}