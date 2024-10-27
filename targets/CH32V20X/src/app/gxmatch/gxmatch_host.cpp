#include "gxmatch.hpp"
#include "host/host.hpp"

#include "machine/scara/scara.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/gpio/port_virtual.hpp"


using Sys::t;

namespace gxm{


auto create_default_config(){
    return GrabModule::Config{
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
                .right_basis_radian = real_t(PI/2 - 0.15),
            },
            .claw_config = {
                
            },
            .nozzle_config = {
                
            },
        },
        
        .zaxis_config = {
            .tray_height_mm = 0,
            .free_height_mm = 0,
            .ground_height_mm = 0,
        },
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

    using MG995 = PwmRadianServo;
    
    MG995 servo_left{pca[0]};
    MG995 servo_right{pca[1]};

    auto config = create_default_config();
    
    JointLR joint_left{
        config.scara_config.joint_config,
        servo_left
    };

    JointLR joint_right{
        config.scara_config.joint_config,
        servo_right
    };

    ZAxis zaxis = {
        config.zaxis_config
    };
    
    Claw claw = {
        config.scara_config.claw_config
    };
    
    Nozzle nozzle = {config.scara_config.nozzle_config, GpioNull, GpioNull};

    Scara scara{
        config.scara_config, {
            joint_left,
            joint_right,
            claw,
            nozzle
        }
    };
    
    GrabModule grab_module{
        config, {
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
        grab_module.moveXY(Vector2(real_t(0.02), 0).rotated(t) + Vector2(0, real_t(0.24)));
        // grab_module.moveXY(Vector2(0, real_t(0.24)));
        // grab_module.goHome();
        logger.println(joint_left.getRadian(), joint_right.getRadian());
        // pwm = real_t(0.5);
        // logger.println(duty);
        delay(20);
    }
    
    
};


}