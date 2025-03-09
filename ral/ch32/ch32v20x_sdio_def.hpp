#include <cstdint>

struct R32_SDIO_POWER{
    uint32_t PWRCTRL:2;
    uint32_t :30;
};

struct R32_SDIO_CLKCR{
    uint32_t CLKDIV:8;
    uint32_t CLKEN:1;
    uint32_t PWRSAV:1;
    uint32_t BYPASS:1;
    uint32_t WIDBUS:2;
    uint32_t NEGEDGE:1;
    uint32_t HWFC_EN:1;
    uint32_t :17;
};

struct R32_SDIO_ARG{
    uint32_t ARG;
};

struct R32_SDIO_CMD{
    uint32_t CMDINDEX:6;
    uint32_t WAITRESP:2;
    uint32_t WAITINT:1;
    uint32_t WAITPEND:1;
    uint32_t CPSMEN:1;
    uint32_t SDIOSUSPEND:1;
    uint32_t ENCMDCOMP:1;
    uint32_t NIEN:1;
    uint32_t ATACMD:1;
    uint32_t :17;
};

struct R32_SDIO_RESPCMD{

};

struct R128_SDIO_RESPX{

};

struct R32_SDIO_DTIMER{

};

struct R32_SDIO_DLEN{

};

struct R32_SDIO_DCTRL{

};

struct R32_SDIO_DCOUNT{

};

struct R32_SDIO_STA{

};

struct R32_SDIO_ICR{

};

struct R32_SDIO_MASK{

};

struct R32_SDIO_FIFOCNT{

};

struct R32_SDIO_FIFO{

};