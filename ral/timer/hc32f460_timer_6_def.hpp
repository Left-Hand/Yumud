#pragma once

#include <cstdint>

namespace ymd::ral::hc32::hc32f460{

struct TMR6_GCONR{
    uint32_t START:1;
    uint32_t DIR:1;
    uint32_t MODE:1;
    uint32_t :1;
    uint32_t CKDIV:4;
    uint32_t OVSTP:1;
    uint32_t :7;
    uint32_t ZMSKREV:1;
    uint32_t ZMSKPOS:1;
    uint32_t ZMSKVAL:2;
    uint32_t :12;
};

struct TMR6_ICONR{
    uint32_t INTENA:1;
    uint32_t INTENB:1;
    uint32_t INTENC:1;
    uint32_t INTEND:1;
    uint32_t INTENE:1;
    uint32_t INTENF:1;
    uint32_t INTENOVF:1;
    uint32_t INTENUDF:1;
    uint32_t INTENDTE:1;
    uint32_t :7;
    uint32_t INTENSAU:1;
    uint32_t INTENSAD:1;
    uint32_t INTENSBU:1;
    uint32_t INTENSBD:1;
    uint32_t :12;
};

struct TMR6_BCONR{
    uint32_t BENA:1;
    uint32_t BSEA:1;
    uint32_t BTRDA:1;
    uint32_t BENB:1;
    uint32_t BSEB:1;
    uint32_t BTRYB:1;
    uint32_t BTRDB:1;
    uint32_t BENP:1;
    uint32_t BSEP:1;
    uint32_t BTRUP:1;
    uint32_t BTRDP:1;
    uint32_t :4;

    uint32_t BENSPA:1;
    uint32_t BSESPA:1;
    uint32_t BTRUSPA:1;
    uint32_t BTRDSPA:1;
    uint32_t BENSPB:1;
    uint32_t BSESPB:1;
    uint32_t BTRUSPB:1;
    uint32_t BTRDSPB:1;
    uint32_t :8;
};

struct TMR6_DCONR{
    uint32_t DTCEN:1;
    uint32_t SEPA:1;
    uint32_t :2;
    uint32_t DTBENU:1;
    uint32_t DTBEND:1;
    uint32_t DTBTRU:1;
    uint32_t DTBTRD:1;
    uint32_t :24;
};

struct TMR6_PCNAR{
    uint32_t STAC:2;
    uint32_t STPC:2;
    uint32_t OVFCA:2;
    uint32_t UDFCA:2;
    uint32_t CMAUCA:2;
    uint32_t CMADCA:2;
    uint32_t CMBUCA:2;
    uint32_t CMBDCA:2;
    
    uint32_t FORCA:2;
    uint32_t :2;
    
    uint32_t EMBRA:2;
    uint32_t EMBSA:2;
    uint32_t :2;
    uint32_t OUTENA:1;
    uint32_t :2;
    uint32_t CAPMDA:1;
};


}