#include "smc.h"

using namespace ymd::nvcv2;
using SMC::CoastFinder;
using ymd::nvcv2::Shape::Seed;


namespace SMC{

std::tuple<Point, Rangei> SmartCar::get_entry(const ImageReadable<Binary> & src){
    auto last_seed_pos = measurer.seed_pos;
    // auto last_road_window = measurer.road_window;

    Rangei road_valid_pixels = {WorldUtils::pixels(config.valid_road_meters.from),
                                WorldUtils::pixels(config.valid_road_meters.to)};

    real_t road_align_pixels = {WorldUtils::pixels(config.road_width)};
    auto align_mode = switches.align_mode;
    auto align_right = (LR)align_mode;
    auto y = last_seed_pos.y ? last_seed_pos.y : src.get_size().y - config.seed_height_base;

    //定义本次找到的x窗口
    Rangei new_x_range;
    if(last_seed_pos.x == 0){//如果上次没有找到种子 这次就选取最靠近吸附的区域作为种子窗�?

        new_x_range = get_side_range(src, y, road_valid_pixels.from, align_mode);
        // DEBUG_PRINTLN(new_x_range, road_valid_pixels.from);
        //如果最长的区域都小于路�? 那么就视为找不到种子
        if(new_x_range.length() < road_valid_pixels.from){
            return {Vector2i{}, Rangei{}};
            // return {last_seed_pos, last_road_window};
        }

    }else{//如果上次有种�?
        new_x_range = get_h_range(src,last_seed_pos);
        //在上次种子的基础上找新窗�?

        //如果最长的区域都小于路�? 那么就视为找不到种子
        if(new_x_range.length() < road_valid_pixels.from){
            return {Vector2i{}, Rangei{}};
        }
    }
    //能到这里 说明找到可行的区域了
    is_blind = (new_x_range.length() >= road_valid_pixels.to) && (switches.element_type == ElementType::CROSS);
    // is_blind = false;
    if(new_x_range.length() >= road_valid_pixels.from){
        Point new_seed_pos;


        if(align_right){
            new_seed_pos = Vector2i(new_x_range.to - int(road_align_pixels/2),y);
        }else{
            new_seed_pos = Vector2i(new_x_range.from + int(road_align_pixels/2), y);
        }

        if(last_seed_pos.x != 0)//如果上次有找�? 那么进行简单的均值滤�? 否则直接赋�?
            return {(last_seed_pos + new_seed_pos) / 2, new_x_range};
        else
            return {new_seed_pos, new_x_range};
    }

    //不应该运行到这里
    //如果找不到种�? 就返回空
    // ASSERT_WITH_HALT(false, "should not run here");
    return {Vector2i(), Rangei()};
}

void SmartCar::reset(){
    turn_ctrl.reset();
    side_ctrl.reset();
    velocity_ctrl.reset();
    side_velocity_observer.reset();
    element_holder.reset();
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
    DEBUGGER.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_radix(10);

}

void SmartCar::init_periphs(){
    bkp.init();
    
    {
        portD[4].outpp(1);
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
    started = true;
    parse();
}


void SmartCar::stop(){
    switches.hand_mode = false;
    flags.disable_trig = true;
    motor_strength.chassis = 0;
    started = false;
    parse();
}

void SmartCar::update_holder(){
    // DEBUG_PRINTLN("update_holder");
    element_holder.update();
    // DEBUG_PRINTLN("update_holder end");
}

void SmartCar::parse(){

    flagReg = flags;
    if(flags.data != 0){//有标志位被置�?
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
    flagReg = flags;
};

void SmartCar::init_it(){
    GenericTimer & timer = timer2;
    timer.init(ctrl_freq);
    timer.bindCb(TimerIT::Update, [this](){this->ctrl();});
    timer.enableIt(TimerIT::Update, NvicPriority{0,0});
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
    element_holder.reset();

    constexpr auto pic_size = Vector2i(188, 60);
    auto pers_gray_image = make_image<Grayscale>(pic_size);
    auto pers_bina_image  = make_bina_mirror(pers_gray_image);
    auto diff_gray_image = make_image<Grayscale>(pic_size);
    auto diff_bina_image = make_bina_mirror(diff_gray_image);
    auto sketch = make_image<RGB565>(pic_size);

    #define CHECK_PLOTDE()\
    if(switches.plot_de)\
    return;\

    [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale> & src, const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture(area.intersection(Rect2i(area.position, src.get_size())), src.get_data());
    };

    [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture(area, src.get_data());
    };

    
    [[maybe_unused]] auto plot_sketch = [&](const Rect2i & area){
        CHECK_PLOTDE();
        painter.bindImage(tftDisplayer);
        tftDisplayer.puttexture(area, sketch.get_data());
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

    [[maybe_unused]] auto plot_segment = [&](const Segment seg,  const Vector2i & pos, const RGB565 & color = RGB565::RED){


        painter.bindImage(sketch);
        painter.setColor(color);
        
        painter.drawLine(seg.first + pos, seg.second + pos);
    };

    [[maybe_unused]] auto plot_points = [&](const Points & pts,  const Vector2i & pos, const RGB565 & color = RGB565::PINK){
        painter.bindImage(sketch);
        painter.setColor(color);
        for(const auto & pt : pts){
            painter.drawFilledCircle(pt + pos, 2);
        }
    };


    [[maybe_unused]] auto plot_coast_points = [&](const Coast & pts,  const Vector2i & pos, const RGB565 & color = RGB565::PINK){
        painter.bindImage(sketch);
        painter.setColor(color);
        for(const auto & pt : pts){
            painter.drawFilledCircle(pt + pos, 2);
        }
    };

    [[maybe_unused]] auto plot_bound = [&](const Boundry & bound,  const Vector2i & pos, const RGB565 & color = RGB565::RED){
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
        painter.drawLine(Vector2i{bound.second.from, bound.first}, {bound.second.to, bound.first});
    }; 


    [[maybe_unused]] auto plot_vec3 = [&](const Vector3 & vec3,  const Vector2i & pos){
        scexpr auto square_length = 50;
        auto arm_length = vec3.length();

        scexpr auto radius = 3;
        scexpr auto x_unit = Vector2(1, 0);
        scexpr auto y_unit = Vector2(0.5, -0.73);
        scexpr auto z_unit = Vector2(0, -1);

        auto x_axis = Vector3(arm_length, 0, 0);
        auto y_axis = Vector3(0, arm_length, 0);
        auto z_axis = Vector3(0, 0, arm_length);

        scexpr auto x_color = RGB565::RED;
        scexpr auto y_color = RGB565::GREEN;
        scexpr auto z_color = RGB565::BLUE;
        scexpr auto bg_color = RGB565::BLACK;

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
        plot_vec3(measurer.get_acc().normalized() * 15, {190, 0});
        // plot_vec3((measurer.get_gyr()).clampmax(15), {190, 60});
        // plot_vec3(measurer.get_magent().normalized() * 15, {190, 120});

        painter.setColor(RGB565::WHITE);
        
        [[maybe_unused]] auto pos = Vector2i{190, 180};

        #define DRAW_STR(str)\
            painter.drawString(pos, str);\
            pos += Vector2i(0, 9);
        
        painter.drawFilledRect(Rect2i(pos, Vector2i{60, 60}),RGB565::BLACK);



        DRAW_STR("元素" + toString(int(switches.element_type)));
        DRAW_STR("子状" + toString(int(switches.element_status)));
        DRAW_STR("元侧" + toString(int(switches.element_side)));
        DRAW_STR("附模" + toString(int(switches.align_mode)));
        DRAW_STR("�?:" + toString(int(motor_strength.left * 99)));
        DRAW_STR("�?:" + toString(int(motor_strength.right * 99)));
        DRAW_STR("�?:" + toString(int(motor_strength.hri * 99)));
    };

    DEBUGGER.onRxDone([&](){parse_line(DEBUGGER.readString());});

    
    camera.setExposureValue(1000);

    delay(500);

    #define CREATE_BENCHMARK(val) val = millis() - current_ms;\

    #define CREATE_START_TICK()\
        auto current_ms = millis();\
        {\
            static auto last_ms = current_ms;\
            benchmark.frame_ms = current_ms - last_ms;\
            last_ms = current_ms;\
        }\

    while(true){
        recordRunStatus(RunStatus::GUI);
        {
            plot_sketch({0, 120, 188,60});
            sketch.fill(RGB565::BLACK);
        }

        plot_gui();


        recordRunStatus(RunStatus::BEG);

        CREATE_START_TICK();



        //----------------------
        //对输入进行解�?
        recordRunStatus(RunStatus::EVENTS);
        if(start_key) start();
        if(stop_key) stop();
        //----------------------

        //----------------------
        //对事件进行解�?
        recordRunStatus(RunStatus::EVENTS);
        parse();
        //----------------------



        /* #region */
        //开始进行图像处�?
        recordRunStatus(RunStatus::IMG_B);

        CREATE_BENCHMARK(benchmark.cap);
        Geometry::perspective(pers_gray_image, camera);//进行透视变换
        plot_gray(pers_gray_image, Rect2i{0, 60, 188, 60});

        CREATE_BENCHMARK(benchmark.pers);
        Pixels::inverse(pers_gray_image);//对灰度图取反(边线为白色方便提�?)

        // fast_bina_opera(
        Pixels::fast_diff_opera(diff_gray_image, pers_gray_image);//进行加速后的差分算�?

        // plot_gray(diff_gray_image, Rect2i{0, 0, 188, 60});
        CREATE_BENCHMARK(benchmark.gray);
        Pixels::binarization(pers_bina_image, pers_gray_image, config.edge_threshold);
        CREATE_BENCHMARK(benchmark.bina);
        plot_bina(pers_bina_image, Rect2i{0, 0, 188, 60});


        auto ccd_image = camera.clone({0,73,188,6});
        auto ccd_diff = ccd_image.space();
        auto ccd_bina = make_bina_mirror(ccd_image);

        Pixels::fast_diff_opera(ccd_diff, ccd_image);//进行加速后的差分算�?
        Pixels::binarization(ccd_bina, ccd_diff, config.edge_threshold);
        Shape::anti_pepper_x(ccd_bina, ccd_bina);
        Shape::anti_pepper_x(ccd_bina, ccd_bina);

        recordRunStatus(RunStatus::IMG_E);
        //图像处理结束
        /* #endregion */



        /* #region */
        // 对种子进行搜�?
        recordRunStatus(RunStatus::SEED_B);

        auto & img = pers_gray_image;
        auto & img_bina = pers_bina_image;

        Vector2i new_seed_pos;
        Rangei new_road_window;
        std::tie(measurer.seed_pos, measurer.road_window) = get_entry(img_bina);

        auto ccd_range = get_h_range(ccd_bina, Vector2i{measurer.seed_pos.x, 0});
        // if(ccd_range){
        //     if(bool(new_seed_pos) == false || img_bina[new_seed_pos] == false){
        //         new_seed_pos = {ccd_range.get_center(), 60 - config.seed_height_base};
        //     }
        // }
        // if(new_seed_pos && new_road_window) measurer.update_seed_pos(new_seed_pos, new_road_window);

        // DEBUG_PRINTLN(ccd_range);
        // plot_point(measurer.get_seed_pos);
        plot_pile({0, ccd_range}, RGB565::FUCHSIA);

        recordRunStatus(RunStatus::SEED_B);
        //对种子的搜索结束
        /* #endregion */


        /* #region */
        // 寻找两侧的赛道轮廓并修剪为非自交的形�?
        recordRunStatus(RunStatus::COAST_B);
        
        // {

        auto get_coast = [&](const Vector2i & seed_pos, const LR side) -> Coast{
            auto ret = CoastUtils::form(img_bina, seed_pos, side);
            ret = CoastUtils::trim(ret, img.get_size());
            return ret;
        };

        auto left_coast = get_coast(measurer.seed_pos, LEFT);
        auto right_coast = get_coast(measurer.seed_pos, RIGHT);
        // auto right_coast = CoastUtils::trim(coast_right_unfixed, img.size);
    
        // ASSERT(CoastUtils::is_self_intersection(left_coast) == false, "left self ins");
        // ASSERT(CoastUtils::is_self_intersection(right_coast) == false, "right self ins");

        // ASSERT_WITH_HALT(left_coast.size() != 0, "left coast size 0");
        // ASSERT_WITH_HALT(right_coast.size() != 0, "right coast size 0");
        recordRunStatus(RunStatus::COAST_E);
        //修剪结束
        /* #endregion */


        /* #region */
        //开始进行对轮廓进行抽稀
        recordRunStatus(RunStatus::DP_B);

        auto left_track = CoastUtils::douglas_peucker(left_coast, config.dpv);
        auto right_track = CoastUtils::douglas_peucker(right_coast, config.dpv);

        plot_coast(left_track, {0, 0}, RGB565::RED);
        plot_coast(right_track, {0, 0}, RGB565::BLUE);

        recordRunStatus(RunStatus::DP_E);
        //对轮廓的抽稀结束
        /* #endregion */



        /* #region */

        //进行角点检�?
        recordRunStatus(RunStatus::CORNER_B);

        recordRunStatus(RunStatus::CORNER_L);
        const auto left_corners = CoastUtils::search_corners(left_track, CornerType::ALL);
        recordRunStatus(RunStatus::CORNER_R);
        const auto right_corners = CoastUtils::search_corners(right_track, CornerType::ALL);

        // DEBUG_PRINTLN(left_corners.size(), right_corners.size());

        auto left_a_points = CornerUtils::a_points(left_corners);
        auto right_a_points = CornerUtils::a_points(right_corners);

        auto left_v_points = CornerUtils::v_points(left_corners);
        auto right_v_points = CornerUtils::v_points(right_corners);

        plot_coast_points(left_a_points, {0, 0}, RGB565::AQUA);
        plot_coast_points(left_v_points, {0, 0}, RGB565::YELLOW);

        plot_coast_points(right_a_points, {0, 0}, RGB565::AQUA);
        plot_coast_points(right_v_points, {0, 0}, RGB565::YELLOW);


        // DEBUG_PRINTLN(left_corners.size(), right_corners.size());
    
        recordRunStatus(RunStatus::CORNER_E);
        //角点检测结�?
        /* #endregion */




        /* #region */

    
        //开始进行元素识�?
        [[maybe_unused]]auto sign_with_dead_zone = [](const int a, const int b, const int zone){
            auto diff = a-b;
            if(std::abs(diff) < std::abs(zone)) return 0;
            return sign(diff);
        };

        [[maybe_unused]]auto straight_detect = [&]() -> DetectResult {
            return {(left_corners.size() == 2) && (right_corners.size() == 2)};
        };

        [[maybe_unused]]auto none_detect = [&]() -> DetectResult {
            // DEBUG_PRINTLN(left_corners.size(), right_corners.size());
            return {(left_corners.size() <= 2) && (right_corners.size() <= 2)};
        };

        [[maybe_unused]] auto zebra_beg_detect = [&]() -> DetectResult {
            auto edges = get_x_edges(img_bina, 18);
            edges = std::max(get_x_edges(img_bina, 15), edges);
            edges = std::max(get_x_edges(img_bina, 12), edges);
            edges = std::max(get_x_edges(img_bina, 9), edges);
            edges = std::max(get_x_edges(img_bina, 6), edges);
            // DEBUG_PRINTLN(edges);
            return(edges > zebra_threshold);
            // auto edges = get_x_edges(img, 18);
            // edges = std::max(get_x_edges(img, 15), edges);
            // edges = std::max(get_x_edges(img, 12), edges);
            // edges = std::max(get_x_edges(img, 9), edges);
            // return(edges > 1500);
        };

        [[maybe_unused]] auto barrier_beg_detect = [&]() -> DetectResult {
           [[maybe_unused]] auto side_barrier_detect = [&](const Corners & _corners, const LR side) -> Vector2i {
                //少于两个拐点 没法判断有没有障�?
                if(_corners.size() < 2) return {0,0};

                auto box = CoastUtils::bounding_box(left_track);
                box.merge(CoastUtils::bounding_box(right_track));

                if(box.size.x > 30) return {0,0};
                //从起点开始搜索首次出现的a角点和v角点
                const auto * a_corner_ptr = CornerUtils::find_a(_corners);
                // const Corner * a_corner_ptr = nullptr;
                const auto * v_corner_ptr = CornerUtils::find_v(_corners);

                //能够找到a角点和v角点
                if(bool(a_corner_ptr) && bool(v_corner_ptr)){
                    scexpr int ignore_y = 28;
                    if(Vector2i(*a_corner_ptr).y < ignore_y) return {0,0};

                    scexpr int max_y_diff = 5;
                    scexpr int min_x_diff = 7;

                    Vector2i diff = Vector2i(v_corner_ptr->point) - Vector2i(a_corner_ptr->point);
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

            scexpr int least_y_diff = 20;

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
                // ASSERT_WITH_HALT(false, "detected dual barrier");
                return {false};
            use_left:
                return {true, LR::LEFT};
            use_right:
                return {true, LR::RIGHT};
        };

        [[maybe_unused]] auto barrier_end_detect = none_detect;

        [[maybe_unused]] auto side_is_curve_detect = [&](const Corners & _corners) -> bool {
            return _corners.size() == 0;
        };

        [[maybe_unused]] auto curve_detect = [&]() -> DetectResult {
            return side_is_curve_detect(left_corners) && side_is_curve_detect(right_corners);
            // return {false};//TODO
        };

        [[maybe_unused]] auto side_has_hook_detect = [&](const Corners & _corners, const Coast & track, LR side) -> const CoastItem *{
            bool has_three_points = (track.size() >= 3);
            if(has_three_points == false) {return nullptr;}

            const auto * corner = CornerUtils::find_a(_corners);
            bool first_is_a = bool(corner == _corners.begin());
            // scexpr int min_y_diff = 4;
            scexpr int min_x_diff = 6;
            if(first_is_a){
                const auto * ret = &(corner->point);
                const Vector2i diff = Vector2i(track[2]) - Vector2i(track[1]);
                // if(diff.y < min_y_diff) return nullptr;
                switch(side){
                    case LEFT:
                        return diff.x < -min_x_diff ? ret : nullptr;
                    case RIGHT:
                        return diff.x > min_x_diff ? ret : nullptr;
                    default:
                        return nullptr;
                }
            }else{
                return nullptr;
            }
        };

        [[maybe_unused]] auto side_is_straight_detect = [&](const Coast & track) -> bool{
            return track.size() == 2;
        };

        [[maybe_unused]] auto cross_beg_detect = [&]() -> DetectResult {
            [[maybe_unused]] auto side_cross_entry_detect = [&](const Corners & _corners, const Coast & _track, const LR side) -> const Corner *{

                const auto * point = CornerUtils::find_a(_corners);
                switch(side){
                    case LEFT:
                    case RIGHT:
                        if(point == _corners.begin() && point->point.y > 38){
                                scexpr int _min_x_diff = 2;

                                if(_track.size() < 3) return nullptr;

                                auto vec_diff = Vector2i(_track[2]) - Vector2i(_track[1]);

                                if(side == LR::LEFT){
                                    if(vec_diff.x > -_min_x_diff) return nullptr;
                                }else{
                                    if(vec_diff.x < _min_x_diff) return nullptr;
                                }
                            return point;
                        }else{
                            return nullptr;
                        }
                    default:
                        return nullptr;
                }
            };

            //FIXME

            const auto * left_detected = side_cross_entry_detect(left_corners, left_track, LR::LEFT);
            const auto * right_detected = side_cross_entry_detect(right_corners, right_track, LR::RIGHT);

            //找不到左或右
            if(left_detected == nullptr || right_detected == nullptr) return {false};   
            
            const auto & left_point = left_detected->point;
            const auto & right_point = right_detected->point;

            scexpr int max_y_diff = 10;
            scexpr int min_x_diff = 17;
            
            if(std::abs(left_point.y - right_point.y) >= max_y_diff) return {false};
            if(std::abs(left_point.x - right_point.x) <= min_x_diff) return {false};

            return {true};
        };

        // [[maybe_unused]] auto cross_end_detect = [&]() -> DetectResult {
        //     [[maybe_unused]] auto side_cross_leave_detect = [&](const Corners & _corners, const LR side) -> bool {
        //         switch(side){
        //             case LEFT:
        //             case RIGHT:{

        //                 // if(measurer.get_road_length_meters() < max)
        //                 const auto * first_corner_ptr = CornerUtils::find_corner(_corners);

        //                 //没有拐点�? 那就可以 
        //                 if(first_corner_ptr == nullptr) return true;

        //                 const auto & first_point = *first_corner_ptr;

        //                 //十字�?
        //                 if(first_point.point.y < 30) return true;

        //                 return false;
        //             }
        //             default:
        //                 return false;
        //         }
        //     };

        //     bool left_detected = side_cross_leave_detect(left_corners, LR::LEFT);
        //     bool right_detected = side_cross_leave_detect(right_corners, LR::RIGHT);

        //     return (left_detected && right_detected);
        // };

        [[maybe_unused]] auto ring_beg_detect = [&]() -> DetectResult {

            bool left_detected = bool(side_has_hook_detect(left_corners, left_track, LEFT)) && bool(side_is_straight_detect(right_track));
            bool right_detected = bool(side_has_hook_detect(right_corners, right_track, RIGHT)) && bool(side_is_straight_detect(left_track));

            return {bool(left_detected ^ right_detected), left_detected ? LR::LEFT : LR::RIGHT};
        };

        [[maybe_unused]] auto ring_in_detect = [&](const LR known_side) -> DetectResult {
            [[maybe_unused]] auto side_ring_in_detect = [&](const Corners & _corners, const LR _side, const bool is_expected_side) -> bool {
                if(is_expected_side){
                    // DEBUG_PRINTLN(known_side);
                    const auto & track = _side == LR::RIGHT ? right_track : left_track;
                    bool ret = CoastUtils::is_ccw(track, _side == LR::RIGHT ? false : true);
                    ret &= (_corners.size() == 0);
                    ret &= (track.size() > 3);
                    return ret;
                    // return Coast
                }else{
                    //对立边应该是直道
                    // return (_side == LR::RIGHT ? right_track : left_track).size() == 2;
                    return true;
                }
            };

            bool left_detected = side_ring_in_detect(left_corners, known_side, known_side == LR::LEFT);
            bool right_detected = side_ring_in_detect(right_corners, known_side, known_side == LR::RIGHT);

            return (left_detected && right_detected);
        };

        [[maybe_unused]] auto ring_running_detect = curve_detect;

        [[maybe_unused]] auto ring_out_detect = [&](const LR known_side) -> DetectResult {
            const auto & track = (known_side == RIGHT) ? left_track : right_track; 

            //选取对立侧拐�?
            // const auto & corners = (known_side == LR::RIGHT)? left_corners : right_corners;
            // const auto & corners = (known_side == LR::RIGHT)? left_corners : right_corners;
            if(track.size() < 3) return false;

            scexpr auto least_x_diff = 10;


            if(known_side == LR::RIGHT) if((track[2] - track[1]) < -least_x_diff) return {true};
            if(known_side == LR::LEFT) if((track[2] - track[1]) > least_x_diff) return {true};
            //判断有没有拐�?
            // if(track.size() == 2) return {true};
            return {false};
        };

        [[maybe_unused]] auto ring_end_detect = [&](const LR known_side) -> DetectResult {
            // return ring_beg_detect().side != known_side;
            const auto & corners = (known_side == LR::RIGHT)? left_corners : right_corners;
            return side_is_curve_detect(corners);
        };
    
        [[maybe_unused]] auto ring_exit_detect = none_detect;

        #define RESULT_GETTER(x) update_detect(x)


        [[maybe_unused]] auto side_to_align = [](const LR side) -> AlignMode{
            return (side == LR::LEFT) ? AlignMode::LEFT : AlignMode::RIGHT;
        };

        [[maybe_unused]] auto co_side_to_align = [](const LR side) -> AlignMode{
            return (side == LR::RIGHT) ? AlignMode::LEFT : AlignMode::RIGHT;
        };

        recordRunStatus(RunStatus::ELEMENT_B);


        #define CREATE_LOCKER(time, travel) (ElementLocker(time, travel))

        auto element_type = switches.element_type;
        switch(element_type){
            case ElementType::NONE:
            case ElementType::STRAIGHT:
                {
                    if(is_startup()) break;
                    //判断何时处理 状态机 of 斑马�?
                    if(true){
                        auto zebra_result = RESULT_GETTER(zebra_beg_detect());
                        // DEBUG_VALUE(bool(zebra_result));
                        if(zebra_result){
                            sw_element(ElementType::ZEBRA, Cross::Status::BEG, zebra_result.side, AlignMode::BOTH, CREATE_LOCKER(0, 1.3));
                            break;
                        }
                    }

                    //判断何时处理 状态机 of 障碍�?
                    if(true){
                    // if(false){
                        auto result = RESULT_GETTER(barrier_beg_detect());
                        if(result){
                            sw_element(ElementType::BARRIER, Barrier::Status::BEG, result.side, co_side_to_align(result.side), CREATE_LOCKER(0.8, 1.1));
                            break;
                        }
                    }

                    //判断何时处理 状态机 of 十字
                    if(true){
                    // if(false){
                        auto cross_result = RESULT_GETTER(cross_beg_detect());
                        if(cross_result){
                            // DEBUG_PRINTLN("cross detected");
                            sw_element(ElementType::CROSS, Cross::Status::BEG, cross_result.side, AlignMode::BOTH, {0.5,1.1});
                            break;
                        }
                    }

                    //判断何时处理 状态机 of 圆环
                    if(true){
                        auto ring_result = RESULT_GETTER(ring_beg_detect());
                        if(ring_result){
                            sw_element(ElementType::RING, Cross::Status::BEG, ring_result.side, co_side_to_align(ring_result.side), {0, ring_config.s1});
                            break;
                        }
                    }

                }
                break;
            
            //已经处于斑马线元素状�?
            case ElementType::ZEBRA:
                {

                    using ZebraStatus = Zebra::Status;
                    auto zebra_status = switches.zebra_status;
                    switch(zebra_status) {
                        //判断何时退出斑马线状�?

                        case ZebraStatus::BEG:{
                            // sw_element(ElementType::ZEBRA, (ZebraStatus::END), LR::LEFT, AlignMode::BOTH, {0, 1.2});
                            stop();
                        }break;

                        case ZebraStatus::END:{
                            stop();
                        }break;
                        default:
                            break;
                    }
                }
                break;

            //已经处于障碍物元素状�?
            case ElementType::BARRIER:
                {
                    using BarrierStatus = Barrier::Status;
                    auto barrier_status = switches.barrier_status;
                    // DEBUG_PRINTLN(element_type, barrier_status);
                    switch(barrier_status) {
                        //判断何时退出障碍物状�?
                        case BarrierStatus::BEG:if(true){
                            auto result = RESULT_GETTER(barrier_end_detect());
                            // DEBUG_PRINTLN(result.detected);
                            if(result){
                                // DEBUG_PRINTLN("barrier ended detected")
                                sw_element(ElementType::NONE, BarrierStatus::END, switches.element_side, AlignMode::BOTH);
                            }
                        }break;
                        case BarrierStatus::END:if(true){
                            // auto result = RESULT_GETTER(barrier_beg_detect());
                            auto result = true;
                            if(result){
                                sw_element(ElementType::NONE, BarrierStatus::END, switches.element_side, AlignMode::BOTH);
                            }
                        }break;
                        default:
                            break;
                    }
                }
                break;

            //已经处于十字元素状�?
            case ElementType::CROSS:
                {
                    using CrossStatus = Cross::Status;
                    auto cross_status = switches.cross_status;
                    switch(cross_status){
                        //判断何时退出十字状�?
                        case CrossStatus::BEG:if(true){
                            // auto result = RESULT_GETTER(cross_beg_detect());
                            auto result = true;
                            if(result){
                                sw_element(ElementType::NONE, CrossStatus::END, LEFT, AlignMode::BOTH, {0, 0});
                            }
                        }break;
                        default:
                            break;
                    }
                }
                break;

            //已经处于圆环状�?
            case ElementType::RING:
                {
                    using RingStatus = Ring::Status;
                    auto ring_status = switches.ring_status;
                    auto ring_side = switches.element_side;
                    switch(ring_status){
                        //判断何时入环
                        case RingStatus::BEG:if(true){//判断何时单调
                            auto result = RESULT_GETTER(ring_in_detect(ring_side));
                            if(result){
                                measurer.reset_angle();
                                sw_element(ElementType::RING, RingStatus::IN, ring_side, side_to_align(ring_side), {0,ring_config.c1});
                            }
                        }break;

                        //判断何时进环
                        case RingStatus::IN: if(true){//判断何时全曲
                            auto result = RESULT_GETTER(ring_running_detect());
                            if(result){
                                sw_element(ElementType::RING, RingStatus::RUNNING, ring_side, side_to_align(ring_side), {0, ring_config.o});
                            }
                        }break;

                        //判断何时出环
                        case RingStatus::RUNNING: if(true){//判断何时对立有拐�?
                            auto result = RESULT_GETTER(ring_out_detect(ring_side));
                            if(result){
                                sw_element(ElementType::RING, RingStatus::OUT, ring_side, side_to_align(ring_side), {0, ring_config.c2});
                            }
                        }break;

                        //判断何时退出圆�?
                        case RingStatus::OUT: if(std::abs(measurer.get_angle()) > 1.42){//判断何时回到圆起�?
                            auto result = RESULT_GETTER(ring_end_detect(ring_side));
                            if(result){
                                sw_element(ElementType::RING, RingStatus::END, ring_side, co_side_to_align(ring_side), {0, ring_config.s2});
                            }
                        }break;

                        case RingStatus::END: if(true){//判断何时
                            auto result = RESULT_GETTER(ring_exit_detect());
                            if(result){
                                sw_element(ElementType::NONE, RingStatus::END, ring_side, AlignMode::BOTH);
                            }
                        }break;
                    }
                }
                break;

            default:
                break;
        }
        //在自动模式下 如果识别不到赛道 就关断小�? 避免跑飞时撞�?

        update_holder();

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
                scexpr real_t k = 1.2;

                auto coast_point_valid = [](const CoastItem & host, const CoastItem & guest, const LR _is_right) -> bool{
                    Vector2i delta = Vector2i(guest) - Vector2i(host);

                    //如果两个点坐标一�? 那么通过
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

                        //如果需要丢�? 添加到列�?
                        remove_indexes[j] |= abandon;
                    }
                };

                Coast ret;


                //对初点做特殊讨论
                //此处以返回值的第二个点为参�?

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
                        //第一个点不可�? 后面也找不到可取�? 等价与返回空

                        return {};
                    }
                }else{
                    for(size_t i = 0; i < coast.size(); ++i){
                        ret.push_back(coast[i]);
                    }
                }
                return ret;
            };

            left_track = coast_fix_lead(left_track, LR::LEFT);
            right_track = coast_fix_lead(right_track, LR::RIGHT);

            plot_coast(left_track, {0, 0}, RGB565::GREEN);
            plot_coast(right_track, {0, 0}, RGB565::CORAL);
        };

        //引导修正结束
        /* #endregion */

        /* #region */
        //开始提取轮廓主向量
        recordRunStatus(RunStatus::VEC_B);
        if(true){
            do{
                if(left_track.size() >= 2 and right_track.size() >= 2){
                    auto vec_valid = [](const Vector2 & vec){return bool(vec) && vec.length() > 5;}; 
                    auto road_align_pixels = WorldUtils::pixels(config.road_width);
                    Segment seg_left = SegmentUtils::shift({
                            left_track.front(), *std::next(left_track.begin())}, 
                            Vector2i(int(road_align_pixels/2), 0));

                    Segment seg_right = SegmentUtils::shift({
                            right_track.front(), *std::next(right_track.begin())}, 
                            Vector2i(-int(road_align_pixels/2), 0));

                    // plot_segment(seg_left, {0, 0}, RGB565::YELLOW);
                    // plot_segment(seg_right, {0, 0}, RGB565::GREEN);

                    //提取根向�?
                    Vector2 left_root_vec = SegmentUtils::vec(seg_left);
                    Vector2 right_root_vec = SegmentUtils::vec(seg_right);
                    Vector2 root_vec;

                    //如果根向量相似且左右间隔是合法的赛道宽度(排除在识别元素时的干�?) 那么进行根向量合�? 否则根据吸附的左右选择对应的根向量
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
                                //计算两者按权重相加的向量（不需要考虑模长�?
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

                    if(is_blind == false){
                        if (bool(root_vec)) {
                            measurer.update_dir(root_vec.angle());
                        }
                    }else{
                        measurer.update_dir((PI/2));
                    }

                    // DEBUG_PRINTLN(measurer.get_dir());

                    plot_segment({measurer.seed_pos, measurer.seed_pos + Vector2(10.0, 0).rotated(-measurer.get_dir())}, {0, 0}, RGB565::PINK);
                }
            }while(false);
        }
        // DEBUG_PRINTLN(measurer.get_dir());
        recordRunStatus(RunStatus::VEC_E);
        //对轮廓的主向量提取结�?
        /* #endregion */
    
        // DEBUG_PRINTLN(measurer.get_angle());
        // plot_coast(track_left, {0, 0}, RGB565::RED);
        // plot_coast(track_right, {0, 0}, RGB565::BLUE);

        // DEBUG_PRINTLN(measurer.get_angle());
 
        // DEBUG_PRINTLN("!!");
 

        // plot_bound(bound_center, {0, 0}, RGB565::PINK);
        // plot_bound(bound_right, {0, 0}, RGB565::BLUE);
        // plot_bound(bound_center, {0, 0}, RGB565::YELLOW);

        recordRunStatus(RunStatus::END);
        // DEBUG_PRINTLN("isccw", CoastUtils::is_ccw(left_track, true));
        // DEBUG_PRINTLN("isccw", CoastUtils::is_ccw(right_track, false));
    }
}

}


void smc_main(){
    SMC::SmartCar car;
    car.main();
};
