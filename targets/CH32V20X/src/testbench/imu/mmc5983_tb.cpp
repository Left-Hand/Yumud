#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"


#include "robots/gesture/mahony.hpp"
#include "robots/gesture/mag_cali.hpp"

#include "drivers/IMU/Magnetometer/MMC5983/MMC5983.hpp"
#include "thirdparty/sstl/include/sstl/vector.h"


using namespace ymd;

using namespace ymd::drivers;

// #define UART uart2
#define UART hal::uart2
#define SCL_GPIO hal::PB<3>()
#define SDA_GPIO hal::PB<5>()
static constexpr uint FS = 100;
static constexpr auto INV_FS = (1.0_q24 / FS);
// #define MAG_ACTIVATED

static void mmc5983_test(drivers::MMC5983 & imu){

    imu.init({}).examine();

    imu.enable_auto_mag_sr(EN).examine();

    // DEBUG_PRINTLN(do_set().examine());

    // while(true);
    // const auto m = millis();
    const auto up = (imu.do_magreset().examine() + imu.do_magreset().examine())/ 2;
    const auto down = (imu.do_magset().examine() + imu.do_magset().examine())/ 2;
    const auto base = (up + down)/2;
    // PANIC(millis() - m);
    // sstl::vector<Vec3<q24>, 64> data;
    robots::EllipseCalibrator calibrator;

    #if 0
    // for(size_t i = 0; i < 100; i++){
    //     while(imu.is_mag_meas_done().examine() == false);
    //     imu.update().examine();
    // }
    // while(true){
    //     while(imu.is_mag_meas_done().examine() == false);
    //     imu.update().examine();
    //     const auto mag = imu.read_mag().examine() - base;

    //     calibrator.add_data(mag);
    //     const auto prog = calibrator.get_progress();
    //     DEBUG_PRINTLN(prog, mag);
    //     if(calibrator.get_percentage() >= 48){
    //         // const auto [mean, soft_iron] = calibrate_magfield(data);
    //         std::tie(mean, soft_iron) = calibrator.get_result();
    //         break;
    //     }
    // }


    for(size_t i = 0; i < 100; i++){
        const auto v2 = robots::EllipseCalibrator::project_idx_to_v2(i, 100);
        // const auto p = project_v2_to_v3();
        const auto p = Vec3<q24>();
        DEBUG_PRINTLN(p,i, v2);
        clock::delay(1ms);
        calibrator.add_data(p + Vec3<q24>(0.2_r,1,1));
    }
    const auto [mean, soft_iron] = calibrator.dignosis();

    for(const auto & mag : calibrator.data()){
        DEBUG_PRINTLN(mag);
        clock::delay(1ms);
    }
    #endif
    while(true){
        while(imu.is_mag_meas_done().examine() == false);
        imu.update().examine();
        const auto mag = imu.read_mag().examine() - base;

        DEBUG_PRINTLN((mag), 0);
    }

    #if 0
    // imu.set_mode(MMC5983::Mode::Cont4).examine();
    // DEBUG_PRINTLN("app started");

    // Quat<q24> gest;
    // auto measure = [&](){
    //     imu.update().examine();
    //     const auto dir = imu.read_mag().examine();
    //     gest = gest.slerp(Quat<q24>::from_direction(dir), 0.05_r);
    // };

    // timer1.init(FS);
    // timer1.attach(TimerIT::Update, {0,0},[&]{
    //     measure();
    // });
    
    // while(true){
    //     // DEBUG_PRINTLN(imu.update());
    //     DEBUG_PRINTLN(millis(), gest, imu.read_mag());
    // }
    #endif
}

void mmc5983_main(){
    UART.init({
        .baudrate = 576_KHz
    });

    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    // I2cSw i2c{hal::PA<12>(), hal::PA<15>()};
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};
    // i2c.init(400_KHz);
    // i2c.init();
    
    clock::delay(200ms);
    
    i2c.init({2000_KHz});

    MMC5983 imu{&i2c};

    // auto & spi = spi1;
    // spi.init(18_MHz);
    // MMC5983 imu = {SpiDrv(spi, spi.allocate_cs_gpio(hal::PA<15>()).value())};

    mmc5983_test(imu);
}