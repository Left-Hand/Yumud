#include "config.hpp"

using namespace gxm;

SysConfig gxm::create_default_config(){
    return SysConfig{
        .joint_config = {
            .max_rad_delta = real_t(1.13),
            // .left_basis_radian = real_t(-PI/2 + 0.2),
            .left_basis_radian = real_t(-PI/2 + 0.34),
            // .right_basis_radian = real_t(PI/2 - 0.10),
            // .right_basis_radian = real_t(PI/2 - 0.20),
            // .right_basis_radian = real_t(PI/2 + 0.3),
            // .right_basis_radian = real_t(PI/2 - 0.2),
            // .right_basis_radian = real_t(PI/2 + 0.16),
            .right_basis_radian = real_t(PI/2 - 0.01),
            // .right_basis_radian = real_t(0),
            // .z_basis_radian = real_t(PI/2 + 0.17),
            .z_basis_radian = real_t(PI/2 + 0.25),
        },

        .scara_config = {
            .solver_config = {
                .should_length_meter = real_t(0.08),
                .upperarm_length_meter = real_t(0.10),
                .forearm_length_meter = real_t(0.18),
            },

            .claw_config = {
                .press_radian = real_t(PI/2),
                .release_radian = 0
            },

            .nozzle_config = {
                // .sustain = 400
            },
        },
        
        .zaxis_config = {
            // .max_height = 0.25_r,
            // .tray_height = 0.2_r,
            // .free_height = 0.15_r,
            // .ground_height = 0.12_r,
            
            // .basis_radian = real_t(0),
            .solver_config = CrossSolver::Config{
                .xoffs_length_meter = 0.042_r,
                .upperarm_length_meter = 0.082_r,
                .forearm_length_meter = 0.1_r
            }
            // //1m / (3cm * PI)
            // .meter_to_turns_scaler = real_t(1 / (0.03 * PI)),
            
            // //1mm tolerance
            // .reached_threshold = real_t(0.001),  
        },

        .grab_config = {
            .tray_xy = {
                Vector2{-0.0983_r    ,   0.144_r},
                Vector2{-0.004_r     ,   0.144_r},
                Vector2{0.0905_r     ,   0.144_r}
            },

            .tray_z = 0.1333_r,

            .free_z = 0.169_r,

            .catch_z = 0.056_r,

            .z_bias = 0.00005_r,
        
            .catch_xy = {
                Vector2{-0.1_r, 0.24_r},
                Vector2{-0.0_r, 0.24_r},
                Vector2{0.1_r, 0.24_r},
            },
            
            .inspect_xyz = Vector3{0, 0.24_r, 0.16_r},
            .idle_xyz = Vector3{0, 0.145_r, 0.16_r},
            
            .home_xyz = Vector3(0, 0.24_r, 0.16_r),

            .safe_aabb = AABB{
                Vector3{-0.13_r, 0.12_r, 0.0_r},
                Vector3{0.26_r, 0.15_r, 0.17_r}
            },

            .max_spd = 1.03_r,
            .max_acc = 0.8_r,

            .max_z_spd = 1.07_r,
            .max_z_acc = 0.8_r,
            .nozzle_sustain = 200
        },


        .wheels_config = {
            .wheel_config = {
                // .wheel_radius = 0.0315_r,//轮子半径
                .wheel_radius = 0.0309_r,//轮子半径
                // .wheel_radius = 0.0307_r,//轮子半径
                .max_tps = 1,//最大每秒转圈数
                .max_aps = 1,//最大每秒圈速加速度
            },

            .max_curr = 0.6_r
        },

        .chassis_config = {
            .solver_config = {
                .chassis_width_meter = 0.157_r,
                .chassis_height_meter = 0.237_r
            },

            .rot_ctrl_config = {
                .kp = 4,
                .kd = 1.6_r
            },

            .pos_ctrl_config = {
                .kp = 4,
                .kd = 1.6_r
            },

            .max_acc = 0.85_r,
            .max_spd = 1.7_r,
            
            .max_agr = 1.7_r,
            .max_spr = 2.8_r,

            .still_time = 0.5_r
        }

    };
}