#pragma once

// https://github.com/openrr/k/blob/main/src/ik.rs

namespace ymd::kinematics{


// JacobianIkSolver::solve
// └── JacobianIkSolver::solve_with_constraints
//     └── JacobianIkSolver::solve_with_constraints_internal
//         └── loop num_max_try 次
//             └── JacobianIkSolver::solve_one_loop_with_constraints
//                 ├── calc_pose_diff_with_constraints
//                 │   └── calc_pose_diff
//                 ├── jacobian (arm)
//                 ├── add_positions_with_multiplier
//                 ├── arm.set_joint_positions_clamped
//                 └── calc_pose_diff_with_constraints
//                     └── calc_pose_diff
//         └── target_diff_to_len_rot_diff（用于判断收敛）
}