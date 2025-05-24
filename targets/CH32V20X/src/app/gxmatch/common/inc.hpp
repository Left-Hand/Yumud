#pragma once

#include "types/vector3/vector3.hpp"
#include "types/vector2/vector2.hpp"

#include "robots/kinematics/Mecanum4/mecanum4_solver.hpp"
#include "robots/kinematics/Scara5/scara5_solver.hpp"
#include "robots/kinematics/Cross/CrossSolver_t.hpp"

#include "types/Ray2/Ray2.hpp"
#include "types/aabb/AABB.hpp"


#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{

    using namespace ymd;
    using namespace ymd::drivers;

    using Vector3 = Vector3<real_t>;
    using Vector3i = Vector3<int>;
    using AABB = AABB_t<real_t>;
    using Ray = Ray2_t<real_t>;
    
    using Mecanum4Solver = Mecanum4Solver_t<real_t>;
    using Scara5Solver = Scara5Solver_t<real_t>;
    using CrossSolver = CrossSolver_t<real_t>;
}

using namespace ymd::hal;

#include "decl/RotorInfo.hpp"
#include "decl/RowInfo.hpp"
#include "decl/InventoryInfo.hpp"

#include "enum.hpp"

#include "hal/bus/uart/uarthw.hpp"