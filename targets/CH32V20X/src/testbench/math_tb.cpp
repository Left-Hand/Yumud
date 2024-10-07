#include "tb.h"

#include "sys/debug/debug_inc.h"

#include "types/matrix/matrix.hpp"
#include "types/matrix/ceres/ceres.hpp"
#include "types/plane/plane_t.hpp"
#include "types/aabb/aabb.hpp"
#include "types/transform3d/transform3d_t.hpp"
#include "types/transform2d/transform2d.hpp"

#include "robots/kinematics/Scara5/scara5_solver.hpp"
#include "robots/kinematics/Mecanum4/mecanum4_solver.hpp"
#include "robots/kinematics/WheelLeg/wheelleg_solver.hpp"
#include <ranges>

#define EQUAL_ASSERT(a, b)\
do{\
    auto _a = (a);\
    auto _b = (b);\
    if(is_equal_approx(a,b) == false) \
        DEBUG_PRINTS("!!!assert failed:", #a, "\r\n\r\n\t", _a, "!=", _b, "\r\n")\
    else DEBUG_PRINTS(__LINE__, "passed\r\n");\
}while(false);\


//for directly test in godot
#define print(...) DEBUG_PRINTLN(__VA_ARGS__);
#define float real_t
#define var auto


void math_tb(UartHw & logger){

    // logger.init(576000, CommMethod::Blocking);
    logger.init(576000, CommMethod::Dma);
    logger.setEps(4);

    using Vector3 = Vector3_t<real_t>;
    using Plane = Plane_t<real_t>;
    using Basis = Basis_t<real_t>;
    // using Quat = Quat_t<real_t>;
    using Transform3D = Transform3D_t<real_t>;

    EQUAL_ASSERT(Plane(Vector3(1,1,1), -sqrt(real_t(3))).distance_to({0,0,0}), sqrt(real_t(3)))

    DEBUG_PRINTLN(Plane(Vector3(1,1,1), sqrt(real_t(3))).get_center())
    DEBUG_PRINTLN(Plane(Vector3(1,1,1), sqrt(real_t(3))).get_center())
    // EQUAL_ASSERT(real_t(0.2), real_t(0.1));
    
    print(Plane(Vector3(1,1,1), sqrt(float(3))).intersects_segment(Vector3(0,0,0), Vector3(10,10,10)));
    print(Plane(Vector3(1,1,1), sqrt(float(3))));
    print(Plane(Vector3(1,1,1), sqrt(float(3))).normalized());
    print(Plane(Vector3(1,1,1), sqrt(float(3))).get_center());
	// print(Plane(Vector3(3,0,0), Vector3(0,3,0), Vector3(0,0,3)).normalized().has_point(Vector3(1,1,1)));
	print(Plane(Vector3(3,0,0), Vector3(0,3,0), Vector3(0,0,3)).normalized());
	print(Plane(Vector3(3,0,0), Vector3(0,3,0), Vector3(0,0,3)).normalized().has_point(Vector3(1,1,1)));
    print(Plane(Vector3(3,0,0), Vector3(0,3,0), Vector3(0,0,3)));
	print(Plane(Vector3(3,0,0), Vector3(0,3,0), Vector3(0,0,3)).intersects_segment(Vector3(0,0,0), Vector3(10,10,10)));
    print(Vector3(0,0,0)- Vector3(10,10,10));
    // print(Transform3D(Basis(), Vector3(0,0,0)))

    var a = AABB_t<float>(Vector3(0,0,0), Vector3(1,1,1));

    Transform2D_t<real_t> transform2d;

    // 遍历整数序列
    for (auto i : std::ranges::iota_view(0, 8)) {
        print(a.get_endpoint(i))
    }

    var transform = Transform3D();

    transform.origin = Vector3(5, 5, 5);

    // transform.basis = 
    // var b = Basis().rotated(Vector3(0, 1, 0), float(PI / 4));
    var b = Basis().rotated(Vector3(0, 1, 0), float(PI / 4));
    print(b)
    // transform.basis.scale(Vector3(2, 2, 2));

    // DEBUG_PRINTLN(transform);

    // [[maybe_unused]] using Vector2 = Vector2_t<real_t>;
    using Scara5Solver = Scara5Solver_t<real_t>;
    using Mecanum4Solver = Mecanum4Solver_t<real_t>;

    auto config_s5s = Scara5Solver::Config{
        .should_length_meter = real_t(0.06),
        .forearm_length_meter = real_t(0.18),
        .upperarm_length_meter = real_t(0.12)
    };

    auto config_m4s = Mecanum4Solver::Config{
        .chassis_width_meter = real_t(0.26),  
        .chassis_height_meter = real_t(0.26)
    };

    Scara5Solver s5s{config_s5s};
    Mecanum4Solver m4s{config_m4s};

    // using ws_real = double;
    using ws_real = real_t;
    WheelLegSolver_t<ws_real> wls{WheelLegSolver_t<ws_real>::Config{
        .pelvis_length_meter = ws_real(0.12),
        .thigh_length_meter = ws_real(0.12),
        .shin_length_meter = ws_real(0.12),
    }};
    
    while(true){
        
        // auto a = plane.intersects_segment({0,0,0}, {10,10,10});
        // DEBUG_PRINTLN(plane.distance_to({2,2,2}))
        // auto targ_pos = Vector2(real_t(0), real_t(0.12)) + Vector2(real_t(0.06), real_t(0)).rotated(t);
        // auto [l, r] = s5s.invrese(targ_pos);
        // auto est_pos = s5s.forward(l, r);
        // DEBUG_PRINTLN(targ_pos - est_pos);
        // DEBUG_PRINTLN(m4s.inverse({0,1}, 1));

        auto left_pos = Vector3_t<ws_real>(ws_real(-0.1), ws_real(-0.2), 0);
        auto right_pos = Vector3_t<ws_real>(ws_real(0.1), ws_real(-0.1), ws_real(0.02));
        auto pitch_rad = ws_real(0.143);
        // DEBUG_PRINTLN(wls.foot_plane(left_pos, right_pos, pitch_rad));

        auto begin_micros = micros();
        auto transform = wls.get_ground_viewer().get_pelvis_transform(left_pos, right_pos, pitch_rad);
        auto delta_micros = micros() - begin_micros;
        DEBUG_PRINTLN(delta_micros, transform);
        Sys::Clock::reCalculateTime();
    }
}