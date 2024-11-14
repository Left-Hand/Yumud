#include "gxmatch.hpp"
#include "host/host.hpp"

#include "machine/scara/scara.hpp"
#include "machine/actuator/zaxis_stp.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "robots/foc/remote/remote.hpp"

#include "hal/bus/i2c/i2csw.hpp"

using Sys::t;

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;

namespace gxm{


using Vector2 = Vector2_t<real_t>;
scexpr real_t squ_len = 96;
scexpr Vector2 pos_begin = {111, 46};
scexpr Vector2 pos_end = pos_begin + Vector2{squ_len,squ_len};
scexpr Vector2 pos_center = Vector2(pos_begin) + Vector2(squ_len / 2, squ_len / 2);
scexpr Vector2 pos_pending = Vector2(pos_center) - Vector2(80, 0);


[[maybe_unused]] static Vector2 get_square_rounded_position(uint8_t index){
    // xymm 110.5 30
    // xymm 208.5 126
    index = index - 1;
    const auto x_i = 1 - (real_t(index % 3) / 2);
    const auto y_i = real_t(index / 3) / 2;

    return Vector2(pos_begin.x + (pos_end.x - pos_begin.x) * x_i, pos_begin.y + (pos_end.y - pos_begin.y) * y_i);
}


[[maybe_unused]] static Vector2 get_square_rounded_position(real_t rad){
    auto rad_90 = fmod(rad + pi_4, pi_2) - pi_4;
    auto distance = (squ_len / 2) / cos(rad_90);
    return pos_center + Vector2{-distance, 0}.rotated(rad);
}


struct GrabSysConfig{
    Scara::Config scara_config;
    ZAxis::Config zaxis_config;
    GrabModule::Config grab_config;
};

auto create_default_config(){
    return GrabSysConfig{
        .scara_config = {
            .solver_config = {
                .should_length_meter = real_t(0.072),
                .forearm_length_meter = real_t(0.225),
                .upperarm_length_meter = real_t(0.185),
            },
            
            .joint_config = {
                .max_rad_delta = real_t(0.02),
                // .left_radian_clamp = {0,0},
                // .right_radian_clamp = {0,0},
                .left_basis_radian = real_t(-PI/2 + 0.154),
                .right_basis_radian = real_t(PI/2 - 0.3),
            },
            .claw_config = {
                .press_radian = real_t(PI/2),
                .release_radian = 0
            },
            .nozzle_config = {
                
            },
        },
        
        .zaxis_config = {
            .max_height = 0.25_r,
            .tray_height = 0.2_r,
            .free_height = 0.15_r,
            .ground_height = 0.12_r,
            

            //1m / (3cm * PI)
            .meter_to_turns_scaler = real_t(1 / (0.03 * PI)),
            
            //1mm tolerance
            .reached_threshold = real_t(0.001),  
        },

        .grab_config = {
            .tray_pos = {
                Vector2{-0.12_r   , 0.20_r},
                Vector2{0       , 0.20_r},
                Vector2{0.12_r    , 0.20_r}
            }
        }

    };
}
    
void host_main(){
    uart2.init(576000);
    auto & logger = uart2;

    auto i2c = I2cSw{portD[2], portC[12]};
    i2c.init(1250_KHz);
    // i2c.init(0);//ok
    
    PCA9685 pca{i2c};
    pca.init();
    
    pca.setFrequency(servo_freq, real_t(1.09));


    MG995 servo_left{pca[0]};
    MG995 servo_right{pca[1]};

    SG90 claw_servo{pca[2]};
    RemoteFOCMotor z_motor{logger, can1, 1};
    

    auto config = create_default_config();
    
    JointLR joint_left{
        config.scara_config.joint_config,
        servo_left
    };

    JointLR joint_right{
        config.scara_config.joint_config,
        servo_right
    };

    ZAxisStepper zaxis{
        config.zaxis_config,
        z_motor
    };
    
    Claw claw{
        config.scara_config.claw_config,
        claw_servo
    };
    
    Nozzle nozzle{
        config.scara_config.nozzle_config, 
        GpioNull, GpioNull
    };

    Scara scara{
        config.scara_config, {
            joint_left,
            joint_right,
            claw,
            nozzle
        }
    };
    
    GrabModule grab_module{
        config.grab_config, {
            zaxis,
            scara
        }
    };


    
    auto & timer = timer2;
    timer.init(servo_freq);

    timer.bindCb(TimerUtils::IT::Update, [&](){
        joint_left.tick();
        joint_right.tick();
    });
    
    timer.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});

    CrossSolver::Config cross_config = {
        .xoffs_length_meter = real_t(0.04),
        .forearm_length_meter = real_t(0.1),
        .upperarm_length_meter = real_t(0.1)
    };
    
    CrossSolver cross_solver{
        cross_config
    };
     
    while(true){

        // auto duty = real_t(0.5) + (sin(t) >> 1);
        // duty = LERP(real_t(0.025), real_t(0.125), duty);
        // duty = (real_t(0.025) + real_t(0.125))/2;
        // duty *= real_t(0.2);
        // pwm = duty;
        // pwm2 = duty;

        // joint_left.setRadian(real_t(PI/2) * (1 + sin(t)));
        // grab_module.moveXY(Vector2(real_t(0.12), 0).rotated(t/2) + Vector2(0, real_t(0.24)));
        // grab_module.moveXY(Vector2(real_t(0.12), 0).rotated(t) + Vector2(0, real_t(0.24)));
        // grab_module.moveXY(Vector2(real_t(0.12) * sign(sin(t)), 0) + Vector2(0, real_t(0.24)));
        // grab_module.moveXY(Vector2(0, real_t(0.24)));
        // grab_module.goHome();

        // #ifdef 
        scara.moveXY(Vector2(real_t(0.09), 0).rotated(t) + Vector2(0, real_t(0.24)));
        // scara.goHome();
        {
            bool c = (millis() % 2000 > 1000);
            c ? scara.pickUp() : scara.putDown();
        }
        // logger.println(joint_left.getRadian(), joint_right.getRadian());
        // logger.println(cross_solver.inverse(real_t(0.07) + real_t(0.05) * sin(t)));
        {
            // auto rad = real_t(PI/4) + real_t(PI/4) * sin(t);
            // logger.println(std::setprecision(3), cross_solver.forward(rad));
        }
        
        {
            auto height = real_t(0.14) + real_t(0.06) * sin(t);
            auto inv_rad = cross_solver.inverse(height);
            auto f_height = cross_solver.forward(inv_rad);
            logger.println(std::setprecision(3), inv_rad, height, f_height);
        }
        // pwm = real_t(0.5);
        // logger.println(duty);
        delay(20);
    }
    
    
};


}