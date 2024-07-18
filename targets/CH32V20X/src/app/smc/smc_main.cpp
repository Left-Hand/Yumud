#include "smc.h"

using namespace SMC;
using namespace NVCV2;
using SMC::CoastFinder;
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
    auto last_seed_pos = measurer.seed_pos;
    // auto last_road_window = measurer.road_window;

    Rangei road_valid_pixels = {WorldUtils::pixels(config.valid_road_meters.from),
                                WorldUtils::pixels(config.valid_road_meters.to)};

    real_t road_align_pixels = {WorldUtils::pixels(config.road_width)};
    auto align_mode = switches.align_mode;
    auto align_right = (LR)align_mode;
    auto y = last_seed_pos.y ? last_seed_pos.y : src.size.y - config.seed_height_base;

    //定义本次找到的x窗口
    Rangei new_x_range;
    if(last_seed_pos.x == 0){//如果上次没有找到种子 这次就选取最靠近吸附的区域作为种子窗口

        new_x_range = get_side_range(src, y, road_valid_pixels.from, align_mode);
        // DEBUG_PRINTLN(new_x_range, road_valid_pixels.from);
        //如果最长的区域都小于路宽 那么就视为找不到种子
        if(new_x_range.length() < road_valid_pixels.from){
            return {Vector2i{}, Rangei{}};
            // return {last_seed_pos, last_road_window};
        }

    }else{//如果上次有种子
        new_x_range = get_h_range(src,last_seed_pos);
        //在上次种子的基础上找新窗口

        //如果最长的区域都小于路宽 那么就视为找不到种子
        if(new_x_range.length() < road_valid_pixels.from){
            return {Vector2i{}, Rangei{}};
        }
    }
    //能到这里 说明找到可行的区域了

    if(new_x_range.length() >= road_valid_pixels.from){
        Point new_seed_pos;


        if(align_right){
            new_seed_pos = Vector2i(new_x_range.to - int(road_align_pixels/2),y);
        }else{
            new_seed_pos = Vector2i(new_x_range.from + int(road_align_pixels/2), y);
        }

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
    // body.reset();
}



void SmartCar::printRecordedRunStatus(){
    auto temp = getRunStatus();
    
    powerOnTimesReg = uint16_t(int(powerOnTimesReg) + 1);
    DEBUG_PRINTLN("power on time:", int(powerOnTimesReg));

    DEBUG_PRINTS("last Stat:\t", temp._to_string());
    DEBUG_PRINTS("last Flag:\t", toString(int(uint16_t(flagReg)), 2));
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

    measurer.init();

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
            flags.enable_trig = false;
        }

        if(flags.disable_trig){
            reset();
            body.enable(false);
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

    cali();


    init_it();

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
        plot_vec3(measurer.get_accel().normalized() * 15, {190, 0});
        plot_vec3((measurer.get_gyro()).clampmax(15), {190, 60});
        // plot_vec3(measurer.get_magent().normalized() * 15, {190, 120});

        painter.setColor(RGB565::WHITE);
        
        [[maybe_unused]] auto pos = Vector2i{190, 180};

        #define DRAW_STR(str)\
            painter.drawString(pos, str);\
            pos += Vector2i(0, 9);
        
        // painter.drawFilledRect(Rect2i(pos, Vector2i{60, 60}),RGB565::BLACK);



        // DRAW_STR("自转" + toString(measurer.get_omega()));
        // DRAW_STR("向差" + toString(measurer.dir_error));
        // DRAW_STR("侧移" + toString(measurer.lane_offset * 100));
    };

    DEBUGGER.bindRxPostCb([&](){parse_line(DEBUGGER.readString());});

    
    camera.setExposureValue(500);

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


        recordRunStatus(RunStatus::BEG);

        //----------------------
        //对输入进行解析
        recordRunStatus(RunStatus::EVENTS);
        if(start_key) start();
        if(stop_key) stop();
        //----------------------

        //----------------------
        //对事件进行解析
        recordRunStatus(RunStatus::EVENTS);
        parse();
        //----------------------



        /* #region */
        //开始进行图像处理
        recordRunStatus(RunStatus::IMG_B);

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


        auto ccd_image = camera.clone({0,73,188,6});
        auto ccd_diff = ccd_image.space();
        auto ccd_bina = make_bina_mirror(ccd_image);

        fast_diff_opera(ccd_diff, ccd_image);//进行加速后的差分算法
        Pixels::binarization(ccd_bina, ccd_diff, config.edge_threshold);
        Shape::dilate_x(ccd_bina, ccd_bina);
        Shape::dilate_x(ccd_bina, ccd_bina);

        auto ele_view = measurer.get_view(pers_bina_image);
        plot_bina(ele_view, Rect2i(Vector2i(188,120), ele_view.get_size()));
        recordRunStatus(RunStatus::IMG_E);
        //图像处理结束
        /* #endregion */


        /* #region */
        // 对种子进行搜索
        recordRunStatus(RunStatus::SEED_B);

        auto & img = pers_gray_image;
        auto & img_bina = pers_bina_image;

        std::tie(measurer.seed_pos, measurer.road_window) = get_entry(img_bina);

        //如果找不到种子 跳过本次遍历
        if(!measurer.seed_pos){
            // DEBUG_PRINTLN("no seed");
            stop();
        }

        auto ccd_range = get_h_range(ccd_bina, Vector2i{measurer.seed_pos.x, 0});
        
        plot_point(measurer.seed_pos);
        plot_pile({0, ccd_range}, RGB565::FUCHSIA);

        recordRunStatus(RunStatus::SEED_B);
        //对种子的搜索结束
        /* #endregion */


        /* #region */
        // 寻找两侧的赛道轮廓并修剪为非自交的形式
        recordRunStatus(RunStatus::COAST_B);
        
        auto coast_left_unfixed =    CoastUtils::form(img_bina, measurer.seed_pos, LEFT);
        auto coast_right_unfixed =   CoastUtils::form(img_bina, measurer.seed_pos, RIGHT);

        auto coast_left = CoastUtils::trim(coast_left_unfixed, img.size);
        auto coast_right = CoastUtils::trim(coast_right_unfixed, img.size);
    
        ASSERT(CoastUtils::is_self_intersection(coast_left) == false, "left self ins");
        ASSERT(CoastUtils::is_self_intersection(coast_right) == false, "right self ins");

        ASSERT_WITH_DOWN(coast_left.size() != 0, "left coast size 0");
        ASSERT_WITH_DOWN(coast_right.size() != 0, "right coast size 0");
        recordRunStatus(RunStatus::COAST_E);
        //修剪结束
        /* #endregion */


        /* #region */
        //开始进行对轮廓进行抽稀
        recordRunStatus(RunStatus::DP_B);

        auto track_left = CoastUtils::douglas_peucker(coast_left, config.dpv);
        auto track_right = CoastUtils::douglas_peucker(coast_right, config.dpv);

        plot_coast(track_left, {0, 0}, RGB565::RED);
        plot_coast(track_right, {0, 0}, RGB565::BLUE);

        recordRunStatus(RunStatus::DP_E);
        //对轮廓的抽稀结束
        /* #endregion */



        /* #region */
        //进行角点检测
        recordRunStatus(RunStatus::CORNER_B);

        auto left_a_points = CoastUtils::a_points(track_left);
        auto right_a_points = CoastUtils::a_points(track_right);

        auto left_v_points = CoastUtils::v_points(track_left);
        auto right_v_points = CoastUtils::v_points(track_right);

        plot_points(left_a_points, {0, 0}, RGB565::AQUA);
        plot_points(left_v_points, {0, 0}, RGB565::YELLOW);

        plot_points(right_a_points, {0, 0}, RGB565::AQUA);
        plot_points(right_v_points, {0, 0}, RGB565::YELLOW);

        recordRunStatus(RunStatus::CORNER_L);
        const auto left_corners = CoastUtils::search_corners(track_left, CornerType::ALL);
        recordRunStatus(RunStatus::CORNER_R);
        const auto right_corners = CoastUtils::search_corners(track_right, CornerType::ALL);
        // DEBUG_PRINTLN(left_corners.size(), right_corners.size());
    
        recordRunStatus(RunStatus::CORNER_E);
        //角点检测结束
        /* #endregion */




        /* #region */
        //开始进行元素识别
        [[maybe_unused]]auto sign_with_dead_zone = [](const int a, const int b, const int zone){
            auto diff = a-b;
            if(std::abs(diff) < std::abs(zone)) return 0;
            return sign(diff);
        };

        [[maybe_unused]] auto zebra_beg_detect = [&]() -> DetectResult {
            return {false};
        };

        [[maybe_unused]]auto barrier_beg_detect = [&]() -> DetectResult {
           [[maybe_unused]] auto side_barrier_detect = [&](const Corners & _corners, const LR side) -> Vector2i {
                //少于两个拐点 没法判断有没有障碍
                if(_corners.size() < 2) return {0,0};

                //从起点开始搜索首次出现的a角点和v角点
                const auto * a_corner_ptr = CornerUtils::find_a(_corners);
                // const Corner * a_corner_ptr = nullptr;
                const auto * v_corner_ptr = CornerUtils::find_v(_corners);

                //能够找到a角点和v角点
                if(bool(a_corner_ptr) && bool(v_corner_ptr)){
                    static constexpr int ignore_y = 40;
                    if(Vector2i(*a_corner_ptr).y > ignore_y) return {0,0};

                    static constexpr int max_y_diff = 5;
                    static constexpr int min_x_diff = 7;

                    Vector2i diff = Vector2i(v_corner_ptr->point) - Vector2i(a_corner_ptr->point);
                    // DEBUG_PRINTLN(diff);
                    //A角点出现在V角点之前
                    if(a_corner_ptr < v_corner_ptr){
                        
                        if((std::abs(diff.y) < max_y_diff)
                            && (std::abs(diff.x) > min_x_diff)
                            && (side == LR::LEFT ? diff.x > 0 : diff.x < 0)
                        ){
                            return *a_corner_ptr;
                        }
                    }
                }
                
                return {0,0};
            };

            Vector2i left_detected = side_barrier_detect(left_corners, LR::LEFT);
            Vector2i right_detected = side_barrier_detect(right_corners, LR::RIGHT);

            static constexpr int least_y_diff = 20;

            if(left_detected && right_detected) {
                switch(sign_with_dead_zone(left_detected.y, right_detected.y,least_y_diff)){
                    case 1:
                        goto use_left;
                    case -1:
                        goto use_right;
                    default:
                        goto unknown;
                }
            }else{
                if(left_detected) goto use_left;
                if(right_detected) goto use_right;

                //not detected
                return {false};
            }

            unknown:
                ASSERT_WITH_DOWN(false, "detected dual barrier");
                return {false};
            use_left:
                return {true, LR::LEFT};
            use_right:
                return {true, LR::RIGHT};
        };

        [[maybe_unused]]auto straight_detect = [&]() -> DetectResult {
            return {(coast_left.size() == 2) && (coast_left.size() == 2)};
        };

        [[maybe_unused]]auto none_detect = [&]() -> DetectResult {
            return {(coast_left.size() <= 2) && (coast_left.size() <= 2)};
        };

        [[maybe_unused]]auto curve_detect = [&]() -> DetectResult {
            [[maybe_unused]] auto side_curve_detect = [&](const Corners & _corners, const LR side) -> bool {
                // auto sign =  CoastUtils::sigle(coast);
                return true;
            };

            return {false};//TODO
        };

        [[maybe_unused]]auto cross_beg_detect = [&]() -> DetectResult {
            [[maybe_unused]] auto side_cross_entry_detect = [&](const Corners & _corners, const LR side) -> bool {
                switch(side){
                    case LEFT:
                        return (CornerUtils::find_a(_corners) == _corners.begin());
                    case RIGHT:
                        return (CornerUtils::find_a(_corners) == _corners.begin());
                    default:
                        return 0;
                }
            };

            //FIXME

            bool left_detected = side_cross_entry_detect(left_corners, LR::LEFT);
            bool right_detected = side_cross_entry_detect(right_corners, LR::RIGHT);

            return (left_detected && right_detected);
        };

        [[maybe_unused]]auto cross_end_detect = [&]() -> DetectResult {
            [[maybe_unused]] auto side_cross_leave_detect = [&](const Corners & _corners, const LR side) -> bool {
                switch(side){
                    case LEFT:
                    case RIGHT:{
                        const auto * first_corner_ptr = CornerUtils::find_corner(_corners);

                        //没有拐点了 那就可以 
                        if(first_corner_ptr == nullptr) return true;

                        const auto & first_point = *first_corner_ptr;

                        //十字后
                        if(first_point.point.y < 30) return true;

                        return false;
                    }
                    default:
                        return false;
                }
            };

            //FIXME 应该使用上点而不是下点
            bool left_detected = side_cross_leave_detect(left_corners, LR::LEFT);
            bool right_detected = side_cross_leave_detect(right_corners, LR::RIGHT);

            return (left_detected && right_detected);
        };

        [[maybe_unused]]auto ring_beg_detect = [&]() -> DetectResult {
            [[maybe_unused]] auto side_ring_entry_detect = [&](const Corners & _corners, const LR side) -> bool {
                switch(side){
                    case LEFT:
                        return (CornerUtils::find_a(_corners) == _corners.begin());
                    case RIGHT:
                        return (CornerUtils::find_a(_corners) == _corners.begin());
                    default:
                        return 0;
                }
            };

            bool left_detected = side_ring_entry_detect(left_corners, LR::LEFT);
            bool right_detected = side_ring_entry_detect(right_corners, LR::RIGHT);

            if(left_detected ^ right_detected){
                //判断只有一边有A角点

                return true;
            }
        };

        [[maybe_unused]]auto ring_in_detect = [&](const LR known_side) -> DetectResult {
            [[maybe_unused]] auto side_ring_in_detect = [&](const Corners & _corners, const LR _side, const bool is_expected_side) -> bool {
                if(is_expected_side){
                    return CoastUtils::is_single(_side == LR::RIGHT ? coast_right : coast_left, _side);
                }else{
                    //对立边应该是直道
                    return _corners.size() == 0;
                }
            };

            bool left_detected = side_ring_in_detect(left_corners, known_side, known_side == LR::LEFT);
            bool right_detected = side_ring_in_detect(right_corners, known_side, known_side == LR::RIGHT);

            return (left_detected && right_detected);
        };

        [[maybe_unused]]auto ring_running_detect = curve_detect;

        [[maybe_unused]]auto ring_out_detect = [&](const LR known_side) -> DetectResult {
            return ring_beg_detect().side != known_side;
        };

        [[maybe_unused]]auto ring_end_detect = [&](const LR known_side) -> DetectResult {
            // return ring_beg_detect().side != known_side;
            return true;
        };
    
        [[maybe_unused]]auto ring_exit_detect = [&](const LR known_side) -> DetectResult {
            // return ring_beg_detect().side != known_side;
            return false;
        };

        #define RESULT_GETTER(x) update_detect(x)


        [[maybe_unused]] auto side_to_align = [](const LR side) -> AlignMode{
            return (side == LR::LEFT) ? AlignMode::LEFT : AlignMode::RIGHT;
        };

        [[maybe_unused]] auto co_side_to_align = [](const LR side) -> AlignMode{
            return (side == LR::RIGHT) ? AlignMode::LEFT : AlignMode::RIGHT;
        };

        recordRunStatus(RunStatus::ELEMENT_B);
    
        switch(switches.element_type){
            case ElementType::NONE:
            case ElementType::STRAIGHT:
                {
                    //判断何时处理 状态机 of 斑马线
                    if(false){
                        auto result = RESULT_GETTER(zebra_beg_detect());
                        if(result){
                            sw_element(ElementType::CROSS, (uint8_t)Cross::Status::BEG, result.side);
                            sw_align(AlignMode::UPPER);
                        }
                    }

                    //判断何时处理 状态机 of 障碍物
                    if(true){
                        auto result = RESULT_GETTER(barrier_beg_detect());
                        if(result){
                            sw_element(ElementType::BARRIER, (uint8_t)Barrier::Status::BEG, result.side);
                            sw_align(co_side_to_align(result.side));
                        }
                    }

                    //判断何时处理 状态机 of 十字
                    if(false){
                        auto result = RESULT_GETTER(cross_beg_detect());
                        if(result){
                            sw_element(ElementType::CROSS, (uint8_t)Cross::Status::BEG, result.side);
                            sw_align(AlignMode::UPPER);
                        }
                    }

                    //判断何时处理 状态机 of 圆环
                    if(false){
                        auto result = RESULT_GETTER(ring_beg_detect());
                        if(result){
                            sw_element(ElementType::RING, (uint8_t)Cross::Status::BEG, result.side);
                            sw_align(co_side_to_align(result.side));
                        }
                    }

                }
                break;
            
            //已经处于斑马线元素状态
            case ElementType::ZEBRA:
                {
                    using ZebraStatus = Zebra::Status;
                    auto zebra_status = switches.zebra_status;
                    switch(zebra_status) {
                        //判断何时退出斑马线状态

                        case ZebraStatus::BEG: if(false){
                            auto result = RESULT_GETTER(zebra_beg_detect());
                            if(result){
                                sw_element(ElementType::NONE, (uint8_t)(ZebraStatus::END), result.side);
                                sw_align(AlignMode::BOTH);
                            }
                        }break;

                        default:
                            break;
                    }
                }
                break;

            //已经处于障碍物元素状态
            case ElementType::BARRIER:
                {
                    using BarrierStatus = Barrier::Status;
                    auto barrier_status = switches.barrier_status;
                    switch(barrier_status) {
                        //判断何时退出障碍物状态
                        case BarrierStatus::END:if(true){
                            auto result = RESULT_GETTER(barrier_beg_detect());
                            if(result){
                                sw_element(ElementType::NONE, (uint8_t)BarrierStatus::END, result.side);
                                sw_align(AlignMode::BOTH);
                            }
                        }break;
                        default:
                            break;
                    }
                }
                break;

            //已经处于十字元素状态
            case ElementType::CROSS:
                {
                    using CrossStatus = Cross::Status;
                    auto cross_status = switches.cross_status;
                    switch(cross_status){
                        //判断何时退出十字状态
                        case CrossStatus::END:if(false){
                            auto result = RESULT_GETTER(cross_beg_detect());
                            if(result){
                                sw_element(ElementType::NONE, (uint8_t)CrossStatus::END, result.side);
                                sw_align(AlignMode::BOTH);
                            }
                        }break;
                        default:
                            break;
                    }
                }
                break;

            //已经处于圆环状态
            case ElementType::RING:
                {
                    using RingStatus = Ring::Status;
                    auto ring_status = switches.ring_status;
                    auto ring_side = switches.element_side;
                    switch(ring_status){
                        //判断何时入环
                        case RingStatus::BEG:if(false){
                            auto result = RESULT_GETTER(ring_in_detect(ring_side));
                            if(result){
                                sw_element(ElementType::RING, (uint8_t)RingStatus::IN, result.side);
                                sw_align(side_to_align(ring_side));
                            }
                        }break;

                        //判断何时进环
                        case RingStatus::IN: if(false){
                            auto result = RESULT_GETTER(ring_running_detect());
                            if(result){
                                sw_element(ElementType::RING, (uint8_t)RingStatus::RUNNING, result.side);
                                sw_align(co_side_to_align(ring_side));
                            }
                        }break;

                        //判断何时出环
                        case RingStatus::RUNNING: if(false){
                            auto result = RESULT_GETTER(ring_out_detect(ring_side));
                            if(result){
                                sw_element(ElementType::RING, (uint8_t)RingStatus::OUT, result.side);
                                sw_align(side_to_align(ring_side));
                            }
                        }break;

                        //判断何时退出圆环
                        case RingStatus::OUT: if(false){
                            auto result = RESULT_GETTER(ring_end_detect(ring_side));
                            if(result){
                                sw_element(ElementType::RING, (uint8_t)RingStatus::END, result.side);
                                sw_align(co_side_to_align(ring_side));
                            }
                        }break;

                        case RingStatus::END: if(false){
                            auto result = RESULT_GETTER(ring_exit_detect(ring_side));
                            if(result){
                                sw_element(ElementType::NONE, (uint8_t)RingStatus::END, result.side);
                                sw_align(AlignMode::BOTH);
                            }
                        }break;
                    }
                }
                break;

            default:
                break;
        }
        //在自动模式下 如果识别不到赛道 就关断小车 避免跑飞时撞墙
        // DEBUG_VALUE(measurer.road_window.length())
        if(measurer.get_road_length_meters() < config.valid_road_meters.start){
            if(switches.hand_mode == false){
                stop();
            }
            continue;
        }

        recordRunStatus(RunStatus::ELEMENT_E);

        //元素识别结束
        /* #endregion */
        
        /* #region */
        //进行位置提取
        measurer.update_ccd(ccd_range);
        //位置提取结束
        /* #endregion */


        /* #region */
        //进行引导修正
        if(false){
            [[maybe_unused]]auto coast_fix_lead = [](const Coast & coast, const LR is_right) -> Coast{
                static constexpr real_t k = 1.2;

                auto coast_point_valid = [](const CoastItem & host, const CoastItem & guest, const LR _is_right) -> bool{
                    Vector2i delta = Vector2i(guest) - Vector2i(host);

                    //如果两个点坐标一致 那么通过
                    if(delta.x == 0 && delta.y == 0) return true;

                    int abs_dx = std::abs(delta.x);

                    //快速排除不需要排除的区域
                    if(delta.y < 0 ||
                        ((_is_right == LR::RIGHT) && (delta.x < 0)) ||
                        ((_is_right == LR::LEFT ) && (delta.x > 0))) return true;

                    [[maybe_unused]] int abs_dy = std::abs(delta.y);

                    return (abs_dy > k * abs_dx);

                };

                if(coast.size() == 0) return {};
                if(coast.size() == 1) return {coast.front()};

                std::array<bool, max_item_size> remove_indexes;
                remove_indexes.fill(false);

                for(auto it = coast.begin(); it!= coast.end(); ++it){
                    const auto & host = *it;
                    for(size_t j = 0; j < coast.size(); ++j){
                        const auto & guest = coast[j];

                        //已经加到列表 直接跳过
                        if(remove_indexes[j] == true) continue;
                        
                        //还不在列表内 有待考证
                        bool abandon = coast_point_valid(host, guest, is_right) == false;
                        // bool abandon = false;

                        //如果需要丢弃 添加到列表
                        remove_indexes[j] |= abandon;
                    }
                };

                Coast ret;


                //对初点做特殊讨论
                //此处以返回值的第二个点为参考

                if(remove_indexes[0] == true){
                    size_t secondary_index = 0;
                    for(size_t i = 1; i < coast.size(); ++i){
                        if(remove_indexes[i] == false){
                            secondary_index = i;
                            break;
                        }
                    }

                    if(secondary_index){
                        const auto & secondary_point = coast[secondary_index];
                        const auto & orignal_first_point = coast[0];

                        auto vec_go_until_y = [&](const Point & start_p, const Vector2i vec, const int y) -> Point{

                            if(vec.y == 0) return start_p;
                            else return (start_p + vec * (y - start_p.y) / vec.y);
                        };

                        auto replaced_first_point = vec_go_until_y(secondary_point, is_right == RIGHT ? Vector2i{1, k} : Vector2i{-1, k}, orignal_first_point.y);

                        ret.push_back(replaced_first_point);
                        for(size_t i = 1; i < coast.size(); ++i){
                            if(remove_indexes[i] == false){
                                ret.push_back(coast[i]);
                            }
                        }


                        return ret;
                    }else{
                        //第一个点不可取 后面也找不到可取点 等价与返回空

                        return {};
                    }
                }else{
                    for(size_t i = 0; i < coast.size(); ++i){
                        ret.push_back(coast[i]);
                    }
                }
                return ret;
            };

            track_left = coast_fix_lead(track_left, LR::LEFT);
            track_right = coast_fix_lead(track_right, LR::RIGHT);

            plot_coast(track_left, {0, 0}, RGB565::GREEN);
            plot_coast(track_right, {0, 0}, RGB565::CORAL);
        };

        //引导修正结束
        /* #endregion */

        /* #region */
        //开始提取轮廓主向量
        recordRunStatus(RunStatus::VEC_B);
        if(true){
            do{
                if(track_left.size() >= 2 and track_right.size() >= 2){
                    auto vec_valid = [](const Vector2 & vec){return bool(vec) && vec.length() > 5;}; 
                    auto road_align_pixels = WorldUtils::pixels(config.road_width);
                    Segment seg_left = SegmentUtils::shift({
                            track_left.front(), *std::next(track_left.begin())}, 
                            Vector2i(int(road_align_pixels/2), 0));

                    Segment seg_right = SegmentUtils::shift({
                            track_right.front(), *std::next(track_right.begin())}, 
                            Vector2i(-int(road_align_pixels/2), 0));

                    // plot_segment(seg_left, {0, 0}, RGB565::YELLOW);
                    // plot_segment(seg_right, {0, 0}, RGB565::GREEN);

                    //提取根向量
                    Vector2 left_root_vec = SegmentUtils::vec(seg_left);
                    Vector2 right_root_vec = SegmentUtils::vec(seg_right);
                    Vector2 root_vec;

                    //如果根向量相似且左右间隔是合法的赛道宽度(排除在识别元素时的干扰) 那么进行根向量合并 否则根据吸附的左右选择对应的根向量
                    bool left_valid = vec_valid(left_root_vec);
                    bool right_valid = vec_valid(right_root_vec);
                    bool road_valid = config.valid_road_meters.has(measurer.get_road_length_meters());

                    switch(switches.align_mode){
                        case AlignMode::BOTH:
                            if(left_valid and right_valid and road_valid){
                                goto use_merged_vec;
                            }else{
                                if(left_valid){
                                    goto use_left_vec;
                                }
                                
                                if(right_valid){
                                    goto use_right_vec;
                                }

                                goto none_vec;
                            }
                        case AlignMode::LEFT:
                            if(left_valid){
                                goto use_left_vec;
                            }
                        case AlignMode::RIGHT:
                            if(right_valid){
                                goto use_right_vec;
                            }
                        default:
                            goto none_vec;
                    }
                    

                    do{
                        use_merged_vec:{
                            auto diff_left_l = left_root_vec.length();
                            auto diff_right_l = right_root_vec.length();

                            auto vec_sin = (left_root_vec.cross(right_root_vec)) 
                                    / (diff_left_l * diff_right_l);


                            if(vec_sin < abs(config.dir_merge_max_sin)){
                                //计算两者按权重相加的向量（不需要考虑模长）
                                root_vec = left_root_vec * diff_right_l + right_root_vec * diff_left_l;
                            }else{
                                if(left_valid){
                                    goto use_left_vec;
                                }
                                
                                if(right_valid){
                                    goto use_right_vec;
                                }
                            }
                            break;
                        }
                        use_left_vec:{
                            root_vec = left_root_vec;
                            break;
                        }
                        use_right_vec:{
                            root_vec = right_root_vec;
                            break;
                        }
                        none_vec:
                            break;
                    }while(false);

                    if (bool(root_vec)) {
                        measurer.update_dir(root_vec.angle());
                    }

                    plot_segment({measurer.seed_pos, measurer.seed_pos + Vector2(10.0, 0).rotated(-measurer.get_dir())}, {0, 0}, RGB565::PINK);
                }
            }while(false);
        }
        recordRunStatus(RunStatus::VEC_E);
        //对轮廓的主向量提取结束
        /* #endregion */
    

        // plot_coast(track_left, {0, 0}, RGB565::RED);
        // plot_coast(track_right, {0, 0}, RGB565::BLUE);

        
        {
            plot_sketch({0, 120, 188,60});
            sketch.fill(RGB565::BLACK);
        }
        // DEBUG_PRINTLN("!!");
 

        // plot_bound(bound_center, {0, 0}, RGB565::PINK);
        // plot_bound(bound_right, {0, 0}, RGB565::BLUE);
        // plot_bound(bound_center, {0, 0}, RGB565::YELLOW);

        recordRunStatus(RunStatus::END);

    }
}



void smc_main(){
    SmartCar car;
    car.main();
};
