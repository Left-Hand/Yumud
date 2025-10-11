//这个驱动还在推进状态

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class Si24R1{
public:
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Mode:uint8_t{
        SHUTDOWN,
        STANDBY,
        IDLE_TX,
    };

    enum class Command : uint8_t {
        __RW_MASK = 0b000'11111,
        R_REGISTER        = 0b000'00000, // R_REGISTER
        W_REGISTER        = 0b001'00000, // W_REGISTER
        R_RX_PAYLOAD     = 0b0110'0001,  // R_RX_PAYLOAD
        W_TX_PAYLOAD     = 0b1010'0000,  // W_TX_PAYLOAD
        FLUSH_TX         = 0b1110'0001,  // FLUSH_TX
        FLUSH_RX         = 0b1110'0010,  // FLUSH_RX
        REUSE_TX_PL      = 0b1110'0011,  // REUSE_TX_PL
        R_RX_PL_WID      = 0b0110'0000,  // R_RX_PL_WID
        W_ACK_PAYLOAD    = 0b1010'0000,  // W_ACK_PAYLOAD (注意：可能需要具体的位来替换 PPPP)
        W_TX_PAYLOAD_NO_ACK = 0b1010'0000, // W_TX_PAYLOAD_NO_ACK
        NOP               = 0b1111'1111   // NOP
    };

    using RegAddr = uint8_t;

    

    struct TopConfigReg:public Reg8<>{
        static constexpr RegAddr address = 0x00;
        uint8_t prim_rx:1;
        uint8_t power_up:1;
        uint8_t crc_2bytes:1;
        uint8_t crc_en:1;
        uint8_t mask_max_rt:1;
        uint8_t mask_tx_ds:1;
        uint8_t mask_rx_dr:1;
        uint8_t __resv__:1;
    };

    struct AutoAcknowledgeReg:public Reg8<>{
        static constexpr RegAddr address = 0x01;
        uint8_t p0:1;
        uint8_t p1:1;
        uint8_t p2:1;
        uint8_t p3:1;
        uint8_t p4:1;
        uint8_t p5:1;
        uint8_t __resv__ :2;
    };
    
    struct EnableRxAddressReg:public Reg8<>{
        static constexpr RegAddr address = 0x02;
        uint8_t p0:1;
        uint8_t p1:1;
        uint8_t p2:1;
        uint8_t p3:1;
        uint8_t p4:1;
        uint8_t p5:1;
        uint8_t __resv__ :2;
    };

    struct AddressWidthReg:public Reg8<>{
        static constexpr RegAddr address = 0x03;
        uint8_t pipex_address_width:2;
        uint8_t __resv__:6;
    };

    struct AutoRetransmissionReg:public Reg8<>{
        static constexpr RegAddr address = 0x04;
        // 0000: disabled
        // 0001: up to 1 re-transmit on fail of AA
        // ...
        // 1111: up to 15 re-transmits on fail
        // of AA
        uint8_t retrans_times:4;

        // 0000: wait 250uS
        // 0001: wait 500uS
        // ...
        // 1111: wait 4000uS
        uint8_t retrans_delay:4;
    };

    struct RFChannelReg:public Reg8<>{
        static constexpr RegAddr address = 0x05;

        uint8_t rf_channel:7;
        uint8_t __resv__:1;
    };

    struct RFConfigReg:public Reg8<>{
        static constexpr RegAddr address = 0x06;
        uint8_t rf_power:3;
        uint8_t rf_datarate_highbit:1;
        uint8_t pll_lock:1;
        uint8_t rf_datarate_lowbit:1;
        uint8_t __resv__ :1;
        uint8_t cont_wave:1;
    };

    struct StatusReg:public Reg8<>{
        static constexpr RegAddr address = 0x07;
        uint8_t tx_full:1;
        uint8_t rx_pipe_number:2;
        uint8_t max_rt:1;
        uint8_t tx_ds:1;
        uint8_t rx_dr:1;
        uint8_t __resv__ :1;
    };

    struct TransmissionObservationReg:public Reg8<>{
        static constexpr RegAddr address = 0x08;
        uint8_t arc_cnt:4;
        uint8_t plos_cnt:4;
    };

    struct RssiReg:public Reg8<>{
        static constexpr RegAddr address = 0x09;
        uint8_t rssi_less_than_60dbm:1;
        uint8_t __resv__:7;
    };

    struct RxAddrReg:public Reg8<>{
        static constexpr RegAddr head_address = 0x0A;
        uint8_t data;
    };

    struct TxAddrReg:public Reg8<>{
        static constexpr RegAddr address = 0x10;
        uint64_t data;
    };
    
    struct RxPowerReg:public Reg8<>{
        static constexpr RegAddr head_address = 0x11;
        uint8_t data;
    };

    struct FifoStatusReg:public Reg8<>{
        static constexpr RegAddr address = 0x17;
        uint8_t rx_empty:1;
        uint8_t rx_full:1;
        uint8_t __resv1__:2;
        uint8_t tx_empty:1;
        uint8_t tx_full:1;
        uint8_t tx_reuse:1;
        uint8_t __resv2__:1;
    };

    struct DynamicPayloadLengthReg:public Reg8<>{
        static constexpr RegAddr address = 0x1c;
        uint8_t dpl_p0:1;
        uint8_t dpl_p1:1;
        uint8_t dpl_p2:1;
        uint8_t dpl_p3:1;
        uint8_t dpl_p4:1;
        uint8_t dpl_p5:1;
        uint8_t __resv__:2;
    };

    struct FeatureReg:public Reg8<> {
        static constexpr RegAddr address = 0x1d;
        uint8_t en_dyn_ack : 1;          // Set 1 enables the W_TX_PAYLOAD_NOACK command
        uint8_t en_ack_pay : 1;          // Set 1 enables payload on ACK
        uint8_t en_dpl : 1;              // Set 1 enables dynamic payload length
        uint8_t __resv__:5; 
    };

    TopConfigReg top_config_reg;
    AutoAcknowledgeReg auto_acknowledge_reg;
    EnableRxAddressReg enable_rx_address_reg;
    AddressWidthReg address_width_reg;
    AutoRetransmissionReg auto_retransmission_reg;
    RFChannelReg rf_channel_reg;
    RFConfigReg rf_config_reg;
    StatusReg status_reg;
    TransmissionObservationReg transmission_observation_reg;
    RssiReg rssi_reg;
    RxAddrReg rx_addr_regs[6];
    TxAddrReg tx_addr_reg;
    RxPowerReg rx_power_regs[6];
    FifoStatusReg fifo_status_reg;
    DynamicPayloadLengthReg dynamic_payload_length_reg;
    FeatureReg feature_reg;


    IResult<> write_reg(RegAddr addr, const auto & value){
        addr &= ~uint8_t(Command::__RW_MASK);
        addr |= uint8_t(Command::W_REGISTER);
        spi_drv_.transceive_single(reinterpret_cast<uint8_t &>(status_reg), (addr), CONT);
        return spi_drv_.write_burst(&(value), sizeof(value));
    }

    IResult<> read_reg(RegAddr addr, auto & value){
        addr &= ~uint8_t(Command::__RW_MASK);
        addr |= uint8_t(Command::R_REGISTER);
        const auto res = spi_drv_.transceive_single(reinterpret_cast<uint8_t &>(status_reg), uint8_t(addr), CONT)
        | spi_drv_.read_burst(&(value), sizeof(value));

    
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_fifo(const std::span<uint8_t> buffer){
        if(buffer.size()){
            const uint8_t size = MIN(buffer.size(), 32);
            const auto res =  spi_drv_.transceive_single(reinterpret_cast<uint8_t &>(status_reg), 
                uint8_t(Command::R_RX_PAYLOAD), CONT)
            | spi_drv_.read_burst<uint8_t>(std::span(buffer.data(), size));
    
            if(res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }
    }

    IResult<> write_fifo(const std::span<const uint8_t> buffer){
        if(buffer.size()){
            const uint8_t size = MIN(buffer.size(), 32);
            const auto res =  spi_drv_.transceive_single(reinterpret_cast<uint8_t &>(status_reg), 
                uint8_t(Command::W_TX_PAYLOAD), CONT)
            | spi_drv_.write_burst<uint8_t>(std::span(buffer.data(), size));

            if(res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }
        return Ok();
    }

    IResult<> write_fifo_no_ack(const std::span<const uint8_t> buffer){
        if(buffer.size()){
            const uint8_t size = MIN(buffer.size(), 32);
            const auto res = spi_drv_.transceive_single(status_reg.as_ref(), 
                uint8_t(Command::W_TX_PAYLOAD_NO_ACK), CONT)
            | spi_drv_.write_burst<uint8_t>(std::span(buffer.data(), size));

            if(res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }
        return Ok();
    }

    IResult<> clear_tx_fifo(){
        const auto res = spi_drv_.transceive_single(status_reg.as_ref(), 
            uint8_t(Command::FLUSH_TX));
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();    
    }

    IResult<> clear_rx_fifo(){
        const auto res = spi_drv_.transceive_single(status_reg.as_ref(), 
            uint8_t(Command::FLUSH_RX));
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();   
    }

    IResult<> update_status(){
        const auto res = spi_drv_.transceive_single(status_reg.as_ref(), uint8_t(Command::NOP));
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();   
    }

protected:
    hal::SpiDrv spi_drv_;
public: 
    Si24R1(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    Si24R1(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}

    IResult<size_t> available(){
        uint8_t size;
        if(const auto res = spi_drv_.transceive_single<uint8_t>(status_reg.as_ref(), 
            uint8_t(Command::R_RX_PL_WID), CONT); res.is_err()) 
            return Err(res.unwrap_err());
        if(const auto res = spi_drv_.read_single<uint8_t>(size); 
            res.is_err()) 
            return Err(res.unwrap_err());
        return Ok(size);
    }
};

};