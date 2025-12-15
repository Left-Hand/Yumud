#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "drivers/IMU/Axis6/ICM42688/icm42688.hpp"
#include "robots/gesture/mahony.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define DBG_UART DEBUGGER_INST
#define SCL_PIN hal::PB<3>()
#define SDA_PIN hal::PB<5>()

static constexpr uint ISR_FREQ = 500;
static constexpr auto INV_FS = (1.0_uq16 / ISR_FREQ);

#define PHY_SEL_I2C 0
#define PHY_SEL_SPI 1

// #define PHY_SEL PHY_SEL_I2C
#define PHY_SEL PHY_SEL_SPI

[[maybe_unused]]
static void icm42688_tb(icm42688::ICM42688 & imu){

    DEBUG_PRINTLN("init started");


    imu.init({
        .acc_odr = icm42688::AccOdr::_500Hz,
        .acc_fs = icm42688::AccFs::_4G,
        // .gyr_odr = icm42688::GyrOdr::_500Hz,
        .gyr_odr = icm42688::GyrOdr::_200Hz,
        .gyr_fs = icm42688::GyrFs::_1000deg
    }).examine();

    iq24 z_radians = 0;
    Microseconds exe_us_ = 0us;

    Vec3<iq24> gyr_ = Vec3<iq24>::ZERO;
    Vec3<iq24> acc_ = Vec3<iq24>::ZERO;

    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(ISR_FREQ),
        .count_mode = hal::TimerCountMode::Up
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            const auto u0 = clock::micros();
            imu.update().examine();
            const auto gyr = imu.read_gyr().examine();
            const auto acc = imu.read_acc().examine();

            z_radians = z_radians + gyr.z * INV_FS;
            // mahony.myupdate_v2(gyr, acc);
            exe_us_ = clock::micros() - u0;

            gyr_ = gyr;
            acc_ = acc;
            break;
        }
        default: break;
        }
    });

    timer.start();

    while(true){
        DEBUG_PRINTLN(
            gyr_, 
            acc_,
            z_radians
        );
        clock::delay(5ms);

    }
}

void icm42688_main(){
    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    DEBUGGER.no_fieldname(EN);

    clock::delay(200ms);

    #if PHY_SEL == PHY_SEL_I2C
    // I2cSw i2c{hal::PA<12>(), hal::PA<15>()};

    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;
    hal::I2cSw i2c{&scl_pin_, &sda_pin_};
    // i2c.init(400_KHz);
    i2c.init({2000_KHz});

    static constexpr auto i2c_addr = ICM42688::I2cAddrBuilder{
        .ad0_level = HIGH
    }.to_i2c_addr();
    auto imu = ICM42688{
        &i2c,
        i2c_addr
    };
    #elif PHY_SEL == PHY_SEL_SPI

    // SPI1_MOSI_GPIO pb5
    // SPI1_MISO_GPIO pb4
    // SPI1_CS_GPIO pa15
    // SPI1_SCLK_GPIO pb3

    auto cs_gpio = hal::PA<15>();
    auto & spi = hal::spi1;
    spi.init({
        .remap = hal::SPI1_REMAP_PB3_PB4_PB5_PA15,
        .baudrate = hal::NearestFreq(2_MHz)
    });

    auto imu = icm42688::ICM42688{
        hal::SpiDrv(
            &spi, 
            spi.allocate_cs_pin(&cs_gpio).unwrap()
        )
    };

    #endif
    
    icm42688_tb(imu);
}