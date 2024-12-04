#include "config.hpp"

using namespace gxm;

SysConfig gxm::create_default_config(){
    return SysConfig{
        .joint_config = {
            .max_rad_delta = real_t(0.07),
            .left_basis_radian = real_t(-PI/2 + 0.33),
            // .right_basis_radian = real_t(PI/2 - 0.10),
            // .right_basis_radian = real_t(PI/2 - 0.20),
            // .right_basis_radian = real_t(PI/2 + 0.3),
            // .right_basis_radian = real_t(PI/2 - 0.2),
            .right_basis_radian = real_t(PI/2 + 0.5),
            // .right_basis_radian = real_t(0),
            .z_basis_radian = real_t(PI/2 + 0.17),
        },

        .scara_config = {
            .solver_config = {
                .should_length_meter = real_t(0.08),
                .forearm_length_meter = real_t(0.10),
                .upperarm_length_meter = real_t(0.18),
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
                .forearm_length_meter = 0.082_r,
                .upperarm_length_meter = 0.1_r
            }
            // //1m / (3cm * PI)
            // .meter_to_turns_scaler = real_t(1 / (0.03 * PI)),
            
            // //1mm tolerance
            // .reached_threshold = real_t(0.001),  
        },

        .grab_config = {
            .tray_xy = {
                Vector2{-0.12_r   , 0.20_r},
                Vector2{0       , 0.20_r},
                Vector2{0.12_r    , 0.20_r}
            },

            .tray_z = 0.10_r,

            .free_z = 0.14_r,

            .catch_z = 0.06_r,

            .z_bias = 0.005_r,
        
            .catch_xy = Vector2{0, 0.15_r},

            .inspect_xyz = Vector3{0, 0.12_r, 0.15_r},
            
            .home_xyz = Vector3(0, 0.12_r, 0.15_r),

            .safe_aabb = AABB{
                Vector3{-0.13_r, 0.12_r, 0.0_r},
                Vector3{0.26_r, 0.14_r, 0.17_r}
            },

            .max_spd = 0.2_r,
            .max_acc = 0.6_r,

            .nozzle_sustain = 500
        },

        .wheel_config = {
            .wheel_radius = 0.06_r,//轮子直径
            .max_tps = 1,//最大每秒转圈数
            .max_aps = 1,//最大每秒圈速加速度
        },

        .wheels_config = {},

        .est_config = {
            .calibrate_times = 1,//校准次数
            .force_calibrate = false,//强制校准而不是使用先验偏置
            .pre_bias = {

            },//偏置 空为未给

            .rot_obs_config = {//转向观测器配置
                .kq = 0.5_r,
                .ko = 0.5_r,
            }
        }
    };
}