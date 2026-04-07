#include "mcp2515.hpp"

using namespace ymd;
using namespace ymd::drivers;


#if 0
namespace {

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



uint8_t MCP2515::getStatus(void)
{

}

MCP2515::ERROR MCP2515::setConfigMode()
{
    return setMode(CANCTRL_REQOP_CONFIG);
}

MCP2515::ERROR MCP2515::setListenOnlyMode()
{
    return setMode(CANCTRL_REQOP_LISTENONLY);
}

MCP2515::ERROR MCP2515::setSleepMode()
{
    return setMode(CANCTRL_REQOP_SLEEP);
}

MCP2515::ERROR MCP2515::setLoopbackMode()
{
    return setMode(CANCTRL_REQOP_LOOPBACK);
}

MCP2515::ERROR MCP2515::setNormalMode()
{
    return setMode(CANCTRL_REQOP_NORMAL);
}

MCP2515::ERROR MCP2515::setNormalOneShotMode()
{
    return setMode(CANCTRL_REQOP_OSM);
}


#if 0
MCP2515::ERROR MCP2515::setMode(const CANCTRL_REQOP_MODE mode)
{
    modify_reg(RegAddr::CANCTRL, CANCTRL_REQOP | CANCTRL_OSM, mode);

    unsigned long endTime = clock::millis().count() + 10;
    bool modeMatch = false;
    while (clock::millis().count() < endTime) {
        uint8_t newmode = read_reg(RegAddr::CANSTAT);
        newmode &= CANSTAT_OPMOD;

        modeMatch = newmode == mode;

        if (modeMatch) {
            break;
        }
    }

    return modeMatch ? ERROR_OK : ERROR_FAIL;

}
#endif


MCP2515::ERROR MCP2515::setClkOut(const CAN_CLKOUT divisor)
{
    if (divisor == CLKOUT_DISABLE) {
	/* Turn off CLKEN */
	modify_reg(RegAddr::CANCTRL, CANCTRL_CLKEN, 0x00);

	/* Turn on CLKOUT for SOF */
	modify_reg(RegAddr::CNF3, CNF3_SOF, CNF3_SOF);
        return ERROR_OK;
    }

    /* Set the prescaler (CLKPRE) */
    modify_reg(RegAddr::CANCTRL, CANCTRL_CLKPRE, divisor);

    /* Turn on CLKEN */
    modify_reg(RegAddr::CANCTRL, CANCTRL_CLKEN, CANCTRL_CLKEN);

    /* Turn off CLKOUT for SOF */
    modify_reg(RegAddr::CNF3, CNF3_SOF, 0x00);
    return ERROR_OK;
}

void MCP2515::prepareId(uint8_t *buffer, const bool ext, const uint32_t id)
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

MCP2515::ERROR MCP2515::setFilterMask(const MASK mask, const bool ext, const uint32_t ulData)
{
    ERROR res = setConfigMode();
    if (res != ERROR_OK) {
        return res;
    }
    
    uint8_t tbufdata[4];
    prepareId(tbufdata, ext, ulData);

    RegAddr reg;
    switch (mask) {
        case MASK0: reg = RegAddr::RXM0SIDH; break;
        case MASK1: reg = RegAddr::RXM1SIDH; break;
        default:
            return ERROR_FAIL;
    }

    write_bulk(reg, tbufdata, 4);
    
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::setFilter(const RXF num, const bool ext, const uint32_t ulData)
{
    ERROR res = setConfigMode();
    if (res != ERROR_OK) {
        return res;
    }

    RegAddr reg;

    switch (num) {
        case RXF0: reg = RegAddr::RXF0SIDH; break;
        case RXF1: reg = RegAddr::RXF1SIDH; break;
        case RXF2: reg = RegAddr::RXF2SIDH; break;
        case RXF3: reg = RegAddr::RXF3SIDH; break;
        case RXF4: reg = RegAddr::RXF4SIDH; break;
        case RXF5: reg = RegAddr::RXF5SIDH; break;
        default:
            return ERROR_FAIL;
    }

    uint8_t tbufdata[4];
    prepareId(tbufdata, ext, ulData);
    write_bulk(reg, tbufdata, 4);

    return ERROR_OK;
}

MCP2515::ERROR MCP2515::transmit_frame(const TXBn txbn, const hal::ClassicCanFrame & can_frame){
    const TXBn_REGS *txbuf = &TXB[txbn];

    uint8_t buf[13];

    bool ext = (can_frame.is_extended());
    bool rtr = (can_frame.is_remote());
    uint32_t id = (can_frame.id_u32() & (ext ? CAN_EFF_MASK : CAN_SFF_MASK));

    prepareId(buf, ext, id);

    buf[MCP_DLC] = rtr ? (can_frame.can_dlc | RTR_MASK) : can_frame.can_dlc;

    memcpy(&buf[MCP_DATA], can_frame.buf, can_frame.can_dlc);

    write_bulk(txbuf->SIDH, buf, 5 + can_frame.can_dlc);

    modify_reg(txbuf->CTRL, TXB_TXREQ, TXB_TXREQ);

    uint8_t ctrl = read_reg(txbuf->CTRL);
    if ((ctrl & (TXB_ABTF | TXB_MLOA | TXB_TXERR)) != 0) {
        return ERROR_FAILTX;
    }
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::transmit_frame(const hal::ClassicCanFrame & can_frame){
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

MCP2515::ERROR MCP2515::receive_frame(const RXBn rxbn, hal::ClassicCanFrame & can_frame){
    const RXBn_REGS *rxb = &RXB[rxbn];

    uint8_t tbufdata[5];

    read_regs(rxb->SIDH, tbufdata, 5);

    uint32_t id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);

    if ( (tbufdata[MCP_SIDL] & TXB_EXIDE_MASK) ==  TXB_EXIDE_MASK ) {
        id = (id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        id = (id<<8) + tbufdata[MCP_EID8];
        id = (id<<8) + tbufdata[MCP_EID0];
        id |= CAN_EFF_FLAG;
    }

    uint8_t dlc = (tbufdata[MCP_DLC] & DLC_MASK);
    uint8_t ctrl = read_reg(rxb->CTRL);
    if (ctrl & RXBnCTRL_RTR) {
        id |= CAN_RTR_FLAG;
    }

    can_frame.can_id = id;
    can_frame.can_dlc = dlc;

    read_regs(rxb->DATA, can_frame.buf, dlc);

    modify_reg(RegAddr::CANINTF, rxb->CANINTF_RXnIF, 0);

    return ERROR_OK;
}

MCP2515::ERROR MCP2515::receive_frame(hal::ClassicCanFrame & can_frame)
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

uint8_t MCP2515::errorCountRX(void)                             
{
    return read_reg(RegAddr::REC);
}

uint8_t MCP2515::errorCountTX(void)                             
{
    return read_reg(RegAddr::TEC);
}


MCP2515::ERROR MCP2515::reset(void){
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
        ERROR result = setFilter(filters[i], ext, 0);
        if (result != ERROR_OK) {
            return result;
        }
    }

    MASK masks[] = {MASK0, MASK1};
    for (int i=0; i<2; i++) {
        ERROR result = setFilterMask(masks[i], true, 0);
        if (result != ERROR_OK) {
            return result;
        }
    }

    return ERROR_OK;
}

uint8_t MCP2515::read_reg(const RegAddr reg)
{
    return 0;
}

void MCP2515::read_regs(const RegAddr reg, uint8_t * values, const uint8_t n)
{

}

void MCP2515::write_reg(const RegAddr reg, const uint8_t value)
{

}

void MCP2515::write_bulk(const RegAddr reg, const uint8_t * values, const uint8_t n)
{

}

void MCP2515::modify_reg(const RegAddr reg, const uint8_t mask, const uint8_t buf)
{
}

#endif