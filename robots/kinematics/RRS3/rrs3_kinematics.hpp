#pragma once

#include "../kinematics_utils.hpp"

#include <cmath>
#include <array>

#include "types/regions/ray2.hpp"
#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"
#include "types/regions/plane.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/nth.hpp"

namespace ymd::robots{




template<arithmetic T>
class RR2_Kinematics final{
public:

    struct Config{
        T base_length;
        T link_length;
    };

    struct Gesture{
        const Vec2<T> shift_position;
    };

    struct Solution{
        struct Absolute{
            T j1_abs_rad;
            T j2_abs_rad;

            friend OutputStream & operator<<(OutputStream & os, Absolute const & self){
                return os << "j1_abs_rad: " << self.j1_abs_rad << ", j2_abs_rad: " << self.j2_abs_rad;
            }
        };

        struct Relative{
            T j1_rel_rad;
            T j2_rel_rad;

            friend OutputStream & operator<<(OutputStream & os, Relative const & self){
                return os << "j1_rel_rad: " << self.j1_rel_rad << ", j2_rel_rad: " << self.j2_rel_rad;
            }
        };

        constexpr Absolute to_absolute() const {
            return {j1_abs_rad_, j2_abs_rad_};
        }

        constexpr Relative to_relative() const {
            return {j1_abs_rad_, j2_abs_rad_ - j1_abs_rad_};
        }


        static constexpr Solution from_absolute(const Absolute & solu){
            Solution self;
            self.j1_abs_rad_ = solu.j1_abs_rad; 
            self.j2_abs_rad_ = solu.j2_abs_rad;
            return self;
        }
        static constexpr Solution from_relative(const Relative & solu){
            Solution self;
            self.j1_abs_rad_ = solu.j1_abs_rad; 
            self.j2_abs_rad_ = solu.j2_abs_rad + solu.j1_abs_rad;
            return self;
        }

    private:
        T j1_abs_rad_{};
        T j2_abs_rad_{};
    };


    static constexpr Gesture forward(const Config & cfg, const Solution & solu){
        const auto B = cfg.base_length;
        const auto L = cfg.link_length;
        const auto R1 = solu.to_absolute().j1_abs_rad;
        const auto R2 = solu.to_absolute().j2_abs_rad;
        return {Vec2<T>(B, 0).rotated(R1) + Vec2<T>(L, 0).rotated(R2)};
    }

    static constexpr Option<Solution> inverse(const Config & cfg, const Gesture & gest){
        const auto pos = gest.shift_position;
        const T L_squ = pos.length_squared();
        const T L = std::sqrt(L_squ);
        //关节活动超出约束空间(远区)
        if(L_squ > square(cfg.base_length + cfg.link_length)) 
            return None;

        //关节活动超出约束空间(近死区)
        if(L_squ < square(cfg.base_length - cfg.link_length)) 
            return None;

        //TODO 减少超越函数计算量
        const T L1 = cfg.base_length;
        const T L2 = cfg.link_length;
        const T L1_squ = square(L1);
        const T L2_squ = square(L2);


        const T phi = std::atan2(pos.y, pos.x);

        //计算基关节角度
        const T R1 = phi - std::acos((L1_squ + L_squ - L2_squ) / (2*L1*L));

        //计算串联关节角度
        const T R2 = phi + std::acos((L2_squ + L_squ - L1_squ) / (2*L2*L));

        return Some(Solution::from_absolute({R1,R2}));

    }
};





template<arithmetic T>
class RRS_Kinematics final{
public:
    using Error = kinematics::prelude::Error;
    template<typename U = void>
    using IResult = Result<U, Error>;

    struct Config{
        T base_length;//基座摇臂长度(米)
        T link_length;///上摇臂长度(米)
        T base_plate_radius;//基座中心轴到关节的距离(米)
        T top_plate_radius;///顶座中心轴到关节的距离(米)
    };

    struct Gesture{
        Quat<T> orientation;
        T z;

        struct Initializer{
            real_t yaw;
            real_t pitch;
            real_t height;
        };

        static constexpr Gesture from(const Initializer & iz){
            const Gesture gest{
                .orientation = Quat<real_t>::from_euler<EulerAnglePolicy::XYZ>({
                    .x = iz.yaw, 
                    .y = iz.pitch, 
                    .z = 0
                }),

                .z = iz.height,
            };

            return gest;
        }
    };


    using SideSolution = typename RR2_Kinematics<T>::Solution;
    using Solution = std::array<SideSolution, 3>;

    constexpr RRS_Kinematics(const Config & cfg){
        reconf(cfg);
    }

    constexpr Option<Solution> inverse(const Gesture & gest) const{
        const auto vars_0 = inverse_single_axis(cfg_, norms_[0], gest);
        const auto vars_1 = inverse_single_axis(cfg_, norms_[1], gest);
        const auto vars_2 = inverse_single_axis(cfg_, norms_[2], gest);
        if(vars_0.is_some() && vars_1.is_some() && vars_2.is_some()){
            return Some(Solution{
                vars_0.unwrap(),
                vars_1.unwrap(),
                vars_2.unwrap(),
            });
        }else{
            return None;
        }
    }

    // 正解需要考虑yaw角错位 难度极高
    // void forward(const Solution & solu) const {
    //     const auto p0 = forward_point(cfg_, norms_[0], solu[0]);
    //     const auto p1 = forward_point(cfg_, norms_[1], solu[1]);
    //     const auto p2 = forward_point(cfg_, norms_[2], solu[2]);

    //     const auto center = (p0 + p1 + p2) / 3;
    //     const auto z = center.z;
    //     const auto ori = Quat<T>::from_shortest_arc({0,0,1}, (p0 - center).normalized());
    //     DEBUG_PRINTLN(p0, p1, p2);
    //     DEBUG_PRINTLN((p0 - p1).length(), (p1 - p2).length());
    //     DEBUG_PRINTLN(ori,z);
    // }

    //evil fn
    constexpr void reconf(const Config & cfg){
        cfg_ = cfg;

        // for(size_t i = 0; i < 3; i++)
        //     norms_[i] = get_xynorm_from_nth(i);
    }
private:
    Config cfg_ {};

    // static constexpr SQRT3 =  1.732050807568877;
    static constexpr std::array<Vec2<T>, 3> norms_ {
        Vec2<T>(1, 0),
        Vec2<T>(T(-0.5), T(SQRT3/2)),
        Vec2<T>(T(-0.5), T(-SQRT3/2))
    };

    //pure fn
    static constexpr Vec2<T> get_xynorm_from_nth(const Nth nth){
        return Vec2<T>::RIGHT.rotated(nth.count() * T(TAU / 3));
    };

    //pure fn
    //输入关节的方位单位向量
    //获取基座铰链的位置
    static constexpr Vec3<T> get_base_point(const Config & cfg, const Vec2<T> xy_norm){
        const auto base_point_2 = Vec2<T>{cfg.base_plate_radius, 0}.improduct(xy_norm);
        return Vec3<T>{base_point_2.x, base_point_2.y, 0};
    }

    //pure fn
    //输入关节的方位单位向量(寻找对应的铰链)以及姿态 
    //获取顶部铰链的位置
    static constexpr Vec3<T> get_top_point(const Config & cfg, const Vec2<T> xy_norm, const Gesture & gest){
        const auto top_point_2 = Vec2<T>{cfg.top_plate_radius, 0}.improduct(xy_norm);
        return gest.orientation.xform(Vec3<T>{top_point_2.x, top_point_2.y, 0}) + Vec3<T>(0, 0, gest.z);
    }

    struct Sphere{
        Vec3<T> org;
        T radius;

        friend OutputStream & operator << (OutputStream & os, const Sphere & self){
            return os << os.brackets<'{'>() << 
                self.org << os.splitter() << self.radius << 
                os.brackets<'}'>();
        }
    };

    struct CirclePassZAxis{
        //这个圆所在的平面包含z轴 自由度特殊化
        Vec3<T> org;
        T radius;

        friend OutputStream & operator << (OutputStream & os, const CirclePassZAxis & self){
            return os << os.brackets<'{'>() << 
                self.org << os.splitter() << self.radius << 
                os.brackets<'}'>();
        }
    };

    //pure fn
    //输入关节的方位单位向量(寻找对应的铰链)以及姿态 
    //获取顶部铰链的活动球面
    static constexpr Sphere get_top_sphere(const Config & cfg, const Vec2<T> xy_norm, const Gesture & gest){
        return Sphere{get_top_point(cfg, xy_norm, gest), cfg.link_length};
    }

    //pure fn
    //输入活动球面和方位单位向量
    //获取切片圆
    static constexpr Option<CirclePassZAxis> project_sphere_to_circle(const Sphere & sphere, const Vec2<T> xy_norm){
        // 方位单位向量的结构体绑定
        // [x', y'] = xy_norm

        // 方位单位向量的二维直线方程
        // x / x' = y / y'
        // => x' * y - y' * x = 0

        // 求解点到直线(即活动球心到平面)的距离
        // dist = |x' * x - y' * y| / sqrt(x'^2 + y'^2)
        // => dist = |x' * x - y' * y|

        // 当圆未与平面相交时 返回None
        
        const T xy_dist = std::abs(xy_norm.x * sphere.org.y - xy_norm.y * sphere.org.x);
        if(const auto temp = square(sphere.radius) - square(xy_dist); temp >= 0){
            const T circle_radius = std::sqrt(temp);
            return Some(CirclePassZAxis{
                .org = sphere.org, 
                .radius = circle_radius
            });
        }else{
            return None;
        }
    }

    //pure fn
    //通过计算底部，顶部铰链位置和切片圆的半径 将问题转换为二维平面上的双转动副问题
    static constexpr Option<SideSolution> inverse_single_axis(
        const Config & cfg, 
        const Vec2<T> xy_norm, 
        const Gesture & gest
    ){
        const auto sphere = get_top_sphere(cfg, xy_norm, gest);
        const auto circle = ({
            const auto may_circle = project_sphere_to_circle(sphere, xy_norm);
            if(may_circle.is_none()) return None;
            may_circle.unwrap();
        });

        return RR2_Kinematics<T>::inverse(
            {.base_length = cfg.base_length, .link_length = circle.radius},//大臂长度和小臂长度
            {.shift_position = Vec2<T>{circle.org.length(), circle.org.z} 
                - Vec2<T>{cfg.base_plate_radius, 0}}            //目的位置
        );
    }

    // 正解需要考虑yaw角错位 难度极高
    // static constexpr Vec3<T> forward_point(const Config & cfg, const Vec2<T> xy_norm, const SideSolution & solu){
    //     const auto p_2d = RR2_Kinematics<T>::forward(
    //         //大臂长度和小臂长度
    //         {.base_length = cfg.base_length, .link_length = cfg.link_length}, 

    //         //关节角度
    //         solu,
    //         //基位置
    //         Vec2<T>{cfg.base_plate_radius,0}
    //     );

    //     const auto p_3d = Vec3<T>{
    //         p_2d.x * xy_norm.x, 
    //         p_2d.x * xy_norm.y, 
    //         p_2d.y
    //     };

    //     return p_3d;
    // }

};

}