#include "tb.h"

#include "sys/debug/debug_inc.h"

#include "hal/bus/sdi/sdi.hpp"

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
        logger.prints("!!!assert failed:", #a, "\r\n\r\n\t", _a, "!=", _b, "\r\n");\
    else logger.prints(__LINE__, "passed\r\n");\
}while(false);\


//for directly test in godot


#define print(...)\
logger.println(__VA_ARGS__);\

#define float real_t
#define var auto


void math_tb(UartHw & logger){
    logger.init(576000, CommMethod::Dma);
    logger.setEps(4);
    

    // using Vector3 = Vector3_t<real_t>;
    // using Plane = Plane_t<real_t>;
    // using Basis = Basis_t<real_t>;
    // using Transform3D = Transform3D_t<real_t>;

    // sdi.init();

    // scexpr auto cnt = 200;
    // sstl::vector<int, cnt> cnts;
    // while(true){
    //     // sdi.println(millis());
    //     cnts.clear();
    //     print(millis());
        
    //     for(size_t i = 0; i < cnt; i++){
    //         cnts.push_back(logger.pending());
    //         delay(1);
    //     }
        
    //     print(cnts);

    //     delay(200);
    // }
    logger.println("/?");
    while(true){
        // if(logger.pending() == 0) logger.println(millis());
        while(logger.pending());
        logger.println(millis());
        // else{
            // logger.println(logger.pending());
            // delay(1);
        // }
        delay(10);
    }
    // #define  WHEELLEG_TB


    #ifdef PLANE_TB
    EQUAL_ASSERT(Plane(Vector3(1,1,1), -sqrt(real_t(3))).distance_to({0,0,0}), sqrt(real_t(3)))
    print(Plane(Vector3(1,1,1), sqrt(real_t(3))).get_center())
    print(Plane(Vector3(1,1,1), sqrt(real_t(3))).get_center())
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
    #endif

    
    #ifdef TRANFORM_TB
    var a = AABB_t<float>(Vector3(0,0,0), Vector3(1,1,1));

    Transform2D_t<real_t> transform2d;

    // 遍历整数序列
    for (auto i : std::ranges::iota_view(0, 8)) {
        print(a.get_endpoint(i))
    }

    var transform = Transform3D();

    transform.origin = Vector3(5, 5, 5);
    var b = Basis().rotated(Vector3(0, 1, 0), float(PI / 4));
    print(b)
    #endif
    

    #ifdef SCARA_TB
    using Scara5Solver = Scara5Solver_t<real_t>;
    auto config_s5s = Scara5Solver::Config{
        .should_length_meter = real_t(0.06),
        .forearm_length_meter = real_t(0.18),
        .upperarm_length_meter = real_t(0.12)
    };
    Scara5Solver s5s{config_s5s};
    while(true){
        
        // auto [l, r] = s5s.invrese(targ_pos);
        // auto est_pos = s5s.forward(l, r);
    }

    #endif

    #ifdef  MECANUM_TB

    using Mecanum4Solver = Mecanum4Solver_t<real_t>;
    auto config_m4s = Mecanum4Solver::Config{
        .chassis_width_meter = real_t(0.26),  
        .chassis_height_meter = real_t(0.26)
    };


    Mecanum4Solver m4s{config_m4s};
    while(true){
        
        // print(targ_pos - est_pos);
        // print(m4s.inverse({0,1}, 1));
    }
    #endif


    #ifdef WHEELLEG_TB
    // using WheelLegSolver_t = WheelLegSolver_t<real_t>;
    WheelLegSolver_t<real_t> wls{WheelLegSolver_t<real_t>::Config{
        .pelvis_length_meter = real_t(0.12),
        .thigh_length_meter = real_t(0.12),
        .shin_length_meter = real_t(0.12),
    }};
    
    while(true){

        auto left_pos = Vector3_t<real_t>(real_t(-0.1), real_t(-0.2), 0);
        auto right_pos = Vector3_t<real_t>(real_t(0.1), real_t(-0.1), real_t(0.02));
        auto pitch_rad = real_t(0.143);
        // print(wls.foot_plane(left_pos, right_pos, pitch_rad));

        auto begin_micros = micros();
        auto transform = wls.get_ground_viewer().get_pelvis_transform(left_pos, right_pos, pitch_rad);
        auto delta_micros = micros() - begin_micros;
        print(delta_micros, transform);
        Sys::Clock::reCalculateTime();
    }
    #endif

    while(true);
}