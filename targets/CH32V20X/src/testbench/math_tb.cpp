#include "tb.h"

#include "sys/debug/debug_inc.h"

#include "types/matrix/matrix.hpp"
#include "types/matrix/ceres/ceres.hpp"
#include "types/plane/plane_t.hpp"
#include "types/aabb/aabb.hpp"
#include "types/transform3d/transform3d_t.hpp"
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

    logger.init(576000, CommMethod::Blocking);
    logger.setEps(4);

    using Vector3 = Vector3_t<real_t>;
    using Plane = Plane_t<real_t>;
    using Basis = Basis_t<real_t>;
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
    print(Transform3D(Basis(), Vector3(0,0,0)))

    var a = AABB_t<float>(Vector3(0,0,0), Vector3(1,1,1));

    // 遍历整数序列
    for (auto i : std::ranges::iota_view(0, 8)) {
        print(a.get_endpoint(i))
    }
    while(true){
        
        // auto a = plane.intersects_segment({0,0,0}, {10,10,10});
        // DEBUG_PRINTLN(plane.distance_to({2,2,2}))
        Sys::Clock::reCalculateTime();
    }
}