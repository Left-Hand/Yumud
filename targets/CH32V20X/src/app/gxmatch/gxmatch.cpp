#include "misc.hpp"

namespace gxm{

auto create_default_config(){
    return SysConfig{
        .joint_config = {
            .max_rad_delta = real_t(0.07),
            .left_basis_radian = real_t(-PI/2 + 0.18),
            // .right_basis_radian = real_t(PI/2 - 0.10),
            // .right_basis_radian = real_t(PI/2 - 0.20),
            // .right_basis_radian = real_t(PI/2 + 0.3),
            .right_basis_radian = real_t(PI/2),
            // .right_basis_radian = real_t(0),
            .z_basis_radian = real_t(PI/2),
        },

        .scara_config = {
            .solver_config = {
                .should_length_meter = real_t(0.072),
                .forearm_length_meter = real_t(0.225),
                .upperarm_length_meter = real_t(0.185),
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

            .inspect_xy = Vector2{0, 0.12_r},

            .blocked_area = AABB{
                Vector3{-0.05_r, -0.05_r, 0.05_r},
                Vector3{0.05_r, 0.05_r, 0.05_r}
            },

            .max_spd = 0.2_r,
            .max_acc = 0.2_r,

            .nozzle_sustain = 500
        }
    };
}
    
void host_main(){
    uart2.init(576000);
    auto & logger = uart2;

    auto i2c = I2cSw{portD[2], portC[12]};
    i2c.init(400_KHz);
    
    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];


    #ifdef CH32V30X
    auto & spi = spi2;
    #else
    auto & spi = spi1;
    #endif
    
    spi.bindCsPin(lcd_cs, 0);
    spi.init(144_MHz, CommMethod::Blocking, CommMethod::None);

    ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 135});

    initDisplayer(tftDisplayer);
    
    auto painter = Painter<RGB565>{};
    painter.bindImage(tftDisplayer);



    if(false){
        auto solver = TrapezoidSolver_t<real_t>{4,10.0_r,0.1_r};
        scexpr auto delta = 0.001_r;
        
        DEBUG_PRINTLN(std::setprecision(4));
        for(real_t x = 0; x <= solver.period(); x += delta){
            auto y = solver.forward(x);
            delayMicroseconds(500);
            DEBUG_PRINTLN(x, y);
        } 
        PANIC();
    }
    

    if(false){
        MPU6050 acc_gyr_sensor{i2c};
        QMC5883L mag_sensor{i2c};

        acc_gyr_sensor.init();
        mag_sensor.init();

        ComplementaryFilter::Config rot_config = {
            .kq = real_t(0.92),
            .ko = real_t(0.2)
        };
        
        ComplementaryFilter rot_obs = {rot_config};
    
        DEBUG_PRINTLN(std::setprecision(4))
        while(true){

            acc_gyr_sensor.update();
            mag_sensor.update();

            // const auto acc3 = Vector3{acc_gyr_sensor.getAcc()};
            const auto gyr3 = Vector3{acc_gyr_sensor.getGyr()};
            const auto mag3 = Vector3{mag_sensor.getMagnet()};

            // const auto acc2 = Vector2{acc3.x, acc3.y};

            const auto rot = -atan2(mag3.y, mag3.x);
            const auto gyr = gyr3.z;

            auto rot_ = rot_obs.update(rot, gyr, t);
            DEBUG_PRINTLN(rot, gyr, rot_);
            delay(5);
        }
    }

    if(false){
        using Type = Vector2;
        using Topic = Topic_t<Type>;

        
        Topic topic;

        // 创建一个 Publisher
        auto publisher = topic.createPublisher();

        // 创建多个 Subscriber
        auto subscriber1 = topic.createSubscriber([](const Type & message) {
            DEBUGGER << "Subscriber 1 received: " << message << "\r\n";
        });

        auto subscriber2 = topic.createSubscriber([](const Type & message) {
            DEBUGGER << "Subscriber 2 received: " << message << "\r\n";
        });


        // 发布消息
        publisher.publish({0,0});
        publisher.publish({1,1});
    }



    if(false){
        auto limits = SequenceLimits{
            .max_gyr = 2,
            .max_agr = real_t(0.5),
            .max_spd = real_t(0.5),
            .max_acc = real_t(0.2)
        };

        auto params = SequenceParas{
            .freq = 200
        };
        
        auto sequencer = Sequencer(limits, params);
        auto curve = Curve{};

        Map map{};
        Planner::Config planner_config = {.duration = real_t(3.2)};
        Planner planner{planner_config, map, sequencer};

        const auto m = micros();

        planner.plan(
            curve,
            Field{FieldType::Garbage},
            Field{FieldType::Staging}
        );

        planner.plan(
            curve,
            Field{FieldType::Staging},
            Field{FieldType::RoughProcess}
        );

        planner.plan(
            curve,
            Field{FieldType::RoughProcess},
            Field{FieldType::Garbage}
        );

        DEBUG_PRINTLN(micros() - m, curve.size());

        draw_curve(painter, curve);

        DEBUG_PRINTLN(std::setprecision(4));
        auto idx = 0;
        for(auto it = curve.begin(); it != curve.end(); ++it){
            // while(millis() < size_t((idx * 5) + 1000));
            delay(1);
            idx++;
            draw_turtle(painter, Ray(*it));
        }
        DEBUG_PRINTLN("done");
        while(true);
    }


    if(true){
        
        PCA9685 pca{i2c};
        pca.init();
        
        pca.setFrequency(servo_freq, real_t(1.09));


        MG995 servo_left{pca[0]};
        MG995 servo_right{pca[15]};

        SG90 claw_servo{pca[2]};

        SG90 servo_cross{pca[3]};


        // RemoteFOCMotor z_motor{logger, can1, 1};
        

        auto config = create_default_config();
        auto & joint_config = config.joint_config;
        JointLR joint_left{
            joint_config.max_rad_delta,
            joint_config.left_basis_radian,
            servo_left
        };

        JointLR joint_right{
            joint_config.max_rad_delta,
            joint_config.right_basis_radian,
            servo_right
        };

        JointLR joint_z{
            joint_config.max_rad_delta,
            joint_config.z_basis_radian,
            servo_cross
        };

        joint_z.inverse();

        ZAxisCross zaxis{
            config.zaxis_config,
            joint_z
        };
        
        Claw claw{
            config.scara_config.claw_config,
            claw_servo
        };
        
        Nozzle nozzle{
            config.scara_config.nozzle_config, 
            pca[4]
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
        
        auto tick_1khz = [&](){
            grab_module.tick();
        };
        auto tick_50hz = [&](){
            joint_left.tick();
            joint_right.tick();
            joint_z.tick();
        };

        if(true){//绑定舵机更新回调函数
            auto & timer = timer2;
            timer.init(servo_freq);

            timer.bindCb(TimerUtils::IT::Update, tick_50hz);
            timer.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});
            DEBUG_PRINTLN("tick50 binded");
        }


        if(true){//绑定滴答时钟
            bindSystickCb(tick_1khz);
            DEBUG_PRINTLN("tick1k binded");
        }

        // auto transz_rad = [](const real_t x) -> real_t{
        //     return real_t(PI - x);
        // }

        // if(false){
        if(false){
            // test_joint(joint_z, [](const real_t time)->real_t{
            //     // return sin(time) * real_t(0.2) + real_t(0.6);
            //     return sin(time) * real_t(0.6);
            //     // return (sin(time))* real_t(PI/2);
            //     // return 0;
            //     // return real_t(PI);
            //     // return 0.2_r;
            // });
            joint_z.setRadian(0.4_r);
            // joint_left.setRadian(real_t(PI/2));
            // joint_left.setRadian(real_t(PI/2));
            joint_left.setRadian(real_t(PI));
            // joint_left.setRadian(0);
            // joint_right.setRadian(real_t(PI));
            // test_joint(joint_left, [](const real_t time)->real_t{
            //     // return real_t(PI);
            //     // return real_t(PI/2);
            //     return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
            // });


            // test_servo(servo_right, [](const real_t time)->real_t{
            //     // return real_t(PI);
            //     // return real_t(PI*0.75) + sin(t) * real_t(PI/4);
            //     return real_t(PI*0.25) + sin(t) * real_t(PI*0.25);
            //     // return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
            // });

            test_joint(joint_right, [](const real_t time)->real_t{
                // return real_t(PI);
                // return real_t(PI*0.75) + sin(t) * real_t(PI/4);
                return real_t(PI*0.25) + sin(t) * real_t(PI*0.25);
                // return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
            });

            // while(true);
        }

        if(false){//测试升降
            CrossSolver cross_solver{config.zaxis_config.solver_config};
            while(true){
                auto height = LERP(0, 0.17_r, (sin(t) + 1) >> 1);
                auto inv_rad = cross_solver.inverse(height);
                // auto f_height = cross_solver.forward(inv_rad);
                // DEBUG_PRINTLN(std::setprecision(4), inv_rad, height, f_height);
                                
                joint_z.setRadian(inv_rad);
                delay(20);
            }
        }


        if(true){//测试xyz
            Scara5Solver solver{config.scara_config.solver_config};
            CrossSolver cross_solver{config.zaxis_config.solver_config};
            while(true){
                auto pos = Vector2(0, 0.19_r) + Vector2(0.10_r, 0).rotated(t);
                // auto pos = Vector2(0, 0.19_r) + Vector2(0.0_r, 0).rotated(t);
                auto inv_rad = solver.inverse(pos);

                auto [rad_left, rad_right] = inv_rad;
                joint_left.setRadian(rad_left);
                joint_right.setRadian(rad_right);

                auto height = LERP(0.12_r, 0.17_r, (sin(t) + 1) >> 1);
                auto inv_radz = cross_solver.inverse(height);

                DEBUG_PRINTLN(joint_left.getRadian(), joint_right.getRadian(), joint_z.getRadian());
                joint_z.setRadian(inv_radz);
                delay(20);
            }
        }

        if(false){//测试机械臂位置反馈
            while(true){
                auto pos = Vector2(0, 0.19_r) + Vector2(0.10_r, 0).rotated(t);
                auto height = LERP(0.12_r, 0.17_r, (sin(t) + 1) >> 1);

                grab_module.moveTo(Vector3(pos.x, pos.y, height));

                auto p3 = grab_module.getPos();
                DEBUG_PRINTLN(pos.x, pos.y, height, p3.x, p3.y, p3.z);
                delay(20);
            }
        }

        if(true){//测试

            grab_module.init();
            getline(logger);
            size_t i = 0;
            scexpr auto pos_arr = std::to_array<Vector3>({
                {0.02_r, 0.12_r, 0.17_r},
                {-0.04_r, 0.12_r, 0.17_r},
                // {0, 0.2_r, 0.12_r},
            });

            while(true){
                // DEBUG_PRINTLN("next", i);

                // auto pos = pos_arr[i];
                auto pos = Vector3(Vector2(0, 0.19_r) + Vector2(0.10_r, 0).rotated(t), LERP(0.12_r, 0.17_r, (sin(t) + 1) >> 1));

                // grab_module.move(pos);
                grab_module.moveTo(pos);


                getline(logger);
                
                i = (i + 1)%pos_arr.size();
            }
        }


        if(false){//测试动作组
            while(true){
                grab_module.test();
                // grab_module.take();
                DEBUG_PRINTLN("next");
                while(true){
                    if(logger.available()){
                        logger.readString();
                        break;
                    }
                }
            }
        }





        if(true){
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
                    c ? scara.press() : scara.release();
                }
                // logger.println(joint_left.getRadian(), joint_right.getRadian());
                // logger.println(cross_solver.inverse(real_t(0.07) + real_t(0.05) * sin(t)));
                {
                    // auto rad = real_t(PI/4) + real_t(PI/4) * sin(t);
                    // logger.println(std::setprecision(3), cross_solver.forward(rad));
                }

                delay(20);
            }
        }
    }
    
};


}