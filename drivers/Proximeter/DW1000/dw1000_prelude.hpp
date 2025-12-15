#pragma once
#include <cstdint>

// MIT license
// https://github.com/F-Army/arduino-dw1000-ng/blob/master/src/DW1000NgConstants.hpp

namespace ymd::drivers{
namespace dw1000{


static constexpr float TIME_RES     = 0.000015650040064103f;
static constexpr float TIME_RES_INV = 63897.6f;

/* Speed of radio waves (light) [m/s] * timestamp resolution [~15.65ps] of DW1000Ng */
static constexpr float DISTANCE_OF_RADIO     = 0.0046917639786159f;
static constexpr float DISTANCE_OF_RADIO_INV = 213.139451293f;

// timestamp uint8_t length - 40 bit -> 5 uint8_t
static constexpr uint8_t LENGTH_TIMESTAMP = 5;

// timer/counter overflow (40 bits) -> 4overflow approx. every 17.2 seconds
static constexpr int64_t TIME_OVERFLOW = 0x10000000000; //1099511627776LL
static constexpr int64_t TIME_MAX      = 0xffffffffff;

// time factors (relative to [us]) for setting delayed transceive
static constexpr float SECONDS      = 1e6;
static constexpr float MILLISECONDS = 1e3;
static constexpr float MICROSECONDS = 1;
static constexpr float NANOSECONDS  = 1e-3;

/* preamble codes (CHAN_CTRL - RX & TX _CODE) - reg:0x1F, bits:31-27,26-22 */

enum class PreambleCode : uint8_t {
    _1 = 1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    _10,
    _11,
    _12,
    _17 = 17,
    _18,
    _19,
    _20
};

/* Validity matrix for 16 MHz PRF preamble codes */
static constexpr uint8_t preamble_validity_matrix_PRF16[8][2] = {
    {0,0}, /* Channel 0 doesn't exist */
    {1, 2},
    {3, 4},
    {5, 6},
    {7, 8},
    {3, 4},
    {0,0}, /* Channel 5 doesn't exist */
    {7, 8}
};

/* Validity matrix for 64 MHz PRF preamble codes */
static constexpr uint8_t preamble_validity_matrix_PRF64[8][4] = {
    {0,0,0,0}, /* Channel 0 doesn't exist */
    {9, 10, 11, 12},
    {9, 10, 11, 12},
    {9, 10, 11, 12},
    {17, 18, 19, 20},
    {9, 10, 11, 12},
    {0,0,0,0}, /* Channel 5 doesn't exist */
    {17, 18, 19, 20}
};

/* transmission/reception bit rate (TXBR) - reg:0x08, bits:14,13 */
enum class DataRate : uint8_t {
    RATE_110KBPS, 
    RATE_850KBPS, 
    RATE_6800KBPS
};

/* transmission pulse frequency (TXPRF) - reg:0x08, bits:17,16
* 0x00 is 4MHZ, but receiver in DW1000Ng does not support it (!??) */
enum class PulseFrequency : uint8_t {
    FREQ_16MHZ = 0x01,
    FREQ_64MHZ
};

/* preamble length (PE + TXPSR) - reg:0x08, bits:21,20,19,18 - table 16 */
enum class PreambleLength : uint8_t {
    LEN_64   = 0x01,
    LEN_128  = 0x05,
    LEN_256  = 0x09,
    LEN_512  = 0x0D,
    LEN_1024 = 0x02,
    LEN_1536 = 0x06,
    LEN_2048 = 0x0A,
    LEN_4096 = 0x03
};


/* PAC size (DRX_TUNE2) - reg:0x08, sub-reg:0x27, bits:26,25 - table 33
* The value to program the sub-register changes in based of RXPRF */
enum class PacSize : uint8_t {
    SIZE_8  = 8,
    SIZE_16 = 16,
    SIZE_32 = 32,
    SIZE_64 = 64
};

/* channel of operation (CHAN_CTRL - TX & RX _CHAN) - reg:0x1F, bits:3-0,7-4 */
enum class Channel : uint8_t {
    CHANNEL_1 = 1,
    CHANNEL_2,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_7 = 7
};

/* Register is 6 bit, 7 = write, 6 = sub-adressing, 5-0 = register value
 * Total header with sub-adressing can be 15 bit. */
static constexpr uint8_t WRITE      = 0x80; // regular write
static constexpr uint8_t WRITE_SUB  = 0xC0; // write with sub address
static constexpr uint8_t READ       = 0x00; // regular read
static constexpr uint8_t READ_SUB   = 0x40; // read with sub address
static constexpr uint8_t RW_SUB_EXT = 0x80; // R/W with sub address extension

/* clocks available. */
static constexpr uint8_t SYS_AUTO_CLOCK = 0x00;
static constexpr uint8_t SYS_XTI_CLOCK  = 0x01;
static constexpr uint8_t SYS_PLL_CLOCK  = 0x02;
static constexpr uint8_t TX_PLL_CLOCK = 0x20;
static constexpr uint8_t LDE_CLOCK = 0x03;

/* range bias tables - APS011*/

static constexpr double BIAS_TABLE[18][5] = {
    {61, -198, -110, -275, -295},
    {63, -187, -105, -244, -266},
    {65, -179, -100, -210, -235},
    {67, -163, -93, -176, -199},
    {69, -143, -82, -138, -150},
    {71, -127, -69, -95, -100},
    {73, -109, -51, -51, -58},
    {75, -84, -27, 0, 0},
    {77, -59, 0, 42, 49},
    {79, -31, 21, 97, 91},
    {81, 0, 35, 158, 127},
    {83, 36, 42, 210, 153},
    {85, 65, 49, 254, 175},
    {87, 84, 62, 294, 197},
    {89, 97, 71, 321, 233},
    {91, 106, 76, 339, 245},
    {93, 110, 81, 356, 264},
    {95, 112, 86, 394, 284}
};

enum class DriverAmplifierValue : uint8_t {
    dB_18,
    dB_15,
    dB_12,
    dB_9,
    dB_6,
    dB_3,
    dB_0,
    OFF
};

enum class TransmitMixerValue : uint8_t {
    dB_0, 
    dB_0_5, 
    dB_1, 
    dB_1_5, 
    dB_2, 
    dB_2_5,
    dB_3,
    dB_3_5,
    dB_4,
    dB_4_5,
    dB_5,
    dB_5_5,
    dB_6,
    dB_6_5,
    dB_7,
    dB_7_5,
    dB_8,
    dB_8_5,
    dB_9,
    dB_9_5,
    dB_10,
    dB_10_5,
    dB_11,
    dB_11_5,
    dB_12,
    dB_12_5,
    dB_13,
    dB_13_5,
    dB_14,
    dB_14_5,
    dB_15,
    dB_15_5
};

enum class SFDMode {STANDARD_SFD, DECAWAVE_SFD};

enum class TransmitMode {IMMEDIATE, DELAYED};

enum class ReceiveMode {IMMEDIATE, DELAYED};

enum class SpiClock {SLOW, FAST};


// no sub-address for register write
static constexpr uint16_t NO_SUB = 0xFF;

// device id register
static constexpr uint16_t DEV_ID = 0x00;
static constexpr uint16_t LEN_DEV_ID = 4;

// extended unique identifier register
static constexpr uint16_t EUI = 0x01;
static constexpr uint16_t LEN_EUI = 8;

// PAN identifier, short address register
static constexpr uint16_t PANADR = 0x03;
static constexpr uint16_t LEN_PANADR = 4;

// device configuration register
static constexpr uint16_t SYS_CFG = 0x04;
static constexpr uint16_t FFEN_BIT = 0;
static constexpr uint16_t FFBC_BIT = 1;
static constexpr uint16_t FFAB_BIT = 2;
static constexpr uint16_t FFAD_BIT = 3;
static constexpr uint16_t FFAA_BIT = 4;
static constexpr uint16_t FFAM_BIT = 5;
static constexpr uint16_t FFAR_BIT = 6;
static constexpr uint16_t FFA4_BIT = 7;
static constexpr uint16_t FFA5_BIT = 8;
static constexpr uint16_t HIRQ_POL_BIT = 9;
static constexpr uint16_t SPI_EDGE_BIT = 10;
static constexpr uint16_t DIS_FCE_BIT = 11;
static constexpr uint16_t DIS_DRXB_BIT = 12;
static constexpr uint16_t DIS_PHE_BIT = 13;
static constexpr uint16_t DIS_RSDE_BIT = 14;
static constexpr uint16_t FCS_INIT2F_BIT = 15;
static constexpr uint16_t PHR_MODE_0_BIT = 16;
static constexpr uint16_t PHR_MODE_1_BIT = 17;
static constexpr uint16_t DIS_STXP_BIT = 18;
static constexpr uint16_t RXM110K_BIT = 22;
static constexpr uint16_t RXWTOE_BIT = 28;
static constexpr uint16_t RXAUTR_BIT = 29;
static constexpr uint16_t AUTOACK_BIT = 30;
static constexpr uint16_t AACKPEND_BIT = 31;
static constexpr uint16_t LEN_SYS_CFG = 4;

// device control register
static constexpr uint16_t SYS_CTRL = 0x0D;
static constexpr uint16_t LEN_SYS_CTRL = 4;
static constexpr uint16_t SFCST_BIT = 0;
static constexpr uint16_t TXSTRT_BIT = 1;
static constexpr uint16_t TXDLYS_BIT = 2;
static constexpr uint16_t TRXOFF_BIT = 6;
static constexpr uint16_t WAIT4RESP_BIT = 7;
static constexpr uint16_t RXENAB_BIT = 8;
static constexpr uint16_t RXDLYS_BIT = 9;

// system event status register
static constexpr uint16_t SYS_STATUS = 0x0F;
static constexpr uint16_t SYS_STATUS_SUB = 0x04;
static constexpr uint16_t IRQS_BIT = 0;
static constexpr uint16_t CPLOCK_BIT = 1;
static constexpr uint16_t ESYNCR_BIT = 2;
static constexpr uint16_t AAT_BIT = 3;
static constexpr uint16_t TXFRB_BIT = 4;
static constexpr uint16_t TXPRS_BIT = 5;
static constexpr uint16_t TXPHS_BIT = 6;
static constexpr uint16_t TXFRS_BIT = 7;
static constexpr uint16_t RXPRD_BIT = 8;
static constexpr uint16_t RXSFDD_BIT = 9;
static constexpr uint16_t LDEDONE_BIT = 10;
static constexpr uint16_t RXPHD_BIT = 11;
static constexpr uint16_t RXPHE_BIT = 12;
static constexpr uint16_t RXDFR_BIT = 13;
static constexpr uint16_t RXFCG_BIT = 14;
static constexpr uint16_t RXFCE_BIT = 15;
static constexpr uint16_t RXRFSL_BIT = 16;
static constexpr uint16_t RXRFTO_BIT = 17;
static constexpr uint16_t LDEERR_BIT = 18;
static constexpr uint16_t RXOVRR_BIT = 20;
static constexpr uint16_t RXPTO_BIT = 21;
static constexpr uint16_t GPIOIRQ_BIT = 22;
static constexpr uint16_t SLP2INIT_BIT = 23;
static constexpr uint16_t RFPLL_LL_BIT = 24;
static constexpr uint16_t CLKPLL_LL_BIT = 25;
static constexpr uint16_t RXSFDTO_BIT = 26;
static constexpr uint16_t HPDWARN_BIT = 27;
static constexpr uint16_t TXBERR_BIT = 28;
static constexpr uint16_t AFFREJ_BIT = 29;
static constexpr uint16_t HSRBP_BIT = 30;
static constexpr uint16_t ICRBP_BIT = 31;
static constexpr uint16_t RXRSCS_BIT = 0;
static constexpr uint16_t RXPREJ_BIT = 1;
static constexpr uint16_t TXPUTE_BIT = 2;
static constexpr uint16_t LEN_SYS_STATUS = 4;
static constexpr uint16_t LEN_SYS_STATUS_SUB = 1;

// system event mask register
// NOTE: uses the bit definitions of SYS_STATUS (below 32)
static constexpr uint16_t SYS_MASK = 0x0E;
static constexpr uint16_t LEN_SYS_MASK = 4;

// system time counter
static constexpr uint16_t SYS_TIME = 0x06;
static constexpr uint16_t LEN_SYS_TIME = 5;

// RX timestamp register
static constexpr uint16_t RX_TIME = 0x15;
static constexpr uint16_t LEN_RX_TIME = 14;
static constexpr uint16_t RX_STAMP_SUB = 0x00;
static constexpr uint16_t FP_AMPL1_SUB = 0x07;
static constexpr uint16_t LEN_RX_STAMP = 5;
static constexpr uint16_t LEN_FP_AMPL1 = 2;

// RX frame quality
static constexpr uint16_t RX_FQUAL = 0x12;
static constexpr uint16_t LEN_RX_FQUAL = 8;
static constexpr uint16_t STD_NOISE_SUB = 0x00;
static constexpr uint16_t FP_AMPL2_SUB = 0x02;
static constexpr uint16_t FP_AMPL3_SUB = 0x04;
static constexpr uint16_t CIR_PWR_SUB = 0x06;
static constexpr uint16_t LEN_STD_NOISE = 2;
static constexpr uint16_t LEN_FP_AMPL2 = 2;
static constexpr uint16_t LEN_FP_AMPL3 = 2;
static constexpr uint16_t LEN_CIR_PWR = 2;

// TX timestamp register
static constexpr uint16_t TX_TIME = 0x17;
static constexpr uint16_t LEN_TX_TIME = 10;
static constexpr uint16_t TX_STAMP_SUB = 0;
static constexpr uint16_t LEN_TX_STAMP = 5;

// timing register (for delayed RX/TX)
static constexpr uint16_t DX_TIME = 0x0A;
static constexpr uint16_t LEN_DX_TIME = 5;

// Receive Frame Wait Timeout Period
static constexpr uint16_t RX_WFTO = 0x0C;
static constexpr uint16_t LEN_RX_WFTO = 2;

// transmit data buffer
static constexpr uint16_t TX_BUFFER = 0x09;
static constexpr uint16_t LEN_TX_BUFFER = 1024;
static constexpr uint16_t LEN_UWB_FRAMES = 127;
static constexpr uint16_t LEN_EXT_UWB_FRAMES = 1023;

// RX frame info
static constexpr uint16_t RX_FINFO = 0x10;
static constexpr uint16_t LEN_RX_FINFO = 4;

// receive data buffer
static constexpr uint16_t RX_BUFFER = 0x11;
static constexpr uint16_t LEN_RX_BUFFER = 1024;

// transmit control
static constexpr uint16_t TX_FCTRL = 0x08;
static constexpr uint16_t LEN_TX_FCTRL = 5;

// channel control
static constexpr uint16_t CHAN_CTRL = 0x1F;
static constexpr uint16_t LEN_CHAN_CTRL = 4;
static constexpr uint16_t DWSFD_BIT = 17;
static constexpr uint16_t TNSSFD_BIT = 20;
static constexpr uint16_t RNSSFD_BIT = 21;

// user-defined SFD
static constexpr uint16_t USR_SFD = 0x21;
static constexpr uint16_t LEN_USR_SFD = 41;
static constexpr uint16_t SFD_LENGTH_SUB = 0x00;
static constexpr uint16_t LEN_SFD_LENGTH = 1;

// OTP control (for LDE micro code loading only)
static constexpr uint16_t OTP_IF = 0x2D;
static constexpr uint16_t OTP_ADDR_SUB = 0x04;
static constexpr uint16_t OTP_CTRL_SUB = 0x06;
static constexpr uint16_t OTP_RDAT_SUB = 0x0A;
static constexpr uint16_t LEN_OTP_ADDR = 2;
static constexpr uint16_t LEN_OTP_CTRL = 2;
static constexpr uint16_t LEN_OTP_RDAT = 4;

// AGC_TUNE1/2/3 (for re-tuning only)
static constexpr uint16_t AGC_TUNE = 0x23;
static constexpr uint16_t AGC_TUNE1_SUB = 0x04;
static constexpr uint16_t AGC_TUNE2_SUB = 0x0C;
static constexpr uint16_t AGC_TUNE3_SUB = 0x12;
static constexpr uint16_t LEN_AGC_TUNE1 = 2;
static constexpr uint16_t LEN_AGC_TUNE2 = 4;
static constexpr uint16_t LEN_AGC_TUNE3 = 2;

// EXT_SYNC (External Synchronization Control)
static constexpr uint16_t EXT_SYNC = 0x24;
static constexpr uint16_t EC_CTRL_SUB = 0x00;
static constexpr uint16_t PLLLDT_BIT = 2;
static constexpr uint16_t EC_RXTC_SUB = 0x04;
static constexpr uint16_t EC_GOLP_SUB = 0x08;
static constexpr uint16_t LEN_EC_CTRL = 4;
static constexpr uint16_t LEN_EC_RXTC = 4;
static constexpr uint16_t LEN_EC_GOLP = 4;

// DRX_TUNE2 (for re-tuning only)
static constexpr uint16_t DRX_TUNE = 0x27;
static constexpr uint16_t DRX_TUNE0b_SUB = 0x02;
static constexpr uint16_t DRX_TUNE1a_SUB = 0x04;
static constexpr uint16_t DRX_TUNE1b_SUB = 0x06;
static constexpr uint16_t DRX_TUNE2_SUB = 0x08;
static constexpr uint16_t DRX_SFDTOC_SUB = 0x20;
static constexpr uint16_t DRX_PRETOC_SUB = 0x24;
static constexpr uint16_t DRX_TUNE4H_SUB = 0x26;
static constexpr uint16_t DRX_CAR_INT_SUB = 0x28;
static constexpr uint16_t RXPACC_NOSAT_SUB = 0x2C;
static constexpr uint16_t LEN_DRX_TUNE0b = 2;
static constexpr uint16_t LEN_DRX_TUNE1a = 2;
static constexpr uint16_t LEN_DRX_TUNE1b = 2;
static constexpr uint16_t LEN_DRX_TUNE2 = 4;
static constexpr uint16_t LEN_DRX_SFDTOC = 2;
static constexpr uint16_t LEN_DRX_PRETOC = 2;
static constexpr uint16_t LEN_DRX_TUNE4H = 2;
static constexpr uint16_t LEN_DRX_CAR_INT = 3;
static constexpr uint16_t LEN_RXPACC_NOSAT = 2;

// LDE_CFG1 (for re-tuning only)
static constexpr uint16_t LDE_IF = 0x2E;
static constexpr uint16_t LDE_CFG1_SUB = 0x0806;
static constexpr uint16_t LDE_RXANTD_SUB = 0x1804;
static constexpr uint16_t LDE_CFG2_SUB = 0x1806;
static constexpr uint16_t LDE_REPC_SUB = 0x2804;
static constexpr uint16_t LEN_LDE_CFG1 = 1;
static constexpr uint16_t LEN_LDE_CFG2 = 2;
static constexpr uint16_t LEN_LDE_REPC = 2;
static constexpr uint16_t LEN_LDE_RXANTD = 2;

// DIG_DIAG (Digital Diagnostics Interface)
static constexpr uint16_t DIG_DIAG = 0x2F;
static constexpr uint16_t EVC_CTRL_SUB = 0x00;
static constexpr uint16_t EVC_STO_SUB = 0x10;
static constexpr uint16_t EVC_PTO_SUB = 0x12;
static constexpr uint16_t EVC_FWTO_SUB = 0x14;
static constexpr uint16_t DIAG_TMC_SUB = 0x24;
static constexpr uint16_t LEN_EVC_CTRL = 4;
static constexpr uint16_t LEN_EVC_STO = 2;
static constexpr uint16_t LEN_EVC_PTO = 2;
static constexpr uint16_t LEN_EVC_FWTO = 2;
static constexpr uint16_t LEN_DIAG_TMC = 2;

// TX_POWER (for re-tuning only)
static constexpr uint16_t TX_POWER = 0x1E;
static constexpr uint16_t LEN_TX_POWER = 4;

// RF_CONF (for re-tuning only)
static constexpr uint16_t RF_CONF = 0x28;
static constexpr uint16_t RF_CONF_SUB = 0x00;
static constexpr uint16_t RF_RXCTRLH_SUB = 0x0B;
static constexpr uint16_t RF_TXCTRL_SUB = 0x0C;
static constexpr uint16_t LEN_RX_CONF_SUB = 4;
static constexpr uint16_t LEN_RF_RXCTRLH = 1;
static constexpr uint16_t LEN_RF_TXCTRL = 4;

// TX_CAL (for re-tuning only)
static constexpr uint16_t TX_CAL = 0x2A;
static constexpr uint16_t TC_PGDELAY_SUB = 0x0B;
static constexpr uint16_t LEN_TC_PGDELAY = 1;
static constexpr uint16_t TC_SARC = 0x00;
static constexpr uint16_t TC_SARL = 0x03;

// FS_CTRL (for re-tuning only)
static constexpr uint16_t FS_CTRL = 0x2B;
static constexpr uint16_t FS_PLLCFG_SUB = 0x07;
static constexpr uint16_t FS_PLLTUNE_SUB = 0x0B;
static constexpr uint16_t FS_XTALT_SUB = 0x0E;
static constexpr uint16_t LEN_FS_PLLCFG = 4;
static constexpr uint16_t LEN_FS_PLLTUNE = 1;
static constexpr uint16_t LEN_FS_XTALT = 1;

// AON
static constexpr uint16_t AON = 0x2C;
static constexpr uint16_t AON_WCFG_SUB = 0x00;
static constexpr uint16_t ONW_RADC_BIT = 0;
static constexpr uint16_t ONW_RX_BIT = 1;
static constexpr uint16_t ONW_LEUI_BIT = 3;
static constexpr uint16_t ONW_LDC_BIT = 6;
static constexpr uint16_t ONW_L64P_BIT = 7;
static constexpr uint16_t ONW_PRES_SLEEP_BIT = 8;
static constexpr uint16_t ONW_LLDE_BIT = 11;
static constexpr uint16_t ONW_LLDO_BIT = 12;
static constexpr uint16_t LEN_AON_WCFG = 2;

static constexpr uint16_t AON_CTRL_SUB = 0x02;
static constexpr uint16_t RESTORE_BIT = 0;
static constexpr uint16_t SAVE_BIT = 1;
static constexpr uint16_t UPL_CFG_BIT = 2;
static constexpr uint16_t LEN_AON_CTRL = 1;

static constexpr uint16_t AON_CFG0_SUB = 0x06;
static constexpr uint16_t SLEEP_EN_BIT = 0;
static constexpr uint16_t WAKE_PIN_BIT = 1;
static constexpr uint16_t WAKE_SPI_BIT = 2;
static constexpr uint16_t WAKE_CNT_BIT = 3;
static constexpr uint16_t LPDIV_EN_BIT = 4;
static constexpr uint16_t LEN_AON_CFG0 = 4;

static constexpr uint16_t AON_CFG1_SUB = 0x0A;
static constexpr uint16_t SLEEP_CEN_BIT = 0;
static constexpr uint16_t SMXX_BIT = 1;
static constexpr uint16_t LPOSC_CAL_BIT = 2;
static constexpr uint16_t LEN_AON_CFG1 = 2;

// PMSC
static constexpr uint16_t PMSC = 0x36;
static constexpr uint16_t PMSC_CTRL0_SUB = 0x00;
static constexpr uint16_t GPDCE_BIT = 18;
static constexpr uint16_t KHZCLKEN_BIT = 23;
static constexpr uint16_t PMSC_SOFTRESET_SUB = 0x03;
static constexpr uint16_t PMSC_CTRL1_SUB = 0x04;
static constexpr uint16_t ATXSLP_BIT = 11;
static constexpr uint16_t ARXSLP_BIT = 12;
static constexpr uint16_t PMSC_LEDC_SUB = 0x28;
static constexpr uint16_t BLNKEN = 8;
static constexpr uint16_t LEN_PMSC_CTRL0 = 4;
static constexpr uint16_t LEN_PMSC_SOFTRESET = 1;
static constexpr uint16_t LEN_PMSC_CTRL1 = 4;
static constexpr uint16_t LEN_PMSC_LEDC = 4;

// TX_ANTD Antenna delays
static constexpr uint16_t TX_ANTD = 0x18;
static constexpr uint16_t LEN_TX_ANTD = 2;

// Acknowledgement time and response time
static constexpr uint16_t ACK_RESP_T = 0x1A;
static constexpr uint16_t ACK_RESP_T_W4R_TIME_SUB = 0x00;
static constexpr uint16_t LEN_ACK_RESP_T_W4R_TIME_SUB = 3;
static constexpr uint16_t LEN_ACK_RESP_T = 4;

// GPIO
static constexpr uint16_t GPIO_CTRL = 0x26;
static constexpr uint16_t GPIO_MODE_SUB = 0x00;
static constexpr uint16_t LEN_GPIO_MODE = 4;

struct DeviceConfig{
    bool extendedFrameLength;
    bool receiverAutoReenable;
    bool smartPower;
    bool frameCheck;
    bool nlos;
    SFDMode sfd;
    Channel channel;
    DataRate dataRate;
    PulseFrequency pulseFreq;
    PreambleLength preambleLen;
    PreambleCode preaCode;
} ;

struct InterruptConfig {
    bool interruptOnSent;
    bool interruptOnReceived;
    bool interruptOnReceiveFailed;
    bool interruptOnReceiveTimeout;
    bool interruptOnReceiveTimestampAvailable;
    bool interruptOnAutomaticAcknowledgeTrigger;
} ;

struct FrameFilteringConfig{
    bool behaveAsCoordinator;
    bool allowBeacon;
    bool allowData;
    bool allowAcknowledgement;
    bool allowMacCommand;
    bool allowAllReserved;
    bool allowReservedFour;
    bool allowReservedFive;
};

struct SleepConfig{
    bool onWakeUpRunADC;
    bool onWakeUpReceive;
    bool onWakeUpLoadEUI;
    bool onWakeUpLoadL64Param;
    bool preserveSleep;
    bool enableSLP;
    bool enableWakePIN;
    bool enableWakeSPI;
} ;
}
}