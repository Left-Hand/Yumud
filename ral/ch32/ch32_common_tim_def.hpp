#pragma once

#include <cstdint>


#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace ymd::ral::ch32::common_tim{

struct [[nodiscard]] R16_TIM_CTLR1{
    uint16_t CEN:1;
    uint16_t UDIS:1;
    uint16_t URS:1;
    uint16_t OPM:1;
    uint16_t DIR:1;
    uint16_t CMS:2;
    uint16_t APRE:1;
    uint16_t CKD:2;

    uint16_t :2;
    uint16_t BKSEL:1;

    uint16_t :1;
    uint16_t TMR_CAP_OV_EN:1;
    uint16_t TMR_CAP_LVL_EN:1;
};

struct [[nodiscard]] R16_TIM_CTLR2{
    uint16_t CCPC:1;
    uint16_t :1;
    uint16_t CCUS:1;
    uint16_t CCDS:1;
    uint16_t MMS:1;
    uint16_t TI1S:2;
    uint16_t OIS1:1;
    uint16_t OIS1N:2;
    uint16_t OIS2:2;
    uint16_t OIS2N:2;
    uint16_t OIS3:2;
    uint16_t OIS3N:2;
    uint16_t OIS4:2;
    uint16_t :1;
};

struct [[nodiscard]] R16_TIM_SMCFGR{
    uint16_t SMS:3;
    uint16_t :1;
    uint16_t TS:3;
    uint16_t MSM:1;
    uint16_t ETF:4;
    uint16_t ETPS:2;
    uint16_t ECE:1;
    uint16_t ETP:1;
};

#define DEF_TIM_EVENT_FLAGS_FIELDS\
uint16_t UIE:1;\
uint16_t CC1IE:1;\
uint16_t CC2IE:1;\
uint16_t CC3IE:1;\
\
uint16_t CC4IE:1;\
uint16_t CCMIE:1;\
uint16_t TIE:1;\
uint16_t BIE:1;\
\
uint16_t UDE:1;\
uint16_t CC1DE:1;\
uint16_t CC2DE:1;\
uint16_t CC3DE:1;\
\
uint16_t CC4DE:1;\
uint16_t COMDE:1;\
uint16_t TDE:1;\
uint16_t :1;\

struct [[nodiscard]] R16_TIM_DMAINTENR{
    uint16_t UIE:1;
    uint16_t CC1IE:1;
    uint16_t CC2IE:1;
    uint16_t CC3IE:1;
    uint16_t CC4IE:1;
    uint16_t CCMIE:1;
    uint16_t TIE:1;
    uint16_t BIE:1;
    uint16_t UDE:1;
    uint16_t CC1DE:1;
    uint16_t CC2DE:1;
    uint16_t CC3DE:1;
    uint16_t CC4DE:1;
    uint16_t COMDE:1;
    uint16_t TDE:1;
    uint16_t :1;
};

struct [[nodiscard]] R16_TIM_INTFR{
    uint16_t UIF:1;
    uint16_t CC1IF:1;
    uint16_t CC2IF:1;
    uint16_t CC3IF:1;
    uint16_t CC4IF:1;
    uint16_t COMIF:1;
    uint16_t TIF:1;
    uint16_t BIF:1;
    uint16_t :1;
    uint16_t CC1OF:1;
    uint16_t CC2OF:1;
    uint16_t CC3OF:1;
    uint16_t CC4OF:1;
    uint16_t :3;
};

struct [[nodiscard]] R16_TIM_SWEVGR{
    uint16_t UG:1;
    uint16_t CC1G:1;
    uint16_t CC2G:1;
    uint16_t CC3G:1;
    uint16_t CC4G:1;
    uint16_t COMG:1;
    uint16_t TG:1;
    uint16_t BG:1;
    uint16_t :8;
};

struct [[nodiscard]] R16_TIM_CHCTLR1{
    uint16_t CC1S:2;
    union{
        struct{[[nodiscard]] 
            uint16_t OC1FE:1;
            uint16_t OC1PE:1;
        };
        uint16_t IC1PSC:2;
    };

    union{
        struct{[[nodiscard]] 
            uint16_t OC1M:3;
            uint16_t OC1CE:1;
        };
        uint16_t IC1F:4;
    };

    uint16_t CC2S:2;
    union{
        struct{[[nodiscard]] 
            uint16_t OC2FE:1;
            uint16_t OC2PE:1;
        };
        uint16_t IC2PSC:2;
    };

    union{
        struct{[[nodiscard]] 
            uint16_t OC2M:3;
            uint16_t OC2CE:1;
        };
        uint16_t IC2F:4;
    };
};

struct [[nodiscard]] R16_TIM_CHCTLR2{
    uint16_t CC3S:2;
    union{
        struct{[[nodiscard]] 
            uint16_t OC3FE:1;
            uint16_t OC3PE:1;
        };
        uint16_t IC3PSC:2;
    };

    union{
        struct{[[nodiscard]] 
            uint16_t OC3M:3;
            uint16_t OC3CE:1;
        };
        uint16_t IC3F:4;
    };

    uint16_t CC4S:2;
    union{
        struct{[[nodiscard]] 
            uint16_t OC4FE:1;
            uint16_t OC4PE:1;
        };
        uint16_t IC4PSC:2;
    };

    union{
        struct{[[nodiscard]] 
            uint16_t OC4M:3;
            uint16_t OC4CE:1;
        };
        uint16_t IC4F:4;
    };
};

struct [[nodiscard]] R16_TIM_CCER{
    uint16_t CC1E:1;
    uint16_t CC1P:1;
    uint16_t CC1NE:1;
    uint16_t CC1NP:1;

    uint16_t CC2E:1;
    uint16_t CC2P:1;
    uint16_t CC2NE:1;
    uint16_t CC2NP:1;

    uint16_t CC3E:1;
    uint16_t CC3P:1;
    uint16_t CC3NE:1;
    uint16_t CC3NP:1;

    uint16_t CC4E:1;
    uint16_t CC4P:1;
};


using R16_TIM_CNT = uint16_t;
using R16_TIM_PSC = uint16_t;
using R16_TIM_ATRLR = uint16_t;
using R16_TIM_CHCVR = uint16_t;

struct [[nodiscard]] R16_TIM_RPTCR{
    uint8_t REP;
    uint8_t :8;
};

struct [[nodiscard]] R16_TIM_BDTR{
    uint16_t DTG:8;

    uint16_t LOCK:2;
    uint16_t OSSI:1;
    uint16_t OSSR:1;

    uint16_t BKE:1;
    uint16_t BKP:1;
    uint16_t AOE:1;
    uint16_t MOE:1;
};

struct [[nodiscard]] R16_TIM_DMACFGR{
    uint16_t DACFGR:5;
    uint16_t :3;
    uint16_t DBL:5;
    uint16_t :3;
};

struct [[nodiscard]] R16_TIM_DMAADR{
    uint16_t DADDR;
};

struct [[nodiscard]] TIM_Def{
    volatile R16_TIM_CTLR1 CTLR1;
    uint16_t :16;
    volatile R16_TIM_CTLR2 CTLR2;
    uint16_t :16;
    
    volatile R16_TIM_SMCFGR SMCFGR;
    uint16_t :16;
    volatile R16_TIM_DMAINTENR DMAINTENR;
    uint16_t :16;
    volatile R16_TIM_INTFR INTFR;
    uint16_t :16;
    volatile R16_TIM_SWEVGR SWEVGR;
    uint16_t :16;
    
    volatile R16_TIM_CHCTLR1 CHCTLR1;
    uint16_t :16;
    volatile R16_TIM_CHCTLR2 CHCTLR2;
    uint16_t :16;
    volatile R16_TIM_CCER CCER;
    uint16_t :16;
    
    volatile R16_TIM_CNT CNT;
    uint16_t :16;
    volatile R16_TIM_PSC PSC;
    uint16_t :16;
    volatile R16_TIM_ATRLR ATRLR;
    uint16_t :16;
    volatile R16_TIM_RPTCR RPTCR;
    uint16_t :16;
    volatile R16_TIM_CHCVR CH1CVR;
    uint16_t :16;
    volatile R16_TIM_CHCVR CH2CVR;
    uint16_t :16;
    volatile R16_TIM_CHCVR CH3CVR;
    uint16_t :16;
    volatile R16_TIM_CHCVR CH4CVR;
    uint16_t :16;
    
    volatile R16_TIM_BDTR BDTR;
    uint16_t :16;
    volatile R16_TIM_DMACFGR DMACFGR;
    uint16_t :16;
    volatile R16_TIM_DMAADR DMAADR;
    uint16_t :16;
    
    constexpr void enable(const Enable en){
        CTLR1.CEN = en;
    }

    constexpr void enable_generate_update_event(const Enable en){
        CTLR1.UDIS = en;
    }

    constexpr void exhibit_ug_trigger_uev(const Enable en){
        CTLR1.URS = en;
    }

    constexpr void set_single_pulse_mode(const Enable en){
        CTLR1.OPM = en;
    }

    constexpr void set_dec_count_mode(const Enable en){
        CTLR1.DIR = en;
    }


    // 00：边沿对齐模式。计数器依据方向位(DIR)向上或
    // 向下计数。
    // 01：中央对齐模式 1。计数器交替地向上和向下计
    // 数。配置为输出的通道(CHCTLRx 寄存器中 CCxS=00)
    // 的输出比较中断标志位，只在计数器向下计数时被
    // 设置。
    // 10：中央对齐模式 2。计数器交替地向上和向下计
    // 数。配置为输出的通道(CHCTLRx 寄存器中 CCxS=00)
    // 的输出比较中断标志位，只在计数器向上计数时被
    // 设置。
    // 11：中央对齐模式 3。计数器交替地向上和向下计
    // 数。配置为输出的通道(CHCTLRx 寄存器中 CCxS=00)
    // 的输出比较中断标志位，在计数器向上和向下计数
    // 时均被设置。
    // 注：在计数器使能

    constexpr void set_align_mode(const uint8_t mode){
        CTLR1.CMS = mode;
    }

    constexpr void enable_auto_preload(const Enable en){
        CTLR1.APRE = en;
    }

    constexpr void set_dts_prescale(const uint8_t prescale){
        CTLR1.CKD = prescale;
    }

    constexpr void enable_bkin_from_cmp(const Enable en){
        CTLR1.BKSEL = en;
    }

    constexpr void enable_cap_saturation(const Enable en){
        CTLR1.TMR_CAP_OV_EN = en;
    }

    constexpr void enable_cap_level_indicate(const Enable en){
        CTLR1.TMR_CAP_LVL_EN = en;
    }

    constexpr void enable_cc_preload(const Enable en){
        CTLR2.CCPC = en;
    }

    constexpr void enable_cc_update_by_com_trgi(const Enable en){
        CTLR2.CCUS = en;
    }

    constexpr void set_cc_dma_select(const uint8_t sel){
        CTLR2.CCDS = sel;
    }

    constexpr void set_trgo_source(const uint8_t source){
        CTLR2.MMS = source;
    }

    constexpr void set_ti1_select(const uint8_t sel){
        CTLR2.TI1S = sel;
    }

    constexpr void set_co_output_idle_state(const uint8_t ch, const bool stat){
        switch(ch){
            case 1: CTLR2.OIS1N = stat; 
            case 2: CTLR2.OIS2N = stat; 
            case 3: CTLR2.OIS3N = stat; 
            default: __builtin_trap();
        }
    }

    constexpr void set_output_idle_state(const uint8_t ch, const bool stat){
        switch(ch){
            case 1: CTLR2.OIS1 = stat; 
            case 2: CTLR2.OIS2 = stat; 
            case 3: CTLR2.OIS3 = stat; 
            case 4: CTLR2.OIS4 = stat; 
            default: __builtin_trap();
        }
    }

    constexpr void set_input_mode(const uint8_t mode){
        SMCFGR.SMS = mode;
    }

    constexpr void set_trigger_source(const uint8_t source){
        SMCFGR.TS = source;
    }

    constexpr void enable_master_slave_mode(const Enable en){
        SMCFGR.MSM = en;
    }

    constexpr void set_slave_mode(const uint8_t mode){
        SMCFGR.SMS = mode;
    }

    constexpr void set_ext_trigger_filter(const uint8_t filt){
        SMCFGR.ETF = filt;
    }

    constexpr void set_ext_trigger_prescale(const uint8_t prescale){
        SMCFGR.ETPS = prescale;
    }

    constexpr void enable_ext_clockmode2(const Enable en){
        SMCFGR.ECE = en;
    }

    constexpr void enable_ext_trigger_inversion(const Enable en){
        SMCFGR.ETP = en;
    }

    struct [[nodiscard]] Events{
        DEF_TIM_EVENT_FLAGS_FIELDS
    };

    constexpr void set_dma_interrupts_en(const Events events){
        const_cast<R16_TIM_DMAINTENR &>(DMAINTENR) = 
            BIT_CAST(R16_TIM_DMAINTENR, events);
    }

    constexpr void add_dma_interrupts_en(const Events events){
        (uint16_t &)(DMAINTENR) |= BIT_CAST(uint16_t, events);
    }

    constexpr void remove_dma_interrupts_en(const Events events){
        (uint16_t &)(DMAINTENR) &= ~BIT_CAST(uint16_t, events);
    }
};

}