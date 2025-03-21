//这个驱动还在推进状态


#include "core/io/regs.hpp"
#include "core/utils/result.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{


struct _NRF24L01_Regs{

    using RegAddress = uint8_t;    

    struct R8_Config:public Reg8<>{
        static constexpr RegAddress address = 0x00;
        uint8_t prim_rx:1;
        uint8_t pwr_up:1;
        uint8_t crco:1;
        uint8_t en_crc:1;
        uint8_t mask_max_rt:1;
        uint8_t mask_tx_ds:3;
    }DEF_R8(config_reg)
    
    struct R8_EnAA:public Reg8<>{
        static constexpr RegAddress address = 0x01;
        uint8_t enaa_p5:1;
        uint8_t enaa_p4:1;
        uint8_t enaa_p3:1;
        uint8_t enaa_p2:1;
        uint8_t enaa_p1:1;
        uint8_t enaa_p0:1;
        uint8_t reserved:2;
    }DEF_R8(en_aa_reg)

    struct R8_EnRxAddr:public Reg8<>{
        static constexpr RegAddress address = 0x02;
        uint8_t erx_p5:1;
        uint8_t erx_p4:1;
        uint8_t erx_p3:1;
        uint8_t erx_p2:1;
        uint8_t erx_p1:1;
        uint8_t erx_p0:1;
        uint8_t reserved:2;
    };

    struct R8_SetupAddressWidth:public Reg8<>{
        static constexpr RegAddress address = 0x03;
        uint8_t aw:2;
        uint8_t __resv__:6;
    }DEF_R8(setup_addrwidth_reg)

    struct R8_SetupAutoRetransmit:public Reg8<>{
        static constexpr RegAddress address = 0x04;
        uint8_t ard:4;
        uint8_t arc:4;
    }DEF_R8(setup_auto_retransmit_reg)

    struct R8_RfChannel:public Reg8<>{
        static constexpr RegAddress address = 0x05;
        uint8_t ch:7;
        uint8_t __resv__:1;
    }DEF_R8(rf_channel_reg)

    struct R8_RfSetup:public Reg8<>{
        static constexpr RegAddress address = 0x06;
        uint8_t lna_hcurr:1;
        uint8_t pf_pwr:2;
        uint8_t rf_dr:1;
        uint8_t pll_lock:1;
        uint8_t __rsv__:3;
    }DEF_R8(rf_setup_reg)

    struct R8_Status:public Reg8<>{
        static constexpr RegAddress address = 0x07;
        uint8_t tx_full:1;
        uint8_t rx_p_no:3;
        uint8_t max_rt:1;
        uint8_t tx_ds:1;
        uint8_t rx_dr:1;
        uint8_t __resv__:1;
    }DEF_R8(status_reg)

    struct R8_ObserveTx:public Reg8<>{
        static constexpr RegAddress address = 0x08;
        uint8_t plos_cnt:4;
        uint8_t arc_cnt:4;
    }DEF_R8(observe_tx_reg)


    struct R8_CarrierDetect:public Reg8<>{
        static constexpr RegAddress address = 0x09;
        uint8_t cd:1;
        uint8_t __resv__:7;
    }DEF_R8(carrier_detect_reg)

    struct _R8_RxPwPn:public Reg8<>{
        static constexpr RegAddress address = 0x09;
        uint8_t rx_pw:6;
        uint8_t __resv__:2;
    };

    _R8_RxPwPn rx_pw_p0, rx_pw_p1, rx_pw_p2, rx_pw_p3, rx_pw_p4, rx_pw_p5;

    struct R8_FifoStatus{
        static constexpr RegAddress address = 0x17;
        uint8_t rx_empty:1;
        uint8_t rx_full:1;
        uint8_t __resv1__:2;
        
        uint8_t tx_empty:1;
        uint8_t tx_full:1;
        uint8_t tx_reuse:1;
        uint8_t __resv2__:1;
    }DEF_R8(fifo_status_reg);
};

// constexpr auto a = sizeof(_NRF24L01_Regs)


class NRF24L01{
public:
    using Error = BusError;

    enum class Package{
        NRF24L01,
    };
protected:
    using SpiDrvProxy = std::optional<hal::SpiDrv>;
    SpiDrvProxy p_spi_drv_ = std::nullopt;

    using Regs = _NRF24L01_Regs;
    Regs regs_;
    
    class Command{
    public:
        enum Type:uint8_t{
            R_REGISTER = 0x00,
            W_REGISTER = 0x20,
            R_RX_PAYLOAD = 0x61,
            FLUSH_TX = 0xE1,
            FLUSH_RX = 0xE2,
            REUSE_TX_PL = 0xE3,
            NOP = 0xFF
        };

        static constexpr Command READ(const uint8_t addr){
            return Command(0x00 | (addr & 0b11111));
        }

        static constexpr Command WRITE(const uint8_t addr){
            return Command(0x20 | (addr & 0b11111));
        }

        static constexpr Command WRITE_ACK_PAYLOAD(const uint8_t ppp){
            return Command(0xA8 | (ppp & 0b111));
        }

        explicit operator uint8_t() const{return cmd_;}
    protected:
        constexpr Command(const uint8_t cmd): cmd_(cmd){;}
        constexpr Command(const Type cmd): cmd_(static_cast<uint8_t>(cmd)){;}

        uint8_t cmd_;
    };

    [[nodiscard]] Result<void, BusError> write_command(const Command cmd);
    [[nodiscard]] Result<void, BusError> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> write_reg(const T & reg){return write_reg(reg.address, reg);}

    [[nodiscard]] Result<void, BusError> read_reg(const uint8_t addr, uint8_t & data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> read_reg(T & reg){return read_reg(reg.address, reg);}

    

public:
    NRF24L01(const hal::SpiDrv spi_drv):p_spi_drv_(spi_drv){;}
    NRF24L01(const NRF24L01 & other) = delete;
    NRF24L01(NRF24L01 && other) = delete;

    Result<void, Error> verify();

    Result<void, Error> init();
    
    Result<void, Error> update();

    Result<size_t, Error> transmit(std::span<std::byte> buf);

    Result<size_t, Error> receive(std::span<std::byte> buf);

    // Result<Regs, Error> dump();
};

};

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::NRF24L01::Error, BusError> {
        static Result<T, drivers::NRF24L01::Error> convert(const BusError berr){
            using Error = drivers::NRF24L01::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return NRF24L01::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}