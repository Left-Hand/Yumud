#include "smc.h"

using namespace SMC;
using namespace NVCV2;
using NVCV2::Shape::CoastFinder;
using NVCV2::Shape::Seed;


static void fast_bina_opera(Image<Binary, Binary>out, Image<Grayscale, Grayscale> & em, const uint8_t et, Image<Grayscale, Grayscale>& dm, const uint8_t dt) {
    const auto size = (Rect2i(Vector2i(), em.size).intersection(Rect2i(Vector2i(), dm.size))).size;
    const auto area = size.x * size.y;

    for (auto i = 0; i < area; i++) {
        out[i] = Binary(((uint8_t)em[i] > et) || ((uint8_t)dm[i] > dt));
    }
}



void SmartCar::recordRunStatus(const RunStatus status){
    runStatusReg = uint16_t(uint8_t(status));
}

void SmartCar::printRecordedRunStatus(){
    uint16_t id = runStatusReg;
    auto temp = RunStatus::_from_integral_unchecked(uint16_t(runStatusReg));
    DEBUG_PRINTLN("last Stat:\t", temp._to_string(), id);
    DEBUG_PRINTLN("last Flag:\t", toString(int(uint16_t(flagReg)), 2), id);
    // DEBUG_PRINTLN(temp._to_string(), id);
}



void SmartCar::ctrl(){
    Sys::Clock::reCalculateTime();

    static constexpr real_t target_dir = real_t(PI / 2);
    body.update();

    mpu.update();
    qml.update();

    msm.accel = msm.accel_offs.xform(Vector3(mpu.getAccel()));
    msm.gyro = (Vector3(mpu.getGyro()) - msm.gyro_offs);
    msm.magent = msm.magent_offs.xform(Vector3(qml.getMagnet()));


    {
        static constexpr real_t wheel_l = 0.182;
        odo.update();

        real_t now_pos = odo.getPosition() * wheel_l;
        static real_t last_pos = now_pos;
        
        real_t pos_delta = now_pos - last_pos;
        last_pos = now_pos;

        real_t now_spd = pos_delta == 0 ? 0 : pos_delta * ctrl_freq;
        
        static LowpassFilterZ_t<real_t> lpf{0.8};
        msm.front_spd = lpf.update(now_spd);
        // DEBUG_PRINTLN(now_pos, front_spd);
    }

    // real_t rot;
    {
        // static LowpassFilterZ_t<real_t> lpf{0.8};
        msm.rot = msm.gyro.z;
    }



    real_t turn_output = turn_ctrl.update(target_dir, msm.current_dir, msm.gyro.z);

    real_t side_acc = msm.accel.y;
    real_t side_volocity = side_velocity_observer.update(side_offs_err, side_acc);
    real_t side_output = side_ctrl.update(0, side_offs_err, -side_volocity);

    real_t speed_output = speed_ctrl.update();

    if(flags.hand_mode == false){
        motor_strength.left = turn_output;
        motor_strength.right = -turn_output;

        motor_strength.left += speed_output;
        motor_strength.right += speed_output;

        motor_strength.hri = side_output;
    }

}

void SmartCar::init_debugger(){
    LOGGER.init(115200, CommMethod::Blocking);
    LOGGER.setEps(4);
    LOGGER.setRadix(10);

}

void SmartCar::init_periphs(){
    bkp.init();
    
    {
        portD[4].outpp();
        portD[4] = 1;
    }

    powerOnTimesReg = uint16_t(int(powerOnTimesReg) + 1);
    printRecordedRunStatus();
    // DEBUG_PRINTLN("pwon", int(powerOnTimesReg));

    spi2.init(72000000);
    spi2.bindCsPin(portD[0], 0);

    timer4.init(24000);
    timer5.init(24000);
    timer8.init(234, 1);
    timer8.oc(1).init();
    timer8.oc(2).init();
    // timer8.initBdtr(0, AdvancedTimer::LockLevel::Off);

    // timer1.initAsEncoder();
    enc.init();
    odo.inverse(true);


    delay(200);
}

void SmartCar::init_fans(){
    chassis_fan.init();
    vl_fan.init();
    vr_fan.init();
    hri_fan.init();
    
    // body.init();
    // body.enable(true);
}

void SmartCar::unlock(){
    // // chassis_fan.enable();p
    // chassis_fan.left_fan.interface.init();
    // chassis_fan.right_fan.interface.init();
    // auto entry = millis();
    // while(millis() - entry < 3000){
    //     chassis_fan.left_fan.interface.enable();
    //     chassis_fan.right_fan.interface.enable();
    //     DEBUG_PRINTLN("???", millis());
    //     delay(20);
    // }

    DShotChannel ch1{timer8.oc(1)};
    DShotChannel ch2{timer8.oc(2)};

    ch1.init();
    ch2.init();


    auto entry = millis();
    while(millis() - entry < 3000){
        // ch1.enable();
        // ch2.enable();
        ch1 = 0;
        ch2 = 0;

        delay(20);
    }
}

void SmartCar::init_lcd(){



    tftDisplayer.init();
    tftDisplayer.setFlipX(true);
    tftDisplayer.setFlipY(false);
    tftDisplayer.setSwapXY(true);
    tftDisplayer.setFormatRGB(true);
    tftDisplayer.setFlushDirH(false);
    tftDisplayer.setFlushDirV(false);
    tftDisplayer.setInversion(true);

    painter.bindImage(tftDisplayer);
}

void SmartCar::init_sensor(){

    i2c.init(400000);
    sccb.init(10000);

    mpu.init();
    qml.init();

    start_key.init();
    stop_key.init();
}

void SmartCar::init_camera(){
    uint8_t camera_init_retry_times = 0;
    constexpr uint8_t camera_retry_timeout = 7;
    sccb.init(10000);
    while(!camera.init()){
        camera_init_retry_times++;
        if(camera_init_retry_times >= camera_retry_timeout){
            DEBUG_LOG("camera init failed");
            while(true);
        }
    }
}

void SmartCar::process_eve(){

    flagReg = flags;

    flags.enable_trig |= start_key;
    flags.disable_trig |= stop_key;

    if(flags.enable_trig){
        body.enable(false);
        DEBUG_PRINTLN("enabled");
        flags.enable_trig = false;
    }

    if(flags.disable_trig){
        body.enable(true);
        DEBUG_PRINTLN("disabled");
        flags.disable_trig = false;
    }


    flagReg = flags;
};

void SmartCar::init_it(){
    GenericTimer & timer = timer2;
    timer.init(ctrl_freq);
    timer.bindCb(TimerUtils::IT::Update, [this](){this->ctrl();});
    timer.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});
}

void SmartCar::main(){
    init_debugger();
    init_periphs();
    delay(200);
    init_fans();
    init_sensor();
    init_lcd();
    init_camera();

    unlock();
    init_it();


    {
        static constexpr int cali_times = 100;

        Vector3 temp_gravity = Vector3();
        Vector3 temp_gyro_offs = Vector3();
        Vector3 temp_magent = Vector3();
        
        for(int i = 0; i < cali_times; ++i){
            temp_gravity += Vector3(mpu.getAccel());
            temp_gyro_offs += Vector3(mpu.getGyro());    
            temp_magent += Vector3(qml.getMagnet());
            delay(5);
        }

        Vector3 g = temp_gravity / cali_times;
        msm.accel_offs = Quat(Vector3(0,0,-1),g/g.length()).inverse();

        Vector3 m = temp_magent / cali_times;
        msm.magent_offs = Quat(Vector3(0,0,-1), m/m.length()).inverse();

        msm.gyro_offs = temp_gyro_offs / cali_times;
    }

    constexpr auto pic_size = Vector2i(188, 60);
    auto pers_gray_image = make_image<Grayscale>(pic_size);
    auto pers_bina_image  = make_bina_mirror(pers_gray_image);
    auto diff_gray_image = make_image<Grayscale>(pic_size);
    auto diff_bina_image = make_bina_mirror(diff_gray_image);
    auto sketch = make_image<RGB565>(pic_size);

    #define CHECK_PLOTDE()\
    if(flags.plot_de)\
    return;\

    [[maybe_unused]] auto plot_gray = [&](Image<Grayscale, Grayscale> & src, const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](Image<Binary, Binary> & src, const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    
    [[maybe_unused]] auto plot_sketch = [&](const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture_unsafe(area, sketch.data.get());
    };

    [[maybe_unused]] auto plot_coast = [&](const Coast & coast,  const Vector2i & pos, const RGB565 & color = RGB565::RED){
        if(coast.size() <= 1){
            return;
        }

        painter.bindImage(sketch);
        painter.setColor(color);
        auto it = coast.begin();

        while(it != std::prev(coast.end())){
            auto & currpoint = *it;
            auto & nextpoint = *std::next(it);
            painter.drawLine(currpoint, nextpoint);
            ++it;
        }
    };

    [[maybe_unused]] auto plot_coast_colored = [&](const Coast & coast,  const Vector2i & pos, const RGB565 & color = RGB565::WHITE){

        if(coast.size() <= 2){
            return;
        }

        painter.bindImage(sketch);
        painter.setColor(color);
        auto it = coast.begin();

        while(it != std::prev(coast.end())){
            auto & currpoint = *it;
            auto & nextpoint = *std::next(it);
            painter.drawLine(currpoint, nextpoint);
            ++it;
        }
    };

    [[maybe_unused]] auto plot_segment = [&](const std::pair<Vector2i, Vector2i> seg,  const Vector2i & pos, const RGB565 & color = RGB565::RED){


        painter.bindImage(sketch);
        painter.setColor(color);
        
        painter.drawLine(seg.first + pos, seg.second + pos);
    };

    [[maybe_unused]] auto plot_points = [&](const Points & pts,  const Vector2i & pos, const RGB565 & color = RGB565::PINK){
        painter.bindImage(sketch);
        painter.setColor(color);
        for(auto && pt : pts){
            painter.drawPixel(pt + pos);
        }
    };

    [[maybe_unused]] auto plot_bound = [&](const Boundry & bound,  const Vector2i & pos, const RGB565 & color = RGB565::RED){
        // return;
        // if(bound.size() < 2) return;
        painter.bindImage(sketch);
        for(auto && pt : bound){
            painter.drawPixel(Vector2i{pt.second, pt.first}+ pos, color);
        }
    };

    [[maybe_unused]] auto plot_point = [&](const Vector2i & pos, const RGB565 & color = RGB565::RED, const int & radius = 2){
        painter.bindImage(sketch);
        painter.setColor(color);
        painter.drawPixel(pos, color);
    };

    [[maybe_unused]] auto plot_pile = [&](const Pile & bound, const RGB565 & color = RGB565::RED){
        painter.bindImage(sketch);
        painter.drawLine(Vector2i{bound.second.start, bound.first}, {bound.second.end, bound.first});
    }; 


    [[maybe_unused]] auto plot_vec3 = [&](const Vector3 & vec3,  const Vector2i & pos){
        static constexpr auto square_length = 50;
        auto arm_length = vec3.length();

        static constexpr auto radius = 3;
        static constexpr auto x_unit = Vector2(1, 0);
        static constexpr auto y_unit = Vector2(0.5, -0.73);
        static constexpr auto z_unit = Vector2(0, -1);

        auto x_axis = Vector3(arm_length, 0, 0);
        auto y_axis = Vector3(0, arm_length, 0);
        auto z_axis = Vector3(0, 0, arm_length);

        static constexpr auto x_color = RGB565::RED;
        static constexpr auto y_color = RGB565::GREEN;
        static constexpr auto z_color = RGB565::BLUE;
        static constexpr auto bg_color = RGB565::BLACK;

        auto vec3n = vec3.normalized();
        const Quat rot = Quat(Vector3(0, 0, -1), vec3n);
        const Vector2i center_point = pos + Vector2i(square_length, square_length) / 2;

        auto plot_vec3_to_plane = [&](const Vector3 & axis, const char & chr, const RGB565 & color){
            Vector3 end = rot.xform(axis);
            Vector2i end_point = center_point + (x_unit * end.x + y_unit * end.y + z_unit * end.z);
            painter.setColor(color);
            painter.drawLine(center_point, end_point);
            painter.drawFilledCircle(end_point, radius);
        };

        painter.bindImage(tftDisplayer);

        painter.drawFilledRect(Rect2i{pos, Vector2i{square_length, square_length}}, bg_color);
        plot_vec3_to_plane(x_axis, 'X', x_color);
        plot_vec3_to_plane(y_axis, 'Y', y_color);
        plot_vec3_to_plane(z_axis, 'Z', z_color);
    };

    [[maybe_unused]] auto plot_gui = [&](){
        plot_vec3(msm.accel.normalized() * 15, {190, 0});
        plot_vec3(msm.gyro * 7, {190, 60});
        plot_vec3(msm.magent.normalized() * 20, {190, 120});
    };

    DEBUGGER.bindRxPostCb([&](){parse_line(DEBUGGER.readString());});

    camera.setExposureValue(300);
    while(true){
        plot_gui();
        #define CREATE_BENCHMARK(val) val = millis() - current_ms;\

        auto current_ms = millis();
        {
            static uint last_ms = current_ms;
            benchmark.frame_ms = current_ms - last_ms;
            last_ms = current_ms;
        }

        recordRunStatus(RunStatus::NEW_ROUND);

        recordRunStatus(RunStatus::PROCESSING_EVENTS);
        process_eve();

        recordRunStatus(RunStatus::PROCESSING_IMG_BEGIN);


        plot_gray(camera, Rect2i{0,0,188,60});

        CREATE_BENCHMARK(benchmark.cap);

        Geometry::perspective(pers_gray_image, camera);
        plot_gray(pers_gray_image, Rect2i{0, 60, 188, 60});

        CREATE_BENCHMARK(benchmark.pers);


        Pixels::inverse(pers_gray_image);
        //对灰度图取反(边线为白色方便提取)

        Shape::convo_roberts_x(diff_gray_image, pers_gray_image);
        //获取x方向上的差分图

        CREATE_BENCHMARK(benchmark.gray);

        // Pixels::binarization(diff_bina_image, diff_gray_image, config.edge_threshold);
        //将差分图二值化


        // Pixels::binarization(pers_bina_image, pers_gray_image, config.positive_threshold);
        // Pixels::or_with(pers_bina_image, diff_bina_image);

        fast_bina_opera(pers_bina_image, diff_gray_image, config.edge_threshold, pers_gray_image, config.positive_threshold);
        // Pixels::adaptive_threshold(pers_bina_image, diff_gray_image);
        CREATE_BENCHMARK(benchmark.bina);
        // Shape::dilate_xy(pers_bina_image);
        // Shape::dilate_xy(pers_bina_image);
        // Shape::erosion_x(pers_bina_image);
        // Shape::erosion_y(pers_bina_image);

        //获取差分图和灰度图的并集
        //差分图对边缘敏感 灰度图对色块敏感 两者合围可消除缺陷
        plot_bina(pers_bina_image,  Rect2i{Vector2i{0, 180}, Vector2i{188, 60}});
        // continue;

        // Shape::erosion_xy(affine_bina_image);
        //腐蚀图像 用于去除赛道上可能存在的斑点
        //可能会在寻斑马线时去除

        recordRunStatus(RunStatus::PROCESSING_IMG_END);


        auto & img = pers_gray_image;
        auto & img_bina = pers_bina_image;

        std::tie(msm.seed_pos, msm.road_window) = get_entry(img_bina, Vector2i{msm.seed_pos.x, img.size.y -1 - config.safety_seed_height}, RIGHT);


        if(msm.road_window.length() < 16){
            body.enable(false);
            continue;
        }
        // continue;

        if(!msm.seed_pos) continue;
        side_offs_err = -(msm.seed_pos.x - (img.get_size().x / 2) + 2) * 0.02;

        plot_point(msm.seed_pos);


        //---------------------
        //寻找两侧的赛道轮廓并修剪为非自交的形式
        recordRunStatus(RunStatus::PROCESSING_COAST);
        
        auto coast_left_unfixed =    CoastUtils::form(img_bina, msm.seed_pos, LEFT);
        auto coast_right_unfixed =   CoastUtils::form(img_bina, msm.seed_pos, RIGHT);

        auto coast_left = CoastUtils::trim(coast_left_unfixed, img.size);
        auto coast_right = CoastUtils::trim(coast_right_unfixed, img.size);
    
        ASSERT(!CoastUtils::is_self_intersection(coast_left), "left self ins");
        ASSERT(!CoastUtils::is_self_intersection(coast_right), "right self ins");
        //----------------------


        ASSERT_WITH_DOWN(coast_left.size(), "left coast size 0");
        ASSERT_WITH_DOWN(coast_right.size(), "right coast size 0");

        if(!coast_left.size()) continue;
        if(!coast_right.size()) continue;

        recordRunStatus(RunStatus::PROCESSING_COAST_END);

        auto track_left = CoastUtils::douglas_peucker(coast_left, config.dpv);
        auto track_right = CoastUtils::douglas_peucker(coast_right, config.dpv);

        plot_coast(track_left, {0, 0}, RGB565::RED);
        plot_coast(track_right, {0, 0}, RGB565::BLUE);

        // continue;

        recordRunStatus(RunStatus::PROCESSING_DP_END);

        if(track_left.size() >= 2 and track_right.size() >= 2){//update current_dir

            Segment follow_left = std::make_pair(track_left.front() + Vector2i(config.align_space_width, 0), *std::next(track_left.begin()) + Vector2i(config.align_space_width, 0));
            Segment follow_right = std::make_pair(track_right.front() - Vector2i(config.align_space_width, 0), *std::next(track_right.begin()) - Vector2i(config.align_space_width, 0));
            
            plot_segment(follow_left, {0, 0}, RGB565::YELLOW);
            plot_segment(follow_right, {0, 0}, RGB565::GREEN);

            Vector2 diff_left = SegmentUtils::vec(follow_left);
            Vector2 diff_right = SegmentUtils::vec(follow_right);
            if(bool(diff_left) and bool(diff_right)){

                auto diff_left_l = diff_left.length();
                auto diff_right_l = diff_right.length();
                auto vec_sin = diff_left.cross(diff_right) / diff_left_l / diff_right_l;
                if(vec_sin < abs(config.dir_merge_max_sin)){
                    msm.current_dir = (diff_left * diff_right_l + diff_right * diff_left_l).angle();
                }
            }else{
                if(flags.align_right and bool(diff_right)){
                    msm.current_dir = diff_right.angle();
                }else if(bool(diff_left)){
                    msm.current_dir = diff_left.angle();
                }
            }
        }

        recordRunStatus(RunStatus::PROCESSING_DIR_END);
        // if(Vector2(SegmentUtils::vec(follow_left)).cos())
        // DEBUG_PRINTLN("done plot shrink");

        auto v_points = CoastUtils::vcorners(track_left);
        auto a_points = CoastUtils::acorners(track_left);

        recordRunStatus(RunStatus::PROCESSING_VA_END);
        // auto track_left = douglas_peucker(CoastUtils::shrink(coast_left, real_t(-align_space_width)), dpv);
        // auto track_right = douglas_peucker(CoastUtils::shrink(coast_right, real_t(align_space_width)), dpv);

        // coast_right = douglas_peucker(coast_right, dpv);
        // auto track_right = get_main_coast(CoastUtils::shrink(coast_right, real_t(-15)));
        // plot_coast(track_left, {0, 0}, RGB565::RED);
        // plot_coast(track_right, {0, 0}, RGB565::BLUE);
        // DEBUG_PRINTLN("done plot track");

        // static constexpr Vector2i shapes_offset = Vector2i(0, 0);

        plot_segment({msm.seed_pos, msm.seed_pos + Vector2(10.0, 0).rotated(-msm.current_dir)}, {0, 0}, RGB565::GRAY);

        plot_points(v_points, {0, 0}, RGB565::YELLOW);
        plot_points(a_points, {0, 0}, RGB565::PINK);
        
        {
            plot_sketch({0, 120, 188,60});
            sketch.fill(RGB565::BLACK);
        }
        // plot_bound(bound_center, {0, 0}, RGB565::PINK);
        // DEBUG_PRINTLN("!!");
 

        // plot_bound(bound_right, {0, 0}, RGB565::BLUE);
        // plot_bound(bound_center, {0, 0}, RGB565::YELLOW);
        // plot_gray(diff_gray_image, {0, 160});

        // delay(200);
        // plot_grayscale(affine_image, {0, 0});
        // plot_binary(affine_bina_image, {0, 80});


        // frame_ms = millis() - start;
        // DEBUG_PRINTLN(coast_left.size() + coast_right.size(), track_left.size() + track_right.size(), v_points.size() + a_points.size())
        recordRunStatus(RunStatus::END_ROUND);
        // DEBUG_PRINTLN("end turn");
        // logger.println(coast_left.size());
    }
}



void smc_main(){
    SmartCar car;
    car.main();
};
