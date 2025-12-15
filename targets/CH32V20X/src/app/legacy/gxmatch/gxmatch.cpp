#include "misc.hpp"
#include "config.hpp"
#include "async/CanMaster.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"

#include "drivers/Camera/MT9V034/mt9v034.hpp"
#include "middlewares/nvcv2/geometry/geometry.hpp"
#include "middlewares/nvcv2/shape/shape.hpp"
#include "middlewares/nvcv2/pixels/pixels.hpp"

#include "machine/chassis_actions.hpp"
#include "common/enum.hpp"
#include "core/system.hpp"


using namespace nvcv2;
using namespace gxm;

#define MY_OS_PRINTLN(...)

class VisionModule:public AsciiProtocolIntf{
protected:
    // UartHw & uart_;
    std::optional<MaterialColor> color_;
    std::optional<Vec2> offset_;

    enum class Mode:uint8_t{
        CLOSED,
        COLOR,
        OFFSET
    };

    Mode mode_ = Mode::CLOSED;
public: 
    VisionModule(UartHw & uart):
        AsciiProtocolIntf(uart){;}

    void parseArgs(const StringViews args) override{
        // DEBUG_PRINTLN(args);
        switch(args[0].hash()){
            case "color"_ha:
                // DEBUG_PRINTLN("c is", uint8_t(args[1][0]));
                switch(args[1][0]){
                    case 'n':
                        // color_ = MaterialColor::None;
                        color_ = std::nullopt;
                        break;
                    case 'r':
                        color_ = MaterialColor::Red;
                        break;
                    case 'g':
                        color_ = MaterialColor::Green;
                        break;
                    case 'b':
                        // DEBUG_PRINTLN("csdbjcnsadjk")
                        color_ = MaterialColor::Blue;
                        
                        break;
                }
            break;

            case "offset"_ha:
                if(args.size() == 3){
                    offset_ = Vec2{
                        real_t(args[1]),
                        real_t(args[2])
                    };
                }else{
                    offset_ = std::nullopt;
                }
        }
    }

    void close(){
        MY_OS_PRINTLN("close");
        mode_ = Mode::CLOSED;
    }

    auto color(){
        if(mode_ != Mode::COLOR){
            MY_OS_PRINTLN("color"); 
            color_ = std::nullopt;
            mode_ = Mode::COLOR;
        }
        return color_.value_or(MaterialColor::None);
    }

    bool has_color(){
        return color_.has_value();
    }

    bool has_offset(){
        return offset_.has_value();
    }
    auto offset(){
        if(mode_ != Mode::OFFSET){
            MY_OS_PRINTLN("offset");
            offset_ = std::nullopt;
            mode_ = Mode::OFFSET;
        }
        return offset_;
    }


};

// class Ascii

namespace gxm{

void host_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD);
    DEBUGGER_INST.init({576000});
    auto & logger = DEBUGGER_INST;

    DEBUG_PRINTLN(std::setprecision(4), "poweron");

    auto i2c = I2cSw{hal::PD<2>(), hal::PC<12>()};
    i2c.init(3400_KHz);
    auto config = create_default_config();

    auto & can = can1;

    can.init(1_MHz);
    can[0].all();
        

    auto displayer{create_displayer()};
    init_displayer(displayer);
    
    auto painter = Painter<RGB565>{};
    painter.bindImage(displayer);

    //#region 测试机械臂
    // if(true){
    if(false){
        
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
                return real_t(PI);
                // return real_t(PI*0.75) + sin(t) * real_t(PI/4);
                // return real_t(PI*0.25) + real_t(PI*0.25) * sin(time());

                // return LERP(0, real_t(PI/2), (sin(t) + 1) >> 1);
            });

            // while(true);
        }

        //#region 测试升降
        if(false){
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
                clock::delay(20ms);
            }
        }
        //#endregion

        //#region 测试xyz
        if(false){
        // if(true){//测试xyz
            Scara5Solver solver{config.scara_config.solver_config};
            CrossSolver cross_solver{config.zaxis_config.solver_config};
            while(true){
                // auto pos = Vec2(-0.007_r, 0.144_r);//center
                auto pos = Vec2(-0.101_r, 0.144_r);//left
                // auto pos = Vec2(0.085_r, 0.144_r);//right
                // auto pos = Vec2(0.0_r, 0.245_r);//inspect
                // auto pos = Vec2(0.0_r, 0.265_r);//catch

                auto inv_rad = solver.inverse(pos);

                auto [rad_left, rad_right] = inv_rad;
                joint_left.setRadian(rad_left);
                joint_right.setRadian(rad_right);

                auto height = LERP(0.14_r, 0.14_r, (sin(time()) + 1) >> 1);
                auto inv_radz = cross_solver.inverse(height);

                // DEBUG_PRINTLN(joint_left.getRadian(), joint_right.getRadian(), joint_z.getRadian());
                joint_z.setRadian(inv_radz);
                // servo_cross.setRadian(real_t(0.4));
                clock::delay(20ms);
            }
        }
        //#endregion

        //#region 测试机械臂位置反馈
        if(false){
            while(true){
                auto pos = Vec2(0, 0.19_r) + Vec2(0.10_r, 0).rotated(time());
                auto height = LERP(0.12_r, 0.17_r, (sin(time()) + 1) >> 1);

                grab_module.rapid(Vec3(pos.x, pos.y, height));

                auto p3 = grab_module.getPos();
                DEBUG_PRINTLN(pos.x, pos.y, height, p3.x, p3.y, p3.z);
                clock::delay(20ms);
            }
        }
        //#endregion

        //#region 测试抓取动作
        if(true){
        // if(false){

            grab_module.init();
            // getline(logger);
            // size_t i = 0;
            // static constexpr auto pos_arr = std::to_array<Vec3>({
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
            grab_module.take(TrayIndex::Center);
            grab_module.give(TrayIndex::Center);
            grab_module.take(TrayIndex::Right);
            grab_module.give(TrayIndex::Right);
            grab_module.take(TrayIndex::Left);
            // while(true){
            //     // DEBUG_PRINTLN(grab_module.pending());
            //     DEBUG_PRINTLN(millis(), grab_module.pending(), grab_module.which());
            //     clock::delay(200ms);
            // }
            while(true);
        }
        //#endregion 

    }

    //#endregion

    //#region 底盘
    if(false){
    // if(false){
        bind_tick_1khz(nullptr);
        auto flow_sensor_{create_pmw()};
        init_pmw(flow_sensor_);

        MPU6050 acc_gyr_sensor_{i2c};
        acc_gyr_sensor_.init();

        QMC5883L mag_sensor_{i2c};
        mag_sensor_.init();

        
        auto & wheel_config = config.wheels_config.wheel_config;

        CanMaster can_master = {can};
        // can.sync(false);
        auto stps = std::array<RemoteFOCMotor, 4>({
            {logger, can, 1},
            {logger, can, 2},
            {logger, can, 3},
            {logger, can, 5},
        });

        auto init_steppers = [&](){
            for(auto & stp_ : stps){
                stp_.reset();
                stp_.locateRelatively(0);
            }    
        };

        init_steppers();
        Wheels wheels = {
            config.wheels_config,
            {
                stps[0],
                stps[1],
                stps[2],
                stps[3]
            },
            can
        };

        wheels[0].inverse();
        wheels[2].inverse();


        //#region 测试整个底盘
        if(true){
            ChassisModule chassis_module {
                config.chassis_config, 
                wheels, 
                acc_gyr_sensor_, 
                mag_sensor_};

            bind_tick_800hz([&]{
                chassis_module.tick800();
            });

            bind_tick_1khz([&]{
                chassis_module.tick();
                
            });

            using namespace ChassisActions;

            Map map{};

            Planner planner{chassis_module, map};

            planner.wait(1500);

            planner.plan(
                Field{FieldType::Garbage},
                Field{FieldType::RoughProcess}
            );

            planner.wait(1500);

            planner.plan(
                Field{FieldType::RoughProcess},
                Field{FieldType::Staging}
            );

            planner.wait(1500);

            planner.plan(
                Field{FieldType::Staging},
                Field{FieldType::Garbage}
            );



            // chassis_module << new DelayAction(2000);
            // chassis_module << new StrictSpinAction(chassis_module, real_t(PI/2));
            // chassis_module << new DelayAction(1000);
            // chassis_module << new StrictSpinAction(chassis_module, real_t(-PI/2));
            // chassis_module << new DelayAction(1000);

            // chassis_module << new DelayAction(1000);

            // chassis_module << new StraightAction(chassis_module, 1.72_r);
            // chassis_module << new SpinAction(chassis_module, real_t(PI/2));
            // chassis_module << new StraightAction(chassis_module, 1.72_r);

            // chassis_module << new SpinAction(chassis_module, real_t(PI/2));
            // chassis_module << new SpinAction(chassis_module, real_t(-PI/2));

            // chassis_module << new StraightAction(chassis_module, -1.72_r);
            // chassis_module << new SpinAction(chassis_module, real_t(-PI/2));
            // chassis_module << new StraightAction(chassis_module, -1.72_r);
            // chassis_module << new SpinAction(chassis_module, real_t(-PI/2));
            // chassis_module << new StraightAction(chassis_module, -1.72_r);

            // chassis_module << new DelayAction(2000);
            // chassis_module << new SpinAction(chassis_module, real_t(PI/2));
            // chassis_module << new SpinAction(chassis_module, real_t(PI/2));

            // chassis_module << new SpinAction(chassis_module, real_t(PI/2));
            // chassis_module << new SpinAction(chassis_module, real_t(-PI/2));

            // chassis_module << new SpinAction(chassis_module, real_t(-PI/2));
            // chassis_module << new SpinAction(chassis_module, real_t(-PI/2));

            while(true){
                // DEBUG_PRINTLN(chassis_module.rot(), chassis_module.gyr());
                clock::delay(5ms);

                // {
                //     clock::delay(2000ms);

                //     static bool fwd = false;
                //     fwd = !fwd;
                    
                // }
                // chassis_module.setCurrent({{0, 0.5_r * sin(3 * t)}, 0});
                // chassis_module.setCurrent({{0.8_r * sin(3 * t), 0}, 0});
                auto ray = chassis_module.jny();
                auto [org, rad] = ray;
                auto [x,y] = org;

                // DEBUG_PRINTLN(std::setprecision(4))
                DEBUG_PRINTLN(x,y);

                // chassis_module.setPosition({0,0,sin(t) * real_t(PI/2)});
                // , x,y,rad);
                // chassis_module.setCurrent({{0,0}, 0.2_r});
                // DEBUG_PRINTLN("???");
            }

        }
        //#endregion

        Mecanum4Solver solver = {config.chassis_config.solver_config};
        //#region 测试位置反馈
        auto test_pos_ret = [&](){


            while(true){
                // DEBUG_PRINTLN(
                //     wheels[0].getPosition(),
                //     wheels[1].getPosition(),
                //     wheels[2].getPosition(),
                //     wheels[3].getPosition()
                // )
                // DEBUG_PRINTLN(est_.rot());
                DEBUG_PRINTLN(mag_sensor_.read_mag());
                clock::delay(5ms);
            }
        };

        if(true){
        // if(false){
            test_pos_ret();
        }

        //#endregion


        //#region 测试观测姿态
        // if(false){
        auto test_gest_obs = [&](){//测试观测姿态
            ComplementaryFilter::Config rot_config = {
                .kq = real_t(0.92),
                .ko = real_t(0.5)
            };
            
            ComplementaryFilter rot_obs = {rot_config};
        
            DEBUG_PRINTLN(std::setprecision(4));
            while(true){

                acc_gyr_sensor_.update();
                mag_sensor_.update();

                const auto gyr3_raw = Vec3{acc_gyr_sensor_.read_gyr()};
                const auto mag3_raw = Vec3{mag_sensor_.read_mag()};

                const auto rot_raw = -atan2(mag3_raw.y, mag3_raw.x);
                const auto gyr_raw = gyr3_raw.z;

                const auto rot = rot_obs.update(rot_raw, gyr_raw, time());

                flow_sensor_.update(rot);

                // const auto [x,y] = flow_sensor_.getPosition();

                // DEBUG_PRINTLN(rot, gyr_raw, x, y);
                DEBUG_PRINTLN(mag3_raw.x, mag3_raw.y, mag3_raw.z, fposmod(rot_raw, real_t(PI/2)));
                // DEBUG_PRINTLN(gyr_raw);
                clock::delay(5ms);
            }
        };

        if(true){
        // if(false){
            test_gest_obs();
        }

        //#endregion


        //#region 测试单个电机
        auto test_single_motor = [&](){
            RemoteFOCMotor stp = {logger, can1, 1};
            stp.reset();
            clock::delay(1000ms);
            while(true){
                auto targ = sin(time());
                stp.setTargetPosition(targ);
                clock::delay(5ms);
                DEBUG_PRINTLN(targ);
            }

            if(true){//测试单个轮子
                Wheel wheel = {wheel_config, stp};
                wheel.setPosition(0.2_r * sin(time()));
                clock::delay(5ms);
            }
        };

        // if(true){
        if(false){
            test_single_motor();
        }
        //#endregion


        //#region 测试轮组
        if(false){
        // if(true){

            wheels.init();

            // auto delta = solver.inverse(Vec2{0, 0}, 0.7_r*sin(t));

            real_t ang;
            bind_tick_200hz(
                [&](){
                // wheels.request();
                // DEBUG_PRINTLN(can.available());
                // can_master.update();
                acc_gyr_sensor_.update();
        
                ang += Vec3(acc_gyr_sensor_.read_gyr()).z * 0.005_r;
                // auto delta = solver.inverse(Vec2{0.4_r*sin(t), 0}, 0);

                // static constexpr real_t delta = {0.003_r};
                // auto delta = solver.inverse(Vec2{0, 0.0003_r}, 0);
                // auto delta = solver.inverse(Vec2{0, 0.3_r * sin(t)}, 0);
                
                // auto delta = solver.inverse(Vec2{0._r, 0.0_r}, 0);
                // auto delta = solver.inverse(Vec2{0, 0}, 0.005_r);
                // auto delta = solver.inverse(Vec2{0.003_r, 0.003_r}, 0);
                // auto delta = solver.inverse(Vec2{-0.003_r, 0.00_r}, 0);
                // auto delta = solver.inverse(Vec2{real_t(1.0/200) * sin(t), 0.00_r}, 0);
                auto delta = solver.inverse(Vec2{0, 0.00_r}, CLAMP2((1-ang) * real_t(9), 1));
                // DEBUG_PRINTLN(millis());
                // wheels.setCurrent(delta);
                // DEBUG_PRINTLN(std::get<0>(delta));
                // wheels.setSpeed(delta);
                // wheels.setPosition(delta);
                wheels.setCurrent(delta);
            }

            );

            while(true){
                // delta = solver.inverse(Vec2{0, 0}, 1.7_r*sin(t));
                // delta = solver.inverse(Vec2{0, 1.7_r*sin(t)});
                
                // DEBUG_PRINTLN(std::get<0>(delta));
                // DEBUG_PRINTLN(delta);
                // DEBUG_PRINTLN(wheels[0].getPosition(), wheels[1].getPosition(), wheels[2].getPosition(), wheels[3].getPosition());
                DEBUG_PRINTLN(ang);
                clock::delay(5ms);
            }
        }

        // if(false){
        if(true){
            wheels.init();

            real_t py;
            bind_tick_200hz(
                [&](){
                flow_sensor_.update();
        
                py = Vec2(flow_sensor_.getPosition()).y;
                // py = Vec2(flow_sensor_.getPosition()).x;
                
                auto delta = solver.inverse(Vec2{0, CLAMP2((0.2_r-py) * real_t(20), 0.8_r)}, 0);
                // auto delta = solver.inverse(Vec2{0, 1}, 0);
                // DEBUG_PRINTLN(delta);
                wheels.setCurrent(delta);
            });

            while(true){
                // delta = solver.inverse(Vec2{0, 0}, 1.7_r*sin(t));
                // delta = solver.inverse(Vec2{0, 1.7_r*sin(t)});
                
                // DEBUG_PRINTLN(std::get<0>(delta));
                // DEBUG_PRINTLN(delta);
                // DEBUG_PRINTLN(wheels[0].getPosition(), wheels[1].getPosition(), wheels[2].getPosition(), wheels[3].getPosition());
                DEBUG_PRINTLN(std::setprecision(4), py);
                clock::delay(5ms);
            }
        }


        //#endregion



    }//#endregion
    
    //#region 视觉
    // if(true){
    if(false){
        bindSystickCb(nullptr);
        [[maybe_unused]] auto plot_gray = [&](const Image<Gray> & src, const Vec2i & pos){
            const auto area = Rect2i(pos, src.size());
            displayer.putTexture(area, src.get_data());
        };

        [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Vec2i & pos){
            const auto area = Rect2i(pos, src.size());
            displayer.putTexture(area, src.get_data());
        };

        [[maybe_unused]] auto plot_rgb = [&](const Image<RGB565> & src, const Vec2i & pos){
            const auto area = Rect2i(pos, src.size());
            displayer.putTexture(area, src.get_data());
        };

        // [[maybe_unused]] auto plot_roi = [&](const Rect2i & rect){
        //     painter.bindImage(sketch);
        //     painter.setColor(RGB565::CORAL);
        // };

        MT9V034     camera{i2c};
        camera.init();
        camera.setExposureValue(500);


        while(false){
            displayer.fill(ColorEnum::BLACK);
            clock::delay(200ms);
            displayer.fill(ColorEnum::WHITE);
            clock::delay(200ms);
        }

        while(true){
            painter.setColor(ColorEnum::WHITE);

            // Image<Gray> img = Shape::x2(camera);
            Image<Gray> img = camera.clone();
            auto ave = Pixels::average(img);
            DEBUG_PRINTLN(millis(), uint8_t(ave));
            plot_gray(img, {0, 0});
            // auto img = raw_img.space();
            // Geometry::perspective(img, raw_img);
            // plot_gray(img, {0, img.size().y * 1});
            continue;
            auto img_ada = img.space();
            // Shape::adaptive_threshold(img_ada, img);
            // plot_gray(img_ada, {0, img.size().y * 2});

            auto img_processed = img.space<Gray>();
            // Shape::canny(img_bina, img, {60, 120});
            Shape::eye(img_processed, img);
            auto img_processed2 = img.space<Gray>();
            Shape::eye(img_processed2, img_processed);
            // Pixels::binarization(img_bina, img_ada, 220);
            // Pixels::inverse(img_bina);
            plot_gray(img_processed, {0, img.size().y * 2});
            plot_gray(img_processed2, {0, img.size().y * 3});

        }
    }

    //#endregion

    // if(true){
    if(false){
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

        MPU6050 acc_gyr_sensor_{i2c};
        acc_gyr_sensor_.init();

        QMC5883L mag_sensor_{i2c};
        mag_sensor_.init();

        auto stps = std::array<RemoteFOCMotor, 4>({
            {logger, can, 1},
            {logger, can, 2},
            {logger, can, 3},
            {logger, can, 5},
        });

        auto idle = [&](){
            pca[0] = 0.001_r;
            pca[1] = 0.001_r;
            pca[14] = 0.001_r;
            pca[15] = 0.001_r;
            // pca.reset();
        };


        Wheels wheels = {
            config.wheels_config,
            {
                stps[0],
                stps[1],
                stps[2],
                stps[3]
            },
            can
        };

        wheels[0].inverse();
        wheels[2].inverse();


        ChassisModule chassis_module {
            config.chassis_config, 
            wheels, 
            acc_gyr_sensor_, 
            mag_sensor_};



        using namespace ChassisActions;
        using namespace GrabActions;

        Map map{};
        auto & vuart = uart2;
        vuart.init(576000, CommStrategy::Blocking);
        VisionModule vision{vuart};

        Planner planner{chassis_module, map};

        // grab_module.init();
        // grab_module.idle();
        chassis_module.init();

        enum class Status:uint8_t{
            NONE,
            GO_ROUGH,
            AT_ROUGH,
            GO_STAGING,
            AT_STAGING,
            END
        };

        // Status status = Status::GO_ROUGH;
        Status status = Status::NONE;

        auto & chassis = chassis_module;
        auto & grab = grab_module;

        auto sm_go_rough = [&](){
            chassis << new StrictShiftAction(chassis, {0.225_r, 0.155_r});
            // chassis << new StrictShiftAction(chassis, {0.225_r, 0});
            // chassis << new StrictShiftAction(chassis, {-0.225_r, 0});
            // chassis << new StrictShiftAction(chassis, {0.225_r, 0});
            // chassis << new StrictShiftAction(chassis, {-0.225_r, 0});
            // chassis << new StrictShiftAction(chassis, {0.225_r, 0});
            // chassis << new StrictShiftAction(chassis, {-0.225_r, 0});

            // chassis << new ShiftAction(chassis, {0.225_r, 0});
            // chassis << new ShiftAction(chassis, {-0.225_r, 0});
            // chassis << new ShiftAction(chassis, {0.225_r, 0});
            // chassis << new ShiftAction(chassis, {-0.225_r, 0});
            // chassis << new ShiftAction(chassis, {0.225_r, 0});
            // chassis << new ShiftAction(chassis, {-0.225_r, 0});


            chassis << new StraightAction(chassis, 1.72_r);
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.825_r);
            chassis << new StrictSpinAction(chassis, real_t(PI/2));
        };

        // debuguart
        enum class Process:uint8_t{
            INIT,
            INSPECT,
            TO_FIND,
            GET_OFFS,
            MOVE,
            GIVE_CENTER,
            MOVE_LEFT,
            GIVE_LEFT,
            MOVE_RIGHT,
            GIVE_RIGHT,
            TAKE_RIGHT,
            REMOVE_LEFT,
            TAKE_LEFT,
            MOVE_CENTER,
            TAKE_CENTER,
            BACK,
            DONE,
        };

        bool field_ok = false;

        Process process = Process::INIT;

        static constexpr real_t g_width = 0.15_r;
        static constexpr real_t g_back = 0.15_r;

        auto process_field = [&](){
            return;
            DEBUG_PRINTLN(int(process));
            switch(process){
                case Process::INIT:
                    field_ok = false;
                    if(true){
                        vision.color();
                        grab.inspect();
                        process = Process::INSPECT;
                    }
                    break;
                case Process::INSPECT:
                    if(grab.pending() == 0){
                        process = Process::TO_FIND;
                    }
                    break;
                case Process::TO_FIND:
                    chassis.setCurrent({0, 0.5_r, 0});
                    // if(vision.has_color()){
                    if(true){
                        vision.offset();
                        process = Process::GET_OFFS;
                    }
                    break;
                case Process::GET_OFFS:
                    // if(vision.offset().has_value()){
                    if(true){
                        vision.close();
                        chassis.strict_shift((Vec2{0, 0.04_r}));
                        process = Process::MOVE;
                    }
                    break;
                case Process::MOVE:
                    if(chassis.pending() == 0){
                        grab.give(TrayIndex::Center);
                        process = Process::GIVE_CENTER;
                    }
                    break;
                case Process::GIVE_CENTER:
                    if(grab.pending() == 0){
                        // grab.take(TrayIndex::Center);
                        chassis.strict_shift(Vec2{-g_width, 0});
                        process = Process::MOVE_LEFT;
                    }
                    break;
                case Process::MOVE_LEFT:
                    if(chassis.pending() == 0){
                        grab.give(TrayIndex::Left);
                        process = Process::GIVE_LEFT;
                    }
                    break;
                case Process::GIVE_LEFT:
                    if(grab.pending() == 0){
                        chassis.strict_shift(Vec2{g_width * 2, 0});
                        process = Process::MOVE_RIGHT;
                    }
                    break;
                case Process::MOVE_RIGHT:   
                    if(chassis.pending() == 0){
                        grab.give(TrayIndex::Right);
                        process = Process::GIVE_RIGHT;
                    }
                    break;
                case Process::GIVE_RIGHT:
                    if(grab.pending() == 0){
                        grab.take(TrayIndex::Right);
                        process = Process::TAKE_RIGHT;
                    }
                    break;
                case Process::TAKE_RIGHT:   
                    if(grab.pending() == 0){
                        chassis.strict_shift({-2 * g_width, 0});
                        process = Process::REMOVE_LEFT;
                    }
                    break;
                case Process::REMOVE_LEFT:
                    if(chassis.pending() == 0){
                        grab.take(TrayIndex::Left);
                        process = Process::TAKE_LEFT;
                    }
                    break;
                case Process::TAKE_LEFT:
                    if(grab.pending() == 0){
                        chassis.strict_shift({g_width, 0});
                        process = Process::MOVE_CENTER;
                    }
                    break;
                case Process::MOVE_CENTER:
                    if(chassis.pending() == 0){
                        grab.take(TrayIndex::Center);
                        process = Process::TAKE_CENTER;
                    }
                    break;
                case Process::TAKE_CENTER:
                    if(grab.pending() == 0){
                        chassis.strict_shift({0, -g_back});
                        process = Process::BACK;
                    }
                    break;
                case Process::BACK:
                    if(chassis.pending() == 0){
                        process = Process::DONE;
                    }
                    break;
                case Process::DONE:
                    field_ok = true;
                    break;
            }
            // grab.inspect();

            // grab.give(TrayIndex::Left);
            // grab.give(TrayIndex::Center);
            // grab.give(TrayIndex::Right);
            // grab.take(TrayIndex::Left);
            // grab.take(TrayIndex::Center);
            // grab.take(TrayIndex::Right);

            // grab.idle();
        };

        auto sm_at_rough = [&](){
            process_field();
        };

        auto sm_go_staging = [&](){
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.825_r);
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.860_r);
            chassis << new StrictSpinAction(chassis, real_t(PI/2));
        };

        auto sm_at_staging = [&](){
            process_field();
        };

        auto check_go_rough = [&]() -> bool{
            return grab.done() and chassis.done();
        };

        auto check_at_rough = [&]() -> bool{
            return grab.done() and chassis.done();
            // return field_ok;
        };

        auto check_go_staging = [&]() -> bool{
            // return grab.done() and chassis.done();
            // return field_ok;
            return true;
        };

        auto check_at_staging = [&]() -> bool{
            return grab.done() and chassis.done();
            // return field_ok;
        };

        auto check_end = [&]() -> bool {
            // return grab.done() and chassis.done();
            // return field_ok;
            return true;
        };

        auto sm_end = [&](){
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.860_r);
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 1.65_r);
            chassis << new StrictShiftAction(chassis, {-0.155_r, 0.225_r});   
        };

        uint cnt = 0;

        auto sw_state = [&](const Status new_st){
            status = new_st;
            cnt = 0;
        };

        [[maybe_unused]] auto mmain = [&](){
            
            switch(status){
                case Status::NONE:
                    if(check_go_rough()){
                        sm_go_rough();
                        sw_state(Status::GO_ROUGH);
                        break;
                    }
                case Status::GO_ROUGH:
                    if(check_at_rough()){
                        sm_at_rough();
                        sw_state(Status::AT_ROUGH);
                    }
                    break;

                case Status::AT_ROUGH:
                    process_field();
                    if(check_go_staging()){
                    // DEBUG_PRINTLN("at rough");
                    // DEBUG_PRINTLN("at rough");
                    // if(true){
                        // grab.idle();
                        idle();
                        sm_go_staging();
                        field_ok = false;
                        process = Process::INIT;
                        sw_state(Status::GO_STAGING);
                    }

                    break;
                case Status::GO_STAGING:
                    // DEBUG_PRINTLN("go st");
                    if(check_at_staging()){
                        sm_at_staging();
                        sw_state(Status::AT_STAGING);
                    }
                    break;

                case Status::AT_STAGING:
                    process_field();
                    if(check_end()){
                        // grab.idle();
                        idle();
                        sm_end();
                        sw_state(Status::END);
                    }
                    break;
                case Status::END:
                    break;
            }

            cnt++;
        };

        auto & led = hal::PC<14>();
        led.outpp();


        auto tick_50hz = [&]{
            joint_left.tick();
            joint_right.tick();
            joint_z.tick();
            led.toggle();
        };

        bind_tick_800hz([&]{
            chassis_module.tick800();
            static uint8_t i = 0;
            i = (i + 1) % 16;
            if(i == 0){
                tick_50hz();
            }
        });


        bind_tick_1khz([&](){
            grab_module.tick();
            chassis_module.tick();
            vision.update();

            // mmain();
        });

        // sm_go_rough();
        // chassis.strict_spin(real_t(PI/2));
        // for(size_t i = 0; i < 10; i ++){
            // chassis.spin(real_t(PI/2));
            
        //     chassis.wait(1500);
        // }
        // chassis.wait(500);
        // chassis.strict_spin(real_t(-PI/2));
        // chassis.spin(real_t(-PI/2));
        // chassis.wait(500);

        // grab.wait(4000);
        // grab.take(TrayIndex::Center);
        // grab.take(TrayIndex::Left);
        // grab.give(TrayIndex::Left);

        // sm_at_rough();

        // sm_go_rough();
        // sm_go_staging();
        // sm_end();

        // clock::delay(1000ms);
        // chassis << new ShiftAction(chassis, {0.255_r, 0.155_r});
        // chassis << new StraightAction(chassis, 1.74_r);
        // chassis << new StrictSpinAction(chassis, real_t(-PI/2));
        // chassis << new StraightAction(chassis, 0.805_r);
        // chassis << new StrictSpinAction(chassis, real_t(PI/2));
        // txo.outpp();
        // uart2.init(115200);
        // chassis.shift({0.2_r, 0});
        // chassis.shift({-0.2_r, 0});
        // chassis.spin(real_t(PI/2));
        // chassis.spin(real_t(PI/2));
        // chassis.spin(real_t(PI/2));
        // chassis.spin(real_t(-PI/2));
        // chassis.spin(real_t(-PI/2));
        // chassis.spin(real_t(-PI/2));
        // chassis.spin(real_t(-PI/2));
        while(true){
            // chassis.setCurrent({0,0, 3 * sin(t)});
            // chassis.setCurrent({0, /sin(t), 0});
            // chassis.setCurrent({0.4_r, 0, 0});
            // wheels.setCurrent({real_t(1), 0, 0, 0});
            // wheels.setCurrent({1, -1, 1, -1});
            // wheels.setCurrent({0, 1, 0, 0});
            // wheels.setCurrent({0, 0, 1, 0});
            // wheels.setCurrent({0, 0, 0, 1});
            // DEBUG_PRINTLN(chassis.gyr(), chassis.rad());
            clock::delay(10ms);
            chassis.setCurrent({0,0,0.1_r * sin(time())});
            // vuart.println("color");
            // vision.offset();
            // clock::delay(2000ms);
            // vuart.println("offset");
            // clock::delay(2000ms);
            // DEBUG_PRINTLN(vision.color(), vision.offset());
            // if(vision.has_color()){
            //     // DEBUG_PRINTLN("color", int(vision.color()));
            //     switch(vision.color()){
            //         case MaterialColor::Blue:
            //             DEBUG_PRINTLN("b")
            //             break;
            //         case MaterialColor::Green:
            //             DEBUG_PRINTLN("g")
            //             break;

            //         case MaterialColor::None:
            //             DEBUG_PRINTLN("n")
            //             break;
            //         case MaterialColor::Red:
            //             DEBUG_PRINTLN("r")
            //             break;
            //     }
            // }else{
            //     DEBUG_PRINTLN("none");
            // }

            // if(vision.has_offset()){
            //     DEBUG_PRINTLN(vision.offset());
            // }
            // clock::delay(200ms);
            // if(vuart.available()){
            //     DEBUG_PRINTLN(vuart.readString());
            // }
            // else{
                // DEBUG_PRINTLN("???")
            // }
            // vuart.println("offset");
            // clock::delay(1000ms);
            continue;
            // DEBUG_PRINTLN(uart2.available());
            // clock::delay(10ms);
            // delay
            // clock::delay(100ms);
            // txo.toggle();
            // uart2.println()
            // {
            //     clock::delay(2000ms);

            //     static bool fwd = false;
            //     fwd = !fwd;
                
            // }
            // chassis_module.setCurrent({{0, 0.5_r * sin(3 * t)}, 0});
            // chassis_module.setCurrent({{0.8_r * sin(3 * t), 0}, 0});
            // auto ray = chassis_module.jny();
            // auto [org, rad] = ray;
            // auto [x,y] = org;
            // if(millis() % 2000 > 1000){grab.meta_press();}
            // else{grab.meta_release();}
            

            // chassis.setCurrent({0,0,0});
            // DEBUG_PRINTLN(chassis.gyr(), 
            //     chassis.rad(), stps[0].readPosition(), stps[1].readPosition(), stps[2].readPosition(),
            //     stps[3].readPosition());]
            // uart2.println("color");
            // DEBUG_PRINTLN(uart2.pending());
            // DEBUG_PRINTLN(int(status));
            // DEBUG_PRINTLN(std::setprecision(4))
            // DEBUG_PRINTLN(int(status));
            // DEBUG_PRINTLN(grab.pending());

            // chassis_module.setPosition({0,0,sin(t) * real_t(PI/2)});
            // , x,y,rad);
            // chassis_module.setCurrent({{0,0}, 0.2_r});
            // DEBUG_PRINTLN("???");
        }
    }

    if(true){
        auto & led = hal::PC<14>();

        led.outpp();

        auto stps = std::array<RemoteFOCMotor, 4>({
            {logger, can, 1},
            {logger, can, 2},
            {logger, can, 3},
            {logger, can, 5},
        });

        // auto & stp = stps[0];
        auto & stp = stps[1];
        // auto & stp = stps[2];
        // auto & stp = stps[3];

        while(true){
            // auto targ = 0.4_r * sin(10 * t);
            // auto targ = 10.4_r * sin(4 * t);
            auto targ = 4.4_r * time();
            // stp.setTargetCurrent(targ);
            stp.setTargetPosition(targ);
            // stp.setTargetSpeed(targ);
            stp.getPosition();
            // stp.setTargetPosition(targ);
            // can.try_write(BxCanFrame{0b0010'00000010, std::make_tuple(targ)});
            // can.try_write(BxCanFrame{0b0010'00000010, std::make_tuple(targ)});

            led.toggle();
            clock::delay(19ms);

            real_t spd = 0;
            if(can.available()){
                // DEBUG_PRINTLN(can.read());
                spd = real_t(can.read());
            }
            DEBUG_PRINTLN(millis(), targ, can.pending(), spd);
        }
    }

    
};


}