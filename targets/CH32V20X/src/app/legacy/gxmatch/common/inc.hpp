#pragma once

#include "algebra/vectors/vec3.hpp"
#include "algebra/vectors/vec2.hpp"

#include "robots/kinematics/Mecanum4/mecanum4_solver.hpp"
#include "robots/kinematics/Scara5/scara5_solver.hpp"
#include "robots/kinematics/Cross/CrossSolver_t.hpp"

#include "algebra/ray2.hpp"
#include "algebra/aabb.hpp"


#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{

    using namespace ymd;
    using namespace ymd::drivers;

    using math::Vec3 = math::Vec3<iq16>;
    using math::Vec3i = math::Vec3<int>;
    using AABB = AABB<iq16>;
    using Ray = Ray2<iq16>;
    
    using Mecanum4Solver = Mecanum4Solver_t<iq16>;
    using Scara5Solver = Scara5Solver_t<iq16>;
    using CrossSolver = CrossSolver_t<iq16>;
}



#include "decl/RotorInfo.hpp"
#include "decl/RowInfo.hpp"
#include "decl/InventoryInfo.hpp"

#include "enum.hpp"

#include "hal/bus/uart/hw_singleton.hpp"