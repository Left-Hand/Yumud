//这个驱动还未完成

//TCA8418是德州仪器的一款按键扫描芯片
#pragma once

// https://blog.csdn.net/qq_34888956/article/details/139613908

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp" 
#include "core/utils/Option.hpp" 

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct TCA8418_Regs{
    using RegAddress = uint8_t;

    struct R8_Cfg:public Reg8<>{
        static constexpr RegAddress address = 0x01;

        // Key events interrupt enable to host processor
        uint8_t KE_IEN:1;

        // GPI interrupt enable to host processor
        uint8_t GPI_IEN:1;

        // Keypad lock interrupt enable
        uint8_t K_LCK_IEN:1;

        // Overflow interrupt enable
        uint8_t OVR_FLOW_EN:1;

        // Interrupt configuration
        uint8_t IN_CFG:1;

        // Overflow mode
        uint8_t OVR_FLOW_M:1;

        // GPI event mode configuration
        uint8_t GPI_E_CFG:1;

        // GPI event mode configuration
        uint8_t AI:1;
    };

    struct R8_INT_STAT:public Reg8<>{
        static constexpr RegAddress address = 0x02;

        uint8_t K_INT:1;
        uint8_t GPI_INT:1;
        uint8_t K_CLK_INT:1;
        uint8_t OVGR_FLOW_INT:1;
        uint8_t CAD_INT:1;
        uint8_t :3;
    };

    struct R8_KEY_LCK_EC:public Reg8<>{
        static constexpr RegAddress address = 0x03;

        uint8_t KEC:4;
        uint8_t LCK1:1;
        uint8_t LCK2:1;
        uint8_t K_LCK_EN:1;
    };

    struct R8_KEY_EVENT:public Reg8<>{
        static constexpr RegAddress address = 0x04;

        uint8_t data;
    };

    struct R8_KP_LCK_TIMER:public Reg8<>{
        static constexpr RegAddress address = 0x05;

        uint8_t data;
    };

    struct R8_KP_UNLOCK_TIMER:public Reg8<>{
        static constexpr RegAddress address = 0x05;

        uint8_t data;
    };

    struct R8_GPIO_INT_STAT1:public Reg8<>{
        static constexpr RegAddress address = 0x11;

        uint8_t R0IS:1;
        uint8_t R1IS:1;
        uint8_t R2IS:1;
        uint8_t R3IS:1;
        uint8_t R4IS:1;
        uint8_t R5IS:1;
        uint8_t R6IS:1;
        uint8_t R7IS:1;
    };

    struct R8_GPIO_INT_STAT2:public Reg8<>{
        static constexpr RegAddress address = 0x12;

        uint8_t C0IS:1;
        uint8_t C1IS:1;
        uint8_t C2IS:1;
        uint8_t C3IS:1;
        uint8_t C4IS:1;
        uint8_t C5IS:1;
        uint8_t C6IS:1;
        uint8_t C7IS:1;
    };

    struct R8_GPIO_INT_STAT3:public Reg8<>{
        static constexpr RegAddress address = 0x13;

        uint8_t C8IS:1;
        uint8_t C9IS:1;
        uint8_t :6;
    };

    struct R8_GPIO_DAT_STAT1:public Reg8<>{
        static constexpr RegAddress address = 0x14;

        uint8_t R0DS:1;
        uint8_t R1DS:1;
        uint8_t R2DS:1;
        uint8_t R3DS:1;
        uint8_t R4DS:1;
        uint8_t R5DS:1;
        uint8_t R6DS:1;
        uint8_t R7DS:1;
    };

    struct R8_GPIO_DAT_STAT2:public Reg8<>{
        static constexpr RegAddress address = 0x15;

        uint8_t C0DS:1;
        uint8_t C1DS:1;
        uint8_t C2DS:1;
        uint8_t C3DS:1;
        uint8_t C4DS:1;
        uint8_t C5DS:1;
        uint8_t C6DS:1;
        uint8_t C7DS:1;
    };

    struct R8_GPIO_DAT_STAT3:public Reg8<>{
        static constexpr RegAddress address = 0x16;

        uint8_t C8DS:1;
        uint8_t C9DS:1;
        uint8_t :6;
    };


    struct R8_GPIO_DAT_OUT1:public Reg8<>{
        static constexpr RegAddress address = 0x17;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_GPIO_DAT_OUT2:public Reg8<>{
        static constexpr RegAddress address = 0x18;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_GPIO_DAT_OUT3:public Reg8<>{
        static constexpr RegAddress address = 0x19;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };



    
    struct R8_GPIO_INT_EN1:public Reg8<>{
        static constexpr RegAddress address = 0x1A;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_GPIO_INT_EN2:public Reg8<>{
        static constexpr RegAddress address = 0x1B;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_GPIO_INT_EN3:public Reg8<>{
        static constexpr RegAddress address = 0x1C;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };

    struct R8_KP_GPIO1:public Reg8<>{
        static constexpr RegAddress address = 0x1D;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_KP_GPIO2:public Reg8<>{
        static constexpr RegAddress address = 0x1E;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_KP_GPIO3:public Reg8<>{
        static constexpr RegAddress address = 0x1F;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };


    struct R8_GPI_EM1:public Reg8<>{
        static constexpr RegAddress address = 0x20;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_GPI_EM2:public Reg8<>{
        static constexpr RegAddress address = 0x21;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_GPI_EM3:public Reg8<>{
        static constexpr RegAddress address = 0x22;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };

    struct R8_GPIO_DIR1:public Reg8<>{
        static constexpr RegAddress address = 0x23;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_GPIO_DIR2:public Reg8<>{
        static constexpr RegAddress address = 0x24;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_GPIO_DIR3:public Reg8<>{
        static constexpr RegAddress address = 0x25;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };

    struct R8_GPIO_INT_LVL1:public Reg8<>{
        static constexpr RegAddress address = 0x26;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_GPIO_INT_LVL2:public Reg8<>{
        static constexpr RegAddress address = 0x27;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_GPIO_INT_LVL3:public Reg8<>{
        static constexpr RegAddress address = 0x28;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };

    struct R8_DEBOUNCE_DIS1:public Reg8<>{
        static constexpr RegAddress address = 0x29;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_DEBOUNCE_DIS2:public Reg8<>{
        static constexpr RegAddress address = 0x2A;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_DEBOUNCE_DIS3:public Reg8<>{
        static constexpr RegAddress address = 0x2B;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };

    struct R8_GPIO_PULL1:public Reg8<>{
        static constexpr RegAddress address = 0x2C;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    };

    struct R8_GPIO_PULL2:public Reg8<>{
        static constexpr RegAddress address = 0x2D;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    };

    struct R8_GPIO_PULL3:public Reg8<>{
        static constexpr RegAddress address = 0x2E;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    };
};

class TCA8418_Error{
public:
    enum Kind:uint8_t{
        HalResult,
        VerifyFailed,
        WrongWhoAmI,
        Unspecified = 0xff,
    };

    constexpr TCA8418_Error(Kind kind):kind_(kind){;} 
    constexpr bool operator==(const TCA8418_Error & rhs) const{return kind_ == rhs.kind_;}
private:
    Kind kind_;
};


class TCA8418_Phy{
public:
    using Error = TCA8418_Error;
    TCA8418_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);
    Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);

    Result<void, Error> write_reg(const auto & reg){
        return write_reg(reg.address, reg.as_val());
    }

    Result<void, Error> read_reg(auto & reg){
        return read_reg(reg.address, reg.as_ref());
    }
private:
    hal::I2cDrv i2c_drv_;
};

class TCA8418{
public:
    using Error = TCA8418_Error;

    class KeyEvent{
    public:


        enum Kind:uint8_t{
            Invalid,
            ArrayPad,
            RowGpi,
            ColumnGpi
        };


        static constexpr KeyEvent from_u8(const uint8_t data){
            if(data >= 1 and data <= 80){
                return KeyEvent{
                    ArrayPad,
                    Some<uint8_t>((data - 1) / 10),
                    Some<uint8_t>((data - 1) % 10)
                };
            }else if(data >= 97 and data <= 104){
                return KeyEvent{
                    RowGpi,
                    Some<uint8_t>(data - 97),
                    None
                };
            }else if(data >= 105 and data <= 114){
                return KeyEvent{
                    ColumnGpi,
                    None,
                    Some<uint8_t>(data - 105)
                };
            }else{
                return KeyEvent{Invalid};
            }
        }

        constexpr Kind kind() const {return kind_;}
        constexpr Option<uint8_t> row() const {return row_;}
        constexpr Option<uint8_t> col() const {return col_;}
    private:
        constexpr KeyEvent(
            const Kind kind, 
            const Option<uint8_t> row = None, 
            const Option<uint8_t> col = None
        ):
            kind_(kind), row_(row), col_(col)
        {;}
        
        Kind kind_ = Invalid;
        Option<uint8_t> row_ = None;
        Option<uint8_t> col_ = None;
    };

    static constexpr auto KEY_EVENT_SIZE = sizeof(KeyEvent);
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0110100);

    TCA8418(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_({i2c, addr}){;}

    Result<void, Error> verify();
    Result<void, Error> init();
private:
    Result<size_t, Error> get_pending_events_cnt();
    Result<bool, Error> is_fifo_empty();


    TCA8418_Phy phy_;
};
}