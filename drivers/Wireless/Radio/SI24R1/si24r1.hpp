//这个驱动还在推进状态

#pragma once

#include "core/io/regs.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"




namespace ymd::drivers{

class Si24R1{
protected:
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

    using RegAddress = uint8_t;

    

    struct TopConfigReg:public Reg8<>{
        scexpr RegAddress address = 0x00;
        bool prim_rx:1;
        bool power_up:1;
        bool crc_2bytes:1;
        bool crc_en:1;
        bool mask_max_rt:1;
        bool mask_tx_ds:1;
        bool mask_rx_dr:1;
        uint8_t __resv__:1;
    };

    struct AutoAcknowledgeReg:public Reg8<>{
        scexpr RegAddress address = 0x01;
        bool p0:1;
        bool p1:1;
        bool p2:1;
        bool p3:1;
        bool p4:1;
        bool p5:1;
        uint8_t __resv__ :2;
    };
    
    struct EnableRxAddressReg:public Reg8<>{
        scexpr RegAddress address = 0x02;
        bool p0:1;
        bool p1:1;
        bool p2:1;
        bool p3:1;
        bool p4:1;
        bool p5:1;
        uint8_t __resv__ :2;
    };

    struct AddressWidthReg:public Reg8<>{
        scexpr RegAddress address = 0x03;
        uint8_t pipex_address_width:2;
        uint8_t __resv__:6;
    };

    struct AutoRetransmissionReg:public Reg8<>{
        scexpr RegAddress address = 0x04;
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
        scexpr RegAddress address = 0x05;

        uint8_t rf_channel:7;
        uint8_t __resv__:1;
    };

    struct RFConfigReg:public Reg8<>{
        scexpr RegAddress address = 0x06;
        uint8_t rf_power:3;
        bool rf_datarate_highbit:1;
        bool pll_lock:1;
        bool rf_datarate_lowbit:1;
        uint8_t __resv__ :1;
        bool cont_wave:1;
    };

    struct StatusReg:public Reg8<>{
        scexpr RegAddress address = 0x07;
        bool tx_full:1;
        uint8_t rx_pipe_number:2;
        bool max_rt:1;
        bool tx_ds:1;
        bool rx_dr:1;
        uint8_t __resv__ :1;
    };

    struct TransmissionObservationReg:public Reg8<>{
        scexpr RegAddress address = 0x08;
        uint8_t arc_cnt:4;
        uint8_t plos_cnt:4;
    };

    struct RssiReg:public Reg8<>{
        scexpr RegAddress address = 0x09;
        bool rssi_less_than_60dbm:1;
        uint8_t __resv__:7;
    };

    struct RxAddrReg:public Reg8<>{
        scexpr RegAddress head_address = 0x0A;
        uint8_t data;
    };

    struct TxAddrReg:public Reg8<>{
        scexpr RegAddress address = 0x10;
        uint64_t data;
    };
    
    struct RxPowerReg:public Reg8<>{
        scexpr RegAddress head_address = 0x11;
        uint8_t data;
    };

    struct FifoStatusReg:public Reg8<>{
        scexpr RegAddress address = 0x17;
        bool rx_empty:1;
        bool rx_full:1;
        uint8_t __resv1__:2;
        bool tx_empty:1;
        bool tx_full:1;
        bool tx_reuse:1;
        uint8_t __resv2__:1;
    };

    struct DynamicPayloadLengthReg:public Reg8<>{
        scexpr RegAddress address = 0x1c;
        bool dpl_p0:1;
        bool dpl_p1:1;
        bool dpl_p2:1;
        bool dpl_p3:1;
        bool dpl_p4:1;
        bool dpl_p5:1;
        uint8_t __resv__:2;
    };

    struct FeatureReg:public Reg8<> {
        scexpr RegAddress address = 0x1d;
        bool en_dyn_ack : 1;          // Set 1 enables the W_TX_PAYLOAD_NOACK command
        bool en_ack_pay : 1;          // Set 1 enables payload on ACK
        bool en_dpl : 1;              // Set 1 enables dynamic payload length
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


    hal::BusError write_reg(RegAddress addr, const auto & value){
        addr &= ~uint8_t(Command::__RW_MASK);
        addr |= uint8_t(Command::W_REGISTER);
        spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), (addr), CONT).unwrap();
        return spi_drv.write_burst(&(value), sizeof(value));
    }

    hal::BusError read_reg(RegAddress addr, auto & value){
        addr &= ~uint8_t(Command::__RW_MASK);
        addr |= uint8_t(Command::R_REGISTER);
        spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), uint8_t(addr), CONT).unwrap();
        return spi_drv.read_burst(&(value), sizeof(value));
    }

    hal::BusError readFifo(uint8_t *buffer, size_t size){
        if(size){
            size = MIN(size, 32);
            spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), 
                uint8_t(Command::R_RX_PAYLOAD), CONT).unwrap();
            return spi_drv.read_burst(buffer, size);
        }
    }

    hal::BusError writeFifo(const uint8_t *buffer, size_t size){
        if(size){
            size = MIN(size, 32);
            spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), 
                uint8_t(Command::W_TX_PAYLOAD), CONT).unwrap();
            return spi_drv.write_burst<uint8_t>(buffer, size);
        }
    }

    hal::BusError writeFifoNoAck(const uint8_t *buffer, size_t size){
        if(size){
            size = MIN(size, 32);
            spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), 
                uint8_t(Command::W_TX_PAYLOAD_NO_ACK), CONT).unwrap();
            return spi_drv.write_burst<uint8_t>(buffer, size);
        }
    }

    hal::BusError clearTxFifo(){
        return spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), 
            uint8_t(Command::FLUSH_TX)).unwrap();
    }

    hal::BusError clearRxFifo(){
        return spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), 
            uint8_t(Command::FLUSH_RX)).unwrap();
    }

    hal::BusError updateStatus(){
        return spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), uint8_t(Command::NOP));
    }
protected:
    hal::SpiDrv spi_drv;
public: 
    Si24R1(const hal::SpiDrv & _spi_drv):spi_drv(_spi_drv){;}
    Si24R1(hal::SpiDrv && _spi_drv):spi_drv(_spi_drv){;}

    size_t available(){
        uint8_t size;
        spi_drv.transfer_single(reinterpret_cast<uint8_t &>(status_reg), uint8_t(Command::R_RX_PL_WID), CONT).unwrap();
        spi_drv.read_single((size)).unwrap();
        return size;
    }
};

};