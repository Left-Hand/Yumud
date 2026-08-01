#pragma once
//这个驱动还在推进状态

#pragma once

#include "core/utils/reg_base.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"
#include "hal/conn/spi/spidrv.hpp"


namespace ymd::drivers{

struct Si24R1_Prelude{
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

    static constexpr uint8_t RW_MASK = 0b000'11111;

    enum class [[nodiscard]] Command : uint8_t {
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

};


struct [[nodiscard]] NRF24L01_Prelude{
    enum class Error_Kind{

    };


    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Package{
        NRF24L01,
    };


    struct Command{
        uint8_t bits;

        constexpr uint8_t to_u8() const noexcept {
            return bits;
        }
    };

    struct CommandFactory{
        static constexpr Command R_RX_PAYLOAD = Command(0x61);
        static constexpr Command FLUSH_TX = Command(0xE1);
        static constexpr Command FLUSH_RX = Command(0xE2);
        static constexpr Command REUSE_TX_PL = Command(0xE3);
        static constexpr Command NOP = Command(0xFF);

        static constexpr Command read(const uint8_t addr){
            return Command(0x00 | (addr & 0b11111));
        }

        static constexpr Command write(const uint8_t addr){
            return Command(0x20 | (addr & 0b11111));
        }

        static constexpr Command write_ack_payload(const uint8_t ppp){
            return Command(0xA8 | (ppp & 0b111));
        }
    };
};

struct [[nodiscard]] NRF24L01_Regset:public NRF24L01_Prelude{

    using RegAddr = uint8_t;    

    struct [[nodiscard]] R8_Config:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};

        uint8_t prim_rx:1;
        uint8_t pwr_up:1;
        uint8_t crco:1;
        uint8_t en_crc:1;
        uint8_t mask_max_rt:1;
        uint8_t mask_tx_ds:3;
    }DEF_R8(config_reg)
    
    struct [[nodiscard]] R8_EnAA:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};

        uint8_t enaa_p5:1;
        uint8_t enaa_p4:1;
        uint8_t enaa_p3:1;
        uint8_t enaa_p2:1;
        uint8_t enaa_p1:1;
        uint8_t enaa_p0:1;
        uint8_t __resv__:2;
    }DEF_R8(en_aa_reg)

    struct [[nodiscard]] R8_EnRxAddr:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};

        uint8_t erx_p5:1;
        uint8_t erx_p4:1;
        uint8_t erx_p3:1;
        uint8_t erx_p2:1;
        uint8_t erx_p1:1;
        uint8_t erx_p0:1;
        uint8_t __resv__:2;
    };

    struct [[nodiscard]] R8_SetupAddressWidth:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};

        uint8_t aw:2;
        uint8_t __resv__:6;
    }DEF_R8(setup_addrwidth_reg)

    struct [[nodiscard]] R8_SetupAutoRetransmit:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};

        uint8_t ard:4;
        uint8_t arc:4;
    }DEF_R8(setup_auto_retransmit_reg)

    struct [[nodiscard]] R8_RfChannel:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};

        uint8_t ch:7;
        uint8_t __resv__:1;
    }DEF_R8(rf_channel_reg)

    struct [[nodiscard]] R8_RfSetup:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x06};

        uint8_t lna_hcurr:1;
        uint8_t pf_pwr:2;
        uint8_t rf_dr:1;
        uint8_t pll_lock:1;
        uint8_t __resv__:3;
    }DEF_R8(rf_setup_reg)

    struct [[nodiscard]] R8_Status:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x07};

        uint8_t tx_full:1;
        uint8_t rx_p_no:3;
        uint8_t max_rt:1;
        uint8_t tx_ds:1;
        uint8_t rx_dr:1;
        uint8_t __resv__:1;
    }DEF_R8(status_reg)

    struct [[nodiscard]] R8_ObserveTx:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x08};

        uint8_t plos_cnt:4;
        uint8_t arc_cnt:4;
    }DEF_R8(observe_tx_reg)


    struct [[nodiscard]] R8_CarrierDetect:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x09};

        uint8_t cd:1;
        uint8_t __resv__:7;
    }DEF_R8(carrier_detect_reg)

    struct [[nodiscard]] _R8_RxPwPn:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x09};

        uint8_t rx_pw:6;
        uint8_t __resv__:2;
    };

    _R8_RxPwPn rx_pw_p0, rx_pw_p1, rx_pw_p2, rx_pw_p3, rx_pw_p4, rx_pw_p5;

    struct [[nodiscard]] R8_FifoStatus{
        static constexpr RegAddr REG_ADDR = RegAddr{0x17};

        uint8_t rx_empty:1;
        uint8_t rx_full:1;
        uint8_t __resv1__:2;

        uint8_t tx_empty:1;
        uint8_t tx_full:1;
        uint8_t tx_reuse:1;
        uint8_t __resv2__:1;
    }DEF_R8(fifo_status_reg);
};

struct Si24R1_Regset:public Si24R1_Prelude{


    struct R8_TopConfig:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};

        uint8_t prim_rx:1;
        uint8_t power_up:1;
        uint8_t crc_2bytes:1;
        uint8_t crc_en:1;
        uint8_t mask_max_rt:1;
        uint8_t mask_tx_ds:1;
        uint8_t mask_rx_dr:1;
        uint8_t __resv__:1;
    };

    struct R8_AutoAcknowledge:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};

        uint8_t p0:1;
        uint8_t p1:1;
        uint8_t p2:1;
        uint8_t p3:1;
        uint8_t p4:1;
        uint8_t p5:1;
        uint8_t __resv__ :2;
    };
    
    struct R8_EnableRxAddress:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};

        uint8_t p0:1;
        uint8_t p1:1;
        uint8_t p2:1;
        uint8_t p3:1;
        uint8_t p4:1;
        uint8_t p5:1;
        uint8_t __resv__ :2;
    };

    struct R8_AddressWidth:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};

        uint8_t pipex_address_width:2;
        uint8_t __resv__:6;
    };

    struct R8_AutoRetransmission:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};

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

    struct R8_RFChannel:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};


        uint8_t rf_channel:7;
        uint8_t __resv__:1;
    };

    struct R8_RFConfig:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x06};

        uint8_t rf_power:3;
        uint8_t rf_datarate_highbit:1;
        uint8_t pll_lock:1;
        uint8_t rf_datarate_lowbit:1;
        uint8_t __resv__ :1;
        uint8_t cont_wave:1;
    };

    struct R8_Status:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x07};

        uint8_t tx_full:1;
        uint8_t rx_pipe_number:3;
        uint8_t max_rt:1;
        uint8_t tx_ds:1;
        uint8_t rx_dr:1;
        uint8_t __resv__ :1;
    };

    struct R8_TransmissionObservation:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x08};

        uint8_t arc_cnt:4;
        uint8_t plos_cnt:4;
    };

    struct R8_Rssi:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x09};

        uint8_t rssi_less_than_60dbm:1;
        uint8_t __resv__:7;
    };

    struct R8_RxAddr:public Reg8{
        static constexpr RegAddr head_address = 0x0A;

        uint8_t data;
    };

    struct R8_TxAddr:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x10};

        uint8_t data;
    };
    
    struct R8_RxPower:public Reg8{
        static constexpr RegAddr head_address = 0x11;

        uint8_t data;
    };

    struct R8_FifoStatus:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x17};

        uint8_t rx_empty:1;
        uint8_t rx_full:1;
        uint8_t __resv1__:2;
        uint8_t tx_empty:1;
        uint8_t tx_full:1;
        uint8_t tx_reuse:1;
        uint8_t __resv2__:1;
    };

    struct R8_DynamicPayloadLength:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1c};

        uint8_t dpl_p0:1;
        uint8_t dpl_p1:1;
        uint8_t dpl_p2:1;
        uint8_t dpl_p3:1;
        uint8_t dpl_p4:1;
        uint8_t dpl_p5:1;
        uint8_t __resv__:2;
    };

    struct R8_Feature:public Reg8 {
        static constexpr RegAddr REG_ADDR = RegAddr{0x1d};

        uint8_t en_dyn_ack : 1;          // Set 1 enables the W_TX_PAYLOAD_NOACK command
        uint8_t en_ack_pay : 1;          // Set 1 enables payload on ACK
        uint8_t en_dpl : 1;              // Set 1 enables dynamic payload length
        uint8_t __resv__:5; 
    };

    VALIDATE_R8(R8_TopConfig)
    VALIDATE_R8(R8_AutoAcknowledge)
    VALIDATE_R8(R8_EnableRxAddress)
    VALIDATE_R8(R8_AddressWidth)
    VALIDATE_R8(R8_AutoRetransmission)
    VALIDATE_R8(R8_RFChannel)
    VALIDATE_R8(R8_RFConfig)
    VALIDATE_R8(R8_Status)
    VALIDATE_R8(R8_TransmissionObservation)
    VALIDATE_R8(R8_Rssi)
    VALIDATE_R8(R8_RxAddr)
    VALIDATE_R8(R8_TxAddr)
    VALIDATE_R8(R8_RxPower)
    VALIDATE_R8(R8_FifoStatus)
    VALIDATE_R8(R8_DynamicPayloadLength)
    VALIDATE_R8(R8_Feature)

    R8_TopConfig top_config_reg;
    R8_AutoAcknowledge auto_acknowledge_reg;
    R8_EnableRxAddress enable_rx_address_reg;
    R8_AddressWidth address_width_reg;
    R8_AutoRetransmission auto_retransmission_reg;
    R8_RFChannel rf_channel_reg;
    R8_RFConfig rf_config_reg;
    R8_Status status_reg;
    R8_TransmissionObservation transmission_observation_reg;
    R8_Rssi rssi_reg;
    R8_RxAddr rx_addr_regs[6];
    R8_TxAddr tx_addr_reg;
    R8_RxPower rx_power_regs[6];
    R8_FifoStatus fifo_status_reg;
    R8_DynamicPayloadLength dynamic_payload_length_reg;
    R8_Feature feature_reg;
};

}