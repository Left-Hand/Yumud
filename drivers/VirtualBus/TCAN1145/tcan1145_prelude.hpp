#pragma once

#include <cstdint>
#include "core/utils/Result.hpp"

namespace ymd::drivers{




struct TCAN1145_Prelude{
    // using Error = Infallible;
    enum class Error:uint8_t{
        UnknownDevice
    };

    template<typename T = void>
    using IResult = Result<T, Error>;




    typedef union
    {
        // Full word of the register
        uint8_t word;

        struct
        {
            // RSVD
            uint8_t reserved1 : 2;

            // SWE timer will be disabled from CANSLNT flag
            uint8_t CANSLNT_SWE_DIS : 1;

            // RSVD
            uint8_t reserved2 : 4;

            // Sleep wake error disable
            uint8_t SWE_DIS : 1;

        };
    } TCAN114x_SWE_DIS;
    //Addr 1Ch





    enum class Mode:uint8_t{
        Sleep,
        Standy,
        Listen,
        Normal
    };


    typedef union {
        // Full word of the register
        uint8_t word;

        struct {
            // Current mode of device
            uint8_t MODE_SEL : 3;

            // Test Mode Enable --> RSVD
            uint8_t reserved : 2;

            // Fault Detection Enable
            uint8_t FAULT_DETECT_EN : 1;

            // Sleep wake error disable --> RMVD
            //uint8_t SWE_DIS : 1;

            // Fail safe disable
            uint8_t DTO_DIS : 1;

            // Selective Wake Enable
            uint8_t SW_EN : 1;
        };
    }ModeControl;
    //Addr 10h


    /**
     * @brief Wake Pin Configuration Register
     */
    typedef union {
        // Full word of the register
        uint8_t word;

        struct {
            // Maximum WAKE pin input pulse width to be considered valid
            uint8_t WAKE_WIDTH_MAX : 2;

            // Pulses less than or equal to this are considered invalid
            uint8_t WAKE_WIDTH_INVALID : 2;

            // Minimum wake pulse width to wake up
            //uint8_t WAKE_WIDTH_MIN : 2;

            // Minimum wake pulse width to wake up
            uint8_t WAKE_STAT : 2;

            // Wake Pin Configuration
            uint8_t WAKE_CONFIG : 2;
        };
    } Wake_Pin_Config;
    //Addr 11h


    /**
     * @brief Pin Configuration
     */
    typedef union {
        // Full word of the register
        uint8_t word;

        struct {
            // GPO pin configuration selection, active low -> RSVD
            uint8_t reserved1 : 2;

            // Configures RXD pin behavior from a wake event. 0 = pull low, 1 = toggle
            uint8_t RXD_WK_CONFIG : 1;

            // Dominate time out disable ->RMVD
            //uint8_t DTO_DIS : 1;

            // nINT configuration selection, active low
            uint8_t nINT_SEL : 2;

            // SDO pin configuration. When configured as SDO and nINT, the pin will behave as SDO when nCS is low and behave as nINT when nCS is high --> RSVD
            uint8_t reserved2 : 2;

            // Sets the WAKE pin expected pulse direction
            uint8_t WAKE_PULSE_CONFIG : 1;
        };
    } Pin_Config;
    //Addr 12h

    /*
    * Watchdog Configuration Registers
    */
    typedef struct {
        union {
            uint8_t word_config_1;
            struct {
                // Watchdog output trigger event action
                uint8_t WD_ACT : 2;

                // Watchdog event error counter threshold
                uint8_t WD_ERR_CNT_SET : 2;

                // Watchdog Prescaler
                uint8_t WD_PRE : 2;

                // Watchdog configuration/mode
                uint8_t WD_CONFIG_MODE : 2;
            };
        };

        union {
            uint8_t word_config_2;
            struct {
                uint8_t reserved : 1;

                // Watchdog Error counter
                uint8_t WD_ERR_CNT : 4;

                // Watchdog window or timeout time
                uint8_t WD_TIMER : 3;
            };
        };

        union {
            uint8_t word_wd_rst_pulse;
            struct {
                // Watchdog reset pulse width
                uint8_t RESET_PULSE_WIDTH : 4;

                uint8_t reserved2 : 4;
            };
        };
    } Watchdog_Config;
    //Addr 13h-14h, 16h

    /*
    * Watchdog QA Configuration Registers
    */
    typedef union {
        uint8_t word_qa_config;
        struct {
            // Watchdog Q&A polynomial seed value loaded when device is in the RESET state
            uint8_t WD_QA_POLY_SEED : 4;

            // Watchdog Q&A polynomial configuration
            uint8_t WD_QA_POLY_CFG : 2;

            // Watchdog Q&A answer generation configuration
            uint8_t WD_ANSW_GEN_CFG : 2;
        };
    } Watchdog_QA_Config;
    //Addr 2Dh

    /*
    * Interrupt Registers
    */
    typedef struct {
        union {
            uint8_t word_int_global;
            struct {
                uint8_t reserved1 : 3;
                uint8_t INT_CANBUS : 1;
                uint8_t INT_3 : 1;
                uint8_t INT_2 : 1;
                uint8_t INT_1 : 1;
                uint8_t GLOBALERR : 1;
            };
        };

        union {
            uint8_t word_int1;
            struct {
                uint8_t CANDOM : 1;
                uint8_t CANTO : 1;
                uint8_t CANSLNT : 1;
                uint8_t FRAME_OVF : 1;
                uint8_t WKERR : 1;
                uint8_t LWU : 1;
                uint8_t CANINT : 1;
                uint8_t WD : 1;
            };
        };

        union {
            uint8_t word_int2;
            struct {
                uint8_t TSDW : 1;
                uint8_t TSD : 1;
                uint8_t UVCC : 1;
                uint8_t UVIO : 1;
                uint8_t UVSUP : 1;
                uint8_t reserved2 : 1;
                uint8_t PWRON : 1;
                uint8_t SMS : 1;
            };
        };

        union {
            uint8_t word_int3;
            struct {
                uint8_t CRC_EEPROM : 1;
                uint8_t reserved_int3 : 4;
                uint8_t FSM : 1;
                uint8_t SWERR : 1;
                uint8_t SPIERR : 1;
            };
        };

        union {
            uint8_t word_canbus;
            struct {
                uint8_t CANBUSBAT : 1;
                uint8_t CANBUSGND : 1;
                uint8_t CANBUSOPEN : 1;
                uint8_t CANLGND : 1;
                uint8_t CANHBAT : 1;
                uint8_t CANHCANL : 1;
                uint8_t CANBUSTERMOPEN : 1;
                uint8_t reserved3 : 1;
            };
        };
    } Interrupts;
    //Addr 50-54h




    struct Dev_ID {
        // DEVID ID address 0
        uint8_t addr0;
        // DEVID ID address 1
        uint8_t addr1;
        // DEVID ID address 2
        uint8_t addr2;
        // DEVID ID address 3
        uint8_t addr3;
        // DEVID ID address 4
        uint8_t addr4;
        // DEVID ID address 5
        uint8_t addr5;
        // DEVID ID address 6
        uint8_t addr6;
        // DEVID ID address 7
        uint8_t addr7;
    };

    struct PN_Config{
        // ID: The ID to look for for selective wake. Right justified whether 11 or 29 bits. If using extended ID, make sure to set IDE to 1.
        uint32_t ID : 29;

        // IDE: ID Extended used. Set to 1 if using an extended ID in the ID field
        // \n Valid values are: 0, 1
        uint8_t IDE : 1;

        // IDMask: The value to be used for masking the ID. All 0's means all bits must match exactly. A 1 means that bit is ignored in the ID field
        uint32_t IDMask : 29;

        // DLC: A DLC field value to match.
        // \n Valid values are: 0 to 15
        uint8_t DLC : 4;

        // Data Mask Enable: Set to 1 if using a data field mask. Turning this on requires the DLC to match, and the data fields are masks. A 1 means match this bit in the data fields. A single match across all the data fields will be a valid WUF
        // \n Valid values are 0, 1
        uint8_t DataMaskEN : 1;

        // CAN Data to match
        uint8_t DATA[8];

        // Selective wake FD passive: Set to 1 to ignore frames with FD enabled. A 0 [default] results in error counter increment
        // \n Valid values are 0 [default], 1
        uint8_t SWFDPassive : 1;

        // CAN Data Rate
        uint8_t CAN_DR : 3;

        // CAN FD Data Rate vs the selected CAN Data Rate
        uint8_t FD_DR : 2;

        // Frame counter threshold: Sets at what value the error counter must reach to set a frame overflow flag (+1). Default is 31
        uint8_t FrameErrorThreshold : 8;

    } ;

    typedef struct {
        uint8_t wd0 : 2;
        uint8_t wd1 : 2;
        uint8_t wd2 : 2;
        uint8_t wd3 : 2;
        uint8_t wd4 : 2;
        uint8_t wd5 : 2;
        uint8_t wd6 : 2;
        uint8_t wd7 : 2;
        uint8_t wd8 : 2;
        uint8_t wd9 : 2;
        uint8_t wd10 : 2;
        uint8_t wd11 : 2;
    } wdt_bits;


static constexpr uint8_t REG_TCAN114X_DEVICEID0 = 0x00; 
static constexpr uint8_t REG_TCAN114X_DEVICEID1 = 0x01; 
static constexpr uint8_t REG_TCAN114X_DEVICEID2 = 0x02; 
static constexpr uint8_t REG_TCAN114X_DEVICEID3 = 0x03; 
static constexpr uint8_t REG_TCAN114X_DEVICEID4 = 0x04; 
static constexpr uint8_t REG_TCAN114X_DEVICEID5 = 0x05; 
static constexpr uint8_t REG_TCAN114X_DEVICEID6 = 0x06; 
static constexpr uint8_t REG_TCAN114X_DEVICEID7 = 0x07; 

static constexpr uint8_t REG_TCAN114X_REVID_MAJOR = 0x08; 
static constexpr uint8_t REG_TCAN114X_REVID_MINOR = 0x09; 

//formula RSVD Registers not included     0x0A+
//Scratch Pad SPI register not included   0x0F

static constexpr uint8_t REG_TCAN114X_MODE_CONTROL = 0x10; 
static constexpr uint8_t REG_TCAN114X_WAKE_PIN_CONFIG = 0x11; 
static constexpr uint8_t REG_TCAN114X_PIN_CONFIG = 0x12; 
static constexpr uint8_t REG_TCAN114X_WD_CONFIG_1 = 0x13; 
static constexpr uint8_t REG_TCAN114X_WD_CONFIG_2 = 0x14; 
static constexpr uint8_t REG_TCAN114X_WD_INPUT_TRIG = 0x15; 
static constexpr uint8_t REG_TCAN114X_WD_RST_PULSE = 0x16; 
static constexpr uint8_t REG_TCAN114X_FSM_CONFIG = 0x17; 
static constexpr uint8_t REG_TCAN114X_FSM_COUNTER = 0x18; 
static constexpr uint8_t REG_TCAN114X_DEVICE_RESET = 0x19; 
static constexpr uint8_t REG_TCAN114X_DEVICE_CONFIG = 0x1A; 

// DEVICE_CONFIG2 not included            0x1B
//
// SWE_DIS not included                   0x1C
//
//#define REG_TCAN114X_SWE_DIS            0x1C
//
//
// SO_CONFIG not included                 0x29

static constexpr uint8_t REG_TCAN114X_WD_QA_CONFIG = 0x2D; 
static constexpr uint8_t REG_TCAN114X_WD_QA_ANSWER = 0x2E; 
static constexpr uint8_t REG_TCAN114X_WD_QA_QUESTION = 0x2F; 

/*
 * Selective wake registers
 */
static constexpr uint8_t REG_TCAN114X_SW_ID_1 = 0x30; 
static constexpr uint8_t REG_TCAN114X_SW_ID_2 = 0x31; 
static constexpr uint8_t REG_TCAN114X_SW_ID_3 = 0x32; 
static constexpr uint8_t REG_TCAN114X_SW_ID_4 = 0x33; 
static constexpr uint8_t REG_TCAN114X_SW_ID_MASK_1 = 0x34; 
static constexpr uint8_t REG_TCAN114X_SW_ID_MASK_2 = 0x35; 
static constexpr uint8_t REG_TCAN114X_SW_ID_MASK_3 = 0x36; 
static constexpr uint8_t REG_TCAN114X_SW_ID_MASK_4 = 0x37; 
static constexpr uint8_t REG_TCAN114X_SW_ID_MASK_DLC = 0x38; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_0 = 0x39; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_1 = 0x3A; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_2 = 0x3B; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_3 = 0x3C; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_4 = 0x3D; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_5 = 0x3E; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_6 = 0x3F; 
static constexpr uint8_t REG_TCAN114X_SW_DATA_7 = 0x40; 

// SW_RSVD_y not included                 0x41+

static constexpr uint8_t REG_TCAN114X_SW_CONFIG_1 = 0x44; 
static constexpr uint8_t REG_TCAN114X_SW_CONFIG_2 = 0x45; 
static constexpr uint8_t REG_TCAN114X_SW_CONFIG_3 = 0x46; 
static constexpr uint8_t REG_TCAN114X_SW_CONFIG_4 = 0x47; 

// SW_CONFIG_RSVD_y not included          0x48+

static constexpr uint8_t REG_TCAN114X_INT_GLOBAL = 0x50; 
static constexpr uint8_t REG_TCAN114X_INT_1 = 0x51; 
static constexpr uint8_t REG_TCAN114X_INT_2 = 0x52; 
static constexpr uint8_t REG_TCAN114X_INT_3 = 0x53; 
static constexpr uint8_t REG_TCAN114X_INT_CANBUS = 0x54; 
static constexpr uint8_t REG_TCAN114X_INT_GLOBAL_ENABLE = 0x55; 
static constexpr uint8_t REG_TCAN114X_INT_ENABLE_1 = 0x56; 
static constexpr uint8_t REG_TCAN114X_INT_ENABLE_2 = 0x57; 
static constexpr uint8_t REG_TCAN114X_INT_ENABLE_3 = 0x58; 
static constexpr uint8_t REG_TCAN114X_INT_ENABLE_CANBUS = 0x59; 

// INT_RSVD_y not included                0x5A+

//_________________________________________________________________________________________________________________

/*
 *  Bit masks
 */

//REG_TCAN114X_DEVICE_RESET
//Addr 19h
//--------------------------------------------------------------------
static constexpr uint8_t REG_BITS_TCAN114X_DEVICE_RESET_HARD_RESET = 0x01; 
static constexpr uint8_t REG_BITS_TCAN114X_DEVICE_RESET_SOFT_RESET = 0x02; 

//MODE_CNTRL
//Addr 10h
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_SW_EN = 0x80; 
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_FD_EN = 0x20;   //0x10
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_MODE_SEL_MSK = 0x07; 
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_MODE_SLEEP = 0x01; 
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_MODE_STANDBY = 0x04; 
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_MODE_LISTEN = 0x05; 
static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_MODE_NORMAL = 0x07; 
//Added

static constexpr uint8_t REG_BITS_TCAN114X_MODE_CNTRL_MODE_DTO_DIS = 0x40; 

/*RSVD now
 *
 *#define REG_BITS_TCAN114X_MODE_CNTRL_TM_EN              0x08
 *
 *#define REG_BITS_TCAN114X_MODE_CNTRL_SWE_DIS            0x20
 *
 */

/*Changed to DTO_DIS
 *
 * #define REG_BITS_TCAN114X_MODE_CNTRL_FS_DIS            0x40
 */

//----------------------------------------------------------------------

//REG_TCAN114X_SWE_DIS
//Addr 1Ch
/*
#define REG_BITS_TCAN114X_SWE_DIS_SWE_DIS               0x80
#define REG_BITS_TCAN114X_SWE_DIS_CANSLNT_SWE_DIS       0x04
*/

//----------------------------------------------------------------------

//INT_GLOBAL
//ADDR 50h
static constexpr uint8_t REG_BITS_TCAN114X_INT_GLOBAL_GLOBALERR = 0x80; 
static constexpr uint8_t REG_BITS_TCAN114X_INT_GLOBAL_INT_1 = 0x40; 
static constexpr uint8_t REG_BITS_TCAN114X_INT_GLOBAL_INT_2 = 0x20; 
static constexpr uint8_t REG_BITS_TCAN114X_INT_GLOBAL_INT_3 = 0x10; 
static constexpr uint8_t REG_BITS_TCAN114X_INT_GLOBAL_INT_CANBUS = 0x08; 

//----------------------------------------------------------------------

//SW_ID_3
//Addr 32h
static constexpr uint8_t REG_BITS_TCAN114X_SW_ID3_IDE = 0x20; 

//SW_IDM_DLC
//Addr 38h
static constexpr uint8_t REG_BITS_TCAN114X_SW_ID_MASK_DLC_DATA_MASK_EN = 0x01; 

//SW_CONFIG_4
//Addr 47h
static constexpr uint8_t REG_BITS_TCAN114X_SW_CONFIG_4_SWCFG = 0x80; 

//WD_CONFIG_1
//Addr 13h
// Not sure if this does what is intended
// This sets WD_CONFIG to Q&A
static constexpr uint8_t REG_BITS_TCAN114X_WD_CONFIG_1_WD_CONFIG_MASK = 0xC0; 

//WD_QA_QUESTION
//Addr 2Fh
static constexpr uint8_t REG_BITS_TCAN114X_WD_QA_QUESTION_QA_ERROR = 0x40; 

static constexpr size_t TCAN114X_PN_DLC_0B = 0;
static constexpr size_t TCAN114X_PN_DLC_1B = 1;
static constexpr size_t TCAN114X_PN_DLC_2B = 2;
static constexpr size_t TCAN114X_PN_DLC_3B = 3;
static constexpr size_t TCAN114X_PN_DLC_4B = 4;
static constexpr size_t TCAN114X_PN_DLC_5B = 5;
static constexpr size_t TCAN114X_PN_DLC_6B = 6;
static constexpr size_t TCAN114X_PN_DLC_7B = 7;
static constexpr size_t TCAN114X_PN_DLC_8B = 8;

static constexpr size_t TCAN114X_PN_CAN_50KBPS = 0;
static constexpr size_t TCAN114X_PN_CAN_100KBPS = 1;
static constexpr size_t TCAN114X_PN_CAN_125KBPS = 2;
static constexpr size_t TCAN114X_PN_CAN_250KBPS = 3;
static constexpr size_t TCAN114X_PN_CAN_500KBPS = 5;
static constexpr size_t TCAN114X_PN_CAN_1MBPS = 7;

static constexpr size_t TCAN114X_PN_CANFD_LESSTHAN4XCANRATE = 0;
static constexpr size_t TCAN114X_PN_CANFD_GT5x_LT10x = 1;
static constexpr size_t TCAN114X_PN_CANFD_8MBPS_WITH500KCAN = 2;

};

}