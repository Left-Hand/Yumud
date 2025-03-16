#include "LT8960L.hpp"
#include "sys/debug/debug.hpp"

#define LT8960L_DEBUG_EN
#define LT8960L_CHEAT_EN

#ifdef LT8960L_DEBUG_EN
#define LT8960L_TODO(...) TODO()
#define LT8960L_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define LT8960L_PANIC(...) PANIC{__VA_ARGS__}
#define LT8960L_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define LT8960L_DEBUG(...)
#define LT8960L_TODO(...) PANIC_NSRC()
#define LT8960L_PANIC(...)  PANIC_NSRC()
#define LT8960L_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


static constexpr size_t packet_len = 64;
static constexpr size_t LT8960L_PACKET_SIZE = 12;

using namespace ymd;
using namespace ymd::drivers;

using Error = LT8960L::Error;


template<typename Fn, typename Fn_Dur>
Result<void, Error> retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    Result<void, Error> res = std::forward<Fn>(fn)();
    if(res.is_ok()) return Ok();
    LT8960L_DEBUG("retry", times);
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}


template<typename Fn>
Result<void, Error> retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}

template<typename Fn>
Result<void, Error> wait(const size_t timeout, Fn && fn){
    return retry(timeout, std::forward<Fn>(fn), [](){delay(1);});
}

void LT8960L::delay_t3(){delayMicroseconds(1);}
void LT8960L::delay_t5(){delayMicroseconds(1);}


Result<bool, Error> LT8960L::is_rfsynth_locked(){
    auto & reg = regs_.rf_synthlock_reg;
    return read_reg(regs_.rf_synthlock_reg).to<bool>(reg.synth_locked);
}


Result<void, Error> LT8960L::set_rf_channel(const Channel ch, const bool tx, const bool rx){
    regs_.rf_config_reg.tx_en = tx;
    regs_.rf_config_reg.rx_en = rx;
    regs_.rf_config_reg.rf_channel_no = ch.into_code();
    return write_regs(regs_.rf_config_reg);
}

Result<void, Error> LT8960L::set_rf_freq_mhz(const uint freq){
    return Ok();
}

Result<void, Error> LT8960L::set_radio_mode(const bool isRx){
    if(isRx){
        regs_.rf_config_reg.tx_en = false;
        regs_.rf_config_reg.rx_en = true;
    }else{
        regs_.rf_config_reg.rx_en = false;
        regs_.rf_config_reg.tx_en = true;
    }
    return write_regs(regs_.rf_config_reg);
}

Result<void, Error> LT8960L::set_pa_current(const uint8_t current){
    regs_.pa_config_reg.pa_current = current;
    return write_regs((regs_.pa_config_reg));
}

Result<void, Error> LT8960L::set_pa_gain(const uint8_t gain){
    regs_.pa_config_reg.pa_gain = gain;
    return write_regs((regs_.pa_config_reg));
}
Result<void, Error> LT8960L::set_brclk_sel(const BrclkSel brclkSel){
    regs_.config1_reg.brclkSel = brclkSel;
    return write_regs((regs_.config1_reg));
}

Result<void, Error> LT8960L::clear_fifo_write_ptr(){
    regs_.fifo_ptr_reg.clear_write_ptr = 1;
    return write_regs((regs_.fifo_ptr_reg));
}

Result<void, Error> LT8960L::clear_fifo_read_ptr(){
    regs_.fifo_ptr_reg.clear_read_ptr = 1;
    return write_regs((regs_.fifo_ptr_reg));
}

Result<void, Error> LT8960L::set_syncword_bits(const SyncWordBits len){
    regs_.config1_reg.syncword_len= len;
    return write_regs((regs_.config1_reg));
}


Result<void, Error> LT8960L::set_syncword(const uint32_t syncword){
    regs_.sync_word0_reg.word[0] = uint8_t(syncword & 0xff);
    regs_.sync_word0_reg.word[1] = uint8_t(syncword >> 8);
    regs_.sync_word1_reg.word[2] = uint8_t(syncword >> 16);
    regs_.sync_word1_reg.word[3] = uint8_t(syncword >> 24);

    return write_regs(regs_.sync_word0_reg, regs_.sync_word1_reg);
    // return write_reg(regs_.sync_word0_reg);
    // return write_reg(regs_.sync_word1_reg);
}

Result<void, Error> LT8960L::set_retrans_time(const uint8_t times){
    regs_.config2_reg.retrans_times = times - 1;
    return write_regs((regs_.config2_reg));
}

Result<void, Error> LT8960L::enable_autoack(const bool en){
    regs_.config3_reg.autoack_en = en;
    return write_regs((regs_.config3_reg));
}

Result<void, Error> LT8960L::reset(){
    // LT8960L Datasheet v1.1 Page16
    // 第一步：延时20ms//保证初次上电,电路稳定。

    while(millis() < 20){delay(4);}

    LT8960L_DEBUG("reset");
    return retry(3, [this](){return this -> into_wake();});
    // return retry(3, [this]() -> Result<void, Error>{return Err(Error::PacketOverlength);});
}

Result<void, Error> LT8960L::verify(){
    uint16_t buf = 0;

    return reset() 
        | read_reg(Regs::R16_ChipId::address, buf)
        .validate(buf == Regs::R16_ChipId::key, Error::ChipIdMismatch)
    // | dev_drv_.verify()
    // .if_err([](auto && e){
    //     LT8960L_DEBUG("verify failed");
    // })
    ;
}

Result<void, Error> LT8960L::init(const Power power, const uint32_t syncword){
    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c

    return verify()
    | write_reg(1, 0x5781)
    | write_reg(26, 0x3A00)

    | set_tx_power(power)

    // 频偏微调 0x1800~0x1807
    | write_reg(28, 0x1800)

    // 数据包配置3Byte前导 32bits同步字 NRZ格式
    | write_reg(32, 0x4800)

    // 重发3次=发1包 重发2包  最大15包
    | write_reg(35, 0x0300) 

    | set_syncword(syncword)

    // 允错1位
    | write_reg(40, 0x4402)

    // 打开CRC校验 FIFO首字节是长度信息
    | write_reg(41, 0xB000)
    | write_reg(42, 0xFDB0) 
    | set_datarate(DataRate::_1M)
    | write_reg(52, 0x8080)
    ;
}

Result<void, Error> LT8960L::init_rf(){
    return Result<void, Error>(Ok())
        // REG 0x01 写0x5781,
        | write_reg(0x01, 0x5781)
        // REG 0x08 写0x6C50,
        | write_reg(0x08, 0x6c50)
        // REG 0x26 写0x3A00, //调制幅度
        | write_reg(0x26, 0x3a00)
        // REG 0x09 写0x7830， //发射功率
        | write_reg(0x09, 0x7830)
        // REG 0x2C 写0x1001, //调制速率62.5Kbps
        | write_reg(0x2c, 0x1001)
        // REG 0x2D 写0x0552, //调制速率62.5Kbps
        | write_reg(0x2d, 0x0552)
        // REG 0x36 写 用户定义段
        // REG 0x39 写 用户定义段
        // REG 0x24 写0x8080, //重置FIFO
        | write_reg(0x24, 0x8080)
    ;
}


Result<void, Error> LT8960L::init_ble(const Power power){
    // LT8960L Datasheet v1.1 Page17

    // return Result<void, Error>(Ok())

    //     // 基础寄存器配置
    //     // REG 0x01 写入0x5781：配置通用模式寄存器（启用射频校准和时钟模式）
    //     | write_reg(0x01, 0x5781)
        
    //     // REG 0x08 写入0x6C90：射频配置寄存器（2.4G模式下建议使用0x6c50）
    //     // 注：此处按注释中的0x6C90配置，但需注意与文档建议值的差异
    //     | write_reg(0x08, 0x6C90)

    //     // REG 0x09 写入0x7830：PA配置寄存器（设置发射功率和PA电流增益）
    //     | write_reg(0x09, 0x7830)

    //     // REG 0x26 写入0x3A00：调制幅度控制寄存器（设置调制信号幅度）
    //     | write_reg(0x26, 0x3A00)

    //     // 启用BLE功能
    //     // REG 0x20 写入0x4A00：配置1寄存器（设置BLE数据包格式）
    //     // - 启用Manchester编码、同步字长度为32bit、尾缀码为4bit
    //     | write_reg(0x20, 0x4A00)

    //     // REG 0x24 写入0xBED6：同步字0寄存器（设置BLE接入地址低16bit）
    //     | write_reg(0x24, 0xBED6)

    //     // REG 0x27 写入0x8E89：同步字1寄存器（设置BLE接入地址高16bit）
    //     | write_reg(0x27, 0x8E89)

    //     // REG 0x2C 写入0x0101：数据速率寄存器（设置BLE传输速率为1Mbps）
    //     | write_reg(0x2C, 0x0101)

    //     // REG 0x2D 写入0x0080：调制选项寄存器（配置BLE调制参数）
    //     // - 设置GFSK调制、1Mbps速率、1/3 FEC编码
    //     | write_reg(0x2D, 0x0080)

    //     // REG 0x0F 写入0xEDCC：功能配置寄存器（启用BLE模式）

    //     // 注：0xEDCC 是示例值，实际需参考数据手册具体配置
    //     | write_reg(0x0F, 0xEDCC)


    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c
    return verify()
        | write_reg(1, 0x5781)
        | write_reg(26, 0x3A00)
        | set_tx_power(power)
        | write_reg(28, 0x1800)    //频偏微调 0x1800~0x1807
        | write_reg(35, 0x0300)    //重发3次=发1包 重发2包  最大15包
        | write_reg(40, 0x4402)        //允错1位
        | write_reg(41, 0xB000)        //打开CRC校验 FIFO首字节是长度信息
        | write_reg(42, 0xFDB0)    
        | write_reg(52, 0x8080)
        | write_reg(15, 0xec4c) // 开启ble模式
        | write_reg(32, 0x4A00)
        | write_reg(36, 0xBED6) // 接入地址
        | write_reg(39, 0x8E89)
        | write_reg(44, 0x0101) // 1Mbps
        | write_reg(45, 0x0080)
    ;
}


Result<void, Error> LT8960L::into_sleep(){
    TODO();
    return Ok();
}

Result<void, Error> LT8960L::into_wake(){
    // 第二步：写0x38寄存器0xBFFE//唤醒射频防止射频正处在SLEEP状态。
    // 第三步：写0x38寄存器0xBFFD//执行复位操作
    regs_.i2c_oper_reg.as_ref() = 0xBFFE;
    return Result<void, Error>(Ok())
        | write_reg(0x38, 0xBFFE)
        | write_reg(0x38, 0xBFFD)
    ;
}

Result<void, Error> LT8960L::set_tx_power(const LT8960L::Power power){
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

    regs_.pa_config_reg.as_ref() = code;
    return write_regs(regs_.pa_config_reg);
}
Result<void, Error> trasmit_rf(const std::span<std::byte> buf){
    return Ok();
}

Result<void, Error> LT8960L::set_datarate(LT8960L::DataRate rate){
    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c

    //确保链式调用顺利完成才改变速率
    auto change_datarate = [this](DataRate rate_){
        this->datarate_ = rate_;
        return Result<void, Error>(Ok());
    };

    switch(rate){
        case DataRate::_1M:
            return Result<void, Error>(Ok())
                | write_reg(15, 0x65CC)
                | write_reg(17, 0x6000)
                | write_reg( 8, 0x6c90)
                | write_reg(44, 0x0100)
                | write_reg(45, 0x0080)
                | change_datarate(rate)    
                ;
                
                
                case DataRate::_250K:
                return Result<void, Error>(Ok())
                | write_reg( 8, 0x6c90)
                | write_reg(44, 0x0400)
                | write_reg(45, 0x0552)
                | change_datarate(rate)    
                ;

        case DataRate::_125K:
            return Result<void, Error>(Ok())
                | write_reg(15, 0x644C)
                | write_reg(17, 0x0000)
                | write_reg( 8, 0x6c90)
                | write_reg(44, 0x0800)
                | write_reg(45, 0x0552)
                | change_datarate(rate)  
                ;
                
                
        case DataRate::_62_5K :
            return Result<void, Error>(Ok())
                | write_reg( 8, 0x6c50)
                | write_reg(44, 0x1000)
                | write_reg(45, 0x0552)
                | change_datarate(rate)  
                ;

        default: LT8960L_PANIC("Invalid data rate");
    }

    __builtin_unreachable();
}


Result<void, Error> LT8960L::sleep(){
    return  write_reg(7,0)
        | write_reg(35,0x4300);
}

Result<void, Error> LT8960L::enable_analog(const bool en){
    if(en){
        return write_reg(15, 0xEC4C)
            | write_reg(17, 0x0000);
    }else{
        return write_reg(15, 0xEDCC)
            | write_reg(17, 0x634F);
    }
}


Result<void, Error> LT8960L::change_carrier(const Channel ch){
    return  write_reg(7,0)
    | write_reg(28,0x1802)
    | write_reg(44,0x0100)            
    | write_reg(45,0x0080)            
    | write_reg(41,0x0000)
    | write_reg(52, 0x8080)
    | fill_fifo(0, 16)
    | write_reg(8,0x6C90)
    | set_rf_channel_and_into_tx(ch)  
    ;
}

Result<void, Error> LT8960L::clear_fifo_write_and_read_ptr(){
    auto & reg = regs_.fifo_ptr_reg;
    reg.clear_read_ptr = 1;
    reg.clear_write_ptr = 1;
    return write_regs(reg);
}

Result<size_t, Error> LT8960L::transmit_rf(Channel ch, std::span<const std::byte> buf){
    // return set_rf_channel_no_tx_rx(ch);

    // auto res2 = clear_fifo_write_and_read_ptr();
    return write_fifo(buf);
    
    // | [&](size_t len) -> Result<size_t, Error> {
    //     return [this] -> Result<size_t, Error> {
    //         switch(datarate_){
    //             case DataRate::_62_5K: return write_reg(8,0x6c50);
    //             default: return write_reg(8,0x6c90);
    //         }
    //     }()
    //     | set_rf_channel_and_into_tx(ch)
    //     .to(len);
    // }
    // return res3;
}

Result<size_t, Error> LT8960L::receive_rf(std::span<std::byte> buf){
// //1mS
// uint8_t Status=0,len;
// if(RXBusy==0)
// {
//     RXBusy=1;
//     LT8960L_WriteReg(7,0,FreqChannel);
//     //LT8960L_Change_0x38();
    
//     #ifdef Air_rate_62K5    
//     LT8960L_WriteReg( 8, 0x6c, 0x50);
//     #else
//     LT8960L_WriteReg( 8, 0x6c, 0x90);
//     #endif
    
//     LT8960L_WriteReg(52, 0x80, 0x80);
//     LT8960L_WriteReg(7,0,FreqChannel|0X80); 
//     return Status;
// }
// if(LT8960L_GetPKT() && RXBusy)
// {
//     LT8960L_ReadReg(48);
//     RXBusy=0; 
//     Rx_TimeOUT=0;
//     if((regs_.LT8960L_regH&0x80)==0)
//     {
//         len = LT8960L_ReadBUF(50,pBuf);
//         return len;
//     }
// }
// Rx_TimeOUT++;            //1mS++ / 或者放在定时器中断+1
// if(Rx_TimeOUT>100)
// {
//     Rx_TimeOUT=0;
//     RXBusy=0;
// }            
// return Status;  
    return Ok(0u);
}

Result<size_t, Error> LT8960L::transmit_ble(std::span<const std::byte> buf){
    // LT8960L_WriteReg(7,0x00,0x00);  //IDLE 
    // LT8960L_WriteReg(52,0x80,0x80);  //Clean fifo
    // LT8960L_WriteBUF_BLE(50,pBuf,length);                

    // if(ChannelEnable&0x01)
    // {
    //     //ble ch37 tx
    //     LT8960L_WriteReg(46, 0x25, 0x00); 
    //     LT8960L_WriteReg(7, 0x01,0);
    //     do
    //     {
    //         LL_mDelay(0);
    //     }
    //     while(LT8960L_GetPKT()==0);    
    // }        


    // if(ChannelEnable&0x02)
    // {
    //     //ble ch38 tx
    //     LT8960L_WriteReg(46, 0x26, 0x00); 
    //     LT8960L_WriteReg(7, 0x01,24);
    //     do
    //     {
    //         LL_mDelay(0);
    //     }
    //     while(LT8960L_GetPKT()==0);            
    // }

    // if(ChannelEnable&0x04)
    // {
    //     //ble ch39 tx
    //     LT8960L_WriteReg(46, 0x27, 0x00);
    //     LT8960L_WriteReg(7, 0x01,78);        
    //     do
    //     {
    //         LL_mDelay(0);
    //     }
    //     while(LT8960L_GetPKT()==0);        
    // }        

    // LT8960L_WriteReg(7,0x00,0x00);  //IDLE 
    return Ok(0u);
}

Result<size_t, Error> LT8960L::receive_ble(std::span<std::byte> buf){
    // uint8_t i, len;
    // LT8960L_start();
    // LT8960L_Send_Byte(50 | 0x80);
    // LT8960L_ack(); // Register address
    // pBuf[0] = LT8960L_Read_Byte();
    // LT8960L_ack_1uS();                   // pdu-type
    // len = pBuf[1] = LT8960L_Read_Byte(); // pdu-length

    // if (len > 37)
    // {
    //     // DataOK_flag=2;
    //     // printf("Fifo overflow");
    //     len = 0; // error
    //     goto RxExit;
    //     // len=37;
    // }

    // for (i = 2; i < len + 2; i++)
    // {
    //     LT8960L_ack_1uS();
    //     pBuf[i] = LT8960L_Read_Byte();
    // }
    // // DataOK_flag=1;

    // RxExit:                    
    //     LT8960L_nack();
    //     LT8960L_stop();
    //     return len;
    return Ok(0u);
}

Result<bool, Error> LT8960L::is_pkt_ready(){
    if(use_hw_pkt_){
        TODO();
    }else{
        auto & reg = regs_.rf_synthlock_reg;
        return read_reg(reg).to<bool>(reg.pkt_flag_txrx);
    }
}

Result<bool, Error> LT8960L::is_rst_done(){
    auto & reg = regs_.rf_synthlock_reg;
    return read_reg(reg).to<bool>(reg.i2c_soft_rstn);
}

Result<void, Error> LT8960L::wait_pkt_ready(const uint timeout){
    return 
    wait(timeout, 
        [this]{return is_pkt_ready()
        .and_then([](bool rdy) -> Result<void, Error>{
            if (rdy) return Ok();
            else return Err(Error::TransmitTimeout);
        });}
    );
}

Result<void, Error> LT8960L::wait_rst_done(const uint timeout){
    return Ok();
}





using LT8960L_Phy = LT8960L::LT8960L_Phy;

Result<void, Error> LT8960L_Phy::write_reg(
    uint8_t address, 
    uint16_t data
){
    auto guard = createGuard();
    
    auto res = bus_.begin(address)
        .then([&]{return bus_.write(data >> 8);})
        .then([&]{return bus_.write(data);})
    ;

    return Result<void, Error>(res);
}

Result<void, Error> LT8960L_Phy::read_reg(
    uint8_t address, 
    uint16_t & data
){
    auto guard = createGuard();
    

    auto res = bus_.begin(address | 0x80);
    [&](){
        uint32_t dummy = 0; 
        const auto err = bus_.read(dummy, ACK); 
        data = (dummy & 0xff)<< 8;
        return err;
    }();

    [&](){
        uint32_t dummy = 0; 
        const auto err = bus_.read(dummy, NACK); 
        data |= (dummy & 0xff);
        return err;
    }();


    return Result<void, Error>(res);

}