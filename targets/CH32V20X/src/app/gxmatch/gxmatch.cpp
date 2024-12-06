#include "misc.hpp"
#include "config.hpp"
#include "async/CanMaster.hpp"

#include "drivers/Camera/MT9V034/mt9v034.hpp"
#include "nvcv2/geometry/geometry.hpp"
#include "nvcv2/shape/shape.hpp"
#include "nvcv2/pixels/pixels.hpp"

#include "machine/chassis_actions.hpp"
#include "common/enum.hpp"


using namespace nvcv2;
using namespace gxm;

class VisionModule:public AsciiProtocolConcept{
protected:
    // UartHw & uart_;
    std::optional<MaterialColor> color_;
    std::optional<Vector2> offset_;

    enum class Mode:uint8_t{
        CLOSED,
        COLOR,
        OFFSET
    };

    Mode mode_ = Mode::CLOSED;
public: 
    VisionModule(UartHw & uart):
        AsciiProtocolConcept(uart){;}

    void parseArgs(const Strings & args) override{
        switch(args[0].hash()){
            case "color"_ha:
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
                        color_ = MaterialColor::Blue;
                        break;
                }
            break;

            case "offset"_ha:
                if(args.size() == 3){
                    offset_ = Vector2{
                        real_t(args[1]),
                        real_t(args[2])
                    };
                }else{
                    offset_ = std::nullopt;
                }
        }
    }

    void close(){
        os.println("close");
    }

    auto color(){
        if(mode_ != Mode::COLOR){
            os.println("color"); 
            color_ = std::nullopt;
        }
        return color_;
    }

    auto offset(){
        if(mode_ != Mode::OFFSET){
            os.println("offset");
            offset_ = std::nullopt;
        }
        return offset_;
    }


};

// class Ascii

namespace gxm{

void host_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD);
    auto & logger = DEBUGGER_INST;

    DEBUG_PRINTLN(std::setprecision(4), "poweron");

    auto i2c = I2cSw{portD[2], portC[12]};
    i2c.init(400_KHz);
    auto config = create_default_config();

    // auto & wuart = uart7;
    // wuart.init(115200);
    // spi1.init(9_MHz);
    // spi2.init(144_MHz, CommMethod::Blocking, CommMethod::None);

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
                // return real_t(PI);
                // return real_t(PI*0.75) + sin(t) * real_t(PI/4);
                return real_t(PI*0.25) + sin(t) * real_t(PI*0.25);
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
                delay(20);
            }
        }
        //#endregion

        //#region 测试xyz
        if(false){
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
        //#endregion

        //#region 测试机械臂位置反馈
        if(false){
            while(true){
                auto pos = Vector2(0, 0.19_r) + Vector2(0.10_r, 0).rotated(t);
                auto height = LERP(0.12_r, 0.17_r, (sin(t) + 1) >> 1);

                grab_module.rapid(Vector3(pos.x, pos.y, height));

                auto p3 = grab_module.getPos();
                DEBUG_PRINTLN(pos.x, pos.y, height, p3.x, p3.y, p3.z);
                delay(20);
            }
        }
        //#endregion

        //#region 测试抓取动作
        if(true){
        // if(false){

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
            grab_module.take(TrayIndex::Center);
            grab_module.give(TrayIndex::Center);
            grab_module.take(TrayIndex::Right);
            grab_module.give(TrayIndex::Right);
            grab_module.take(TrayIndex::Left);
            // while(true){
            //     // DEBUG_PRINTLN(grab_module.pending());
            //     DEBUG_PRINTLN(millis(), grab_module.pending(), grab_module.which());
            //     delay(200);
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
                delay(5);

                // {
                //     delay(2000);

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
                DEBUG_PRINTLN(mag_sensor_.getMagnet())
                delay(5);
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

                // const auto [x,y] = flow_sensor_.getPosition();

                // DEBUG_PRINTLN(rot, gyr_raw, x, y);
                DEBUG_PRINTLN(mag3_raw.x, mag3_raw.y, mag3_raw.z, fposmodp(rot_raw, real_t(PI/2)));
                // DEBUG_PRINTLN(gyr_raw);
                delay(5);
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

            // auto delta = solver.inverse(Vector2{0, 0}, 0.7_r*sin(t));

            real_t ang;
            bind_tick_200hz(
                [&](){
                // wheels.request();
                // DEBUG_PRINTLN(can.available());
                // can_master.update();
                acc_gyr_sensor_.update();
        
                ang += Vector3(acc_gyr_sensor_.getGyr()).z * 0.005_r;
                // auto delta = solver.inverse(Vector2{0.4_r*sin(t), 0}, 0);

                // scexpr real_t delta = {0.003_r};
                // auto delta = solver.inverse(Vector2{0, 0.0003_r}, 0);
                // auto delta = solver.inverse(Vector2{0, 0.3_r * sin(t)}, 0);
                
                // auto delta = solver.inverse(Vector2{0._r, 0.0_r}, 0);
                // auto delta = solver.inverse(Vector2{0, 0}, 0.005_r);
                // auto delta = solver.inverse(Vector2{0.003_r, 0.003_r}, 0);
                // auto delta = solver.inverse(Vector2{-0.003_r, 0.00_r}, 0);
                // auto delta = solver.inverse(Vector2{real_t(1.0/200) * sin(t), 0.00_r}, 0);
                auto delta = solver.inverse(Vector2{0, 0.00_r}, CLAMP2((1-ang) * real_t(9), 1));
                // DEBUG_PRINTLN(millis());
                // wheels.setCurrent(delta);
                // DEBUG_PRINTLN(std::get<0>(delta));
                // wheels.setSpeed(delta);
                // wheels.setPosition(delta);
                wheels.setCurrent(delta);
            }

            );

            while(true){
                // delta = solver.inverse(Vector2{0, 0}, 1.7_r*sin(t));
                // delta = solver.inverse(Vector2{0, 1.7_r*sin(t)});
                
                // DEBUG_PRINTLN(std::get<0>(delta));
                // DEBUG_PRINTLN(delta);
                // DEBUG_PRINTLN(wheels[0].getPosition(), wheels[1].getPosition(), wheels[2].getPosition(), wheels[3].getPosition());
                DEBUG_PRINTLN(ang)
                delay(5);
            }
        }

        // if(false){
        if(true){
            wheels.init();

            real_t py;
            bind_tick_200hz(
                [&](){
                flow_sensor_.update();
        
                py = Vector2(flow_sensor_.getPosition()).y;
                // py = Vector2(flow_sensor_.getPosition()).x;
                
                auto delta = solver.inverse(Vector2{0, CLAMP2((0.2_r-py) * real_t(20), 0.8_r)}, 0);
                // auto delta = solver.inverse(Vector2{0, 1}, 0);
                // DEBUG_PRINTLN(delta);
                wheels.setCurrent(delta);
            });

            while(true){
                // delta = solver.inverse(Vector2{0, 0}, 1.7_r*sin(t));
                // delta = solver.inverse(Vector2{0, 1.7_r*sin(t)});
                
                // DEBUG_PRINTLN(std::get<0>(delta));
                // DEBUG_PRINTLN(delta);
                // DEBUG_PRINTLN(wheels[0].getPosition(), wheels[1].getPosition(), wheels[2].getPosition(), wheels[3].getPosition());
                DEBUG_PRINTLN(std::setprecision(4), py)
                delay(5);
            }
        }


        //#endregion



    }//#endregion
    
    //#region 视觉
    // if(true){
    if(false){
        bindSystickCb(nullptr);
        [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale> & src, const Vector2i & pos){
            const auto area = Rect2i(pos, src.get_size());
            displayer.puttexture(area, src.get_data());
        };

        [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Vector2i & pos){
            const auto area = Rect2i(pos, src.get_size());
            displayer.puttexture(area, src.get_data());
        };

        [[maybe_unused]] auto plot_rgb = [&](const Image<RGB565> & src, const Vector2i & pos){
            const auto area = Rect2i(pos, src.get_size());
            displayer.puttexture(area, src.get_data());
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
            delay(200);
            displayer.fill(ColorEnum::WHITE);
            delay(200);
        }

        while(true){
            painter.setColor(ColorEnum::WHITE);

            // Image<Grayscale> img = Shape::x2(camera);
            Image<Grayscale> img = camera.clone();
            auto ave = Pixels::average(img);
            DEBUG_PRINTLN(millis(), uint8_t(ave));
            plot_gray(img, {0, 0});
            // auto img = raw_img.space();
            // Geometry::perspective(img, raw_img);
            // plot_gray(img, {0, img.get_size().y * 1});
            continue;
            auto img_ada = img.space();
            // Shape::adaptive_threshold(img_ada, img);
            // plot_gray(img_ada, {0, img.get_size().y * 2});

            auto img_processed = img.space<Grayscale>();
            // Shape::canny(img_bina, img, {60, 120});
            Shape::eye(img_processed, img);
            auto img_processed2 = img.space<Grayscale>();
            Shape::eye(img_processed2, img_processed);
            // Pixels::binarization(img_bina, img_ada, 220);
            // Pixels::inverse(img_bina);
            plot_gray(img_processed, {0, img.get_size().y * 2});
            plot_gray(img_processed2, {0, img.get_size().y * 3});

        }
    }

    //#endregion

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
        // mag_sensor_.init();

        auto stps = std::array<RemoteFOCMotor, 4>({
            {logger, can, 1},
            {logger, can, 2},
            {logger, can, 3},
            {logger, can, 5},
        });

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

        Planner planner{chassis_module, map};

        grab_module.init();
        chassis_module.init();

        enum class Status:uint8_t{
            NONE,
            GO_ROUGH,
            AT_ROUGH,
            GO_STAGING,
            AT_STAGING,
            END
        };

        Status status = Status::NONE;

        auto & chassis = chassis_module;
        auto & grab = grab_module;

        auto sm_go_rough = [&](){
            chassis << new ShiftAction(chassis, {0.255_r, 0.155_r});
            chassis << new StraightAction(chassis, 1.74_r);
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.805_r);
            chassis << new StrictSpinAction(chassis, real_t(PI/2));
        };

        auto process_field = [&](){
            grab.inspect();

            grab.give(TrayIndex::Left);
            grab.give(TrayIndex::Center);
            grab.give(TrayIndex::Right);
            grab.take(TrayIndex::Left);
            grab.take(TrayIndex::Center);
            grab.take(TrayIndex::Right);

            grab.idle();
        };

        auto sm_at_rough = [&](){
            process_field();
        };



        auto sm_go_staging = [&](){
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.845_r);
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.850_r);
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
        };

        auto check_go_staging = [&]() -> bool{
            return grab.done() and chassis.done();
        };

        auto check_at_staging = [&]() -> bool{
            return grab.done() and chassis.done();
        };

        auto check_end = [&]() -> bool {
            return grab.done() and chassis.done();
        };

        auto sm_end = [&](){
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 0.850_r);
            chassis << new StrictSpinAction(chassis, real_t(-PI/2));
            chassis << new StraightAction(chassis, 1.65_r);
            chassis << new ShiftAction(chassis, {-0.185_r, 0.265_r});   
        };

        uint cnt = 0;

        auto sw_state = [&](const Status new_st){
            status = new_st;
            cnt = 0;
        };

        auto mmain = [&](){
            
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
                        sm_go_staging();
                        sw_state(Status::GO_STAGING);
                    }

                    break;
                case Status::GO_STAGING:
                    if(check_at_staging()){
                        sm_at_staging();
                        sw_state(Status::AT_STAGING);
                    }
                    break;

                case Status::AT_STAGING:
                    process_field();
                    if(check_end()){
                        sm_end();
                        sw_state(Status::END);
                    }
                    break;
                case Status::END:
                    break;
            }

            cnt++;
        };

        bind_tick_800hz([&]{
            chassis_module.tick800();
        });

        auto & led = portC[14];
        led.outpp();
        bind_tick_50hz([&](){
            joint_left.tick();
            joint_right.tick();
            joint_z.tick();
            led.toggle();
        });

        bind_tick_1khz([&](){
            grab_module.tick();
            chassis_module.tick();
            // mmain();
        });

        sm_go_rough();
        sm_go_staging();
        sm_end();

        // sm_at_rough();
        
        // sm_at_staging();

        while(true){
            // DEBUG_PRINTLN(chassis_module.rot(), chassis_module.gyr());
            delay(10);

            // {
            //     delay(2000);

            //     static bool fwd = false;
            //     fwd = !fwd;
                
            // }
            // chassis_module.setCurrent({{0, 0.5_r * sin(3 * t)}, 0});
            // chassis_module.setCurrent({{0.8_r * sin(3 * t), 0}, 0});
            auto ray = chassis_module.jny();
            auto [org, rad] = ray;
            auto [x,y] = org;

            // DEBUG_PRINTLN(std::setprecision(4))
            // DEBUG_PRINTLN(int(status));
            // DEBUG_PRINTLN(grab.pending());

            // chassis_module.setPosition({0,0,sin(t) * real_t(PI/2)});
            // , x,y,rad);
            // chassis_module.setCurrent({{0,0}, 0.2_r});
            // DEBUG_PRINTLN("???");
        }
    }

    
};


}