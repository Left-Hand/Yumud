#pragma once

#include "config.hpp"


#include "types/vector3/vector3_t.hpp"
#include "types/vector2/vector2_t.hpp"

#include "robots/kinematics/Mecanum4/mecanum4_solver.hpp"
#include "robots/kinematics/Scara5/scara5_solver.hpp"
#include "robots/kinematics/Cross/CrossSolver_t.hpp"


#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{

    using namespace ymd;
    using namespace ymd::drivers;

    using Vector2 = Vector2_t<real_t>;
    using Vector2i = Vector2_t<int>;

    using Vector3 = Vector3_t<real_t>;
    using Vector3i = Vector3_t<int>;

    using Mecanum4Solver = Mecanum4Solver_t<real_t>;
    using Scara5Solver = Scara5Solver_t<real_t>;
    using CrossSolver = CrossSolver_t<real_t>;
}


#include "decl/rotorinfo.hpp"
#include "decl/rowinfo.hpp"

#include "enum/index.hpp"

#include "hal/bus/uart/uarthw.hpp"