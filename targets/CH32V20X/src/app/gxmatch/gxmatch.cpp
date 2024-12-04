#include "misc.hpp"
#include "config.hpp"

namespace gxm{

void host_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD);
    auto & logger = DEBUGGER_INST;

    auto i2c = I2cSw{portD[2], portC[12]};
    i2c.init(400_KHz);
    auto config = create_default_config();
    // spi1.init(9_MHz);
    // spi2.init(144_MHz, CommMethod::Blocking, CommMethod::None);

    can1.init(1_MHz);

    auto displayer{create_displayer()};
    init_displayer(displayer);
    
    auto painter = Painter<RGB565>{};
    painter.bindImage(displayer);



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
        
        pca.setFrequency(50, real_t(1.09));


        MG995 servo_left{pca[0]};
        MG995 servo_right{pca[1]};
        SG90 claw_servo{pca[5]};
        SG90 servo_cross{pca[15]};



        // RemoteFOCMotor z_motor{logger, can1, 1};
        
        // RemoteFOCMotor z_motor{logger, can1, 1};


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
            pca[3]
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



        if(true){//绑定50hz舵机更新回调函数
            auto & timer = timer2;
            timer.init(50);

            timer.bindCb(TimerUtils::IT::Update, tick_50hz);
            timer.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});
            DEBUG_PRINTLN("tick50 binded");
        }




        if(true){//绑定滴答时钟
            bindSystickCb(tick_1khz);
            DEBUG_PRINTLN("tick1k binded");
        }

        // if(true){
        //     // pca.v
        // }

        // auto transz_rad = [](const real_t x) -> real_t{
        //     return real_t(PI - x);
        // }

        // joint_left.setRadian(real_t(PI/2));
        // joint_right.setRadian(real_t(PI/2));
        // while(true);
        // test_joint(joint_left, [](const real_t time)->real_t{
        //     // return real_t(PI);
        //     // return real_t(PI*0.75) + sin(t) * real_t(PI/4);
        //     return real_t(PI*0.75) + sin(t) * real_t(PI*0.25);
        //     // return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
        // });
        
        // test_joint(joint_right, [](const real_t time)->real_t{
        //     // return real_t(PI);
        //     // return real_t(PI*0.75) + sin(t) * real_t(PI/4);
        //     return real_t(PI*0.25) + sin(t) * real_t(PI*0.25);
        //     // return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
        // });

        // SG90 servo_test = {pca[14]};
        // test_servo(servo_test, [](const real_t time)->real_t{
        //     // return real_t(PI);
        //     // auto ret =real_t(PI*0.75) + sin(t) * real_t(PI/4);
        //     auto ret = 0;
        //     DEBUG_PRINTLN(ret);
        //     return ret;
        //     // return real_t(PI*0.25) + sin(t) * real_t(PI*0.25);
        //     // return real_t(PI*0.25) + sin(t) * real_t(PI*0.25);
        //     // return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
        // });

        // test_joint(joint_z, [](const real_t time)->real_t{
        //     // return sin(time) * real_t(0.2) + real_t(0.6);
        //     auto ret = sin(time) * real_t(0.6); 
        //     // auto ret = 0; 
        //     // DEBUG_PRINTLN(ret);
        //     return ret;
        //     // return  0;
        //     // return (sin(time))* real_t(PI/2);
        //     // return 0;
        //     // return real_t(PI);
        //     // return 0.2_r;
        // });
        if(false){
        // if(true){
            // test_joint(joint_z, [](const real_t time)->real_t{
            //     // return sin(time) * real_t(0.2) + real_t(0.6);
            //     return sin(time) * real_t(0.6);
            //     // return (sin(time))* real_t(PI/2);
            //     // return 0;
            //     // return real_t(PI);
            //     // return 0.2_r;
            // });

            // servo_left.setRadian(real_t(PI/2));
            // joint_left.setRadian(real_t(PI/2));
            joint_left.setRadian(real_t(PI/2));
            // joint_right.setRadian(real_t(PI/2));
            // joint_right.setRadian(real_t(PI/2));
            // joint_right.setRadian(real_t(0));
            // servo_right.setRadian(real_t(0));
            // servo_right.setRadian(real_t(PI));

            joint_right.setRadian(real_t(PI/2));
            joint_z.setRadian(0);
            // servo_z.setRadian
            // joint_right.setRadian(
            // servo_right.setRadian(real_t(PI/2));
            // servo_right.setRadian(real_t(PI/2));
            while(true);
            // joint_left.setRadian(real_t(PI/2));
            // joint_left.setRadian(real_t(PI/2));
            joint_left.setRadian(real_t(PI));
            // joint_left.setRadian(0);
            // 
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

        // if(false){//测试升降
        if(true){//测试升降
            scara.press();
            CrossSolver cross_solver{config.zaxis_config.solver_config};
            while(true){
                auto height = LERP(0, 0.16_r, (sin(t) + 1) >> 1);
                // auto height = LERP(0, 0, (sin(t) + 1) >> 1);
                auto inv_rad = cross_solver.inverse(height);
                // auto f_height = cross_solver.forward(inv_rad);
                // DEBUG_PRINTLN(std::setprecision(4), inv_rad, height, f_height);
                                
                joint_z.setRadian(inv_rad);
                delay(20);
            }
        }


        if(false){//测试xyz
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

                // DEBUG_PRINTLN(joint_left.getRadian(), joint_right.getRadian(), joint_z.getRadian());
                joint_z.setRadian(inv_radz);
                // servo_cross.setRadian(real_t(0.4));
                delay(20);
            }
        }

        if(false){//测试机械臂位置反馈
            while(true){
                auto pos = Vector2(0, 0.19_r) + Vector2(0.10_r, 0).rotated(t);
                auto height = LERP(0.12_r, 0.17_r, (sin(t) + 1) >> 1);

                grab_module.rapid(Vector3(pos.x, pos.y, height));

                auto p3 = grab_module.getPos();
                DEBUG_PRINTLN(pos.x, pos.y, height, p3.x, p3.y, p3.z);
                delay(20);
            }
        }

        if(true){//测试

            grab_module.init();
            ;
            // getline(logger);
            size_t i = 0;
            scexpr auto pos_arr = std::to_array<Vector3>({
                {0.02_r, 0.2_r, 0.12_r},
                {-0.04_r, 0.2_r, 0.12_r},
                {-0.09_r, 0.2_r, 0.12_r},
            });

            for(const auto & pos : pos_arr){
                grab_module.move(pos);
                // DEBUG_PRINTLN(pos)
                grab_module.press();
                grab_module.release();
                // grab_module << new DelayAction(1000);
            }
            // while(true){
            //     // DEBUG_PRINTLN(grab_module.pending());
            //     DEBUG_PRINTLN(millis(), grab_module.pending(), grab_module.which());
            //     delay(200);
            // }
            while(true){
                // DEBUG_PRINTLN("next", i);

                // auto pos = pos_arr[i];
                auto pos = Vector3(Vector2(0, 0.17_r) + Vector2(0.03_r, 0).rotated(t), LERP(0.12_r, 0.16_r, (sin(t) + 1) >> 1));
                // DEBUG_PRINTLN("???");
                // DEBUG_PRINTLN(pos.x, pos.y, pos.z);

                grab_module.move(pos);
                // grab_module.moveTo(pos);


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

    if(true){
        auto tick_200hz = [&](){
            // joint_left.tick();
            // joint_right.tick();
            // joint_z.tick();
        };

        auto bind_tick200hz = [](std::function<void(void)> && func){
            auto & timer = timer1;
            timer.init(200);

            timer.bindCb(TimerUtils::IT::Update, std::move(func));
            timer.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});
            DEBUG_PRINTLN("tick200 binded");
        };

        bind_tick200hz(tick_200hz);

        if(true){//测试单个电机
            RemoteFOCMotor stp = {logger, can1, 1};
            stp.reset();
            delay(1000);
            while(true){
                stp.setTargetPosition(sin(t));
            }
        }

        if(true){//测试多个电机
            auto stps = std::array<RemoteFOCMotor, 4>({
                {logger, can1, 1},
                {logger, can1, 2},
                {logger, can1, 3},
                {logger, can1, 4},
            });

            for(auto & stp : stps){
                stp.reset();
            }    

            delay(1000);
            while(true){
                auto p0 = sin(t);
                auto p1 = frac(t);
                auto p2 = real_t(int(t));
                auto p3 = ABS(sin(t));

                stps[0].setTargetPosition(p0);
                stps[1].setTargetPosition(p1);
                stps[2].setTargetPosition(p2);
                stps[3].setTargetPosition(p3);
            }
        }
        
        if(false){

            auto flow_sensor_{create_pmw()};
            init_pmw(flow_sensor_);

            MPU6050 acc_gyr_sensor_{i2c};
            acc_gyr_sensor_.init();

            QMC5883L mag_sensor_{i2c};
            mag_sensor_.init();

            if(false){//测试观测姿态
                ComplementaryFilter::Config rot_config = {
                    .kq = real_t(0.92),
                    .ko = real_t(0.5)
                };
                
                ComplementaryFilter rot_obs = {rot_config};
            
                DEBUG_PRINTLN(std::setprecision(4))
                while(true){

                    acc_gyr_sensor_.update();
                    mag_sensor_.update();

                    const auto gyr3_raw = Vector3{acc_gyr_sensor_.getGyr()};
                    const auto mag3_raw = Vector3{mag_sensor_.getMagnet()};

                    const auto rot_raw = -atan2(mag3_raw.y, mag3_raw.x);
                    const auto gyr_raw = gyr3_raw.z;

                    const auto rot = rot_obs.update(rot_raw, gyr_raw, t);

                    flow_sensor_.update(rot);

                    const auto [x,y] = flow_sensor_.getPosition();

                    DEBUG_PRINTLN(rot, gyr_raw, x, y);
                    delay(5);
                }
            }

            if(false){//测试状态观测器
                Estimator est_ = {
                    config.est_config,
                    acc_gyr_sensor_,
                    mag_sensor_,
                    flow_sensor_
                };

                est_.init();

                while(true){
                    est_.update(t);
                    
                    DEBUG_PRINTLN(est_.pos());
                }
            }
        }

        if(true){//测试单个轮子
            // Wheel wheel = stp
        }

        if(true){//测试多个轮子
            // Wheel
        }
    }

};


}