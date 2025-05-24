#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/PackedImage.hpp"

namespace ymd::drivers{

struct SSD13XX_Config{
    bool flip_x = false;
    bool flip_y = false;
};

struct SSD13XX_Presets{
    struct _72X40   :public SSD13XX_Config{};
    struct _128X64  :public SSD13XX_Config{};
    struct _128X32  :public SSD13XX_Config{};
    struct _88X48   :public SSD13XX_Config{};
    struct _64X48   :public SSD13XX_Config{};
    struct _128X80  :public SSD13XX_Config{};
};


template<typename T>
struct _oled_preset;




class SSD1306_Phy final{
public:
    using Error = DisplayerError;
    template<typename T = void>
    using IResult = Result<T, Error>;
protected:

    hal::I2cDrv i2c_drv_;
    static constexpr uint8_t CMD_TOKEN = 0x00;
    static constexpr uint8_t DATA_TOKEN = 0x40;
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x78);
public:
    SSD1306_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){};
    SSD1306_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){};
    SSD1306_Phy(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){};

    [[nodiscard]] IResult<> init(){
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint32_t cmd){
        return IResult<>(i2c_drv_.write_reg<uint8_t>(CMD_TOKEN, uint8_t(cmd)));
    }

    [[nodiscard]] IResult<> write_data(const uint32_t data){
        return IResult<>(i2c_drv_.write_reg<uint8_t>(DATA_TOKEN, uint8_t(data)));
    }

    [[nodiscard]] IResult<> write_burst(const is_stdlayout auto * pdata, size_t len){
        if constexpr(sizeof(*pdata) != 1){
            return IResult<>(i2c_drv_.write_burst(DATA_TOKEN, std::span(pdata, len), LSB));
        }else {
            return IResult<>(i2c_drv_.write_burst(DATA_TOKEN, std::span(pdata, len)));
        }
    }

    [[nodiscard]] IResult<> write_burst(const is_stdlayout auto data, size_t len){
        if constexpr(sizeof(data) != 1){
            return IResult<>(i2c_drv_.write_repeat(DATA_TOKEN, std::span(data, len), LSB));
        }else {
            return IResult<>(i2c_drv_.write_repeat(DATA_TOKEN, data, len));
        }
    }

    [[nodiscard]] IResult<> write_u8(const uint8_t data, size_t len) {
        return write_burst<uint8_t>(data, len);
    }

    [[nodiscard]] IResult<> write_u8(const uint8_t * data, size_t len) {
        return write_burst<uint8_t>(data, len);
    }
};
    

class SSD13XX final{
public:
    using Phy = SSD1306_Phy;
    using Error = DisplayerError;
    using Config = SSD13XX_Config;

    template<typename T = void>
    using IResult = Result<T, Error>;


    template<typename Cfg, typename T = std::decay_t<Cfg>>
    SSD13XX(Phy && phy, Cfg && cfg):
        phy_(std::move(phy)),
        offset_(_oled_preset<T>::offset), 
        cmds_(std::span(_oled_preset<T>::buf)),
        config_(cfg),
        frame_(_oled_preset<T>::size){
        }

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x78);

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> enable(const bool en = true);

    [[nodiscard]] IResult<> turn_display(const bool i);

    [[nodiscard]] IResult<> enable_flip_y(const bool flip = true){return phy_.write_command(0xA0 | flip);}
    [[nodiscard]] IResult<> enable_flip_x(const bool flip = true){return phy_.write_command(0xC0 | (flip << 3));}
    [[nodiscard]] IResult<> enable_inversion(const bool inv = true){return phy_.write_command(0xA7 - inv);}  

    [[nodiscard]] Vector2u size() const {return frame_.size();}
    VerticalBinaryImage & fetch_frame() {return frame_;};

private:

    Phy phy_;
    [[nodiscard]] IResult<> setarea_unsafe(const Rect2u & area) {
        return setpos_unsafe(area.position);
    }

    [[nodiscard]] IResult<> putpixel_unsafe(const Vector2u & pos, const Binary color){
        auto & frame = fetch_frame();
        frame.putpixel_unsafe(pos, color);
        return Ok();
    }

    [[nodiscard]] IResult<> setpos_unsafe(const Vector2u & pos) ;

    [[nodiscard]] IResult<> set_flush_pos(const Vector2u & pos);
    [[nodiscard]] IResult<> set_offset();

    const Vector2u offset_;
    const std::span<const uint8_t> cmds_;
    const Config config_;
    VerticalBinaryImage frame_;


    [[nodiscard]] IResult<> preinit_by_cmds();
};



namespace details{
template<typename T>
static constexpr Vector2u oled_display_size_v = _oled_preset<T>::size;

template<typename T>
static constexpr Vector2u oled_display_offset_v = _oled_preset<T>::size;

template<typename T>
static constexpr Vector2u oled_initcmd_v = _oled_preset<T>::initcmd;
}

template<>
struct _oled_preset<SSD13XX_Presets::_72X40>{
    static constexpr Vector2u size = Vector2u(72, 40);
    static constexpr Vector2u offset = Vector2u(28, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0xD5,0xF0,0xA8,0X27,0XD3,0X00,0X40,
        0X8D,0X14,0X20,0X02,0XA1,0XC8,0XDA,0X12,
        0XAD,0X30,0X81,0XFF,0XD9,0X22,0XDB,0X20,
        0XA4,0XA6,0X0C,0X11
    });
};

template<>
struct _oled_preset<SSD13XX_Presets::_128X64>{
    static constexpr Vector2u size = Vector2u(128, 64);
    static constexpr Vector2u offset = Vector2u(0, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        // 0xAE, 0xD5, 0xF0, 0xA8, 0X27, 0XD3, 0X00, 0X40,
        // 0X8D, 0X14, 0X20, 0X02, 0XA1, 0XC8, 0XDA, 0X12,
        // 0XAD, 0X30, 0X81, 0XFF, 0XD9, 0X22, 0XDB, 0X20,
        // 0XA4, 0XA6, 0X0C, 0X11
            0xAE,    // 关闭显示
            0x00,    // 设置低列地址
            0x10,    // 设置高列地址
            0x40,    // 设置起始行地址    【5:0】
            0xB0,    // 设置页地址
            0x81,     // 对比度设置，可设置亮度
            0x1F,    // 1~255(亮度设置,越大越亮)
            0xA1,    // 设置段(SEG)的起始映射地址 bit0:0,0->0;1,0->127;
            0xA7,    // 正常显示；0xa7逆显示
            0xA8,    // 设置驱动路数（16~64）
            0x3F,    // 64duty    默认0X3f(1/64) 0x1f(1/32)        
                        // --------0.96:0X3f ；0.91寸:0x1f----------
            0xC8,    // 重映射模式，COM[N-1]~COM0扫描
            0xD3,    // 设置显示偏移
            0x00,    // 无偏移
            0xD5,    // 设置震荡器分频
            0x80,    // 分频因子，使用默认值
            0xD9,    // 设置 Pre-Charge Period    预充电周期
            0xF1,    // 使用官方推荐值，[3:0],PHASE 1;[7:4],PHASE 2;
            0xDA,    // 设置 com pin configuartion
            0x12,    // 使用默认值    0.96寸分辨率是128*64 ,设置12h；
                        // -------------0.91寸分辨率是128*32 ,设置02h-----------
            0xDB,    // 设置 Vcomh，电压倍率，可调节亮度（默认）
            0x40,    // 使用官方推荐值 [6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;
            0x8D,    // 设置OLED电荷泵
            0x14,    // 开显示
            0x20,//设置内存地址模式
            0x02,//[[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
            0xA4,//全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
            0xA6,//设置显示方式;bit0:1,反相显示;0,正常显示
            0x2E,//停用滚动条
            0xAF,    // 开启OLED面板显示
    });
};



template<>
struct _oled_preset<SSD13XX_Presets::_128X32>{
    static constexpr Vector2u size = Vector2u(128, 32);
    static constexpr Vector2u offset = Vector2u(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE, 0x00, 0x10, 0x00, 0xB0, 0X81, 0XFF, 0XA1, 0XA6,
        0XA8, 0X1F, 0XC8, 0XD3, 0X00, 0XD5, 0X80, 0XD9,
        0X1F, 0XDA, 0X00, 0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
};

template<>
struct _oled_preset<SSD13XX_Presets::_88X48>{
    static constexpr Vector2u size = Vector2u(88, 48);
    static constexpr Vector2u offset = Vector2u(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 
        0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,
        0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
};

template<>
struct _oled_preset<SSD13XX_Presets::_64X48>{
    static constexpr Vector2u size = Vector2u(64, 48);
    static constexpr Vector2u offset = Vector2u(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 
        0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,
        0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
};


template<>
struct _oled_preset<SSD13XX_Presets::_128X80>{
    static constexpr Vector2u size = Vector2u(128, 80);
    static constexpr Vector2u offset = Vector2u(2, 0);
    static constexpr auto buf = std::to_array<uint8_t>({ 
        0xAE,0x00, 0x10,0x00, 0xB0, 0X81, 0XFF, 0XA1, 
        0XA6, 0XA8,0X1F,0XC8,0XD3,0X00, 0XD5, 0X80,
        0XD9, 0X1F,0XDA,0X00,0XDB, 0X40, 0X8D, 0X14, 0XAF
    });
};



}
