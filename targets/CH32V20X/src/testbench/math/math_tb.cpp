#include <ranges>

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/int/int_t.hpp"
#include "core/math/realmath.hpp"
#include "core/math/matrix/static_matrix.hpp"
#include "core/math/matrix/ceres/ceres.hpp"

#include "hal/bus/sdi/sdi.hpp"

#include "algebra/regions/plane.hpp"
#include "algebra/regions/aabb.hpp"
#include "algebra/regions/segment2.hpp"
#include "algebra/regions/line2.hpp"
#include "algebra/regions/ray2.hpp"

#include "algebra/transforms/transform3d.hpp"
#include "algebra/transforms/transform2d.hpp"

#include "algebra/shapes/Arc2.hpp"
#include "algebra/shapes/Bezier2.hpp"
#include "algebra/regions/perspective_rect.hpp"


#include "robots/kinematics/Scara5/scara5_kinematics.hpp"
#include "robots/kinematics/Mecanum4/mecanum4_kinematics.hpp"
#include "robots/kinematics/WheelLeg/wheelleg_kinematics.hpp"

#include "hal/bus/uart/uarthw.hpp"



using namespace ymd;


#define EQUAL_ASSERT(a, b)\
do{\
    auto _a = (a);\
    auto _b = (b);\
    if(is_equal_approx(a,b) == false) \
        DEBUGGER.prints("!!!assert failed:", #a, "\r\n\r\n\t", _a, "!=", _b, "\r\n");\
    else DEBUGGER.prints(__LINE__, "passed\r\n");\
}while(false);\


//for directly test in godot


#define print(...)\
DEBUGGER.println(__VA_ARGS__);\

// #define iq16 iq16
#define var auto


void math_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    // using Vec3 = Vec3<iq16>;
    // using Plane = Plane<iq16>;
    // using Basis = Basis<iq16>;
    // using Transform3D = Transform3D<iq16>;

    // sdi.init();

    // static constexpr auto cnt = 200;
    // sstl::vector<int, cnt> cnts;
    // while(true){
    //     // sdi.println(millis());
    //     cnts.clear();
    //     print(millis());
        

    //     print(cnts);

    //     clock::delay(200ms);
    // }

    // #define  WHEELLEG_TB

    #define PPR_TB

    #ifdef PPR_TB
    constexpr std::array<Vec2<iq16>, 4> dst = {
        Vec2<iq16>{0, 0},
        Vec2<iq16>{1, 0},
        Vec2<iq16>{3, 4},
        Vec2<iq16>{0, 1}
    };
    
    volatile size_t n = 0;
    constexpr auto H = compute_homography_from_unit_rect(
        std::span(dst));

    const auto elapsed = measure_total_elapsed_us([&]{
        const auto mat = compute_homography_from_unit_rect(
            std::span(dst)
        );
        (void)mat;
        n++;
    }, 10000);

    while(true){
        DEBUG_PRINTLN(H, map_uv(H, {0.5_r, 0.5_r}), elapsed, n);
        clock::delay(5ms);
    }
    #endif
    

    #ifdef PLANE_TB
    EQUAL_ASSERT(Plane(Vec3(1,1,1), -sqrt(iq16(3))).distance_to({0,0,0}), sqrt(iq16(3)))
    print(Plane(Vec3(1,1,1), sqrt(iq16(3))).get_center())
    print(Plane(Vec3(1,1,1), sqrt(iq16(3))).get_center())
    // EQUAL_ASSERT(iq16(0.2), iq16(0.1));
    
    print(Plane(Vec3(1,1,1), sqrt(iq16(3))).intersects_segment(Vec3(0,0,0), Vec3(10,10,10)));
    print(Plane(Vec3(1,1,1), sqrt(iq16(3))));
    print(Plane(Vec3(1,1,1), sqrt(iq16(3))).normalized());
    print(Plane(Vec3(1,1,1), sqrt(iq16(3))).get_center());
	// print(Plane(Vec3(3,0,0), Vec3(0,3,0), Vec3(0,0,3)).normalized().has_point(Vec3(1,1,1)));
	print(Plane(Vec3(3,0,0), Vec3(0,3,0), Vec3(0,0,3)).normalized());
	print(Plane(Vec3(3,0,0), Vec3(0,3,0), Vec3(0,0,3)).normalized().has_point(Vec3(1,1,1)));
    print(Plane(Vec3(3,0,0), Vec3(0,3,0), Vec3(0,0,3)));
	print(Plane(Vec3(3,0,0), Vec3(0,3,0), Vec3(0,0,3)).intersects_segment(Vec3(0,0,0), Vec3(10,10,10)));
    print(Vec3(0,0,0)- Vec3(10,10,10));
    // print(Transform3D(Basis(), Vec3(0,0,0)))
    #endif

    
    #ifdef TRANFORM_TB
    var a = AABB<iq16>(Vec3(0,0,0), Vec3(1,1,1));

    Transform2D<iq16> transform2d;

    // 遍历整数序列
    for (auto i : std::ranges::iota_view(0, 8)) {
        print(a.get_endpoint(i))
    }

    var transform = Transform3D();

    transform.origin = Vec3(5, 5, 5);
    var b = Basis().rotated(Vec3(0, 1, 0), iq16(PI / 4));
    print(b)
    #endif
    

    #ifdef SCARA_TB
    using Scara5Solver = Scara5Solver_t<iq16>;
    auto config_s5s = Scara5Solver::Config{
        .should_length_meter = iq16(0.06),
        .forearm_length_meter = iq16(0.18),
        .upperarm_length_meter = iq16(0.12)
    };
    Scara5Solver s5s{config_s5s};
    while(true){
        
        // auto [l, r] = s5s.invrese(targ_pos);
        // auto est_pos = s5s.forward(l, r);
    }

    #endif

    #ifdef  MECANUM_TB

    using Mecanum4Solver = Mecanum4Solver_t<iq16>;
    auto config_m4s = Mecanum4Solver::Config{
        .chassis_width_meter = iq16(0.26),  
        .chassis_height_meter = iq16(0.26)
    };


    Mecanum4Solver m4s{config_m4s};
    while(true){
        
        // print(targ_pos - est_pos);
        // print(m4s.inverse({0,1}, 1));
    }
    #endif


    #ifdef WHEELLEG_TB
    // using WheelLegSolver_t = WheelLegSolver_t<iq16>;
    WheelLegSolver_t<iq16> wls{WheelLegSolver_t<iq16>::Config{
        .pelvis_length_meter = iq16(0.12),
        .thigh_length_meter = iq16(0.12),
        .shin_length_meter = iq16(0.12),
    }};
    
    while(true){

        auto left_pos = Vec3<iq16>(iq16(-0.1), iq16(-0.2), 0);
        auto right_pos = Vec3<iq16>(iq16(0.1), iq16(-0.1), iq16(0.02));
        auto pitch_rad = iq16(0.143);
        // print(wls.foot_plane(left_pos, right_pos, pitch_rad));

        auto begin_micros = micros();
        auto transform = wls.get_ground_viewer().get_pelvis_transform(left_pos, right_pos, pitch_rad);
        auto delta_micros = micros() - begin_micros;
        print(delta_micros, transform);
        Sys::Clock::reCalculateTime();
    }
    #endif


    // #define  SEGMENT_TB
    #ifdef SEGMENT_TB

    using Segment = Segment2D_t<iq16>;

    auto seg = Segment{}
    #endif


    #define  LINE_TB
    #ifdef LINE_TB

    using Line = Line2<iq16>;


    auto line = Line{Vec2<iq16>{1,0}, Vec2<iq16>{0,1}};
    auto other = Line::from_point_and_angle(Vec2<iq16>{0,0}, Angular<iq16>::QUARTER);
    print("line", line);
    print("other",other);

    print(line.x_at_y(1));
    print(line.y_at_x(-1));
    print("abc:", line.abc());
    print("angle:", line.orientation);
    print("abs", line.abs());
    print("dist", line.distance_to(Vec2<iq16>{0.5_r, 0.5_r}));
    print("dist", line.distance_to(Vec2<iq16>{0.5_r, 0.4_r}));
    print("intersection", line.intersection(
        Line::from_point_and_angle(
            Vec2<iq16>{0,0}, 
            Angular<iq16>::from_radians(math::atan(iq16(0.3333_r)))
        ),
        0.00001_iq16
    ));
    print("foot", line.foot_of(Vec2<iq16>{0, 0.5_r}));
    print("mirror", line.mirror(Vec2<iq16>{0, 0.5_r}));
    print("perpendicular", line.perpendicular(Vec2<iq16>{0, 0.5_r}));
    print("orthogonal_with", line.is_orthogonal_with(
        Line::from_point_and_angle(Vec2<iq16>{0,0}, Angular<iq16>::QUARTER), 
        Angular<iq16>::from_turns(iq16(1e-6))));
    print("rebase", line.rebase(Vec2<iq16>{-1,0}));
    print("rotated", line.rotated(Vec2<iq16>{-1,0}, Angular<iq16>::QUARTER));
    print("normal", line.normal(Vec2<iq16>{-1,0}));
    #endif

    #define MATRIX_TB


    while(true);
}



// // 测试工具函数
// template<typename T>
// constexpr bool matrix_near(const Matrix3x3<T>& a, const Matrix3x3<T>& b, T epsilon = 1e-6) {
//     for (size_t i = 0; i < 3; ++i)
//         for (size_t j = 0; j < 3; ++j)
//             if (std::abs(a[i][j] - b[i][j]) > epsilon)
//                 return false;
//     return true;
// }

// // 测试用例1：单位矩形到放大矩形的变换
// constexpr bool test_unit_rect_scaling() {
//     constexpr std::array<Vec2<float>, 4> dst = {
//         Vec2<float>{0, 0},
//         Vec2<float>{2, 0},
//         Vec2<float>{2, 2},
//         Vec2<float>{0, 2}
//     };

//     constexpr std::array<Vec2<float>, 4> src = {
//         Vec2<float>{0, 0},
//         Vec2<float>{1, 0},
//         Vec2<float>{1, 1},
//         Vec2<float>{0, 1}
//     };
    
//     // constexpr auto H = compute_homography_from_unit_rect(
//     //     std::span<const Vec2<float>, 4>(dst));
//     constexpr auto H = compute_homography(
//         std::span(src), std::span(dst));
    
//     // constexpr auto expected = Matrix3x3<float>{
//     //     2, 0, 0,
//     //     0, 2, 0,
//     //     0, 0, 1
//     // };
    
//     return matrix_near(H);
// }

// // 测试用例2：仿射变换验证
// constexpr bool test_affine_transform() {
//     constexpr std::array<Vec2<double>, 4> src = {
//         Vec2<double>{0, 0},
//         Vec2<double>{1, 0},
//         Vec2<double>{1, 1},
//         Vec2<double>{0, 1}
//     };
    
//     constexpr std::array<Vec2<double>, 4> dst = {
//         Vec2<double>{0, 0},
//         Vec2<double>{2, 0},
//         Vec2<double>{2, 3},
//         Vec2<double>{0, 3}
//     };
    
//     constexpr auto H = compute_homography(
//         std::span<const Vec2<double>, 4>(src),
//         std::span<const Vec2<double>, 4>(dst));
    
//     constexpr auto expected = Matrix3x3<double>{
//         2, 0, 0,
//         0, 3, 0,
//         0, 0, 1
//     };
    
//     return matrix_near(H, expected);
// }

// // 测试用例3：透视变换验证
// constexpr bool test_perspective_transform() {
//     constexpr std::array<Vec2<float>, 4> src = {
//         Vec2<float>{0, 0},
//         Vec2<float>{1, 0},
//         Vec2<float>{1, 1},
//         Vec2<float>{0, 1}
//     };
    
//     constexpr std::array<Vec2<float>, 4> dst = {
//         Vec2<float>{0, 0},
//         Vec2<float>{1, 0},
//         Vec2<float>{0.5f, 1},
//         Vec2<float>{0.5f, 1}
//     };
    
//     constexpr auto H = compute_homography(
//         std::span<const Vec2<float>, 4>(src),
//         std::span<const Vec2<float>, 4>(dst));
    
//     // 预期结果需要根据实际计算确定
//     constexpr auto expected = Matrix3x3<iq16>{
//         1, 0.5f, 0,
//         0, 1, 0,
//         0, 1, 1
//     };
    
//     return matrix_near(H, expected);
// }

[[maybe_unused]] static void  ppr_test() {
    // 编译时测试
    // static_assert(test_unit_rect_scaling(), 
    //     "Unit rectangle to quad scaling failed");
    
    // static_assert(test_affine_transform(), 
    //     "Affine transform test failed");
    
    // static_assert(test_perspective_transform(), 
    //     "Perspective transform test failed");

}