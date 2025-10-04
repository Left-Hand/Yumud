#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/ICM42688/icm42688.hpp"
#include "robots/gesture/mahony.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define DBG_UART hal::uart2
#define SCL_GPIO hal::PB<3>()
#define SDA_GPIO hal::PB<5>()
static constexpr uint ISR_FREQ = 500;
static constexpr auto INV_FS = (1.0_q24 / ISR_FREQ);

#define PHY_SEL_I2C 0
#define PHY_SEL_SPI 1

#define PHY_SEL PHY_SEL_I2C
// #define PHY_SEL PHY_SEL_SPI

[[maybe_unused]]
static void icm42688_tb(ICM42688 & imu){

    DEBUG_PRINTLN("init started");


    imu.init({
        .acc_odr = ICM42688::AccOdr::_200Hz,
        .acc_fs = ICM42688::AccFs::_4G,
        .gyr_odr = ICM42688::GyrOdr::_200Hz,
        .gyr_fs = ICM42688::GyrFs::_1000deg
    }).examine();

    q24 z = 0;
    Microseconds exe = 0us;

    const real_t tau = 1.3_r;
    Mahony mahony{{
        .kp = 2 * tau,
        .ki = tau * tau,
        // .ki = 0,
        .fs = ISR_FREQ
    }};

    hal::timer1.init({ISR_FREQ}, EN);

    Vec3<q24> gyr_ = Vec3<q24>::ZERO;
    Vec3<q24> acc_ = Vec3<q24>::ZERO;
    hal::timer1.attach<hal::TimerIT::Update>({0,0},[&]{
        const auto u0 = clock::micros();
        imu.update().examine();
        const auto gyr = imu.read_gyr().examine();
        const auto acc = imu.read_acc().examine();

        z = z + INV_FS * gyr.z;
        // mahony.update(imu.read_gyr().examine(), imu.read_acc().examine());
        // mahony.update(imu.read_gyr().examine(), imu.read_acc().examine());
        mahony.myupdate_v2(gyr, acc);
        exe = clock::micros() - u0;

        gyr_ = gyr;
        acc_ = acc;
    }, EN);

    while(true){
        // const auto u0 = clock::micros();
        // imu.update().unwrap();
        // const auto u1 = clock::micros();
        // const auto acc = imu.read_acc().unwrap();
        // const auto gyr = imu.read_gyr().unwrap();
        // const auto gest = Quat<real_t>::from_shortest_arc(
        //     acc.normalized(),
        //     {0,0,1}
        // );

        // const auto gest = mahony.result();
        // const auto euler = gest.to_euler();
        DEBUG_PRINTLN(
            // 0
            // ,gest
            // acc_, gyr_
            gyr_, z
            // ,euler
            // ,acc
        //     ,gyr
        //     // ,imu.read_gyr().unwrap()
        //     // ,z, exe
        );
        clock::delay(5ms);
        // DEBUG_PRINTLN(mahony.result().normalized(), z, exe);
        // DEBUG_PRINTLN(mahony.result().to_euler(), z, exe);
        // DEBUG_PRINTLN(z);
        
        // DEBUG_PRINTLN(millis());
    }
}

void icm42688_main(){
    DBG_UART.init({576_KHz});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    clock::delay(200ms);

    #if PHY_SEL == PHY_SEL_I2C
    // I2cSw i2c{hal::PA<12>(), hal::PA<15>()};

    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};
    // i2c.init(400_KHz);
    i2c.init({2000_KHz});

    auto imu = ICM42688{
        &i2c,
        ICM42688::I2cSlaveAddrMaker{.ad0_level = HIGH}.to_i2c_addr()
    };
    #elif PHY_SEL == PHY_SEL_SPI

    auto & spi = spi1;
    spi.init({18_MHz});

    ICM42688 imu = {
        SpiDrv(
            &spi, 
            spi.allocate_cs_gpio(&hal::PA<15>()).unwrap()
        )
    };

    #endif
    
    icm42688_tb(imu);
}