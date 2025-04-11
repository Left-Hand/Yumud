#include "PMW3901.hpp"
#include "types/image/image.hpp"
#include "core/debug/debug.hpp"

#include "core/math/realmath.hpp"


static constexpr uint8_t PMW3901_REG_Product_ID              = 0x00;
static constexpr uint8_t PMW3901_REG_Revision_ID             = 0x01;
static constexpr uint8_t PMW3901_REG_Motion                  = 0x02;
static constexpr uint8_t PMW3901_REG_Delta_X_L               = 0x03;
static constexpr uint8_t PMW3901_REG_Delta_X_H               = 0x04;
static constexpr uint8_t PMW3901_REG_Delta_Y_L               = 0x05;
static constexpr uint8_t PMW3901_REG_Delta_Y_H               = 0x06;
static constexpr uint8_t PMW3901_REG_Squal                   = 0x07;
static constexpr uint8_t PMW3901_REG_RawData_Sum             = 0x08;
static constexpr uint8_t PMW3901_REG_RawData_Max             = 0x09;
static constexpr uint8_t PMW3901_REG_RawData_Min_            = 0x0A;
static constexpr uint8_t PMW3901_REG_Shutter_Lower           = 0x0B;
static constexpr uint8_t PMW3901_REG_Shutter_Upper           = 0x0C;
static constexpr uint8_t PMW3901_REG_Observation             = 0x15;
static constexpr uint8_t PMW3901_REG_Motion_Burst            = 0x16;
static constexpr uint8_t PMW3901_REG_Power_Up_Reset          = 0x3A;
static constexpr uint8_t PMW3901_REG_Shutdown                = 0x3B;
static constexpr uint8_t PMW3901_REG_RawData_Grab            = 0x58;
static constexpr uint8_t PMW3901_REG_RawData_Grab_Status     = 0x59;
static constexpr uint8_t PMW3901_REG_Inverse_Product_ID      = 0x5F;

// #define PMW3901_DEBUG_EN

#ifdef PMW3901_DEBUG_EN
#define PMW3901_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define PMW3901_PANIC(...) PANIC(__VA_ARGS__)
#define PMW3901_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define PMW3901_DEBUG(...)
#define PMW3901_PANIC(...)  PANIC_NSRC()
#define PMW3901_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = PMW3901::Error;

Result<void, Error> PMW3901::write_reg(const uint8_t command, const uint8_t data){
    return spi_drv_.write_single<uint8_t>(command | 0x80, CONT)
    | spi_drv_.write_single<uint8_t>(data);
}


Result<void, Error> PMW3901::read_reg(const uint8_t command, uint8_t & data){
    return spi_drv_.write_single<uint8_t>(command & 0x7f, CONT)
    | spi_drv_.read_single<uint8_t>(data);
}

Result<void, Error> PMW3901::read_burst(const uint8_t command, uint8_t * data, const size_t len){
    return spi_drv_.write_single<uint8_t>(command & 0x7f, CONT)
    | spi_drv_.read_burst<uint8_t>(data, len);
}


// Result<void, Error> PMW3901::read_image(ImageWritable<Grayscale> & img){
//     int count = 0;
//     scexpr uint8_t MASK = 0x0c; //MASK to take bits 2 and 3 from b

//     for (int ii = 0; ii < 1225; ii++) { //for 1 frame of 1225 pixels (35*35)
//         uint8_t a,b, hold;
//         do { 
//         //if data is either invalid status
//         //check status bits 6 and 7
//         //if 01 move upper 6 bits into temp value
//         //if 00 or 11, reread
//         //else lower 2 bits into temp value
//         read_reg(0x58,a); //read register
//         hold = a >> 6; //right shift to leave top two bits for ease of check.
//         } while((hold == 0x03) || (hold == 0x00));
        
//         if (hold == 0x01) { //if data is upper 6 bits
//         read_reg(0x58, b); //read next set to get lower 2 bits
//         uint8_t pixel = a; //set pixel to a
//         pixel = pixel << 2; //push left to 7:2
//         pixel += (b & MASK); //set lower 2 from b to 1:0
//         img[count++] = Grayscale(pixel); //put temp value in fbuffer array
//         //delayMicroseconds(100);
//         }
//     }

//     write_reg(0x70, 0x00);   //More magic? 
//     write_reg(0x58, 0xFF);

//     uint8_t temp, check; 

//     do { //keep reading and testing
//         read_reg(0x58, temp); //read status register
//         check = temp>>6; //rightshift 6 bits so only top two stay 
//     } while(check == 0x03); //while bits aren't set denoting ready state
// }


Result<void, Error> PMW3901::set_led(bool ledOn){
    return write_reg(0x7f, 0x14)
    | write_reg(0x6f, ledOn ? 0x1c : 0x00)
    | write_reg(0x7f, 0x00)
    ;
}

Result<void, Error> PMW3901::read_data(){
    return read_data_burst();
}

Result<void, Error> PMW3901::read_data_slow(){
    std::array<uint8_t, 5> buf;

    for(uint8_t i = 0; i < buf.size(); i++){
        if(const auto res = read_reg(0x02 + i, buf[i]); res.is_err()) return res;
    }

    data_.motion = buf[0];
    data_.dx = (buf[2] << 8) | buf[1];
    data_.dy = (buf[4] << 8) | buf[3];

    return Ok();
}

Result<void, Error> PMW3901::read_data_burst(){
    return read_burst(0x16, &data_.motion, 6);
}


scexpr real_t scale = real_t(13.0/2000);
Result<void, Error> PMW3901::update(){
    return read_data()
    .if_ok([&]{
        x_cm += int16_t(data_.dx) * scale;
        y_cm += int16_t(data_.dy) * scale;
    });

}

Result<void, Error> PMW3901::update(const real_t rad){
    return read_data()
    .if_ok([&]{
        auto delta = Vector2_t<real_t>(data_.dx.as_val(), data_.dy.as_val())
            .rotated(rad - real_t(PI/2)) * scale;
        x_cm += delta.x;
        y_cm += delta.y;
    });

}


Result<bool, Error> PMW3901::assert_reg(const uint8_t command, const uint8_t data){
    uint8_t temp = 0;
    if(const auto res = read_reg(command, temp); res.is_err()) return Err(res.unwrap_err());
    return Ok(temp == data);
}

Result<void, Error> PMW3901::write_list(std::span<const std::pair<uint8_t, uint8_t>> list){
    for(const auto & [cmd, data] : list){
        if(const auto res = write_reg(cmd, data); res.is_err()) return res;
    }
    return Ok();
}

Result<bool, Error> PMW3901::verify(){
    return assert_reg(PMW3901_REG_Inverse_Product_ID, 0xB6)
    | assert_reg(PMW3901_REG_Product_ID, 0x49);
}

Result<void, Error> PMW3901::init() {
    static constexpr auto INIT_LIST1 = std::to_array({
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x00),
        std::make_pair<uint8_t, uint8_t>(0x61, 0xAD),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x03),
        std::make_pair<uint8_t, uint8_t>(0x40, 0x00),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x05),
        std::make_pair<uint8_t, uint8_t>(0x41, 0xB3),
        std::make_pair<uint8_t, uint8_t>(0x43, 0xF1),
        std::make_pair<uint8_t, uint8_t>(0x45, 0x14),
        std::make_pair<uint8_t, uint8_t>(0x5B, 0x32),
        std::make_pair<uint8_t, uint8_t>(0x5F, 0x34),
        std::make_pair<uint8_t, uint8_t>(0x7B, 0x08),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x06),
        std::make_pair<uint8_t, uint8_t>(0x44, 0x1B),
        std::make_pair<uint8_t, uint8_t>(0x40, 0xBF),
        std::make_pair<uint8_t, uint8_t>(0x4E, 0x3F),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x08),
        std::make_pair<uint8_t, uint8_t>(0x65, 0x20),
        std::make_pair<uint8_t, uint8_t>(0x6A, 0x18),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x09),
        std::make_pair<uint8_t, uint8_t>(0x4F, 0xAF),
        std::make_pair<uint8_t, uint8_t>(0x5F, 0x40),
        std::make_pair<uint8_t, uint8_t>(0x48, 0x80),
        std::make_pair<uint8_t, uint8_t>(0x49, 0x80),
        std::make_pair<uint8_t, uint8_t>(0x57, 0x77),
        std::make_pair<uint8_t, uint8_t>(0x60, 0x78),
        std::make_pair<uint8_t, uint8_t>(0x61, 0x78),
        std::make_pair<uint8_t, uint8_t>(0x62, 0x08),
        std::make_pair<uint8_t, uint8_t>(0x63, 0x50),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x0A),
        std::make_pair<uint8_t, uint8_t>(0x45, 0x60),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x00),
        std::make_pair<uint8_t, uint8_t>(0x4D, 0x11),
        std::make_pair<uint8_t, uint8_t>(0x55, 0x80),
        std::make_pair<uint8_t, uint8_t>(0x74, 0x1F),
        std::make_pair<uint8_t, uint8_t>(0x75, 0x1F),
        std::make_pair<uint8_t, uint8_t>(0x4A, 0x78),
        std::make_pair<uint8_t, uint8_t>(0x4B, 0x78),
        std::make_pair<uint8_t, uint8_t>(0x44, 0x08),
        std::make_pair<uint8_t, uint8_t>(0x45, 0x50),
        std::make_pair<uint8_t, uint8_t>(0x64, 0xFF),
        std::make_pair<uint8_t, uint8_t>(0x65, 0x1F),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x14),
        std::make_pair<uint8_t, uint8_t>(0x65, 0x60),
        std::make_pair<uint8_t, uint8_t>(0x66, 0x08),
        std::make_pair<uint8_t, uint8_t>(0x63, 0x78),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x15),
        std::make_pair<uint8_t, uint8_t>(0x48, 0x58),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x07),
        std::make_pair<uint8_t, uint8_t>(0x41, 0x0D),
        std::make_pair<uint8_t, uint8_t>(0x43, 0x14),
        std::make_pair<uint8_t, uint8_t>(0x4B, 0x0E),
        std::make_pair<uint8_t, uint8_t>(0x45, 0x0F),
        std::make_pair<uint8_t, uint8_t>(0x44, 0x42),
        std::make_pair<uint8_t, uint8_t>(0x4C, 0x80),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x10),
        std::make_pair<uint8_t, uint8_t>(0x5B, 0x02),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x07),
        std::make_pair<uint8_t, uint8_t>(0x40, 0x41),
        std::make_pair<uint8_t, uint8_t>(0x70, 0x00),
    });static_assert(sizeof(INIT_LIST1) == 2 * INIT_LIST1.size());

    static constexpr auto INIT_LIST2 = std::to_array({
        std::make_pair<uint8_t, uint8_t>(0x32, 0x44),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x07),
        std::make_pair<uint8_t, uint8_t>(0x40, 0x40),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x06),
        std::make_pair<uint8_t, uint8_t>(0x62, 0xf0),
        std::make_pair<uint8_t, uint8_t>(0x63, 0x00),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x0D),
        std::make_pair<uint8_t, uint8_t>(0x48, 0xC0),
        std::make_pair<uint8_t, uint8_t>(0x6F, 0xd5),
        std::make_pair<uint8_t, uint8_t>(0x7F, 0x00),
        std::make_pair<uint8_t, uint8_t>(0x5B, 0xa0),
        std::make_pair<uint8_t, uint8_t>(0x4E, 0xA8),
        std::make_pair<uint8_t, uint8_t>(0x5A, 0x50),
        std::make_pair<uint8_t, uint8_t>(0x40, 0x80),
    });static_assert(sizeof(INIT_LIST2) == 2 * INIT_LIST2.size());


    spi_drv_.release().unwrap();
    return write_reg(PMW3901_REG_Power_Up_Reset, 0x5A)
    .if_ok([this]{
        delay(5);
        PMW3901_ASSERT(verify().expect("PMW3901 not found!"), "PMW3901 verify failed!");
    })


    | update()
    | write_list(std::span(INIT_LIST1))
    .if_ok([]{delay(1);})    
    | write_list(std::span(INIT_LIST2))
    ;
}
