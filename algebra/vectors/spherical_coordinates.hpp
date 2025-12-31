#pragma once

#include "vec3.hpp"
#include "polar.hpp"

namespace ymd{

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

    [[nodiscard]] Vec3<T> constexpr to_vec3() const {
        const auto [azimuth_s, azimuth_c] = azimuth.sincos();
        const auto [elevation_s, elevation_c] = elevation.sincos();
        return Vec3<T>{
            distance * azimuth_c * elevation_c,
            distance * azimuth_s * elevation_c,
            distance * elevation_s
        };
    }

    // 可以添加反向转换
    static constexpr Self from_vec3(const Vec3<T> & vec) {
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

    [[nodiscard]] Vec3<T> constexpr to_vec3_downplaced() const {
        const auto [x,y,z] = to_vec3();
        return Vec3{x, -y, z};
    }

    // [[nodiscard]] Polar<T> constexpr to_polar() const{
    //     const auto elevation_c = elevation.cos();
    //     return Polar<T>(distance * elevation_c, azimuth);
    // }

    [[nodiscard]] std::tuple<Polar<T>, T> constexpr to_polar_and_height() const{
        const auto [elevation_s, elevation_c] = elevation.sincos();
        return std::make_tuple(Polar<T>(distance * elevation_c, azimuth), distance * elevation_s);
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("distance")(self.distance) << os.splitter()
            << os.field("azimuth")(self.azimuth.to_radians()) << os.splitter()
            << os.field("elevation")(self.elevation.to_radians())
        ;
    }
};

}