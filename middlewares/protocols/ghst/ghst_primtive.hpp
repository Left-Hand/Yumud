#pragma once

#include <cstdint>
#include <array>
#include <span>

// GHST 是由 ImmersionRC 为其 Ghost RC 系统开发的一种高速无线电控制协议，能够实现低延迟的控制数据传输


// https://github.com/cruwaller/rc_receiver_to_usb_hid/blob/master/lib/ghst/ghst.h
// https://deepwiki.com/cruwaller/rc_receiver_to_usb_hid/3.3-ghst-protocol



namespace ymd::ghst{


enum class Addr:uint8_t{
    RADIO             = 0x80,
    TX_MODULE_SYM     = 0x81,     // symmetrical, 400k pulses, 400k telemetry
    TX_MODULE_ASYM    = 0x88,     // asymmetrical, 400k pulses, 115k telemetry
    FC                = 0x82,
    GOGGLES           = 0x83,
    QUANTUM_TEE1      = 0x84,     // phase 2
    QUANTUM_TEE2      = 0x85,
    QUANTUM_GW1       = 0x86,
    CLK_5G            = 0x87,     // phase 3
    RX                = 0x89
};



enum class UplinkType:uint8_t{
    HS4_5TO8   = 0x10,     // High Speed 4 channel, plus CH5-8
    HS4_9TO12  = 0x11,     // High Speed 4 channel, plus CH9-12
    HS4_13TO16 = 0x12,     // High Speed 4 channel, plus CH13-16
    HS4_RSSI   = 0x13,     // primary 4 channel, plus RSSI, LQ, RF Mode, and Tx Power
    HS4_LAST   = 0x1f,     // Last frame type including 4 primary channels
};

static constexpr size_t GHST_UL_RC_CHANS_SIZE       = 12;      // 1 (type) + 10 (data) + 1 (crc)

static constexpr size_t GHST_NUM_OF_CHANNELS        = 16;      // 4 anlogs + 12 switches

enum class DownlinkType:uint8_t{
    OPENTX_SYNC         = 0x20,
    LINK_STAT           = 0x21,
    VTX_STAT            = 0x22,
    PACK_STAT           = 0x23,     // Battery (Pack) Status
};

static constexpr uint16_t GHST_RC_CTR_VAL_12BIT       = 0x7C0;   // servo center for 12 bit values (0x3e0 << 1)
static constexpr uint8_t GHST_RC_CTR_VAL_8BIT        = 0x7C;    // servo center for 8 bit values


static constexpr size_t GHST_PAYLOAD_SIZE_MAX       = 14;

static constexpr size_t GHST_FRAME_SIZE             = 14;      // including addr, type, len, crc, and payload
static constexpr size_t GHST_FRAME_SIZE_MAX         = 24;

struct [[nodiscard]] FlatFrame final {
    uint8_t addr;
    uint8_t len;
    uint8_t type;
    std::array<uint8_t, GHST_PAYLOAD_SIZE_MAX + 1> flex;         // CRC adds 1
};


struct U12X4{
    // uint16_t ch1: 12;
    // uint16_t ch2: 12;
    // uint16_t ch3: 12;
    // uint16_t ch4: 12;
    std::array<uint8_t, 6> bytes;


};

static_assert(sizeof(U12X4) == 6);

/* Pulses payload (channel data), for 4x 8-bit channels */
struct PayloadChannels_s {
    uint8_t cha;
    uint8_t chb;
    uint8_t chc;
    uint8_t chd;
} ;

/* Pulses payload (channel data), with RSSI/LQ, and other related data */
struct PayloadPulsesRssi_s {
    uint8_t lq;                 // 0-100
    uint8_t rssi;               // 0 - 128 sign inverted, dBm
    uint8_t rfProtocol;
    int8_t  txPwrdBm;           // tx power in dBm, use lookup table to map to published mW values
} ;


#if 0
/* Pulses payload (channel data). Includes 4x high speed control channels, plus 4 channels from CH5-CH12 */
struct PayloadPulses_s {
    // 80 bits, or 10 bytes
    ghstPayloadServo4_t ch1to4;
    union {
        ghstPayloadChannels_t aux;
        ghstPayloadPulsesRssi_t stat;
    };
} ;

struct Header_s {
    uint8_t addr;
    uint8_t len;
} ;

struct RcFrame_s {
    ghstHeader_t hdr;
    uint8_t type;
    ghstPayloadPulses_t channels;
    uint8_t crc;
};

#endif

struct TlmDl_s {
    uint16_t voltage;  // mv * 100
    uint16_t current;  // ma * 100
    uint16_t capacity; // mah * 100
    uint8_t rx_voltage;
    uint8_t tbd1;
    uint8_t tbd2;
    uint8_t tbd3;
};

/* This is just to keep other code compatible */
struct Linkstatistics_s {
    uint8_t uplink_RSSI_1;
    uint8_t uplink_RSSI_2;
    uint8_t uplink_Link_quality;
    int8_t  uplink_SNR;
    uint8_t rf_Mode;
};


}