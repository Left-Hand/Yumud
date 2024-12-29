#pragma once

#include <cstdint>

namespace hwspec::CH32V20x::TIM_Regs{

struct CTLR1_Reg{
    uint16_t CEN:1;
    uint16_t UDIS:1;
    uint16_t URS:1;
    uint16_t OPM:1;
    uint16_t DIR:1;
    uint16_t CMS:2;
    uint16_t APRE:1;
    uint16_t CKD:2;
    uint16_t :6;
};

struct CFGHR_Reg{
    uint16_t CCPC:1;
    uint16_t :1;
    uint16_t CCUS:1;
    uint16_t CCDS:1;
    uint16_t MMS:3;

    uint16_t TI1S:1;
    uint16_t OIS1:1;
    uint16_t OIS1n:1;
    uint16_t OIS2:1;
    uint16_t OIS2n:1;
    uint16_t OIS3:1;
    uint16_t OIS3n:1;
    uint16_t OIS4:1;
    uint16_t :1;
};

struct SMCFGR_Reg{
    uint16_t SMS:3;
    uint16_t :1;
    uint16_t TS:3;
    uint16_t MSM:1;
    uint16_t ETF:4;
    uint16_t ETPS:2;
    uint16_t ECE:1;
    uint16_t ETP:1;
};

struct DMAINTENR_Reg{
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

struct INTFR_Reg{
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

struct SWEVGR_Reg{
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

struct CNCCTLR1{
    uint16_t CC1S:2;
    union{
        struct{
            uint16_t OC1FE:1;
            uint16_t OC1PE:1;
        };
        uint16_t IC1PSC:2;
    };

    union{
        struct{
            uint16_t OC1M:3;
            uint16_t OC1CE:1;
        };
        uint16_t IC1F:4;
    };

    uint16_t CC2S:2;
    union{
        struct{
            uint16_t OC2FE:1;
            uint16_t OC2PE:1;
        };
        uint16_t IC2PSC:2;
    };

    union{
        struct{
            uint16_t OC2M:3;
            uint16_t OC2CE:1;
        };
        uint16_t IC2F:4;
    };
};


struct CNCCTLR2{
    uint16_t CC3S:2;
    union{
        struct{
            uint16_t OC3FE:1;
            uint16_t OC3PE:1;
        };
        uint16_t IC3PSC:2;
    };

    union{
        struct{
            uint16_t OC3M:3;
            uint16_t OC3CE:1;
        };
        uint16_t IC3F:4;
    };

    uint16_t CC4S:2;
    union{
        struct{
            uint16_t OC4FE:1;
            uint16_t OC4PE:1;
        };
        uint16_t IC4PSC:2;
    };

    union{
        struct{
            uint16_t OC4M:3;
            uint16_t OC4CE:1;
        };
        uint16_t IC4F:4;
    };
};

struct CCER_Reg{
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

struct TIM_Def{

};

}