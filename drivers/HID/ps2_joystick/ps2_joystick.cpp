#include "ps2_joystick.hpp"



using namespace ymd;
using namespace ymd::drivers;

using Error = Ps2Joystick::Error;

template<typename T = void>
using IResult = Result<T, Error>;


enum class Command:uint8_t{
    RequestData = 0x42
};

struct TxPayload{
    uint8_t right_vibration_strength;
    uint8_t left_vibration_strength;
    uint8_t __resv__[4];

    static constexpr TxPayload from(const uint8_t left, const uint8_t right){
        return {left, right, {}};
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(this), sizeof(*this)};
    }
};

static_assert(sizeof(TxPayload) == 6);

IResult<> Ps2Joystick::init(){
    return Ok();
}


IResult<> Ps2Joystick::update(){

    RxPayload rx_payload;
    if(const auto res = spi_drv_.write_single<uint8_t>((uint8_t)0x01);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint8_t>(
        reinterpret_cast<uint8_t &>(dev_id_), 
        std::bit_cast<uint8_t>(Command::RequestData));

        res.is_err()) return Err(res.unwrap_err());

    uint8_t permit;

    if(const auto res = spi_drv_.transceive_single<uint8_t>(permit, uint8_t(0x00));
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_burst<uint8_t>(
        rx_payload.as_bytes(),
        TxPayload::from(0, 0).as_bytes()
    );  res.is_err()) 
        return Err(res.unwrap_err());

    if(permit == 0x5a){
        rx_payload_ = rx_payload;
    }else{
        // PANIC{permit};
    }

    return Ok();
}

IResult<Ps2Joystick::RxPayload> Ps2Joystick::read_info() const {
    switch(dev_id_){
        case DevId::DIGIT:
            return Err(Error::CantParseAtDigitMode);
        case DevId::ANARED:
            return Ok(rx_payload_);
        default:
            break;
    }
    PANIC{dev_id_};
    return Err(Error::Unreachable);
}



#if 0
// IResult<> send_cmd(){
//     volatile u16 ref=0x01;
//     Data[1] = 0;
//     for(ref=0x01;ref<0x0100;ref<<=1)
//     { 

//         if(ref&CMD)
//         { 

//             DO_H;                   //输出以为控制位
//         }
//         else DO_L;

//         CLK_H;                        //产生时钟
//         delay_us(50);
//         CLK_L;
//         delay_us(50);
//         CLK_H;
//         if(DI)
//             Data[1] = ref|Data[1];
//     }
// }

//手柄配置初始化： 
void PS2_ShortPoll(void) {
    CS_L; 
    delay_us(16); 
    PS2_Cmd(0x01); 
    PS2_Cmd(0x42); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0x00); 
    PS2_Cmd(0x00); 
    CS_H; 
    delay_us(16);
}


//进入配置
void PS2_EnterConfing(void) {
    CS_L;
    delay_us(16); 
    PS2_Cmd(0x01); 
    PS2_Cmd(0x43); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0x01); 
    PS2_Cmd(0x00); 
    PS2_Cmd(0X00);
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    CS_H; 
    delay_us(16);
}


// 发送模式设置 
void PS2_TurnOnAnalogMode(void) {
    CS_L; 
    PS2_Cmd(0x01); 
    PS2_Cmd(0x44); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0x01);//analog=0x01;digital=0x00 软件设置发送模式 
    PS2_Cmd(0xEE);//Ox03 锁存设置，即不可通过按键“MODE ”设置模式。        //0xEE 不锁存软件设置，可通过按键“MODE ”设置模式。 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    CS_H; 
    delay_us(16);
}


// 振动设置
void PS2_VibrationMode(void) {
    CS_L; 
    delay_us(16); 
    PS2_Cmd(0x01); 
    PS2_Cmd(0x4D); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0x00); 
    PS2_Cmd(0X01); 
    CS_H; 
    delay_us(16);
}


// 完成并保存配置
void PS2_ExitConfing(void){
    CS_L;
    delay_us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43); 
    PS2_Cmd(0X00);
    PS2_Cmd(0x00); 
    PS2_Cmd(0x5A); 
    PS2_Cmd(0x5A); 
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A); 
    PS2_Cmd(0x5A); 
    CS_H; 
    delay_us(16);
}


// 手柄配置初始化
void PS2_SetInit(void) 
{
    PS2_ShortPoll();
    PS2_ShortPoll();
    PS2_ShortPoll(); 
    PS2_EnterConfing(); // 进入配置模式 
    PS2_TurnOnAnalogMode(); // “红绿灯”配置模式，并选择是否保存 
    PS2_VibrationMode(); // 开启震动模式 
    PS2_ExitConfing(); // 完成并保存配置
}
/*
可以看出配置函数就是发送命令，发送这些命令后，手柄就会明白自己要做什么了，发送命令时，不需要考虑手柄发来的信息。
手柄配置初始化，PS2_ShortPoll()被执行了3次，主要是为了建立和恢复连接。
具体的配置方式请看注释。
*/
void PS2_Vibration(u8motor1,u8motor2)
{
    CS_L; 
    delay_us(16); 
    PS2_Cmd(0x01); // 开始命令
    PS2_Cmd(0x42);// 请求数据
    PS2_Cmd(0X00);
    PS2_Cmd(motor1);
    PS2_Cmd(motor2); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    PS2_Cmd(0X00); 
    CS_H; 
    delay_us(16);
} 

#endif