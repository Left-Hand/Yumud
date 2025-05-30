#include "LT8960L.hpp"
#include "details/LT8960L_Errno.hpp"

using namespace ymd;
using namespace ymd::drivers;

void LT8960L::States::transition_to(const Kind status){
    status_ = status;
    // LT8960L_DEBUG(uint8_t(status));
};

Result<size_t, Error> LT8960L::transmit_rf(std::span<const uint8_t> buf){
    switch(states_.kind()){
        default:
            LT8960L_PANIC("Invalid state while tx");
            return Err(Error(Error::InvalidState));

        case States::TransmitFailed:
            LT8960L_PANIC("transmit failed");
            [[fallthrough]];

        case States::TransmitSucceed:
            states_ = States::Idle;
            [[fallthrough]];

        case States::Idle:{
            return (exit_tx_rx()
            | clear_fifo_write_and_read_ptr()
            | start_listen_pkt()
            | ensure_correct_0x08()
            | enter_tx())
            .to(0u)
            .if_ok(
                [&]{states_ = States::Transmitting;}
            )
            ;
        }

        case States::Transmitting:{

            const auto pkt_res = is_pkt_ready();
            if (pkt_res.is_err()) return Err(pkt_res.unwrap_err());
            if (false == pkt_res.unwrap()) return Ok(0u);

            auto write_res = write_fifo(buf);
            if(write_res.is_err()) return Err(write_res.unwrap_err());


            {
                // auto cont_res = clear_fifo_write_ptr() | enter_tx() | start_listen_pkt();
                auto cont_res = clear_fifo_write_ptr() | enter_tx() | start_listen_pkt();
                if(cont_res.is_err()) return Err(cont_res.unwrap_err());
            }
    
            return write_res;
        }
    }
}

Result<size_t, Error> LT8960L::receive_rf(std::span<uint8_t> buf){
    switch(states_.kind()){
        default:
            LT8960L_PANIC("Invalid state while rx", uint8_t(states_.kind()));
            return Err(Error(Error::InvalidState));

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
    
            auto read_res =  read_fifo(buf);
            
            if (read_res.is_ok()){
                auto continue_rx_res = 
                        enter_rx() 
                    | start_listen_pkt();

                if(continue_rx_res.is_err()) return Err(continue_rx_res.unwrap_err());
            }else{            
                states_.timeout()++;
                if(states_.timeout() > MAX_RX_RETRY){
                    states_ = States::ReceiveFailed;
                    states_.timeout() = 0;

                    return Err(Error(Error::ReceiveTimeout));
                }
            }

            return read_res;
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
    return validate()
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


template<typename First, typename ... Ts>
constexpr std::array<First, sizeof...(Ts)> make_array(
    Ts && ... args
){
    using Arr = std::array<First, sizeof...(Ts)>;
    return Arr{std::forward<Ts>(args)...};
}

auto LT8960L::transmit_ble(std::span<const uint8_t> buf) -> Result<size_t, Error>{
    using RP = std::pair<uint8_t, uint16_t>;
    static constexpr auto CMDS = make_array<RP>(
        RP{7 , 0x0000},
        RP{52, 0x8080}
    );

    for(auto && [reg, data] : CMDS)
        if(auto res = write_reg(reg, data); res.is_err()) 
            return Err(res.unwrap_err());

    return write_fifo(buf)
        .and_then([](size_t size) -> Result<size_t, Error>{
            LT8960L_DEBUG("transmit successfully");
            return Ok(size);})
        .inspect_err([](Error err){
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

    return phy_.init() 
    | validate()

    // 无具体寄存器说明 直接参考手册
    | write_reg(1, 0x5781)

    // 无具体寄存器说明 直接参考手册
    | write_reg(26, 0x3A00)

    | set_tx_power(power)

    // 频偏微调 0x1800~0x1807
    | write_reg(28, 0x1800)

    // 数据包配置3Byte前导 32bits同步字 NRZ格式
    | set_preamble_bytes(3)
    | set_syncword_bytes(4)
    | set_pack_type(PacketType::Manchester)

    // 重发3次=发1包 重发2包  最大15包
    | set_retrans_time(3) 

    | set_syncword(syncword)

    // 允错1位
    | set_fifo_empty_threshold(8)
    | set_fifo_full_threshold(16)
    | set_syncword_tolerance_bits(1)

    // 打开CRC校验 FIFO首字节是长度信息
    | write_reg(41, 0xB000)
    | write_reg(42, 0xFDB0) 


    | set_datarate(DataRate::_62_5K)
    | clear_fifo_write_and_read_ptr()
    | enable_gain_weaken(EN)
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


Result<void, Error> LT8960L::begin_receive(){
    return (exit_tx_rx()
    | ensure_correct_0x08()
    | clear_fifo_write_and_read_ptr()
    | enter_rx()
    | start_listen_pkt())
    .if_ok([&]{states_ = States::Receiving;})
    ;
}

Result<void, Error> LT8960L::begin_transmit(){
    // return (set_rf_channel_and_exit_tx_rx(ch)
    // | ensure_correct_0x08()
    // | clear_fifo_write_and_read_ptr()
    // | set_rf_channel_and_enter_rx(ch))
    // .to(0u)
    // .if_ok([&]{is_receiving_ = true;})
    // ;
    return Ok();
}