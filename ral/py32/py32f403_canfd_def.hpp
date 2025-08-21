#pragma once

#include <cstdint>


namespace ymd::ral::py32f403{

struct CANFD_TSNCR{
    uint32_t VERSION:16;
    uint32_t CES:1;
    uint32_t ROP:1;
};

struct CANFD_ACBTR{
    uint32_t AC_SEG_1:9;
    uint32_t :7;

    uint32_t AC_SEG_2:7;
    uint32_t :1;
    uint32_t AC_SWJ:7;
    uint32_t :1;
};


struct CANFD_FDBTR{
    uint32_t FD_SEG_1:8;
    uint32_t :8;
    uint32_t FD_SEG_2:6;
    uint32_t :1;
    uint32_t FD_SWJ:7;
    uint32_t :1;
};

struct CANFD_RLSSP{
    uint32_t PRESC:5;
    uint32_t :3;
    uint32_t FD_SSPOFF:8;
    uint32_t :8;
    uint32_t REALIM:3;
    uint32_t :1;
    uint32_t RETLIM:3;
    uint32_t :1;
};


struct CANFD_IFR{
    uint32_t AIF:1;
    uint32_t EIF:1;
    uint32_t TSIF:1;
    uint32_t TPIF:1;
    uint32_t RAFIF:1;
    uint32_t RFIF:1;
    uint32_t ROIF:1;
    uint32_t RIF:1;
    uint32_t BEIF:1;
    uint32_t BLIF:1;
    uint32_t EPIF:1;
    uint32_t TTIF:1;
    uint32_t TEIF:1;
    uint32_t WTIF:1;
    uint32_t :16;

    uint32_t EPASS:1;
    uint32_t EWARN:1;

};

struct CANFD_IER{
    uint32_t :1;
    uint32_t EIE:1;
    uint32_t TSIE:1;
    uint32_t TPIE:1;
    uint32_t RAFIE:1;
    uint32_t RFIE:1;
    uint32_t ROIE:1;
    uint32_t RIE:1;
    uint32_t BEIE:1;
    uint32_t ALIE:1;
    uint32_t EPIE:1;
    uint32_t TTIE:1;
    uint32_t :1;
    uint32_t WTIE:1;
    uint32_t :18;
};

struct CANFD_TSR{
    uint32_t HANDLE_L:8;
    uint32_t TSTAT_L:3;
    uint32_t :5;
    uint32_t HANDLE_H:8;
    uint32_t TSTAT_H:3;
    uint32_t :5;
};

struct CANFD_MCR{
    uint32_t BUSOFF:1;
    uint32_t :4;
    uint32_t LBMI:1;
    uint32_t LBME:1;
    uint32_t RESET:1;
    uint32_t TSA:1;
    uint32_t TSALL:1;
    uint32_t TSONE:1;
    uint32_t TPA:1;
    uint32_t TPE:1;
    uint32_t STBY:1;
    uint32_t LOM:1;
    uint32_t TBSEL:1;

    uint32_t TSSTAT:2;
    uint32_t TSFF:1;
    uint32_t :1;
    uint32_t TTTMB:1;
    uint32_t TSMODE:1;
    uint32_t TSNEXT:1;
    uint32_t FD_ISO:1;
    uint32_t RSTAT:2;
    uint32_t :1;
    
    uint32_t RBALL:1;
    uint32_t RREL:1;
    uint32_t ROV:1;
    uint32_t ROM:1;
    uint32_t SACK:1;
};

}