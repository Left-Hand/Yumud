#include "LT8960L_Errno.hpp"



Result<void, Error> LT8960L::set_pa_current(const uint8_t current){
    regs_.pa_config_reg.pa_current = current;
    return write_regs((regs_.pa_config_reg));
}

Result<void, Error> LT8960L::set_pa_gain(const uint8_t gain){
    regs_.pa_config_reg.pa_gain = gain;
    return write_regs((regs_.pa_config_reg));
}
Result<void, Error> LT8960L::set_brclk_sel(const BrclkSel brclk_sel){
    regs_.config1_reg.brclkSel = uint16_t(brclk_sel);
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

Result<void, Error> LT8960L::set_syncword(const uint32_t syncword){
    regs_.sync_word0_reg.word[0] = uint8_t(syncword & 0xff);
    regs_.sync_word0_reg.word[1] = uint8_t(syncword >> 8);
    regs_.sync_word1_reg.word[2] = uint8_t(syncword >> 16);
    regs_.sync_word1_reg.word[3] = uint8_t(syncword >> 24);

    return write_regs(regs_.sync_word0_reg, regs_.sync_word1_reg);
}

Result<void, Error> LT8960L::set_retrans_time(const uint8_t times){
    regs_.config2_reg.retrans_times = times - 1;
    return write_regs((regs_.config2_reg));
}

Result<void, Error> LT8960L::enable_autoack(const bool en){
    regs_.config3_reg.autoack_en = en;
    return write_regs((regs_.config3_reg));
}


Result<void, Error> LT8960L::wake(){
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
            default: __builtin_unreachable();
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

        default: __builtin_unreachable();
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
    // | fill_fifo(0, 16)
    | write_reg(8,0x6C90)
    | set_rf_channel_and_enter_tx(ch)  
    ;
}

Result<void, Error> LT8960L::clear_fifo_write_and_read_ptr(){
    auto & reg = regs_.fifo_ptr_reg;
    reg.clear_read_ptr = 1;
    reg.clear_write_ptr = 1;
    return write_regs(reg);
}


Result<void, Error> LT8960L::start_listen_pkt(){
    if(use_hw_pkt_){
        return phy_.start_hw_listen_pkt();
    }
    return Ok();
}

Result<bool, Error> LT8960L::is_pkt_ready(){
    if(use_hw_pkt_){
        return phy_.check_and_skip_hw_listen_pkt();
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
            else return Err(Error(Error::TransmitTimeout));
        });}
    );
}

Result<void, Error> LT8960L::wait_rst_done(const uint timeout){
    return Ok();
}

Result<void, Error> LT8960L::enable_gain_weaken(const bool en){
    // 1Mbps数据率传输近距离存在阻塞死区（收发相距15cm内增益过强导致
    // 通讯变差），推荐用户使用62.5Kbps传输距离更远且不存在死区。也可
    // 通过降低接收灵敏度减少死区范围，具体操作: 0x38寄存器写0xBCDF
    // 0x0F 寄存器写0x643C 降低接收灵敏度缩小死区。如需恢复灵敏度，用户
    // 可0x38寄存器写0XBFFF，0x0F寄存器写0x644C恢复灵敏度。
    if(en){
        return write_reg(0x38, 0xbcdf)
            | write_reg(0x0f, 0x643c);
    }else{
        return write_reg(0x38, 0xBFFF)
            | write_reg(0x0f, 0x644C);
    }
}

Result<void, Error> LT8960L::ensure_correct_0x08(){
    auto & reg = regs_.reg8;
    if((!on_ble_) and datarate_ == DataRate::_62_5K){
        if(reg.as_val() != 0x6c50) reg.as_ref() = 0x6c50;
        return write_regs(reg);
    }else{
        if(reg.as_val() != 0x6c90) reg.as_ref() = 0x6c90;
        return write_regs(reg);
    }
}


Result<void, Error> LT8960L_Phy::write_reg(
    uint8_t address, 
    uint16_t data
){
    return retry(2, [&]{return this->_write_reg(address, data);});
}

Result<void, Error> LT8960L_Phy::read_reg(
    uint8_t address, 
    uint16_t & data
){
    return retry(2, [&]{return this->_read_reg(address, data);});
}



Result<void, Error> LT8960L::set_preamble_bytes(const uint bytes){
    LT8960L_ASSERT(bytes <= 0x0f, "preamble bytes must be less than 0x0f");

    auto & reg = regs_.config1_reg;
    reg.preamble_len = bytes - 1;
    return write_regs(reg);
}

Result<void, Error> LT8960L::set_syncword_bytes(const uint bytes){
    LT8960L_ASSERT(bytes <= 0x0f, "preamble bytes must be less than 0x0f");
    
    auto & reg = regs_.config1_reg;
    reg.syncword_len = (bytes / 2) - 1;
    return write_regs(reg);
}

Result<void, Error> LT8960L::set_trailer_bits(const uint bits){
    LT8960L_ASSERT(bits <= 0X0f, "preamble bytes must be less than 0x0f");
    
    auto & reg = regs_.config1_reg;
    reg.trailer_len = (bits - 4) >> 1;
    return write_regs(reg);
}

Result<void, Error> LT8960L_Phy::_write_reg(
    uint8_t address, 
    uint16_t data
){
    auto guard = i2c_.create_guard();
    
    auto res = i2c_.begin(hal::LockRequest(address, 0))
        .then([&]{return i2c_.write(data >> 8);})
        .then([&]{return i2c_.write(data);})
    ;

    if(res.is_ok()) return Ok();
    else return Err(Error(res.unwrap_err()));
}

Result<void, Error> LT8960L_Phy::_read_reg(
    uint8_t address, 
    uint16_t & data
){
    auto guard = i2c_.create_guard();
    

    auto res = i2c_.begin(hal::LockRequest(address | 0x80, 0))
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



Result<size_t, Error> LT8960L::read_fifo(std::span<std::byte> buf){
    return phy_.read_burst(Regs::R16_Fifo::address, buf)
        // .if_ok([&](){clear_fifo_write_and_read_ptr().unwrap();})
    ;
}

Result<void, Error> LT8960L::set_pack_type(const PacketType ptype){
    auto & reg = regs_.config1_reg;
    reg.packet_type = uint8_t(ptype);
    return write_regs(reg);
}

Result<void, Error> LT8960L::set_fifo_full_threshold(const uint thd){
    auto & reg = regs_.threshold_reg;
    reg.fifo_full_threshold = thd;
    return write_regs(reg);
}


Result<void, Error> LT8960L::set_fifo_empty_threshold(const uint thd){
    auto & reg = regs_.threshold_reg;
    reg.fifo_empty_threshold = thd;
    return write_regs(reg);
}


Result<void, Error> LT8960L::set_syncword_tolerance_bits(const uint bits){
    // 认为SYNCWORD为正确的阈值
    // 07H表示可以错6bits，01H表示0bit可以错0bits
    auto & reg = regs_.threshold_reg;
    // reg.syncword_threshold = (bits == 0 ? 0 : bits + 1);
    reg.syncword_threshold = (bits + 1);
    return write_regs(reg);
}


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

Result<void, Error> LT8960L::set_radio_mode(const bool is_rx){
    if(is_rx){
        regs_.rf_config_reg.tx_en = false;
        regs_.rf_config_reg.rx_en = true;
    }else{
        regs_.rf_config_reg.rx_en = false;
        regs_.rf_config_reg.tx_en = true;
    }
    return write_regs(regs_.rf_config_reg);
}


Result<void, Error> LT8960L::reset(){
    // LT8960L Datasheet v1.1 Page16
    // 第一步：延时20ms//保证初次上电,电路稳定。

    while(millis() < 20){delay(4);}
    return retry(3, [this](){return this -> wake();});
    // return retry(3, [this]() -> Result<void, Error>{return Err(Error::PacketOverlength);});
}

Result<void, Error> LT8960L::verify(){
    uint16_t buf = 0;

    return reset() 
        | read_reg(Regs::R16_ChipId::address, buf)
        .validate(buf == Regs::R16_ChipId::key, Error::ChipIdMismatch)
    // | phy_.verify()
    // .if_err([](auto && e){
    //     LT8960L_DEBUG("verify failed");
    // })
    ;
}

Result<bool, Error> LT8960L_Phy::check_and_skip_hw_listen_pkt(){
    return Result<bool, Error>(Ok(
        (bool(i2c_.sda().read()))))
        .if_ok([&]{i2c_.sda().set();});
}

Result<void, Error> LT8960L_Phy::start_hw_listen_pkt(){
    i2c_.scl().clr(); 
    i2c_.sda().set(); 
    i2c_.sda().inpu();

    return Ok();
}


Result<bool, Error> LT8960L::is_receiving(){
    auto & reg = regs_.flag_reg;
    return read_regs(reg).to<bool>(reg.rev_sync);
}



Result<void, Error> LT8960L_Phy::init(){
    i2c_.init(600'000);
    return Ok();
}

Result<size_t, Error> LT8960L_Phy::read_burst(uint8_t address, std::span<std::byte> pbuf){


    auto guard = i2c_.create_guard();
    uint32_t len = 0;
    bool invalid = false;

    LT8960L_ASSERT(pbuf.size() <= 0xff, "app given buf length too long");

    auto res = i2c_.begin(hal::LockRequest{uint32_t(address | 0x80), 0})
        .then([&]() -> hal::HalResult{
            const auto err = i2c_.read(len, ACK);
            if(err.is_err()) return err;
            if(len > LT8960L_PACKET_SIZE || len > pbuf.size()) {
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
                pbuf[i] = std::byte(dummy);
            }
            return hal::HalResult::Ok();
        })
    ;

    LT8960L_ASSERT(res.is_ok(), "error while read burst", res);

    if(res.is_ok()) return Ok(invalid ? 0 : len);
    else return Err(Error(res.unwrap_err()));
}


Result<size_t, Error> LT8960L_Phy::write_burst(uint8_t address, std::span<const std::byte> pbuf){
    
    auto guard = i2c_.create_guard();
    
    LT8960L_ASSERT(pbuf.size() <= 0xff, "buf length too long");

    auto res = i2c_.begin(hal::LockRequest{address, 0})
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

