#include "LT8960L.hpp"
#include "details/LT8960L_Errno.hpp"

using namespace ymd;
using namespace ymd::drivers;


template<typename First, typename ... Ts>
constexpr std::array<First, sizeof...(Ts)> make_array(
    Ts && ... args
){
    using Arr = std::array<First, sizeof...(Ts)>;
    return Arr{std::forward<Ts>(args)...};
}


void LT8960L::States::transition_to(const Kind status){
    status_ = status;
    // LT8960L_DEBUG(uint8_t(status));
};

Result<size_t, Error> LT8960L::transmit_rf(std::span<const uint8_t> buf){
    switch(states_.kind()){
        default:
            LT8960L_PANIC("Invalid state while tx");
            return Err(Error::InvalidState);

        case States::TransmitFailed:
            LT8960L_PANIC("transmit failed");
            [[fallthrough]];

        case States::TransmitSucceed:
            states_ = States::Idle;
            [[fallthrough]];

        case States::Idle:{
            if(const auto res = exit_tx_rx();
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = clear_fifo_write_and_read_ptr();
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = start_listen_pkt();
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = ensure_correct_0x08();
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = enter_tx();
                res.is_err()) return Err(res.unwrap_err());
            states_ = States::Transmitting;

            return Ok(0u);
        }

        case States::Transmitting:{

            if(const auto res = is_pkt_ready(); 
                res.is_err()) return Err(res.unwrap_err());
            else if(res.unwrap() == false) return Ok(0u);

            if(const auto res = write_fifo(buf);
                res.is_err()) return Err(res.unwrap_err());


            {
                // auto cont_res = clear_fifo_write_ptr() | enter_tx() | start_listen_pkt();
                if(const auto res = clear_fifo_write_ptr();
                    res.is_err()) return Err(res.unwrap_err());
                if(const auto res = enter_tx();
                    res.is_err()) return Err(res.unwrap_err());
                if(const auto res = start_listen_pkt();
                    res.is_err()) return Err(res.unwrap_err());
            }
    
            return Ok(0u);
        }
    }
}

Result<size_t, Error> LT8960L::receive_rf(std::span<uint8_t> buf){
    switch(states_.kind()){
        default:
            LT8960L_PANIC("Invalid state while rx", uint8_t(states_.kind()));
            return Err(Error::InvalidState);

        case States::ReceiveFailed:
            LT8960L_DEBUG("receive failed");
            // LT8960L_PANIC("receive failed");
            [[fallthrough]];
        case States::ReceiveSucceed:
            states_ = States::Idle;
            [[fallthrough]];

        case States::Idle:{
            return begin_receive().to(0u).if_ok(
                [&]{states_ = States::Receiving;}
            );
        }

        case States::Receiving:{
            {
                auto res = is_pkt_ready();
                if(res.is_err()) return Err(res.unwrap_err());
                if(false == res.unwrap()) return Ok(0u);
            }
    
            // {
            //     auto res = is_receiving();
            //     if(res.is_err()) return Err(res.unwrap_err());
            //     if(res.unwrap() == true) return Ok(0u);
            // }
    
            const auto len = ({
                const auto res = read_fifo(buf);
                if(res.is_err()){
                    states_.timeout()++;
                    if(states_.timeout() > MAX_RX_RETRY){
                        states_ = States::ReceiveFailed;
                        states_.timeout() = 0;

                        return Err(Error::ReceiveTimeout);
                    }
                }
                res.unwrap();
            });
                
            if(const auto res = enter_rx() ;
                res.is_err()) return Err(res.unwrap_err());
            if(const auto res = start_listen_pkt();
                res.is_err()) return Err(res.unwrap_err());

            return Ok(len);
        }
    }
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
    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(1, 0x5781);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(26, 0x3A00);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_tx_power(power);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(28, 0x1800);    //频偏微调 0x1800~0x1807
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(35, 0x0300);    //重发3次=发1包 重发2包  最大15包
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(40, 0x4402);        //允错1位
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(41, 0xB000);        //打开CRC校验 FIFO首字节是长度信息
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(42, 0xFDB0);    
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(52, 0x8080);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(15, 0xec4c); // 开启ble模式
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(32, 0x4A00);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(36, 0xBED6); // 接入地址
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(39, 0x8E89);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(44, 0x0101); // 1Mbps
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(45, 0x0080);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


auto LT8960L::transmit_ble(std::span<const uint8_t> buf) -> Result<size_t, Error>{
    using RP = std::pair<uint8_t, uint16_t>;
    static constexpr auto CMDS = make_array<RP>(
        RP{7 , 0x0000},
        RP{52, 0x8080}
    );

    for(auto && [reg, data] : CMDS)
        if(const auto res = write_reg(reg, data); 
            res.is_err()) return Err(res.unwrap_err());

    return write_fifo(buf)
        .and_then([](const size_t size) -> Result<size_t, Error>{
            LT8960L_DEBUG("transmit successfully");
            return Ok(size);
        })
        .inspect_err([](const Error err){
            MATCH{err}(
                Error::ReceiveTimeout, []{LT8960L_PANIC("ble timeout");},
                Error::PacketOverlength, []{LT8960L_PANIC("ble overlen");},
                None , []{LT8960L_PANIC("unknown");}
            );
        })
    ;
    
    // write_reg()
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
    //         LL_mclock::delay(0ms);
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
    //         LL_mclock::delay(0ms);
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
    //         LL_mclock::delay(0ms);
    //     }
    //     while(LT8960L_GetPKT()==0);        
    // }        

    // LT8960L_WriteReg(7,0x00,0x00);  //IDLE 
    return Ok(0u);
}

Result<size_t, Error> LT8960L::receive_ble(std::span<uint8_t> buf){
    TODO();
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


Result<void, Error> LT8960L::init(const Power power, const uint32_t syncword){
    // https://github.com/IOsetting/py32f0-template/blob/main/Examples/PY32F002B/LL/GPIO/LT8960L_Wireless/LT8960Ldrv.c

    if(const auto res = phy_.init();
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());

    // 无具体寄存器说明 直接参考手册
    if(const auto res = write_reg(1, 0x5781);
        res.is_err()) return Err(res.unwrap_err());

    // 无具体寄存器说明 直接参考手册
    if(const auto res = write_reg(26, 0x3A00);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = set_tx_power(power);
        res.is_err()) return Err(res.unwrap_err());

    // 频偏微调 0x1800~0x1807
    if(const auto res = write_reg(28, 0x1800);
        res.is_err()) return Err(res.unwrap_err());

    // 数据包配置3Byte前导 32bits同步字 NRZ格式
    if(const auto res = set_preamble_bytes(3);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_syncword_bytes(4);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_pack_type(PacketType::Manchester);
        res.is_err()) return Err(res.unwrap_err());

    // 重发3次=发1包 重发2包  最大15包
    if(const auto res = set_retrans_time(3) ;
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = set_syncword(syncword);
        res.is_err()) return Err(res.unwrap_err());

    // 允错1位
    if(const auto res = set_fifo_empty_threshold(8);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_fifo_full_threshold(16);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_syncword_tolerance_bits(1);
        res.is_err()) return Err(res.unwrap_err());

    // 打开CRC校验 FIFO首字节是长度信息
    if(const auto res = write_reg(41, 0xB000);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(42, 0xFDB0) ;
        res.is_err()) return Err(res.unwrap_err());


    if(const auto res = set_datarate(DataRate::_62_5K);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = clear_fifo_write_and_read_ptr();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = enable_gain_weaken(EN);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

Result<void, Error> LT8960L::init_rf(){
        // REG 0x01 写0x5781,
    if(const auto res = write_reg(0x01, 0x5781);
        res.is_err()) return Err(res.unwrap_err());
        // REG 0x08 写0x6C50,
    if(const auto res = write_reg(0x08, 0x6c50);
        res.is_err()) return Err(res.unwrap_err());
        // REG 0x26 写0x3A00, //调制幅度
    if(const auto res = write_reg(0x26, 0x3a00);
        res.is_err()) return Err(res.unwrap_err());
        // REG 0x09 写0x7830， //发射功率
    if(const auto res = write_reg(0x09, 0x7830);
        res.is_err()) return Err(res.unwrap_err());
        // REG 0x2C 写0x1001, //调制速率62.5Kbps
    if(const auto res = write_reg(0x2c, 0x1001);
        res.is_err()) return Err(res.unwrap_err());
        // REG 0x2D 写0x0552, //调制速率62.5Kbps
    if(const auto res = write_reg(0x2d, 0x0552);
        res.is_err()) return Err(res.unwrap_err());
        // REG 0x36 写 用户定义段
        // REG 0x39 写 用户定义段
        // REG 0x24 写0x8080, //重置FIFO
    if(const auto res = write_reg(0x24, 0x8080);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


Result<void, Error> LT8960L::begin_receive(){
    if(const auto res = exit_tx_rx();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = ensure_correct_0x08();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = clear_fifo_write_and_read_ptr();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = enter_rx();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = start_listen_pkt();
        res.is_err()) return Err(res.unwrap_err());
    states_ = States::Receiving;
    return Ok();
}

Result<void, Error> LT8960L::begin_transmit(){
    // return (set_rf_channel_and_exit_tx_rx(ch)
    // | ensure_correct_0x08()
    // | clear_fifo_write_and_read_ptr()
    // | set_rf_channel_and_enter_rx(ch))
    // .to(0u)
    // .if_ok([&]{is_receiving_ = true;})
    // ;
    TODO();
    return Ok();
}