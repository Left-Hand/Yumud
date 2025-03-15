#include "LT8960L.hpp"
#include "sys/debug/debug.hpp"

// #define LT8960L_DEBUG_EN
#define LT8960L_CHEAT_EN

#ifdef LT8960L_DEBUG_EN
#define LT8960L_TODO(...) TODO()
#define LT8960L_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define LT8960L_PANIC(...) PANIC{__VA_ARGS__}
#define LT8960L_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) readReg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) writeReg(reg.address, reg).loc().expect();
#else
#define LT8960L_DEBUG(...)
#define LT8960L_TODO(...) PANIC_NSRC()
#define LT8960L_PANIC(...)  PANIC_NSRC()
#define LT8960L_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) readReg(reg.address, reg).unwrap();
#define WRITE_REG(reg) writeReg(reg.address, reg).unwrap();
#endif


static constexpr size_t packet_len = 64;

using namespace ymd;
using namespace ymd::drivers;

using Error = LT8960L::Error;


template<typename Fn>
Result<void, Error> retry(const size_t times, Fn && fn){
    Result<void, Error> res = std::forward<Fn>(fn)();
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn));
}

void LT8960L::delayT3(){delayMicroseconds(1);}
void LT8960L::delayT5(){delayMicroseconds(1);}

Result<void, Error> LT8960L::DevDriver::writeReg(const LT8960L::RegAddress address, const uint16_t reg){
    LT8960L_DEBUG("write", reg, "at", uint8_t(address));
    return i2c_drv_.writeReg(uint8_t(address), reg, MSB);
}

Result<void, Error> LT8960L::DevDriver::readReg(const LT8960L::RegAddress address, uint16_t & reg){
    LT8960L_DEBUG("read", reg, "at", uint8_t(address));
    return i2c_drv_.readReg(uint8_t(address), reg, MSB);
}

Result<size_t, Error> LT8960L::DevDriver::writeBurst(const RegAddress address, std::span<const std::byte> buf){
    LT8960L_ASSERT(buf.size() < 256 , "data length overload", buf.size());

    const auto u8_len = std::byte(buf.size());

    const auto err = i2c_drv_.writeBlocks<const std::byte, std::byte>(
        R16_Fifo::address, std::span(&u8_len, 1), buf, LSB);

    return rescond(err.ok(), buf.size(), err);
}

Result<size_t, Error> LT8960L::DevDriver::readBurst(const RegAddress address, std::span<std::byte> buf){
    LT8960L_ASSERT(buf.size() < 256 , "data length overload", buf.size());

    
    if(buf.size() < packet_len) return Err(Error(Error::UNSPECIFIED));
    
    std::byte u8_len;
    const auto err = i2c_drv_.operateBlocks<std::byte, std::byte>(
        R16_Fifo::address, std::span(&u8_len, 1), buf, LSB);

    return rescond(err.ok(), buf.size(), err);
}

Result<void, Error> LT8960L::DevDriver::verify(){
    return i2c_drv_.verify();
}

Result<bool, Error> LT8960L::isRfSynthLocked(){
    auto res = readReg(rf_synthlock_reg);
    return rescond(res.is_ok(), bool(rf_synthlock_reg.synthLocked), res.unwrap_err());
}


Result<void, Error> LT8960L::setRfChannel(const uint8_t ch){
    rf_config_reg.rfChannelNo = ch;
    return writeRegs(rf_config_reg);
}

Result<void, Error> LT8960L::setRfFreqMHz(const uint freq){
    
    return Ok();
}

Result<void, Error> LT8960L::setRadioMode(const bool isRx){
    if(isRx){
        rf_config_reg.txEn = false;
        rf_config_reg.rxEn = true;
    }else{
        rf_config_reg.rxEn = false;
        rf_config_reg.txEn = true;
    }
    return writeRegs(rf_config_reg);
}

Result<void, Error> LT8960L::setPaCurrent(const uint8_t current){
    pa_config_reg.paCurrent = current;
    return writeRegs((pa_config_reg));
}

Result<void, Error> LT8960L::setPaGain(const uint8_t gain){
    pa_config_reg.paGain = gain;
    return writeRegs((pa_config_reg));
}
Result<void, Error> LT8960L::setBrclkSel(const BrclkSel brclkSel){
    config1_reg.brclkSel = brclkSel;
    return writeRegs((config1_reg));
}

Result<void, Error> LT8960L::clearFifoWritePtr(){
    fifo_ptr_reg.clearWritePtr = 1;
    return writeRegs((fifo_ptr_reg));
}

Result<void, Error> LT8960L::clearFifoReadPtr(){
    fifo_ptr_reg.clearReadPtr = 1;
    return writeRegs((fifo_ptr_reg));
}

Result<void, Error> LT8960L::setSyncWordBits(const SyncWordBits len){
    config1_reg.syncWordLen= len;
    return writeRegs((config1_reg));
}


Result<void, Error> LT8960L::setSyncWord(const uint32_t syncword){
    sync_word0_reg.word[0] = uint8_t(syncword & 0xff);
    sync_word0_reg.word[1] = uint8_t(syncword >> 8);
    sync_word1_reg.word[2] = uint8_t(syncword >> 16);
    sync_word1_reg.word[3] = uint8_t(syncword >> 24);

    return writeRegs(sync_word0_reg, sync_word1_reg);
    // return writeReg(sync_word0_reg);
    // return writeReg(sync_word1_reg);
}

Result<void, Error> LT8960L::setRetransTime(const uint8_t times){
    config2_reg.retransTimes = times - 1;
    return writeRegs((config2_reg));
}

Result<void, Error> LT8960L::enableAutoAck(const bool en){
    config3_reg.autoAck = en;
    return writeRegs((config3_reg));
}

Result<void, Error> LT8960L::reset(){
    // LT8960L Datasheet v1.1 Page16
    // 第一步：延时20ms//保证初次上电,电路稳定。
    // 第二步：写0x38寄存器0xBFFE//唤醒射频防止射频正处在SLEEP状态。
    // 第三步：写0x38寄存器0xBFFD//执行复位操作

    while(millis() < 20){delay(1);}
    return retry(3, [this](){return this -> intoWake();});
}

Result<void, Error> LT8960L::verify(){
    return reset() | dev_drv_.verify();
}

Result<void, Error> LT8960L::init(const Power power, const uint32_t syncword){
    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c

    return verify()
    | writeReg(1, 0x5781)
    | writeReg(26, 0x3A00)

    | setTxPower(power)

    // 频偏微调 0x1800~0x1807
    | writeReg(28, 0x1800)

    // 数据包配置3Byte前导 32bits同步字 NRZ格式
    | writeReg(32, 0x4800)

    // 重发3次=发1包 重发2包  最大15包
    | writeReg(35, 0x0300) 

    | setSyncWord(syncword)

    // 允错1位
    | writeReg(40, 0x4402)

    // 打开CRC校验 FIFO首字节是长度信息
    | writeReg(41, 0xB000)
    | writeReg(42, 0xFDB0) 
    | writeReg(52, 0x8080)
    ;
}

Result<void, Error> LT8960L::initRf(){
    return Result<void, Error>(Ok())
        // REG 0x01 写0x5781,
        | writeReg(0x01, 0x5781)
        // REG 0x08 写0x6C50,
        | writeReg(0x08, 0x6c50)
        // REG 0x26 写0x3A00, //调制幅度
        | writeReg(0x26, 0x3a00)
        // REG 0x09 写0x7830， //发射功率
        | writeReg(0x09, 0x7830)
        // REG 0x2C 写0x1001, //调制速率62.5Kbps
        | writeReg(0x2c, 0x1001)
        // REG 0x2D 写0x0552, //调制速率62.5Kbps
        | writeReg(0x2d, 0x0552)
        // REG 0x36 写 用户定义段
        // REG 0x39 写 用户定义段
        // REG 0x24 写0x8080, //重置FIFO
        | writeReg(0x24, 0x8080)
    ;
}


Result<void, Error> LT8960L::initBle(){
    // LT8960L Datasheet v1.1 Page17

    return Result<void, Error>(Ok())

        // 基础寄存器配置
        // REG 0x01 写入0x5781：配置通用模式寄存器（启用射频校准和时钟模式）
        | writeReg(0x01, 0x5781)
        
        // REG 0x08 写入0x6C90：射频配置寄存器（2.4G模式下建议使用0x6c50）
        // 注：此处按注释中的0x6C90配置，但需注意与文档建议值的差异
        | writeReg(0x08, 0x6C90)

        // REG 0x09 写入0x7830：PA配置寄存器（设置发射功率和PA电流增益）
        | writeReg(0x09, 0x7830)

        // REG 0x26 写入0x3A00：调制幅度控制寄存器（设置调制信号幅度）
        | writeReg(0x26, 0x3A00)

        // 启用BLE功能
        // REG 0x20 写入0x4A00：配置1寄存器（设置BLE数据包格式）
        // - 启用Manchester编码、同步字长度为32bit、尾缀码为4bit
        | writeReg(0x20, 0x4A00)

        // REG 0x24 写入0xBED6：同步字0寄存器（设置BLE接入地址低16bit）
        | writeReg(0x24, 0xBED6)

        // REG 0x27 写入0x8E89：同步字1寄存器（设置BLE接入地址高16bit）
        | writeReg(0x27, 0x8E89)

        // REG 0x2C 写入0x0101：数据速率寄存器（设置BLE传输速率为1Mbps）
        | writeReg(0x2C, 0x0101)

        // REG 0x2D 写入0x0080：调制选项寄存器（配置BLE调制参数）
        // - 设置GFSK调制、1Mbps速率、1/3 FEC编码
        | writeReg(0x2D, 0x0080)

        // REG 0x0F 写入0xEDCC：功能配置寄存器（启用BLE模式）
        // 注：0xEDCC 是示例值，实际需参考数据手册具体配置
        | writeReg(0x0F, 0xEDCC)
    ;
}


Result<void, Error> LT8960L::intoSleep(){
    TODO();
    return Ok();
}

Result<void, Error> LT8960L::intoWake(){
    return Result<void, Error>(Ok())
        | writeReg(0x38, 0xBFFE)
        | writeReg(0x38, 0xBFFD)
    ;
}

Result<void, Error> LT8960L::setTxPower(const LT8960L::Power power){
    const uint16_t code = [](const LT8960L::Power power_){
        switch(power_){
            case Power::_8_Db :return 0x7830;
            case Power::_6_Db :return 0x7930;
            case Power::_5_Db :return 0x7a30;
            case Power::_3_4_Db :return 0x7b30;
            case Power::_0_2_Db :return 0x7c30;
            case Power::_n1_5_Db :return 0x7d30;
            case Power::_n4_Db :return 0x7e30;
            case Power::_n7_Db :return 0x7f30;
            case Power::_n9_Db :return 0x3f30;
            case Power::_n13_Db :return 0x3fb0;
            case Power::_n19_Db :return 0x3fc0;
            default: LT8960L_PANIC("Invalid power"); while(true);
        }
    }(power);

    pa_config_reg.as_ref() = code;
    return writeRegs(pa_config_reg);
}
Result<void, Error> trasmitRf(const std::span<std::byte> buf){
    return Ok();
}

Result<void, Error> LT8960L::setDataRate(DataRate rate){
    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c

    switch(rate){
        case DataRate::_1M:
            return Result<void, Error>(Ok())
                | writeReg(0x15, 0x65CC)
                | writeReg(0x17, 0x6000)
                | writeReg(0x08, 0x6c90)
                | writeReg(0x44, 0x0100)
                | writeReg(0x45, 0x0080);


        case DataRate::_250K:
            return Result<void, Error>(Ok())
                | writeReg(0x08, 0x6c90)
                | writeReg(0x44, 0x0400)
                | writeReg(0x45, 0x0552);


        case DataRate::_125K:
            return Result<void, Error>(Ok())
                | writeReg(0x15, 0x644C)
                | writeReg(0x17, 0x0000)
                | writeReg(0x08, 0x6c90)
                | writeReg(0x44, 0x0800)
                | writeReg(0x45, 0x0552);
                
                
        case DataRate::_62_5K :
            return Result<void, Error>(Ok())
                | writeReg(0x08, 0x6c50)
                | writeReg(0x44, 0x1000)
                | writeReg(0x45, 0x0552);
    }

    __builtin_unreachable();
}