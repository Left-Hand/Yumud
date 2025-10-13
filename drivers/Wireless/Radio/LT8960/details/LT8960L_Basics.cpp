#include "LT8960L_Errno.hpp"

using namespace ymd;
using namespace ymd::drivers;

template<typename T = void>
using IResult = Result<T, Error>;

template<typename Fn>
__inline IResult<> wait(const size_t timeout, Fn && fn){
    return retry(timeout, std::forward<Fn>(fn), [](){clock::delay(1ms);});
}

IResult<> LT8960L::set_pa_current(const uint8_t current){
    auto reg = RegCopy(regs_.pa_config_reg);
    reg.pa_current = current;
    return write_reg(reg);
}

IResult<> LT8960L::set_pa_gain(const uint8_t gain){
    auto reg = RegCopy(regs_.pa_config_reg);
    reg.pa_gain = gain;
    return write_reg(reg);
}
IResult<> LT8960L::set_brclk_sel(const BrclkSel brclk_sel){
    auto reg = RegCopy(regs_.config1_reg);
    reg.brclkSel = uint16_t(brclk_sel);
    return write_reg(reg);
}

IResult<> LT8960L::clear_fifo_write_ptr(){
    // regs_.fifo_ptr_reg.clear_write_ptr = 1;
    auto reg = RegCopy(regs_.fifo_ptr_reg);
    reg.clear_write_ptr = 1;
    return write_reg(reg);
}

IResult<> LT8960L::clear_fifo_read_ptr(){
    auto reg = RegCopy(regs_.fifo_ptr_reg);
    reg.clear_read_ptr = 1;
    return write_reg(reg);

}

IResult<> LT8960L::set_syncword(const uint32_t syncword){
    {
        auto reg = RegCopy(regs_.sync_word0_reg);
        reg.word[0] = uint8_t(syncword & 0xff);
        reg.word[1] = uint8_t(syncword >> 8);

        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.sync_word1_reg);
        reg.word[0] = uint8_t(syncword >> 16);
        reg.word[1] = uint8_t(syncword >> 24);

        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> LT8960L::set_retrans_time(const uint8_t times){
    auto reg = RegCopy(regs_.config2_reg);
    reg.retrans_times = times - 1;
    
    return write_reg(reg);
}

IResult<> LT8960L::enable_autoack(const Enable en){
    auto reg = RegCopy(regs_.config3_reg);
    reg.autoack_en = en == EN;
    return write_reg(reg);
}


IResult<> LT8960L::wake(){
    // 第二步：写0x38寄存器0xBFFE//唤醒射频防止射频正处在SLEEP状态。
    // 第三步：写0x38寄存器0xBFFD//执行复位操作
    regs_.i2c_oper_reg.as_ref() = 0xBFFE;
    if(const auto res = write_reg(0x38, 0xBFFE);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x38, 0xBFFD);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> LT8960L::set_tx_power(const LT8960L::Power power){
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
            default: __builtin_unreachable();
        }
    }(power);

    auto reg = RegCopy(regs_.pa_config_reg);
    reg.as_ref() = code;
    return write_reg(reg);
}

IResult<> trasmit_rf(const std::span<uint8_t> buf){
    TODO();
    return Ok();
}

IResult<> LT8960L::set_datarate(LT8960L::DataRate rate){
    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c

    //确保链式调用顺利完成才改变速率
    auto change_datarate = [this](DataRate rate_){
        this->datarate_ = rate_;
        return IResult<>(Ok());
    };

    switch(rate){
        case DataRate::_1M:
            if(const auto res = write_reg(15, 0x65CC);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(17, 0x6000);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg( 8, 0x6c90);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(44, 0x0100);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(45, 0x0080);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = change_datarate(rate);   
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
                
        case DataRate::_250K:
            if(const auto res = write_reg( 8, 0x6c90);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(44, 0x0400);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(45, 0x0552);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = change_datarate(rate);
                res.is_err()) return Err(res.unwrap_err());    
            return Ok();

        case DataRate::_125K:
            if(const auto res = write_reg(15, 0x644C);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(17, 0x0000);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg( 8, 0x6c90);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(44, 0x0800);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(45, 0x0552);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = change_datarate(rate);
                res.is_err()) return Err(res.unwrap_err()); 
            return Ok();
                
                
        case DataRate::_62_5K :
            if(const auto res = write_reg( 8, 0x6c50);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(44, 0x1000);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = write_reg(45, 0x0552);
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = change_datarate(rate); 
                res.is_err()) return Err(res.unwrap_err());
            return Ok();

        default: __builtin_unreachable();
    }

}


IResult<> LT8960L::sleep(){
    if(const auto res = write_reg(7,0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(35,0x4300);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> LT8960L::enable_analog(const Enable en){
    if(en == EN){
        if(const auto res = write_reg(15, 0xEC4C);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = write_reg(17, 0x0000);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else{
        if(const auto res = write_reg(15, 0xEDCC);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = write_reg(17, 0x634F);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
}


IResult<> LT8960L::change_carrier(const Channel ch){
    if(const auto res = write_reg(7,0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(28,0x1802);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(44,0x0100)            ;
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(45,0x0080)            ;
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(41,0x0000);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(52, 0x8080);
        res.is_err()) return Err(res.unwrap_err());
    // if(const auto res = fill_fifo(0, 16);
    //     res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(8,0x6C90);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_rf_channel_and_enter_tx(ch)  ;
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> LT8960L::clear_fifo_write_and_read_ptr(){
auto reg = RegCopy(regs_.fifo_ptr_reg);
    reg.clear_read_ptr = 1;
    reg.clear_write_ptr = 1;
    return write_reg(reg);
}


IResult<> LT8960L::start_listen_pkt(){
    if(use_hw_pkt_){
        return phy_.start_hw_listen_pkt();
    }
    return Ok();
}

IResult<bool> LT8960L::is_pkt_ready(){
    if(use_hw_pkt_){
        return phy_.check_and_skip_hw_listen_pkt();
    }else{
    auto reg = RegCopy(regs_.rf_synthlock_reg);
        return read_reg(reg).to<bool>(reg.pkt_flag_txrx);
    }
}

IResult<bool> LT8960L::is_rst_done(){
auto reg = RegCopy(regs_.rf_synthlock_reg);
    return read_reg(reg).to<bool>(reg.i2c_soft_rstn);
}

IResult<> LT8960L::wait_pkt_ready(const uint timeout){
    return 
    wait(timeout, 
        [this]{return is_pkt_ready()
        .and_then([](bool rdy) -> IResult<>{
            if (rdy) return Ok();
            else return Err(Error(Error::TransmitTimeout));
        });}
    );
}

IResult<> LT8960L::wait_rst_done(const uint timeout){
    return Ok();
}

IResult<> LT8960L::enable_gain_weaken(const Enable en){
    // 1Mbps数据率传输近距离存在阻塞死区（收发相距15cm内增益过强导致
    // 通讯变差），推荐用户使用62.5Kbps传输距离更远且不存在死区。也可
    // 通过降低接收灵敏度减少死区范围，具体操作: 0x38寄存器写0xBCDF
    // 0x0F 寄存器写0x643C 降低接收灵敏度缩小死区。如需恢复灵敏度，用户
    // 可0x38寄存器写0XBFFF，0x0F寄存器写0x644C恢复灵敏度。
    if(en == EN){
        if(const auto res = write_reg(0x38, 0xbcdf);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = write_reg(0x0f, 0x643c);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else{
        if(const auto res = write_reg(0x38, 0xBFFF);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = write_reg(0x0f, 0x644C);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
}

IResult<> LT8960L::ensure_correct_0x08(){
auto reg = RegCopy(regs_.reg8);
    if((!on_ble_) and datarate_ == DataRate::_62_5K){
        if(reg.as_val() != 0x6c50) reg.as_ref() = 0x6c50;
        return write_reg(reg);
    }else{
        if(reg.as_val() != 0x6c90) reg.as_ref() = 0x6c90;
        return write_reg(reg);
    }
}


IResult<> LT8960L_Phy::write_reg(
    uint8_t address, 
    uint16_t data
){
    return retry(2, [&]{return this->_write_reg(address, data);});
}

IResult<> LT8960L_Phy::read_reg(
    uint8_t address, 
    uint16_t & data
){
    return retry(2, [&]{return this->_read_reg(address, data);});
}



IResult<> LT8960L::set_preamble_bytes(const uint bytes){
    LT8960L_ASSERT(bytes <= 0x0f, "preamble bytes must be less than 0x0f");

    auto reg = RegCopy(regs_.config1_reg);
    reg.preamble_len = bytes - 1;
    return write_reg(reg);
}

IResult<> LT8960L::set_syncword_bytes(const uint bytes){
    LT8960L_ASSERT(bytes <= 0x0f, "preamble bytes must be less than 0x0f");
    
    auto reg = RegCopy(regs_.config1_reg);
    reg.syncword_len = (bytes / 2) - 1;
    return write_reg(reg);
}

IResult<> LT8960L::set_trailer_bits(const uint bits){
    LT8960L_ASSERT(bits <= 0X0f, "preamble bytes must be less than 0x0f");
    
    auto reg = RegCopy(regs_.config1_reg);
    reg.trailer_len = (bits - 4) >> 1;
    return write_reg(reg);
}

IResult<> LT8960L_Phy::_write_reg(
    uint8_t address, 
    uint16_t data
){
    auto guard = i2c_.create_guard();
    
    auto res = i2c_.borrow(hal::I2cSlaveAddrWithRw::from_8bits(address))
        .then([&]{return i2c_.write(data >> 8);})
        .then([&]{return i2c_.write(data);})
    ;

    
    if(res.is_err()) return Err(Error(res.unwrap_err()));
    return Ok();
}

IResult<> LT8960L_Phy::_read_reg(
    uint8_t address, 
    uint16_t & data
){
    auto guard = i2c_.create_guard();
    

    auto res = i2c_.borrow(hal::I2cSlaveAddrWithRw::from_8bits(address | 0x80))
        .then([&](){
            uint32_t dummy = 0; 
            const auto err = i2c_.read(dummy, ACK); 
            data = (dummy & 0xff)<< 8;
            return err;
        })

        .then([&](){
            uint32_t dummy = 0; 
            const auto err = i2c_.read(dummy, NACK); 
            data |= (dummy & 0xff);
            return err;
        })

    ;

    if(res.is_ok()) return Ok();
    else return Err(Error(res.unwrap_err()));

}



IResult<size_t> LT8960L::read_fifo(std::span<uint8_t> buf){
    return phy_.read_burst(Regs::R16_Fifo::address, buf)
        // .if_ok([&](){clear_fifo_write_and_read_ptr().unwrap();})
    ;
}

IResult<> LT8960L::set_pack_type(const PacketType ptype){
    auto reg = RegCopy(regs_.config1_reg);
    reg.packet_type = uint8_t(ptype);
    return write_reg(reg);
}

IResult<> LT8960L::set_fifo_full_threshold(const uint thd){
    auto reg = RegCopy(regs_.threshold_reg);
    reg.fifo_full_threshold = thd;
    return write_reg(reg);
}


IResult<> LT8960L::set_fifo_empty_threshold(const uint thd){
    auto reg = RegCopy(regs_.threshold_reg);
    reg.fifo_empty_threshold = thd;
    return write_reg(reg);
}


IResult<> LT8960L::set_syncword_tolerance_bits(const uint bits){
    // 认为SYNCWORD为正确的阈值
    // 07H表示可以错6bits，01H表示0bit可以错0bits
auto reg = RegCopy(regs_.threshold_reg);
    // reg.syncword_threshold = (bits == 0 ? 0 : bits + 1);
    reg.syncword_threshold = (bits + 1);
    return write_reg(reg);
}


IResult<bool> LT8960L::is_rfsynth_locked(){
auto reg = RegCopy(regs_.rf_synthlock_reg);
    return read_reg(reg).to<bool>(reg.synth_locked);
}


IResult<> LT8960L::set_rf_channel(const Channel ch, const bool tx, const bool rx){
auto reg = RegCopy(regs_.rf_config_reg);
    reg.tx_en = tx;
    reg.rx_en = rx;
    reg.rf_channel_no = ch.as_u8();
    return write_reg(reg);
}

IResult<> LT8960L::set_rf_freq_mhz(const uint freq){
    TODO();
    return Ok();
}

IResult<> LT8960L::set_radio_mode(const bool is_rx){
    auto reg = RegCopy(regs_.rf_config_reg);
    if(is_rx){
        reg.tx_en = false;
        reg.rx_en = true;
    }else{
        reg.rx_en = false;
        reg.tx_en = true;
    }
    return write_reg(reg);
}


IResult<> LT8960L::reset(){
    // LT8960L Datasheet v1.1 Page16
    // 第一步：延时20ms//保证初次上电,电路稳定。

    while(clock::millis() < 20ms){clock::delay(4ms);}
    return retry(3, [this](){return this -> wakeup();});
    // return retry(3, [this]() -> IResult<>{return Err(Error::PacketOverlength);});
}

IResult<> LT8960L::validate(){
    uint16_t buf = 0;

    if(const auto res = reset() ;
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(Regs::R16_ChipId::address, buf);
        res.is_err()) return Err(res.unwrap_err());
    if((buf == Regs::R16_ChipId::key))
        return Err(Error::ChipIdMismatch);
    return Ok();
}

IResult<bool> LT8960L_Phy::check_and_skip_hw_listen_pkt(){
    bool is_ok = i2c_.sda().read() == HIGH;
    if(is_ok) i2c_.sda().set();
    return Ok(is_ok);
}

IResult<> LT8960L_Phy::start_hw_listen_pkt(){
    i2c_.scl().clr(); 
    i2c_.sda().set(); 
    i2c_.sda().inpu();

    return Ok();
}


IResult<bool> LT8960L::is_receiving(){
auto reg = RegCopy(regs_.flag_reg);
    return read_regs(reg).to<bool>(reg.rev_sync);
}



IResult<> LT8960L_Phy::init(){
    i2c_.init({600'000});
    return Ok();
}

IResult<size_t> LT8960L_Phy::read_burst(uint8_t address, std::span<uint8_t> pbuf){


    auto guard = i2c_.create_guard();
    uint32_t len = 0;
    bool invalid = false;

    LT8960L_ASSERT(pbuf.size() <= 0xff, "app given buf length too long");

    auto res = i2c_.borrow(hal::I2cSlaveAddrWithRw::from_8bits(static_cast<uint8_t>(address | 0x80)))
        .then([&]() -> hal::HalResult{
            const auto err = i2c_.read(len, ACK);
            if(err.is_err()) return err;
            if(len > LT8960L_MAX_PACKET_SIZE || len > pbuf.size()) {
                // LT8960L_PANIC("read buf length too long", len);
                // return hal::BusError::LengthOverflow;
                invalid = true;
            }
            return hal::HalResult::Ok();
            }
        )

        .then([&]() -> hal::HalResult{
            if(invalid) return hal::HalResult::Ok();
            for(size_t i = 0; i < len; i++){
                uint32_t dummy = 0;
                const auto err = i2c_.read(dummy, (i == len - 1 ? NACK : ACK));
                if(err.is_err()) return err;
                pbuf[i] = uint8_t(dummy);
            }
            return hal::HalResult::Ok();
        })
    ;

    LT8960L_ASSERT(res.is_ok(), "error while read burst", res);

    if(res.is_ok()) return Ok(invalid ? 0 : len);
    else return Err(Error(res.unwrap_err()));
}


IResult<size_t> LT8960L_Phy::write_burst(uint8_t address, std::span<const uint8_t> pbuf){
    
    auto guard = i2c_.create_guard();
    
    LT8960L_ASSERT(pbuf.size() <= 0xff, "buf length too long");

    auto res = i2c_.borrow(hal::I2cSlaveAddrWithRw::from_8bits(address))
        .then([&](){
            return i2c_.write(pbuf.size());
        })

        .then([&]() -> hal::HalResult{
            for(const auto data : pbuf){
                auto err = i2c_.write(uint32_t(data));
                if (err.is_err()) return err;
            }
            return hal::HalResult::Ok();
        })
    ;

    LT8960L_ASSERT(res.is_ok(), "error while write burst", res);

    if(res.is_ok()) return Ok(pbuf.size());
    else return Err(Error(res.unwrap_err()));
}

