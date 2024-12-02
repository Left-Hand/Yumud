#include "Robot.hpp"

using Sys::t;

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;

namespace gxm{


using Vector2 = Vector2_t<real_t>;



void initDisplayer(ST7789 & tftDisplayer){
    tftDisplayer.init();

    tftDisplayer.setFlipX(false);
    tftDisplayer.setFlipY(true);
    if(true){
        tftDisplayer.setSwapXY(true);
        tftDisplayer.setDisplayOffset({40, 52}); 
    }else{
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setDisplayOffset({52, 40}); 
    }
    tftDisplayer.setFormatRGB(true);
    tftDisplayer.setFlushDirH(false);
    tftDisplayer.setFlushDirV(false);
    tftDisplayer.setInversion(true);

    tftDisplayer.fill(ColorEnum::BLACK);
}


auto canvas_transform(const Ray & ray) -> Ray{
    scexpr auto meter = int{2};
    scexpr auto size = Vector2{100,100};
    scexpr auto org =  Vector2{12,12};
    scexpr auto area = Rect2i{org,size};
    
    auto x = LERP(real_t(area.x), real_t(area.x + area.w), ray.org.x / meter);
    auto y = LERP(real_t(area.y + area.h), real_t(area.y), ray.org.y / meter);
    return Ray{Vector2{x,y} + Vector2::ones(12), ray.rad};
};


auto draw_curve(PainterConcept & painter, const Curve & curve) -> void {
    painter.setColor(ColorEnum::BLUE);
    for(auto it = curve.begin(); it != curve.end(); it++){
        auto pos = canvas_transform(Ray(*it)).org;
        painter.drawPixel(pos);
    }
};

auto print_curve(OutputStream & logger, const Curve & curve) -> void{
    logger << std::setprecision(4);
    for(auto it = curve.begin(); it != curve.end(); it++){
        auto [pos, rad] = Ray(*it);
        delay(1);
        logger.println(pos.x, pos.y, rad);
    }
};

auto draw_turtle(PainterConcept & painter, const Ray & ray) -> void{
    scexpr real_t len = 7;
    auto [org, rad] = canvas_transform(ray);
    rad = -rad;//flipy
    auto pf = org + Vector2::from_angle(len, rad);
    auto p1 = org + Vector2::from_angle(len, rad + real_t(  PI * 0.8));
    auto p2 = org + Vector2::from_angle(len, rad + real_t(- PI * 0.8));

    // painter.setColor(ColorEnum::RED);
    painter.setColor(RGB888(HSV888(int(t * 64),255,255)));
    painter.drawFilledTriangle(pf, p1, p2);
    // painter.drawPixel(org);

    painter.setColor(ColorEnum::BLACK);
    painter.drawHollowTriangle(pf, p1, p2);
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
            // .max_height = 0.25_r,
            // .tray_height = 0.2_r,
            // .free_height = 0.15_r,
            // .ground_height = 0.12_r,
            

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

            .max_spd = 1,
            .max_acc = 1
        },

        .cross_config = {
            .xoffs_length_meter = real_t(0.04),
            .forearm_length_meter = real_t(0.1),
            .upperarm_length_meter = real_t(0.1)
        },
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
        
        auto tick_1khz = [&](){
            grab_module.tick();
        };
        auto tick_50hz = [&](){
            joint_left.tick();
            joint_right.tick();
        };

        if(false){//绑定舵机更新回调函数
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

        if(false){//测试升降求解器
            CrossSolver cross_solver{
                config.cross_config
            };

            while(true){
                auto height = real_t(0.14) + real_t(0.06) * sin(t);
                auto inv_rad = cross_solver.inverse(height);
                auto f_height = cross_solver.forward(inv_rad);
                DEBUG_PRINTLN(std::setprecision(3), inv_rad, height, f_height);

                delay(20);
            }
        }

        if(true){//测试动作组
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