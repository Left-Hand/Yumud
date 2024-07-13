#include <unordered_set>
#include <map>
#include "src/testbench/tb.h"

#include "nvcv2.hpp"
#include "../drivers/Camera/MT9V034/mt9v034.hpp"
#include "../drivers/IMU/Gyroscope/QMC5883L/qmc5883l.hpp"
#include "../drivers/Display/DisplayerInterface.hpp"
#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "../targets/CH32V20X/src/testbench/tb.h"
#include "../hal/bkp/bkp.hpp"

#include "../types/quat/Quat_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/vector3/vector3_t.hpp"
#include "body.hpp"
#include "elements.hpp"
#include "fans.hpp"

#include "finder.hpp"
#include "cli.hpp"




constexpr uint32_t SPI1_BaudRate = (144000000/32);
constexpr uint32_t SPI2_BaudRate = (144000000/8);

I2cSw i2csw(portB[3], portB[5]);
Quat accel_offs;
Vector3 gyro_offs;
Quat magent_offs;


Vector3 accel;
Vector3 gyro;
Vector3 magent;
real_t current_dir;

BETTER_ENUM(RunStatus, uint8_t,
    NEW_ROUND = 0,
    PROCESSING_IMG_BEGIN,
    PROCESSING_IMG_END,
    PROCESSING_FINDER,
    PROCESSING_FINDER_END,
    PROCESSING_COAST,
    PROCESSING_COAST_END,
    PROCESSING_DP_END,
    PROCESSING_DIR_END,
    PROCESSING_VA_END,
    PROCESSING_SEGMENT,
    PROCESSING_SEGMENT_END,
    PROCESSING_FANS,
    PROCESSING_FANS_END,
    END_ROUND
)

#define I2C_BaudRate 400000
// Grayscale buf[188][120];

extern "C" void TimBase_IRQHandler(void) __interrupt;

const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

real_t delta = real_t(0);
real_t fps = real_t(0);
I2cDrv i2cdrv(i2csw, 0xd0);
MPU6050 mpu(i2cdrv);

I2cDrv i2cDrvQm(i2csw, 0x1a);
QMC5883L qml(i2cDrvQm);
// real_t t = real_t(0);


// #define VNAME(value) (#value)

void preinit(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOD, ENABLE);
}


using namespace SMC;



using namespace NVCV2;
using NVCV2::Shape::CoastFinder;
using NVCV2::Shape::Seed;
// using NVCV2::Shape

static IOStream & logger = uart7;
real_t measured_offs_err;

auto runStatusReg = bkp[1];
auto powerOnTimesReg = bkp[2];

void recordRunStatus(const RunStatus & status){
    runStatusReg = uint16_t(uint8_t(status));
    if(show_status){
        DEBUG_PRINT("[s]:", status._to_string());
    }
}

void printRecordedRunStatus(){
    uint16_t id = runStatusReg;
    auto temp = RunStatus::_from_integral_unchecked(id);
    DEBUG_PRINT(temp._to_string(), id);
}

SideFan left_fan(timer4.oc(2), timer4.oc(1));
SideFan right_fan(timer5.oc(3), timer5.oc(4));

SideFan vl_fan(timer4.oc(3), timer4.oc(4));//pair ok
SideFan vr_fan(timer5.oc(2), timer5.oc(1));//pair ok

ChassisFan chassis_left_fan(timer8.oc(1));
ChassisFan chassis_right_fan(timer8.oc(2));

HriFanPair hri_fan(vl_fan, vr_fan);
ChassisFanPair chassis_fan(chassis_left_fan, chassis_right_fan);
RigidBody body(motor_strength, left_fan, right_fan, hri_fan, chassis_fan);
    Vector2i seed_pos;
    Rangei road_window;

void ctrl(){
    Sys::Clock::reCalculateTime();
    mpu.update();
    qml.update();

    accel = accel_offs.xform(Vector3(mpu.getAccel()));
    gyro = (Vector3(mpu.getGyro()) - gyro_offs);
    magent = magent_offs.xform(Vector3(qml.getMagnet()));

    static constexpr real_t target_dir = real_t(PI / 2);

    real_t turn_output = turn_ctrl.update(target_dir, current_dir, gyro.z);
    real_t measured_acc = accel.y;

    real_t estimated_velocity = side_velocity_observer.update(measured_offs_err, measured_acc);
    real_t side_output = side_ctrl.update(0, measured_offs_err, -estimated_velocity);

    real_t speed_output = speed_ctrl.update();
    // offs_err = offs_err * 0.8 + 0.2 * measured_offs_err;
    // turn_output += speed_ctrl.update();
    real_t new_x = (Vector2(seed_pos) + (Vector2(real_t(-15.0), real_t(0.0)).rotated(current_dir))).x;
    DEBUG_PRINT(SpecToken::Comma, motor_strength.left, motor_strength.right, motor_strength.hri, seed_pos.x, new_x);
    // real_t side_output = side_ctrl.update(0, -offs_err, -accel.y);

    motor_strength.left = turn_output;
    motor_strength.right = -turn_output;

    motor_strength.left += speed_output;
    motor_strength.right += speed_output;

    motor_strength.hri = side_output;

    body.update();
}

using namespace SMC;

void smc_main(){
    preinit();
    
    uart4.init(921600);
    uart4.setEps(4);

    uart7.init(115200);
    uart7.setSpace(" ");
    bkp.init();
    powerOnTimesReg = powerOnTimesReg + 1;
    printRecordedRunStatus();
    DEBUG_PRINT("pwon", int(powerOnTimesReg));

    spi2.init(72000000);
    spi2.bindCsPin(portD[4], 0);

    timer4.init(24000);
    timer5.init(24000);
    timer8.init(2880, 1000);
    timer8.initBdtr(0, AdvancedTimer::LockLevel::Off);


    timer1.initAsEncoder();


    body.init();
    body.enable(false);

    delay(200);
    SpiDrv SpiDrvLcd = SpiDrv(spi2, 0);
    DisplayInterfaceSpi SpiInterfaceLcd(SpiDrvLcd, portD[7], portB[7]);
    ST7789 tftDisplayer(SpiInterfaceLcd, Vector2i(240, 240));

    tftDisplayer.init();
    tftDisplayer.setFlipX(true);
    tftDisplayer.setFlipY(false);
    tftDisplayer.setSwapXY(true);
    tftDisplayer.setFormatRGB(true);
    tftDisplayer.setFlushDirH(false);
    tftDisplayer.setFlushDirV(false);
    tftDisplayer.setInversion(true);

    Painter<RGB565> painter = Painter<RGB565>();
    painter.bindImage(tftDisplayer);

    i2csw.init(1000000);

    mpu.init();
    qml.init();

    I2cSw i2c(portD[2], portC[12]);
    i2c.init(10000);
    MT9V034 camera(i2c);

    auto & start_key = portE[2];
    start_key.inpu();
    auto & stop_key = portE[3];
    stop_key.inpu();

    {
        uint8_t camera_init_retry_times = 0;
        constexpr uint8_t camera_retry_timeout = 7;
        while(!camera.init()){
            camera_init_retry_times++;
            if(camera_init_retry_times >= camera_retry_timeout){
                DEBUG_LOG("camera init failed");
                while(true);
            }
        }
    }

    auto affine_gray_image = make_image<Grayscale>(Vector2i{188, 60});
    auto affine_bina_image  = make_bina_mirror(affine_gray_image);
    auto diff_bina_image = make_image<Binary>(Vector2i(188, 60));
    auto diff_gray_image = make_gray_mirror(diff_bina_image);
    auto show_image = make_image<RGB565>(Vector2i(188, 60));

    [[maybe_unused]] auto plot_gray = [&](Image<Grayscale, Grayscale> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());

    };

    [[maybe_unused]] auto plot_bina = [&](Image<Binary, Binary> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_coast = [&](const Coast & coast,  const Vector2i & pos, const RGB565 & color = RGB565::RED){

        if(coast.size() <= 1){
            return;
        }

        painter.bindImage(show_image);
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

        painter.bindImage(show_image);
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


        painter.bindImage(show_image);
        painter.setColor(color);
        
        painter.drawLine(seg.first + pos, seg.second + pos);
    };

    [[maybe_unused]] auto plot_points = [&](const Points & pts,  const Vector2i & pos, const RGB565 & color = RGB565::PINK){
        painter.bindImage(show_image);
        painter.setColor(color);
        for(auto && pt : pts){
            painter.drawPixel(pt + pos);
        }
    };

    [[maybe_unused]] auto plot_bound = [&](const Boundary & bound,  const Vector2i & pos, const RGB565 & color = RGB565::RED){
        // return;
        // if(bound.size() < 2) return;
        painter.bindImage(show_image);
        for(auto && pt : bound){
            painter.drawPixel(Vector2i{pt.second, pt.first}+ pos, color);
        }
    };

    [[maybe_unused]] auto plot_point = [&](const Vector2i & pos, const RGB565 & color = RGB565::RED, const int & radius = 2){
        // return;
        // if(bound.size() < 2) return;
        painter.bindImage(show_image);
        painter.setColor(color);
        // for(auto && pt : bound){
        painter.drawPixel(pos, color);
        // painter.drawFilledCircle(pos, radius);
        // }
    };

    [[maybe_unused]] auto plot_pile = [&](const Pile & bound, const RGB565 & color = RGB565::RED){
        painter.bindImage(show_image);
        painter.drawLine(Vector2i{bound.second.start, bound.first}, {bound.second.end, bound.first});
    }; 

    [[maybe_unused]] auto plot_update = [&](const Vector2i & pos){
        painter.bindImage(tftDisplayer);
        painter.drawImage(show_image, pos);
        show_image.fill(RGB565::BLACK);
    };

    [[maybe_unused]] auto plot_vec3 = [&](const Vector3 & vec3,  const Vector2i & pos){
        static constexpr auto square_length = 50;
        auto arm_length = vec3.length();

        static constexpr auto radius = 3;
        static constexpr auto x_unit = Vector2(1, 0);
        static constexpr auto y_unit = Vector2(0.5, -0.73);
        static constexpr auto z_unit = Vector2(0, -1);

        // auto x_axis = Vector3(vec3.x, 0, 0);
        // auto y_axis = Vector3(0, vec3.y, 0);
        // auto z_axis = Vector3(0, 0, vec3.z);

        auto x_axis = Vector3(arm_length, 0, 0);
        auto y_axis = Vector3(0, arm_length, 0);
        auto z_axis = Vector3(0, 0, arm_length);

        static constexpr auto x_color = RGB565::RED;
        static constexpr auto y_color = RGB565::GREEN;
        static constexpr auto z_color = RGB565::BLUE;
        static constexpr auto bg_color = RGB565::BLACK;

        // const auto scale = vec3.length();
        auto vec3n = vec3.normalized();
        const Quat rot = Quat(Vector3(0, 0, -1), vec3n);
        // logger.println(vec3n, rot);
        const Vector2i center_point = pos + Vector2i(square_length, square_length) / 2;

        auto plot_vec3_to_plane = [&](const Vector3 & axis, const char & chr, const RGB565 & color){
            Vector3 end = rot.xform(axis);
            Vector2i end_point = center_point + (x_unit * end.x + y_unit * end.y + z_unit * end.z);
            painter.setColor(color);
            painter.drawLine(center_point, end_point);
            painter.drawFilledCircle(end_point, radius);
            // painter.setColor(RGB565::WHITE);
            // painter.drawChar(end_point, chr);
        };

        painter.bindImage(tftDisplayer);

        painter.drawFilledRect(Rect2i{pos, Vector2i{square_length, square_length}}, bg_color);
        plot_vec3_to_plane(x_axis, 'X', x_color);
        plot_vec3_to_plane(y_axis, 'Y', y_color);
        plot_vec3_to_plane(z_axis, 'Z', z_color);

        CLI::run();
    };




    // Coast coast_left;
    // Coast coast_right;

    // Boundary bound_left;
    // Boundary bound_right;




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
        accel_offs = Quat(Vector3(0,0,-1),g/g.length()).inverse();

        Vector3 m = temp_magent / cali_times;
        magent_offs = Quat(Vector3(0,0,-1), m/m.length()).inverse();

        gyro_offs = temp_gyro_offs / cali_times;
    }

    timer8.bindCb(TimerUtils::IT::Update, std::bind(ctrl));
    timer8.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});

    while(true){
        recordRunStatus(RunStatus::NEW_ROUND);

        {
            // auto[x1,y1,z1] = accel;
            // auto[x2,y2,z2] = gyro;
            // auto[x3,y3,z3] = magent;
            // DEBUG_PRINT(SpecToken::Comma, x3, y3, z3);
            // DEBUG_PRINT(SpecToken::Comma, x1, y1, z2, uint16_t(timer1));
        }

        if(stop_key == false){
            body.enable(false);
        }
        else if(start_key == false){
            body.enable();
        }

        plot_vec3(accel.normalized() * 7, {190, 0});
        plot_vec3(gyro * 7, {190, 60});
        plot_vec3(magent.normalized() * 7, {190, 120});
        auto start = millis();

    // DEBUG_PRINT("new turn");
        recordRunStatus(RunStatus::PROCESSING_IMG_BEGIN);
        plot_gray(camera, Rect2i{Vector2i{0, 60}, Vector2i{188, 60}});
        Geometry::perspective(affine_gray_image, camera);
        plot_gray(affine_gray_image, Rect2i{Vector2i{0, 120}, Vector2i{188, 60}});


        // plot_points(Points{{0, 0}, {40,40}, {80, 50}}, {0, 0})
        // plot_gray(affine_gray_image, Rect2i{Vector2i{0, 120}, Vector2i{188, 60}});
        //从摄像头读取透视变换后的灰度图

        Pixels::inverse(affine_gray_image);
        //对灰度图取反(边线为白色方便提取)

        Shape::convo_roberts_x(diff_gray_image, affine_gray_image);
        //获取x方向上的差分图

        Pixels::binarization(diff_bina_image, diff_gray_image, edge_threshold);
        //将差分图二值化


        Pixels::binarization(affine_bina_image, affine_gray_image, positive_threshold);
        Pixels::or_with(affine_bina_image, diff_bina_image);
        Shape::dilate_xy(affine_bina_image);
        Shape::dilate_xy(affine_bina_image);
        Shape::erosion_x(affine_bina_image);
        Shape::erosion_y(affine_bina_image);

        //获取差分图和灰度图的并集
        //差分图对边缘敏感 灰度图对色块敏感 两者合围可消除缺陷
        plot_bina(affine_bina_image,  Rect2i{Vector2i{0, 180}, Vector2i{188, 60}});
        // continue;

        // Shape::erosion_xy(affine_bina_image);
        //腐蚀图像 用于去除赛道上可能存在的斑点
        //可能会在寻斑马线时去除

        recordRunStatus(RunStatus::PROCESSING_IMG_END);

        auto & img = affine_gray_image;
        auto & img_bina = affine_bina_image;

        // auto bottom_pile = get_h_pile(img_bina, true);
        // auto top_pile = get_h_pile(img_bina, false);
        
        // plot_pile(top_pile, RGB565::YELLOW);
        // plot_pile(bottom_pile, RGB565::YELLOW);

        // auto road_window = get_side_range(src, y);
        std::tie(seed_pos, road_window) = get_entry(img_bina, Vector2i{seed_pos.x, img.size.y -1 - safety_seed_height}, RIGHT);
        // road_window = get_h_range(img_bina, seed_pos);
        if(road_window.length() < 16){
            body.enable(false);
            continue;
        }

        if(enable_flag){
            body.enable(true);
            enable_flag = false;
        }


        if(!seed_pos) continue;
        measured_offs_err = -(seed_pos.x - (img.get_size().x / 2) + 2) * 0.02;

        plot_point(seed_pos);
        recordRunStatus(RunStatus::PROCESSING_COAST);
        auto coast_left_unfixed =    get_coast(img_bina, seed_pos, LEFT);
        auto coast_right_unfixed =   get_coast(img_bina, seed_pos, RIGHT);

        auto coast_left = fix_coast(coast_left_unfixed, img.size);
        auto coast_right = fix_coast(coast_right_unfixed, img.size);
    
        ASSERT(!coast_self_insterction(coast_left), "left self ins");
        ASSERT(!coast_self_insterction(coast_right), "right self ins");
        // auto where_outwindow_left = coast_in_window(coast_left, img.size);
        // auto where_outwindow_right = coast_in_window(coast_right, img.size);

        // ASSERT_WITH_CONTINUE(!where_outwindow_left, "left out window", where_outwindow_left);
        // ASSERT_WITH_CONTINUE(!where_outwindow_right, "right out window", where_outwindow_right);

        // DEBUG_VALUE(coast_left.size());
        // DEBUG_VALUE(coast_right.size());
        // DEBUG_VALUE(seed_pos);


        ASSERT_WITH_DOWN(coast_left.size(), "left coast size 0");
        ASSERT_WITH_DOWN(coast_right.size(), "right coast size 0");

        if(!coast_left.size()) continue;
        if(!coast_right.size()) continue;

        recordRunStatus(RunStatus::PROCESSING_COAST_END);

        // auto coast_left_temp = coast_left;
        // coast_left_temp.resize(1);
        // DEBUG_PRINT("done coast");
        auto track_left = SMC::douglas_peucker(coast_left, dpv);
        auto track_right = SMC::douglas_peucker(coast_right, dpv);

        recordRunStatus(RunStatus::PROCESSING_DP_END);

        // ASSERT_WITH_DOWN(track_left.size(), "left track size 0");
        // ASSERT_WITH_DOWN(track_right.size(), "right track size 0");
        // DEBUG_PRINT("rw", road_window.grow(8));
        // DEBUG_VALUE(seed_pos);
        // auto y_ranges = get_outroad_y_ranges(img_bina, road_window.grow(4), Seed(seed_pos, Direction::U));
        // DEBUG_VALUE(y_ranges.size());
        // DEBUG_PRINT("done doug");
        // bound_left = left_boundary(coast_left);
        // bound_right = right_boundary(coast_right);

        // coast_left = bound_to_coast(bound_left);
        // coast_right = bound_to_coast(bound_right);
        // auto bound_center = SMC::mean_boundry(bound_left, bound_right);

        // auto follow_left = SMC::shrink_coast(track_left, real_t(-align_space_width), img.size);
        // auto follow_right = SMC::shrink_coast(track_right, real_t(align_space_width), img.size);
        // ASSERT_WITH_DOWN(track_left.size()>= 2, "left trakc 1 point");
        // ASSERT_WITH_DOWN(track_right.size() >= 2, "righttrack 1point");


        if(track_left.size() >= 2 and track_right.size() >= 2){//update current_dir

            Segment follow_left = std::make_pair(track_left.front() + Vector2i(align_space_width, 0), *std::next(track_left.begin()) + Vector2i(align_space_width, 0));
            Segment follow_right = std::make_pair(track_right.front() - Vector2i(align_space_width, 0), *std::next(track_right.begin()) - Vector2i(align_space_width, 0));
            
            plot_segment(follow_left, {0, 0}, RGB565::YELLOW);
            plot_segment(follow_right, {0, 0}, RGB565::GREEN);

            Vector2 diff_left = segment_get_diff(follow_left);
            Vector2 diff_right = segment_get_diff(follow_right);
            if(bool(diff_left) and bool(diff_right)){

                auto diff_left_l = diff_left.length();
                auto diff_right_l = diff_right.length();
                auto vec_sin = diff_left.cross(diff_right) / diff_left_l / diff_right_l;
                if(vec_sin < abs(dir_merge_max_sin)){
                    current_dir = (diff_left * diff_right_l + diff_right * diff_left_l).angle();
                }
            }else{
                if(align_right and bool(diff_right)){
                    current_dir = diff_right.angle();
                }else if(bool(diff_left)){
                    current_dir = diff_left.angle();
                }
            }
        }

        recordRunStatus(RunStatus::PROCESSING_DIR_END);
        // if(Vector2(segment_get_diff(follow_left)).cos())
        // DEBUG_PRINT("done plot shrink");

        auto v_points = coast_get_v_points(track_left);
        auto a_points = coast_get_a_points(track_left);

        recordRunStatus(RunStatus::PROCESSING_VA_END);
        // auto track_left = SMC::douglas_peucker(SMC::shrink_coast(coast_left, real_t(-align_space_width)), dpv);
        // auto track_right = SMC::douglas_peucker(SMC::shrink_coast(coast_right, real_t(align_space_width)), dpv);

        // coast_right = SMC::douglas_peucker(coast_right, dpv);
        // auto track_right = SMC::get_main_coast(SMC::shrink_coast(coast_right, real_t(-15)));
        // plot_coast(track_left, {0, 0}, RGB565::RED);
        // plot_coast(track_right, {0, 0}, RGB565::BLUE);
        // DEBUG_PRINT("done plot track");

        // static constexpr Vector2i shapes_offset = Vector2i(0, 0);
        plot_coast(track_left, {0, 0}, RGB565::RED);
        plot_coast(track_right, {0, 0}, RGB565::BLUE);

        plot_segment({seed_pos, seed_pos + Vector2(10.0, 0).rotated(-current_dir)}, {0, 0}, RGB565::GRAY);
        // DEBUG_PRINT("done plot follow");

        // DEBUG_PRINT("a/v points", a_points.size(), v_points.size());
        plot_points(v_points, {0, 0}, RGB565::YELLOW);
        plot_points(a_points, {0, 0}, RGB565::PINK);
        // plot_bound(bound_center, {0, 0}, RGB565::PINK);
        // DEBUG_PRINT("!!");
        plot_update({0, 0});

        // plot_bound(bound_right, {0, 0}, RGB565::BLUE);
        // plot_bound(bound_center, {0, 0}, RGB565::YELLOW);
        // plot_gray(diff_gray_image, {0, 160});

        // delay(200);
        // plot_grayscale(affine_image, {0, 0});
        // plot_binary(affine_bina_image, {0, 80});


        // {
        //     auto affine_plot_image = make_image(RGB565, affine_image.get_size());
        //     NVCV2::Pixels::conv(affine_plot_image, affine_image);

        //     painter.bindImage(affine_plot_image);
        //     auto boud = finder.find();

        //     constexpr Vector2i affine_offset = {0, 120};
        //     painter.bindImage(tftDisplayer);
        //     painter.drawImage(affine_plot_image, affine_offset);
        // }

        frame_ms = millis() - start;
        // DEBUG_PRINT(coast_left.size() + coast_right.size(), track_left.size() + track_right.size(), v_points.size() + a_points.size())

        // DEBUG_PRINT(int(timer8[2]), int(timer8[1]));
        recordRunStatus(RunStatus::END_ROUND);
        // DEBUG_PRINT("end turn");
        // logger.println(coast_left.size());
    }
}
