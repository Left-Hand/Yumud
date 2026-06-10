#include "algebra/gesture/isometry2.hpp"
#include "algebra/gesture/isometry3.hpp"


using namespace ymd;


// fn iso2(x: f64, y: f64, angle: f64) -> Isometry2<f64> {
//     Isometry2::from_parts(Translation2::new(x, y), UnitComplex::from_angle(angle))
// }

// fn iso3(x: f64, y: f64, z: f64, qx: f64, qy: f64, qz: f64, qw: f64) -> Isometry3<f64> {
//     let translation = Translation3::new(x, y, z);
//     let rotation = UnitQuaternion::from_quaternion(Quaternion::new(qx, qy, qz, qw));
//     Isometry3::from_parts(translation, rotation)
// }


// template<typename T>
// static constexpr iso2(
//     T x,
//     T y,
//     T angle
// )