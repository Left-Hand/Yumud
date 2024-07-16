#include "smc.h"

using namespace SMC;
using namespace NVCV2;
using NVCV2::Shape::CoastFinder;
using NVCV2::Shape::Seed;



static void fast_diff_opera(Image<Grayscale, Grayscale> & dst, const Image<Grayscale, Grayscale> & src) {
    if((void *)&dst == (void *)&src){
        auto temp = dst.clone();
        fast_diff_opera(temp, src);
        dst = std::move(temp);
        return;
    }

    auto window = dst.get_window().intersection(src.get_window());
    for (auto y = window.y; y < window.y + window.h; y++) {
        for (auto x = window.x; x < window.x + window.w; x++) {
            const int a = src(Vector2i{x,y});
            const int b = src(Vector2i{x+1,y});
            const int c = src(Vector2i{x,y+1});
            dst[{x,y}] = uint8_t(CLAMP(std::max(
                (ABS(a - c)) * 255 / (a + c),
                (ABS(a - b) * 255 / (a + b))
            ), 0, 255));
        }
    }
}

[[maybe_unused]] static void fast_bina_opera(Image<Binary, Binary> & out,const Image<Grayscale, Grayscale> & em, const uint8_t et,const Image<Grayscale, Grayscale>& dm, const uint8_t dt) {
    const auto size = (Rect2i(Vector2i(), em.size).intersection(Rect2i(Vector2i(), dm.size))).size;
    const auto area = size.x * size.y;

    for (auto i = 0; i < area; i++) {
        out[i] = Binary(((uint8_t)em[i] > et) || ((uint8_t)dm[i] > dt));
    }
}


std::tuple<Point, Rangei> SmartCar::get_entry(const ImageReadable<Binary> & src){
    auto last_seed_pos = msm.seed_pos;
    auto road_minimal_length = config.valid_width.from;
    auto m_right_align = switches.align_right;
    auto y = last_seed_pos.y ? last_seed_pos.y : src.size.y - config.safety_seed_height;

    //本次找到的x窗口
    Rangei new_x_range;

    // DEBUG_VALUE(last_seed_pos);
    if(last_seed_pos.x == 0){//如果上次没有找到种子 这次就选取最靠近吸附的区域作为种子窗口
        new_x_range = get_side_range(src, y, road_minimal_length, m_right_align);

        if(new_x_range.length() < road_minimal_length){//如果最长的区域都小于路宽 那么就视为找不到种子
            return {Vector2i{}, Rangei{}};
        }

    }else{//如果上次有种子
        new_x_range = get_h_range(src,last_seed_pos);
        //在上次种子的基础上找新窗口

        if(new_x_range.length() < road_minimal_length){//如果最长的区域都小于路宽 那么就视为找不到种子
            return {Vector2i{}, Rangei{}};
        }
    }
    

    if(config.valid_width.has(new_x_range.length())){
    // if(true){

        Point new_seed_pos;

        if(m_right_align){
            new_seed_pos = Vector2i(new_x_range.to - config.align_space_width,y);
        }else{
            new_seed_pos = Vector2i(new_x_range.from + config.align_space_width, y);
        }

        // DEBUG_PRINTLN("found", new_seed_pos);

        if(last_seed_pos.x != 0)//如果上次有找到 那么进行简单的均值滤波 否则直接赋值
            return {(last_seed_pos + new_seed_pos) / 2, new_x_range};
        else
            return {new_seed_pos, new_x_range};
    }

    //不应该运行到这里
    //如果找不到种子 就返回空
    // ASSERT_WITH_DOWN(false, "should not run here");
    return {Vector2i(), Rangei()};
}

void SmartCar::reset(){
    turn_ctrl.reset();
    side_ctrl.reset();
    velocity_ctrl.reset();
    side_velocity_observer.reset();
}

void SmartCar::recordRunStatus(const RunStatus status){
    runStatusReg = uint16_t(uint8_t(status));
}

void SmartCar::printRecordedRunStatus(){
    uint16_t id = runStatusReg;
    auto temp = RunStatus::_from_integral_unchecked(uint16_t(runStatusReg));
    
    powerOnTimesReg = uint16_t(int(powerOnTimesReg) + 1);
    DEBUG_PRINTLN("power on time:", int(powerOnTimesReg));

    DEBUG_PRINTS("last Stat:\t", temp._to_string(), id);
    DEBUG_PRINTS("last Flag:\t", toString(int(uint16_t(flagReg)), 2), id);
    // DEBUG_PRINTLN(temp._to_string(), id);
}


void SmartCar::init_debugger(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    DEBUGGER.setEps(4);
    DEBUGGER.setRadix(10);

}

void SmartCar::init_periphs(){
    bkp.init();
    
    {
        portD[4].outpp(1);
        beep_gpio.outpp(0);
    }

    printRecordedRunStatus();

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
    // chassis_fan.init();
    // vl_fan.init();
    // vr_fan.init();
    // hri_fan.init();
    
    body.init();
    // body.enable(true);
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

void SmartCar::start(){
    switches.hand_mode = false;
    flags.enable_trig = true;
    motor_strength.chassis = full_duty;
    parse();
}


void SmartCar::stop(){
    switches.hand_mode = false;
    flags.disable_trig = true;
    motor_strength.chassis = 0;
    parse();
}

void SmartCar::update_beep(const bool en = true){
    if(en){
        beep_gpio.set();
        delay(100);
        return;
    }

    beep_gpio.clr();
}

void SmartCar::parse(){

    flagReg = flags;
    if(flags.data != 0){//有标志位被置位
        if(flags.enable_trig){
            reset();
            body.enable(true);
            DEBUG_PRINTLN("enabled");
            flags.enable_trig = false;
        }

        if(flags.disable_trig){
            reset();
            body.enable(false);
            // DEBUG_PRINTLN("disabled");
            flags.disable_trig = false;
        }

    }
    update_beep(flags.data != 0);
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
    if(switches.plot_de)\
    return;\

    [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale, Grayscale> & src, const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture_unsafe(area.intersection(Rect2i(area.position, src.get_size())), src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](const Image<Binary, Binary> & src, const Rect2i & area){
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
        for(const auto & pt : pts){
            // painter.drawPixel(pt + pos);
            painter.drawFilledCircle(pt + pos, 2);
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
        plot_vec3(msm.magent.normalized() * 15, {190, 120});

        painter.setColor(RGB565::WHITE);
        
        auto pos = Vector2i{190, 180};

        #define DRAW_STR(str)\
            painter.drawString(pos, str);\
            pos += Vector2i(0, 9);
        
        painter.drawFilledRect(Rect2i(pos, Vector2i{60, 60}),RGB565::BLACK);
        DRAW_STR("自转" + toString(msm.omega));
        DRAW_STR("向差" + toString(msm.dir_error));
        DRAW_STR("侧移" + toString(msm.side_offs_err));
    };

    DEBUGGER.bindRxPostCb([&](){parse_line(DEBUGGER.readString());});

    
    camera.setExposureValue(300);

    #define CREATE_BENCHMARK(val) val = millis() - current_ms;\

    #define CREATE_START_TICK()\
        auto current_ms = millis();\
        {\
            static auto last_ms = current_ms;\
            benchmark.frame_ms = current_ms - last_ms;\
            last_ms = current_ms;\
        }\

    while(true){
        plot_gui();
        CREATE_START_TICK();


        recordRunStatus(RunStatus::NEW_ROUND);

        //----------------------
        //对输入进行解析
        recordRunStatus(RunStatus::PROCESSING_EVENTS);
        if(start_key) start();
        if(stop_key) stop();
        //----------------------

        //----------------------
        //对事件进行解析
        recordRunStatus(RunStatus::PROCESSING_EVENTS);
        parse();
        //----------------------



        /* #region */
        //开始进行图像处理
        recordRunStatus(RunStatus::PROCESSING_IMG_BEGIN);

        CREATE_BENCHMARK(benchmark.cap);
        Geometry::perspective(pers_gray_image, camera);//进行透视变换
        plot_gray(pers_gray_image, Rect2i{0, 60, 188, 60});

        CREATE_BENCHMARK(benchmark.pers);
        Pixels::inverse(pers_gray_image);//对灰度图取反(边线为白色方便提取)

        // fast_bina_opera(
        fast_diff_opera(diff_gray_image, pers_gray_image);//进行加速后的差分算法

        plot_gray(diff_gray_image, Rect2i{0, 0, 188, 60});
        CREATE_BENCHMARK(benchmark.gray);
        Pixels::binarization(pers_bina_image, pers_gray_image, config.edge_threshold);
        CREATE_BENCHMARK(benchmark.bina);

        // plot_bina(pers_bina_image,  Rect2i{Vector2i{0, 180}, Vector2i{188, 60}});

        auto ccd_image = camera.clone({0,73,188,6});
        // Pixels::inverse(ccd_image, ccd_image);
        auto ccd_diff = ccd_image.space();
        auto ccd_bina = make_bina_mirror(ccd_image);

        fast_diff_opera(ccd_diff, ccd_image);//进行加速后的差分算法
        Pixels::binarization(ccd_bina, ccd_diff, config.edge_threshold);
        Shape::dilate_x(ccd_bina, ccd_bina);
        Shape::dilate_x(ccd_bina, ccd_bina);


        plot_gray(ccd_image,  Rect2i{Vector2i{0, 180}, Vector2i{188, 60}});

        recordRunStatus(RunStatus::PROCESSING_IMG_END);
        //图像处理结束
        /* #endregion */


        /* #region */
        // 对种子进行搜索
        recordRunStatus(RunStatus::PROCESSING_SEED_BEG);

        auto & img = pers_gray_image;
        auto & img_bina = pers_bina_image;

        std::tie(msm.seed_pos, msm.road_window) = get_entry(img_bina);
        // if(bool(msm.road_window))


        //如果找不到种子 跳过本次遍历
        if(!msm.seed_pos){
            // DEBUG_PRINTLN("no seed");
            stop();
        }

        auto ccd_range = get_h_range(ccd_bina, Vector2i{msm.seed_pos.x, 0});
        
        plot_point(msm.seed_pos);
        plot_pile({0, ccd_range}, RGB565::FUCHSIA);
        auto ccd_center_x = ccd_range.get_center();

        msm.side_offs_err = -0.005 * (ccd_center_x - 94);
        // -(msm.seed_pos.x - (img.get_size().x / 2) + 2) * 0.02;
    
        // DEBUG_PRINTLN(msm.road_window.length(), msm.road_window, config.valid_width, msm.seed_pos);
        recordRunStatus(RunStatus::PROCESSING_SEED_END);
        //对种子的搜索结束
        /* #endregion */


        /* #region */
        // 寻找两侧的赛道轮廓并修剪为非自交的形式
        recordRunStatus(RunStatus::PROCESSING_COAST);
        
        auto coast_left_unfixed =    CoastUtils::form(img_bina, msm.seed_pos, LEFT);
        auto coast_right_unfixed =   CoastUtils::form(img_bina, msm.seed_pos, RIGHT);

        auto coast_left = CoastUtils::trim(coast_left_unfixed, img.size);
        auto coast_right = CoastUtils::trim(coast_right_unfixed, img.size);
    
        ASSERT(CoastUtils::is_self_intersection(coast_left) == false, "left self ins");
        ASSERT(CoastUtils::is_self_intersection(coast_right) == false, "right self ins");

        ASSERT_WITH_DOWN(coast_left.size() != 0, "left coast size 0");
        ASSERT_WITH_DOWN(coast_right.size() != 0, "right coast size 0");
        recordRunStatus(RunStatus::PROCESSING_COAST_END);
        //修剪结束
        /* #endregion */


        /* #region */
        //开始进行对轮廓进行抽稀
        recordRunStatus(RunStatus::PROCESSING_DP_BEG);

        auto track_left = CoastUtils::douglas_peucker(coast_left, config.dpv);
        auto track_right = CoastUtils::douglas_peucker(coast_right, config.dpv);

        // auto track_left = douglas_peucker(CoastUtils::shrink(coast_left, real_t(-align_space_width)), dpv);
        // auto track_right = douglas_peucker(CoastUtils::shrink(coast_right, real_t(align_space_width)), dpv);

        plot_coast(track_left, {0, 0}, RGB565::RED);
        plot_coast(track_right, {0, 0}, RGB565::BLUE);

        recordRunStatus(RunStatus::PROCESSING_DP_END);
        //对轮廓的抽稀结束
        /* #endregion */



        /* #region */
        //进行角点检测
        recordRunStatus(RunStatus::PROCESSING_CORNER_BEG);

        auto v_points = CoastUtils::vcorners(track_left);
        auto a_points = CoastUtils::acorners(track_left);

        recordRunStatus(RunStatus::PROCESSING_CORNER_END);
        //角点检测结束
        /* #endregion */

        /* #region */
        //开始进行元素识别
        
        //在自动模式下 如果识别不到赛道 就关断小车 避免跑飞时撞墙
        if(msm.road_window.length() < config.valid_width.from){
            if(switches.hand_mode == false){
                stop();
            }
            continue;
        }

        //元素识别结束
        /* #endregion */
        
        /* #region */
        //进行位置提取
        // msm.side_offs_err = -(msm.seed_pos.x - (img.get_size().x / 2) + 2) * 0.02;
        
        //位置提取结束
        /* #endregion */

        /* #region */
        //开始提取轮廓主向量
        recordRunStatus(RunStatus::PROCESSING_VEC_BEG);
        do{
            if(track_left.size() >= 2 and track_right.size() >= 2){//update current_dir
                auto vec_valid = [](const Vector2 & vec){return bool(vec) && vec.length() > 5;}; 

                Segment seg_left = SegmentUtils::shift({
                        track_left.front(), *std::next(track_left.begin())}, 
                        Vector2i(config.align_space_width, 0));

                Segment seg_right = SegmentUtils::shift({
                        track_right.front(), *std::next(track_right.begin())}, 
                        Vector2i(-config.align_space_width, 0));

                plot_segment(seg_left, {0, 0}, RGB565::YELLOW);
                plot_segment(seg_right, {0, 0}, RGB565::GREEN);

                // #define MODIFY_DIR(vec) if(vec) (msm.current_dir = vec.angle())
                //提取根向量
                Vector2 left_root_vec = SegmentUtils::vec(seg_left);
                Vector2 right_root_vec = SegmentUtils::vec(seg_right);
                Vector2 root_vec;

                //如果根向量相似且左右间隔是合法的赛道宽度(排除在识别元素时的干扰) 那么进行根向量合并 否则根据吸附的左右选择对应的根向量
                bool left_valid = vec_valid(left_root_vec);
                bool right_valid = vec_valid(right_root_vec);

                if(left_valid and right_valid and config.valid_width.has(msm.road_window.length())){

                    auto diff_left_l = left_root_vec.length();
                    auto diff_right_l = right_root_vec.length();

                    auto vec_sin = (left_root_vec.cross(right_root_vec)) 
                            / (diff_left_l * diff_right_l);

                    if(vec_sin < abs(config.dir_merge_max_sin)){
                        //计算两者按权重相加的向量（不需要考虑模长）
                        root_vec = left_root_vec * diff_right_l + right_root_vec * diff_left_l;
                    }else{
                        if(switches.align_right){
                            root_vec = right_root_vec;
                        }else{
                            root_vec = left_root_vec;
                        }
                    }
                }else if(left_valid || right_valid){
                    root_vec = left_valid ? left_root_vec : right_root_vec;
                }else{
                    break;
                }

                if (bool(root_vec)) {
                    msm.current_dir = root_vec.angle();
                    // DEBUG_PRINTLN(msm.current_dir);
                }

                plot_segment({msm.seed_pos, msm.seed_pos + Vector2(10.0, 0).rotated(-msm.current_dir)}, {0, 0}, RGB565::PINK);
            }
        }while(false);
        recordRunStatus(RunStatus::PROCESSING_VEC_END);
        //对轮廓的主向量提取结束
        /* #endregion */
    





        // coast_right = douglas_peucker(coast_right, dpv);
        // auto track_right = get_main_coast(CoastUtils::shrink(coast_right, real_t(-15)));
        // plot_coast(track_left, {0, 0}, RGB565::RED);
        // plot_coast(track_right, {0, 0}, RGB565::BLUE);
        // DEBUG_PRINTLN("done plot track");

        // static constexpr Vector2i shapes_offset = Vector2i(0, 0);



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
