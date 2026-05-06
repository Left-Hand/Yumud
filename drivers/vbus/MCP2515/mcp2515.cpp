#include "mcp2515.hpp"

using namespace ymd;
using namespace ymd::drivers;



namespace {

static constexpr uint8_t EFLG_RX1OVR = (1<<7);
static constexpr uint8_t EFLG_RX0OVR = (1<<6);
static constexpr uint8_t EFLG_TXBO   = (1<<5);
static constexpr uint8_t EFLG_TXEP   = (1<<4);
static constexpr uint8_t EFLG_RXEP   = (1<<3);
static constexpr uint8_t EFLG_TXWAR  = (1<<2);
static constexpr uint8_t EFLG_RXWAR  = (1<<1);
static constexpr uint8_t EFLG_EWARN  = (1<<0);


static constexpr uint8_t CANSTAT_OPMOD = 0xE0;
static constexpr uint8_t CANSTAT_ICOD = 0x0E;

static constexpr uint8_t CNF3_SOF = 0x80;

static constexpr uint8_t TXB_EXIDE_MASK = 0x08;
static constexpr uint8_t DLC_MASK       = 0x0F;
static constexpr uint8_t RTR_MASK       = 0x40;

static constexpr uint8_t RXBnCTRL_RXM_STD    = 0x20;
static constexpr uint8_t RXBnCTRL_RXM_EXT    = 0x40;
static constexpr uint8_t RXBnCTRL_RXM_STDEXT = 0x00;
static constexpr uint8_t RXBnCTRL_RXM_MASK   = 0x60;
static constexpr uint8_t RXBnCTRL_RTR        = 0x08;
static constexpr uint8_t RXB0CTRL_BUKT       = 0x04;
static constexpr uint8_t RXB0CTRL_FILHIT_MASK = 0x03;
static constexpr uint8_t RXB1CTRL_FILHIT_MASK = 0x07;
static constexpr uint8_t RXB0CTRL_FILHIT = 0x00;
static constexpr uint8_t RXB1CTRL_FILHIT = 0x01;

static constexpr uint8_t MCP_SIDH = 0;
static constexpr uint8_t MCP_SIDL = 1;
static constexpr uint8_t MCP_EID8 = 2;
static constexpr uint8_t MCP_EID0 = 3;
static constexpr uint8_t MCP_DLC  = 4;
static constexpr uint8_t MCP_DATA = 5;

static constexpr uint8_t STAT_RX0IF = (1<<0);
static constexpr uint8_t STAT_RX1IF = (1<<1);

static constexpr uint8_t STAT_RXIF_MASK = STAT_RX0IF | STAT_RX1IF;

static constexpr uint8_t TXB_ABTF   = 0x40;
static constexpr uint8_t TXB_MLOA   = 0x20;
static constexpr uint8_t TXB_TXERR  = 0x10;
static constexpr uint8_t TXB_TXREQ  = 0x08;
static constexpr uint8_t TXB_TXIE   = 0x04;
static constexpr uint8_t TXB_TXP    = 0x03;

static constexpr uint8_t EFLG_ERRORMASK = EFLG_RX1OVR
                                    | EFLG_RX0OVR
                                    | EFLG_TXBO
                                    | EFLG_TXEP
                                    | EFLG_RXEP;

static constexpr uint32_t DEFAULT_SPI_CLOCK = 10000000; // 10MHz


/*
 *  Speed 8M
 */
static constexpr uint8_t MCP_8MHz_1000kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_8MHz_1000kBPS_CFG2 = (0x80);
static constexpr uint8_t MCP_8MHz_1000kBPS_CFG3 = (0x80);

static constexpr uint8_t MCP_8MHz_500kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_8MHz_500kBPS_CFG2 = (0x90);
static constexpr uint8_t MCP_8MHz_500kBPS_CFG3 = (0x82);

static constexpr uint8_t MCP_8MHz_250kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_8MHz_250kBPS_CFG2 = (0xB1);
static constexpr uint8_t MCP_8MHz_250kBPS_CFG3 = (0x85);

static constexpr uint8_t MCP_8MHz_200kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_8MHz_200kBPS_CFG2 = (0xB4);
static constexpr uint8_t MCP_8MHz_200kBPS_CFG3 = (0x86);

static constexpr uint8_t MCP_8MHz_125kBPS_CFG1 = (0x01);
static constexpr uint8_t MCP_8MHz_125kBPS_CFG2 = (0xB1);
static constexpr uint8_t MCP_8MHz_125kBPS_CFG3 = (0x85);

static constexpr uint8_t MCP_8MHz_100kBPS_CFG1 = (0x01);
static constexpr uint8_t MCP_8MHz_100kBPS_CFG2 = (0xB4);
static constexpr uint8_t MCP_8MHz_100kBPS_CFG3 = (0x86);

static constexpr uint8_t MCP_8MHz_80kBPS_CFG1 = (0x01);
static constexpr uint8_t MCP_8MHz_80kBPS_CFG2 = (0xBF);
static constexpr uint8_t MCP_8MHz_80kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_8MHz_50kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_8MHz_50kBPS_CFG2 = (0xB4);
static constexpr uint8_t MCP_8MHz_50kBPS_CFG3 = (0x86);

static constexpr uint8_t MCP_8MHz_40kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_8MHz_40kBPS_CFG2 = (0xBF);
static constexpr uint8_t MCP_8MHz_40kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_8MHz_33k3BPS_CFG1 = (0x47);
static constexpr uint8_t MCP_8MHz_33k3BPS_CFG2 = (0xE2);
static constexpr uint8_t MCP_8MHz_33k3BPS_CFG3 = (0x85);

static constexpr uint8_t MCP_8MHz_31k25BPS_CFG1 = (0x07);
static constexpr uint8_t MCP_8MHz_31k25BPS_CFG2 = (0xA4);
static constexpr uint8_t MCP_8MHz_31k25BPS_CFG3 = (0x84);

static constexpr uint8_t MCP_8MHz_20kBPS_CFG1 = (0x07);
static constexpr uint8_t MCP_8MHz_20kBPS_CFG2 = (0xBF);
static constexpr uint8_t MCP_8MHz_20kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_8MHz_10kBPS_CFG1 = (0x0F);
static constexpr uint8_t MCP_8MHz_10kBPS_CFG2 = (0xBF);
static constexpr uint8_t MCP_8MHz_10kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_8MHz_5kBPS_CFG1 = (0x1F);
static constexpr uint8_t MCP_8MHz_5kBPS_CFG2 = (0xBF);
static constexpr uint8_t MCP_8MHz_5kBPS_CFG3 = (0x87);

/*
 *  speed 16M
 */
static constexpr uint8_t MCP_16MHz_1000kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_16MHz_1000kBPS_CFG2 = (0xD0);
static constexpr uint8_t MCP_16MHz_1000kBPS_CFG3 = (0x82);

static constexpr uint8_t MCP_16MHz_500kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_16MHz_500kBPS_CFG2 = (0xF0);
static constexpr uint8_t MCP_16MHz_500kBPS_CFG3 = (0x86);

static constexpr uint8_t MCP_16MHz_250kBPS_CFG1 = (0x41);
static constexpr uint8_t MCP_16MHz_250kBPS_CFG2 = (0xF1);
static constexpr uint8_t MCP_16MHz_250kBPS_CFG3 = (0x85);

static constexpr uint8_t MCP_16MHz_200kBPS_CFG1 = (0x01);
static constexpr uint8_t MCP_16MHz_200kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_16MHz_200kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_125kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_16MHz_125kBPS_CFG2 = (0xF0);
static constexpr uint8_t MCP_16MHz_125kBPS_CFG3 = (0x86);

static constexpr uint8_t MCP_16MHz_100kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_16MHz_100kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_16MHz_100kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_95kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_16MHz_95kBPS_CFG2 = (0xAD);
static constexpr uint8_t MCP_16MHz_95kBPS_CFG3 = (0x07);

static constexpr uint8_t MCP_16MHz_83k3BPS_CFG1 = (0x03);
static constexpr uint8_t MCP_16MHz_83k3BPS_CFG2 = (0xBE);
static constexpr uint8_t MCP_16MHz_83k3BPS_CFG3 = (0x07);

static constexpr uint8_t MCP_16MHz_80kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_16MHz_80kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_16MHz_80kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_50kBPS_CFG1 = (0x07);
static constexpr uint8_t MCP_16MHz_50kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_16MHz_50kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_40kBPS_CFG1 = (0x07);
static constexpr uint8_t MCP_16MHz_40kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_16MHz_40kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_33k3BPS_CFG1 = (0x4E);
static constexpr uint8_t MCP_16MHz_33k3BPS_CFG2 = (0xF1);
static constexpr uint8_t MCP_16MHz_33k3BPS_CFG3 = (0x85);

static constexpr uint8_t MCP_16MHz_20kBPS_CFG1 = (0x0F);
static constexpr uint8_t MCP_16MHz_20kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_16MHz_20kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_10kBPS_CFG1 = (0x1F);
static constexpr uint8_t MCP_16MHz_10kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_16MHz_10kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_16MHz_5kBPS_CFG1 = (0x3F);
static constexpr uint8_t MCP_16MHz_5kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_16MHz_5kBPS_CFG3 = (0x87);

/*
 *  speed 20M
 */
static constexpr uint8_t MCP_20MHz_1000kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_20MHz_1000kBPS_CFG2 = (0xD9);
static constexpr uint8_t MCP_20MHz_1000kBPS_CFG3 = (0x82);

static constexpr uint8_t MCP_20MHz_500kBPS_CFG1 = (0x00);
static constexpr uint8_t MCP_20MHz_500kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_20MHz_500kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_250kBPS_CFG1 = (0x41);
static constexpr uint8_t MCP_20MHz_250kBPS_CFG2 = (0xFB);
static constexpr uint8_t MCP_20MHz_250kBPS_CFG3 = (0x86);

static constexpr uint8_t MCP_20MHz_200kBPS_CFG1 = (0x01);
static constexpr uint8_t MCP_20MHz_200kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_20MHz_200kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_125kBPS_CFG1 = (0x03);
static constexpr uint8_t MCP_20MHz_125kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_20MHz_125kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_100kBPS_CFG1 = (0x04);
static constexpr uint8_t MCP_20MHz_100kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_20MHz_100kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_83k3BPS_CFG1 = (0x04);
static constexpr uint8_t MCP_20MHz_83k3BPS_CFG2 = (0xFE);
static constexpr uint8_t MCP_20MHz_83k3BPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_80kBPS_CFG1 = (0x04);
static constexpr uint8_t MCP_20MHz_80kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_20MHz_80kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_50kBPS_CFG1 = (0x09);
static constexpr uint8_t MCP_20MHz_50kBPS_CFG2 = (0xFA);
static constexpr uint8_t MCP_20MHz_50kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_40kBPS_CFG1 = (0x09);
static constexpr uint8_t MCP_20MHz_40kBPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_20MHz_40kBPS_CFG3 = (0x87);

static constexpr uint8_t MCP_20MHz_33k3BPS_CFG1 = (0x0B);
static constexpr uint8_t MCP_20MHz_33k3BPS_CFG2 = (0xFF);
static constexpr uint8_t MCP_20MHz_33k3BPS_CFG3 = (0x87);

enum CAN_CLOCK {
    MCP_20MHZ,
    MCP_16MHZ,
    MCP_8MHZ
};

enum CAN_SPEED {
    CAN_5KBPS,
    CAN_10KBPS,
    CAN_20KBPS,
    CAN_31K25BPS,
    CAN_33KBPS,
    CAN_40KBPS,
    CAN_50KBPS,
    CAN_80KBPS,
    CAN_83K3BPS,
    CAN_95KBPS,
    CAN_100KBPS,
    CAN_125KBPS,
    CAN_200KBPS,
    CAN_250KBPS,
    CAN_500KBPS,
    CAN_1000KBPS
};


}


#if 0
//sidh sidl eid0 eid8
struct [[nodiscard]] IdentifierLayout final{
    std::array<uint8_t, 4> bytes;

    static constexpr IdentifierLayout from_stdid(const hal::CanStdId stdid){
        using Self = IdentifierLayout;
        const uint32_t id_u32 = static_cast<uint32_t>(stdid.to_u11());
        Self self;
        self.sidl() = static_cast<uint8_t>((id_u32 << 5) | 0);
        self.sidh() = static_cast<uint8_t>((id_u32 >> 3) | 0);
        self.eid0() = 0;
        self.eid8() = 0;
        return self;
    };

    static constexpr IdentifierLayout from_extid(const hal::CanExtId extid){
        using Self = IdentifierLayout;
        const uint32_t id_u32 = static_cast<uint32_t>(extid.to_u29());
        Self self;
        self.eid0() = static_cast<uint8_t>(id_u32 >> 0);
        self.eid8() = static_cast<uint8_t>(id_u32 >> 8);

        uint8_t temp_sidl = 0;
        temp_sidl |= static_cast<uint8_t>((id_u32 >> 16) & 0x03);
        temp_sidl |= static_cast<uint8_t>((id_u32 >> 16) & 0x1c << 3);
        temp_sidl |= TXB_EXIDE_MASK;

        self.sidl() = temp_sidl;
        self.sidh() = static_cast<uint8_t>((id_u32 >> (16 + 5)));
        return self;
    };

    template<typename Self>
    constexpr auto & sidh(this Self && self){
        return self.bytes[0];
    }

    template<typename Self>
    constexpr auto & sidl(this Self && self){
        return self.bytes[1];
    }

    template<typename Self>
    constexpr auto & eid0(this Self && self){
        return self.bytes[2];
    }


    template<typename Self>
    constexpr auto & eid8(this Self && self){
        return self.bytes[3];
    }
};
#endif


template<typename T = void>
using IResult = MCP2515::IResult<T>;



static constexpr void exact_id(uint8_t *buffer, const bool ext, const uint32_t id)
{
    uint16_t canid = (uint16_t)(id & 0x0FFFF);

    if (ext) {
        buffer[MCP_EID0] = (uint8_t) (canid & 0xFF);
        buffer[MCP_EID8] = (uint8_t) (canid >> 8);
        canid = (uint16_t)(id >> 16);
        buffer[MCP_SIDL] = (uint8_t) (canid & 0x03);
        buffer[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
        buffer[MCP_SIDL] |= TXB_EXIDE_MASK;
        buffer[MCP_SIDH] = (uint8_t) (canid >> 5);
    } else {
        buffer[MCP_SIDH] = (uint8_t) (canid >> 3);
        buffer[MCP_SIDL] = (uint8_t) ((canid & 0x07 ) << 5);
        buffer[MCP_EID0] = 0;
        buffer[MCP_EID8] = 0;
    }
}


IResult<> MCP2515::set_filter_mask(const MASK mask, const bool ext, const uint32_t ulData)
{
    if(const auto res = set_mode(ReqOp::Configure);
        res.is_err()) return Err(res.unwrap_err());
    
    uint8_t buf[4];
    exact_id(buf, ext, ulData);

    RegAddr reg_addr;
    switch (mask) {
        case MASK0: reg_addr = RegAddr::RXM0SIDH; break;
        case MASK1: reg_addr = RegAddr::RXM1SIDH; break;
    }

    return write_bulk(reg_addr, buf, 4);
}



IResult<> MCP2515::set_filter(const RXF num, const bool ext, const uint32_t ulData)
{
    if(const auto res = set_mode(ReqOp::Configure);
        res.is_err()) return Err(res.unwrap_err());

    RegAddr reg_addr;

    switch (num) {
        case RXF0: reg_addr = RegAddr::RXF0SIDH; break;
        case RXF1: reg_addr = RegAddr::RXF1SIDH; break;
        case RXF2: reg_addr = RegAddr::RXF2SIDH; break;
        case RXF3: reg_addr = RegAddr::RXF3SIDH; break;
        case RXF4: reg_addr = RegAddr::RXF4SIDH; break;
        case RXF5: reg_addr = RegAddr::RXF5SIDH; break;
    }

    uint8_t buf[4];
    exact_id(buf, ext, ulData);
    
    return write_bulk(reg_addr, buf, 4);
}




#if 0


IResult<> MCP2515::setClkOut(const ClkDiv divisor)
{
    if (divisor == CLKOUT_DISABLE) {
        /* Turn off CLKEN */
        if(const auto res = modify_reg(RegAddr::CANCTRL, CANCTRL_CLKEN, 0x00);
            res.is_err()) return Err(res.unwrap_err());

        /* Turn on CLKOUT for SOF */
        if(const auto res = modify_reg(RegAddr::CNF3, CNF3_SOF, CNF3_SOF);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    /* Set the prescaler (CLKPRE) */
    if(const auto res = modify_reg(RegAddr::CANCTRL, CANCTRL_CLKPRE, divisor);
        res.is_err()) return Err(res.unwrap_err());

    /* Turn on CLKEN */
    if(const auto res = modify_reg(RegAddr::CANCTRL, CANCTRL_CLKEN, CANCTRL_CLKEN);
        res.is_err()) return Err(res.unwrap_err());

    /* Turn off CLKOUT for SOF */
    if(const auto res = modify_reg(RegAddr::CNF3, CNF3_SOF, 0x00);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


IResult<> MCP2515::set_mode(const CANCTRL_REQOP_MODE mode)
{
    if(const auto res = modify_reg(RegAddr::CANCTRL, CANCTRL_REQOP | CANCTRL_OSM, mode);
        res.is_err()) return res;

    const auto endTime = clock::millis().count() + 10;
    bool modeMatch = false;


    while (clock::millis().count() < endTime) {
        CANCTRL_REQOP_MODE newmode;
        if(const auto res = read_reg(RegAddr::CANSTAT, newmode);
            res.is_err()) return Err(res.unwrap_err());


        newmode &= CANSTAT_OPMOD;

        modeMatch = (newmode == mode);

        if (modeMatch) {
            break;
        }
    }

    return modeMatch ? ERROR_OK : ERROR_FAIL;

}

uint8_t MCP2515::getStatus(void)
{

}

IResult<> MCP2515::set_mode()
{
    return set_mode(CANCTRL_REQOP_CONFIG);
}

IResult<> MCP2515::setListenOnlyMode()
{
    return set_mode(CANCTRL_REQOP_LISTENONLY);
}

IResult<> MCP2515::setSleepMode()
{
    return set_mode(CANCTRL_REQOP_SLEEP);
}

IResult<> MCP2515::setLoopbackMode()
{
    return set_mode(CANCTRL_REQOP_LOOPBACK);
}

IResult<> MCP2515::setNormalMode()
{
    return set_mode(CANCTRL_REQOP_NORMAL);
}

IResult<> MCP2515::setNormalOneShotMode()
{
    return set_mode(CANCTRL_REQOP_OSM);
}


#if 0

#endif


IResult<> MCP2515::transmit_frame(const TXBn txbn, const hal::ClassicCanFrame & can_frame){
    const TXBn_REGS *txbuf = &TXB[txbn];

    uint8_t buf[13];

    bool ext = (can_frame.is_extended());
    bool rtr = (can_frame.is_remote());
    uint32_t id = (can_frame.id_u32() & (ext ? CAN_EFF_MASK : CAN_SFF_MASK));

    exact_id(buf, ext, id);

    buf[MCP_DLC] = rtr ? (can_frame.can_dlc | RTR_MASK) : can_frame.can_dlc;

    memcpy(&buf[MCP_DATA], can_frame.buf, can_frame.can_dlc);

    write_bulk(txbuf->SIDH, buf, 5 + can_frame.can_dlc);

    modify_reg(txbuf->CTRL, TXB_TXREQ, TXB_TXREQ);

    uint8_t ctrl = read_reg(txbuf->CTRL);
    if ((ctrl & (TXB_ABTF | TXB_MLOA | TXB_TXERR)) != 0) {
        return ERROR_FAILTX;
    }
    return Ok();
}

IResult<> MCP2515::transmit_frame(const hal::ClassicCanFrame & can_frame){
    TXBn txBuffers[N_TXBUFFERS] = {TXB0, TXB1, TXB2};

    for (int i=0; i<N_TXBUFFERS; i++) {
        const TXBn_REGS *txbuf = &TXB[txBuffers[i]];
        uint8_t ctrlval = read_reg(txbuf->CTRL);
        if ( (ctrlval & TXB_TXREQ) == 0 ) {
            return transmit_frame(txBuffers[i], can_frame);
        }
    }

    return ERROR_ALLTXBUSY;
}

IResult<> MCP2515::receive_frame(const RXBn rxbn, hal::ClassicCanFrame & can_frame){
    const RXBn_REGS *rxb = &RXB[rxbn];

    uint8_t buf[5];

    read_regs(rxb->SIDH, buf, 5);

    uint32_t id = (buf[MCP_SIDH]<<3) + (buf[MCP_SIDL]>>5);

    if ( (buf[MCP_SIDL] & TXB_EXIDE_MASK) ==  TXB_EXIDE_MASK ) {
        id = (id<<2) + (buf[MCP_SIDL] & 0x03);
        id = (id<<8) + buf[MCP_EID8];
        id = (id<<8) + buf[MCP_EID0];
        id |= CAN_EFF_FLAG;
    }

    uint8_t dlc = (buf[MCP_DLC] & DLC_MASK);
    uint8_t ctrl = read_reg(rxb->CTRL);
    if (ctrl & RXBnCTRL_RTR) {
        id |= CAN_RTR_FLAG;
    }

    can_frame.can_id = id;
    can_frame.can_dlc = dlc;

    read_regs(rxb->DATA, can_frame.buf, dlc);

    modify_reg(RegAddr::CANINTF, rxb->CANINTF_RXnIF, 0);

    return Ok();
}

IResult<> MCP2515::receive_frame(hal::ClassicCanFrame & can_frame)
{
    ERROR rc;
    uint8_t stat = getStatus();

    if ( stat & STAT_RX0IF ) {
        rc = receive_frame(RXB0, can_frame);
    } else if ( stat & STAT_RX1IF ) {
        rc = receive_frame(RXB1, can_frame);
    } else {
        rc = ERROR_NOMSG;
    }

    return rc;
}

bool MCP2515::checkReceive(void)
{
    uint8_t res = getStatus();
    if ( res & STAT_RXIF_MASK ) {
        return true;
    } else {
        return false;
    }
}

bool MCP2515::checkError(void)
{
    uint8_t eflg = getErrorFlags();

    if ( eflg & EFLG_ERRORMASK ) {
        return true;
    } else {
        return false;
    }
}

uint8_t MCP2515::getErrorFlags(void)
{
    return read_reg(RegAddr::EFLG);
}

uint8_t MCP2515::getControlRegister(void) 
{
    return read_reg(RegAddr::CANCTRL);
}

void MCP2515::clearRXnOVRFlags(void)
{
	modify_reg(RegAddr::EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0);
}

uint8_t MCP2515::getInterrupts(void)
{
    return read_reg(RegAddr::CANINTF);
}

void MCP2515::clearInterrupts(void)
{
    write_reg(RegAddr::CANINTF, 0);
}

uint8_t MCP2515::getInterruptMask(void)
{
    return read_reg(RegAddr::CANINTE);
}

void MCP2515::clearTXInterrupts(void)
{
    modify_reg(RegAddr::CANINTF, (CANINTF_TX0IF | CANINTF_TX1IF | CANINTF_TX2IF), 0);
}

void MCP2515::clearRXnOVR(void)
{
	uint8_t eflg = getErrorFlags();
	if (eflg != 0) {
		clearRXnOVRFlags();
		clearInterrupts();
		//modify_reg(RegAddr::CANINTF, CANINTF_ERRIF, 0);
	}
	
}

void MCP2515::clearMERR()
{
	//modify_reg(RegAddr::EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0);
	//clearInterrupts();
	modify_reg(RegAddr::CANINTF, CANINTF_MERRF, 0);
}

void MCP2515::clearERRIF()
{
    //modify_reg(RegAddr::EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0);
    //clearInterrupts();
    modify_reg(RegAddr::CANINTF, CANINTF_ERRIF, 0);
}

uint8_t IResult<>CountRX(void)                             
{
    return read_reg(RegAddr::REC);
}

uint8_t IResult<>CountTX(void)                             
{
    return read_reg(RegAddr::TEC);
}


IResult<> MCP2515::reset(void){
    clock::delay(10ms);

    uint8_t zeros[14];
    memset(zeros, 0, sizeof(zeros));
    write_bulk(RegAddr::TXB0CTRL, zeros, 14);
    write_bulk(RegAddr::TXB1CTRL, zeros, 14);
    write_bulk(RegAddr::TXB2CTRL, zeros, 14);

    write_reg(RegAddr::RXB0CTRL, 0);
    write_reg(RegAddr::RXB1CTRL, 0);

    write_reg(RegAddr::CANINTE, CANINTF_RX0IF | CANINTF_RX1IF | CANINTF_ERRIF | CANINTF_MERRF);

    // receives all valid messages using either Standard or Extended Identifiers that
    // meet filter criteria. RXF0 is applied for RXB0, RXF1 is applied for RXB1
    modify_reg(RegAddr::RXB0CTRL,
                   RXBnCTRL_RXM_MASK | RXB0CTRL_BUKT | RXB0CTRL_FILHIT_MASK,
                   RXBnCTRL_RXM_STDEXT | RXB0CTRL_BUKT | RXB0CTRL_FILHIT);
    modify_reg(RegAddr::RXB1CTRL,
                   RXBnCTRL_RXM_MASK | RXB1CTRL_FILHIT_MASK,
                   RXBnCTRL_RXM_STDEXT | RXB1CTRL_FILHIT);

    // clear filters and masks
    // do not filter any standard frames for RXF0 used by RXB0
    // do not filter any extended frames for RXF1 used by RXB1
    RXF filters[] = {RXF0, RXF1, RXF2, RXF3, RXF4, RXF5};
    for (int i=0; i<6; i++) {
        bool ext = (i == 1);
        ERROR result = set_filter(filters[i], ext, 0);
        if (result != ERROR_OK) {
            return result;
        }
    }

    MASK masks[] = {MASK0, MASK1};
    for (int i=0; i<2; i++) {
        ERROR result = set_filter_mask(masks[i], true, 0);
        if (result != ERROR_OK) {
            return result;
        }
    }

    return Ok();
}

uint8_t MCP2515::read_reg(const RegAddr reg_addr)
{
    return 0;
}

void MCP2515::read_regs(const RegAddr reg_addr, uint8_t * values, const uint8_t n)
{

}

void MCP2515::write_reg(const RegAddr reg_addr, const uint8_t value)
{

}

void MCP2515::write_bulk(const RegAddr reg_addr, const uint8_t * values, const uint8_t n)
{

}

void MCP2515::modify_reg(const RegAddr reg_addr, const uint8_t mask, const uint8_t buf)
{
}

#endif