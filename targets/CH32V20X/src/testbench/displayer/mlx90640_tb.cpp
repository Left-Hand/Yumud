#include "tb.h"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/HumitureSensor/MLX90640/mlx90640_api.h"
#include "drivers/HumitureSensor/MLX90640/mlx90640_i2c_driver.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include <ranges>

template<std::ranges::range R, size_t... Is>
constexpr auto to_array_impl(R&& range, std::index_sequence<Is...>) {
    auto it = std::ranges::begin(range);
    return std::array<typename std::ranges::range_value_t<R>, sizeof...(Is)>{
        ((void)Is, *it++)...
    };
}

template<size_t N, std::ranges::range R>
constexpr auto to_array(R&& range) {
    return to_array_impl(std::forward<R>(range), std::make_index_sequence<N>{});
}


using namespace ymd;
using namespace ymd::drivers;

#if CH32V30X

static constexpr size_t MLX90640_COLS = 32;
static constexpr size_t MLX90640_ROWS = 24;

static constexpr auto LCD_WIDTH = 320;
static constexpr auto LCD_HEIGHT = 170;

static constexpr size_t UART_BAUD = 576000;
static constexpr auto MLX90640_I2CADDR = 0x33;

#define SCL_GPIO hal::PE<9>()
#define SDA_GPIO hal::PE<11>()

hal::Gpio scl_gpio_ = SCL_GPIO;
hal::Gpio sda_gpio_ = SDA_GPIO;

hal::I2cSw i2c_sw_ = {&scl_gpio_, &sda_gpio_};
hal::I2cDrv i2c_drv_ = hal::I2cDrv{&i2c_sw_, hal::I2cSlaveAddr<7>::from_u7(MLX90640_I2CADDR)};


namespace ymd::drivers{
Result<void, MLX90640_Error> MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data){

    if(const auto res = i2c_drv_.read_burst<uint16_t>(startAddress, 
        std::span<uint16_t>((data), size_t(nMemAddressRead)), MSB);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

Result<void, MLX90640_Error> MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data){
    if(const auto res = i2c_drv_.write_reg<uint16_t>(writeAddress, data, MSB);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
}



void mlx90640_main(){
    uint16_t EE[832];
    uint16_t Frame[834];
    float Temp[768];

    paramsMLX90640 MLXPars;
    float Vdd = 0;
    float Ta = 0;
    float Tr = 0;


    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };



    init_debugger();

    #ifdef CH32V30X
    auto & spi = hal::spi2;
    #else
    auto & spi = hal::spi1;
    #endif

    spi.init({144_MHz});
    
    auto lcd_blk = hal::PD<0>();
    lcd_blk.outpp(HIGH);


    auto lcd_dc = hal::PD<7>();
    auto dev_rst = hal::PB<7>();
    auto spi_cs = hal::PD<4>();
    const auto spi_fd = spi.allocate_cs_gpio(&spi_cs).unwrap();

    drivers::ST7789 tft{
        drivers::ST7789_Phy{&spi, spi_fd, &lcd_dc, &dev_rst}, 
        {LCD_WIDTH, LCD_HEIGHT}
    };

    tft.init(drivers::st7789_preset::_320X170{}).examine();


    [[maybe_unused]] auto plot_rgb = [&](
        const RGB565 * src, 
        const Rect2u rect
    ){
        // DEBUG_PRINTLN(line.bounding_box());
        tft.put_texture(
            rect,
            src
        ).examine();
    };

    // Color<q16>


    i2c_sw_.init({180_KHz});
    clock::delay(50ms);                                    //预留一点时间让MLX传感器完成自己的初始化
    // MLX90640_SetRefreshRate(MLX90640_I2CADDR, 0).examine();       //0.5hz
    MLX90640_SetRefreshRate(MLX90640_I2CADDR, MLX90640_DataRate::_64Hz).examine();
    MLX90640_I2CRead(MLX90640_I2CADDR, 0x2400, 832, EE).examine();                     //读取像素校正参数
    MLX90640_ExtractParameters(EE, &MLXPars).examine();    //解析校正参数（计算温度时需要）
    // Ta=MLX90640_GetTa(Frame, &MLXPars);                  //计算实时外壳温度
    while (1){
        if (const auto res = MLX90640_GetFrameData(MLX90640_I2CADDR, Frame); res.is_err()){
            static Milliseconds last_millis_ = 0ms;
            const auto curr_millis_ = clock::millis();
            DEBUG_PRINTLN(res.unwrap_err(), curr_millis_ - last_millis_, real_t::from(Vdd), real_t::from(Ta));
            last_millis_ = curr_millis_;
            continue;
        }      //有转换完成的帧

        // continue;
        // Vdd=MLX90640_GetVdd(Frame, &MLXPars);   //计算Vdd（这句可有可无）
        Ta=MLX90640_GetTa(Frame, &MLXPars);                  //计算实时外壳温度
        Tr=Ta-8.0;         //计算环境温度用于温度补偿

        MLX90640_CalculateTo(Frame, &MLXPars, 0.95, Tr, Temp);    //计算像素点温度


        for(uint16_t j = 0; j < MLX90640_ROWS; j++){
            constexpr size_t VALID_PIXELS = 16;
                // Create a view of valid pixels for current row using ranges
            auto row_pixels = to_array<VALID_PIXELS>( 
                std::views::iota(0u, MLX90640_COLS)
                | std::views::filter([=](uint16_t i) {
                    return (i % 2 == j % 2);
                })
                | std::views::transform([=, &Temp](uint16_t i) {
                    return static_cast<real_t>(real_t::from(Temp[i + j * MLX90640_COLS]));
                })

                | std::views::transform([](const real_t temp) -> RGB565{
                    const auto color = Color<q16>::from_hsv(1-temp / 60, 1, 1, 1);
                    return RGB565::from_r5g6b5(
                        uint8_t(color.r * 31),
                        uint8_t(color.g * 63),
                        uint8_t(color.b * 31)
                    );
                })
            );

            auto row_pixels_scaled = 
                std::array<RGB565, VALID_PIXELS * 4>();

            for(size_t i = 0; i < VALID_PIXELS; i++){
                row_pixels_scaled[i * 4] = row_pixels[i];
                row_pixels_scaled[i * 4 + 1] = row_pixels[i];
                row_pixels_scaled[i * 4 + 2] = row_pixels[i];
                row_pixels_scaled[i * 4 + 3] = row_pixels[i];
            }

            // DEBUG_PRINTLN(std::setprecision(2), row_pixels_view);
            plot_rgb(row_pixels_scaled.data(), Rect2u::from_xywh(0, j, VALID_PIXELS * 4, 1));
            // DEBUG_PRINTLN(row_pixels);

            // clock::delay(2ms);
        }

    }
}

#endif