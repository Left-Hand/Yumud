#include "misc.hpp"
#include "config.hpp"
#include "async/CanMaster.hpp"

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
    auto & can = can1;

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

        Nozzle nozzle{
            config.scara_config.nozzle_config, 
            pca[14]
        };

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

        bind_tick_50hz(tick_50hz);
        bind_tick_1khz(tick_1khz);
        // if(true){
        //     // pca.v
        // }

        // joint_left.setRadian(real_t(PI/2));
        // joint_right.setRadian(real_t(PI/2));
        // while(true);

        // test_servo(servo_cross, [](const real_t time) -> real_t{
        //     // return real_t(PI);
        //     return real_t(1);
        // });
        // test_joint(joint_z, [](const real_t time) -> real_t{
        //     return  0;
        // });

        // joint_left.setRadian(real_t(PI));
        // joint_right.setRadian(real_t(0));
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

        if(false){//测试升降
        // if(true){//测试升降
            scara.press();
            CrossSolver cross_solver{config.zaxis_config.solver_config};
            while(true){


                // auto height = real_t(0.02);//catch_height
                // auto height = real_t(0.163);//free_height
                auto height = real_t(0.14);//tray_height


                // auto height = LERP(0, 0, (sin(t) + 1) >> 1);
                auto inv_rad = cross_solver.inverse(height);
                // auto f_height = cross_solver.forward(inv_rad);
                // DEBUG_PRINTLN(std::setprecision(4), inv_rad, height, f_height);
                                
                joint_z.setRadian(inv_rad);
                delay(20);
            }
        }


        if(false){//测试xyz
        // if(true){//测试xyz
            Scara5Solver solver{config.scara_config.solver_config};
            CrossSolver cross_solver{config.zaxis_config.solver_config};
            while(true){
                // auto pos = Vector2(-0.007_r, 0.144_r);//center
                auto pos = Vector2(-0.101_r, 0.144_r);//left
                // auto pos = Vector2(0.085_r, 0.144_r);//right
                // auto pos = Vector2(0.0_r, 0.245_r);//inspect
                // auto pos = Vector2(0.0_r, 0.265_r);//catch

                auto inv_rad = solver.inverse(pos);

                auto [rad_left, rad_right] = inv_rad;
                joint_left.setRadian(rad_left);
                joint_right.setRadian(rad_right);

                auto height = LERP(0.14_r, 0.14_r, (sin(t) + 1) >> 1);
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

        if(true){//测试抓取动作
        // if(false){//测试抓取动作

            grab_module.init();
            // getline(logger);
            // size_t i = 0;
            // scexpr auto pos_arr = std::to_array<Vector3>({
            //     {0.02_r, 0.2_r, 0.12_r},
            //     {-0.04_r, 0.2_r, 0.12_r},
            //     {-0.09_r, 0.2_r, 0.12_r},
            // });

            // for(const auto & pos : pos_arr){
            //     grab_module.move(pos);
            //     grab_module.press();
            //     grab_module.release();
            // }

            grab_module.inspect();
            // grab_module.take(TrayIndex::Left);
            // for(size_t i = 0; i < 10; i++){
            //     grab_module.press();
            //     grab_module << new DebugAction("pressed");
            //     grab_module.release();
            //     grab_module << new DebugAction("released");
            // }
            // grab_module.take(TrayIndex::Center);
            // grab_module.give(TrayIndex::Center);
            // grab_module.take(TrayIndex::Left);
            grab_module.give(TrayIndex::Left);
            // grab_module.take(TrayIndex::Right);
            grab_module.give(TrayIndex::Right);
            // while(true){
            //     // DEBUG_PRINTLN(grab_module.pending());
            //     DEBUG_PRINTLN(millis(), grab_module.pending(), grab_module.which());
            //     delay(200);
            // }
            while(true);
        }




        if(true){

            auto flow_sensor_{create_pmw()};
            init_pmw(flow_sensor_);

            MPU6050 acc_gyr_sensor_{i2c};
            acc_gyr_sensor_.init();

            QMC5883L mag_sensor_{i2c};
            mag_sensor_.init();

            auto & wheel_config = config.wheels_config.wheel_config;

            // if(true){//测试单个电机
            if(false){//测试单个电机
                RemoteFOCMotor stp = {logger, can1, 1};
                stp.reset();
                delay(1000);
                while(true){
                    auto targ = sin(t);
                    stp.setTargetPosition(targ);
                    delay(5);
                    DEBUG_PRINTLN(targ);
                }

                if(true){//测试单个轮子
                    Wheel wheel = {wheel_config, stp};
                    wheel.setPosition(0.2_r * sin(t));
                    delay(5);
                }
            }

            CanMaster can_master = {can};

            auto stps = std::array<RemoteFOCMotor, 4>({
                {logger, can, 1},
                {logger, can, 2},
                {logger, can, 3},
                {logger, can, 4},
            });

            auto init_steppers = [&](){
                for(auto & stp_ : stps){
                    stp_.reset();
                    stp_.locateRelatively(0);
                }    
            };

            init_steppers();
            can_master.registerNodes(stps.begin(), stps.end());

            Wheels wheels = {
                config.wheels_config,
                {
                    stps[0],
                    stps[1],
                    stps[2],
                    stps[3]
                }
            };

            wheels[0].inverse();
            wheels[2].inverse();


            if(true){//测试多个轮子
                auto tick_200hz = [&](){
                    can_master.update();
                };

                bind_tick_200hz(tick_200hz);

                Mecanum4Solver solver = {config.chassis_config.solver_config};
                while(true){
                    // scexpr real_t delta = {0.003_r};
                    // auto delta = solver.inverse(Vector2{0, 0.005_r}, 0);
                    // auto delta = solver.inverse(Vector2{0, 0}, 0.005_r);
                    // auto delta = solver.inverse(Vector2{0.003_r, 0.003_r}, 0);
                    // auto delta = solver.inverse(Vector2{-0.003_r, 0.00_r}, 0);
                    auto delta = solver.inverse(Vector2{real_t(1.0/200) * sin(t), 0.00_r}, 0);
                    wheels.forward(delta);

                    delay(5);
                }
            }
            // if(false){//测试观测姿态
            if(true){//测试观测姿态
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

            auto & est_config = config.chassis_config.est_config;

            Estimator est_ = {
                est_config,
                acc_gyr_sensor_,
                mag_sensor_,
                flow_sensor_
            };

            if(false){//测试状态观测器

                est_.init();

                while(true){
                    est_.update(t);
                    
                    DEBUG_PRINTLN(est_.pos());
                }
            }

            if(true){
                ChassisModule chassis_module {
                    config.chassis_config, 
                    wheels, est_};
                }
        }   
    }

};


}