#pragma once

#include <cstdint>
#include "core/io/regs.hpp"

//参考资料：https://github.com/Emandhal/MCP251XFD/blob/master/MCP251XFD.h


namespace ymd::drivers{
struct MCP251XFD_Prelude{

//! MCP251XFD registers list
enum class [[nodiscard]] RegAddr:uint16_t{
    // CAN-FD Controller Module Registers
    CiCON              = 0x000,                     //!< CAN Control Register
    CiNBTCFG           = 0x004,                     //!< Nominal BitTime Configuration Register
    CiNBTCFG_SJW       = CiNBTCFG+0,   //!< Nominal BitTime Configuration Register - Synchronization Jump Width
    CiNBTCFG_TSEG2     = CiNBTCFG+1,   //!< Nominal BitTime Configuration Register - Time Segment 2 (Phase Segment 2)
    CiNBTCFG_TSEG1     = CiNBTCFG+2,   //!< Nominal BitTime Configuration Register - Time Segment 1 (Propagation Segment + Phase Segment 1)
    CiNBTCFG_BRP       = CiNBTCFG+3,   //!< Nominal BitTime Configuration Register - Baud Rate Prescaler
    CiDBTCFG           = 0x008,                     //!< Data BitTime Configuration Register
    CiDBTCFG_SJW       = CiDBTCFG+0,   //!< Data BitTime Configuration Register - Synchronization Jump Width
    CiDBTCFG_TSEG2     = CiDBTCFG+1,   //!< Data BitTime Configuration Register - Time Segment 2 (Phase Segment 2)
    CiDBTCFG_TSEG1     = CiDBTCFG+2,   //!< Data BitTime Configuration Register - Time Segment 1 (Propagation Segment + Phase Segment 1)
    CiDBTCFG_BRP       = CiDBTCFG+3,   //!< Data BitTime Configuration Register - Baud Rate Prescaler
    CiTDC              = 0x00C,                     //!< Transmitter Delay Compensation Register
    CiTDC_TDCV         = CiTDC+0,      //!< Transmitter Delay Compensation Register - Transmitter Delay Compensation Value
    CiTDC_TDCO         = CiTDC+1,      //!< Transmitter Delay Compensation Register - Transmitter Delay Compensation Offset
    CiTDC_TDCMOD       = CiTDC+2,      //!< Transmitter Delay Compensation Register - Transmitter Delay Compensation Mode
    CiTDC_CONFIG       = CiTDC+3,      //!< Transmitter Delay Compensation Register - CAN-FD configuration
    CiTBC              = 0x010,                     //!< Time Base Counter Register
    CiTSCON            = 0x014,                     //!< Time Stamp Control Register
    CiTSCON_TBCPRE     = CiTSCON+0,    //!< Time Stamp Control Register - Time Base Counter Prescaler
    CiTSCON_CONFIG     = CiTSCON+2,    //!< Time Stamp Control Register - Time Base Counter Configuration
    CiVEC              = 0x018,                     //!< Interrupt Code Register
    CiVEC_ICODE        = CiVEC+0,      //!< Interrupt Code Register - Interrupt Flag Code
    CiVEC_FILHIT       = CiVEC+1,      //!< Interrupt Code Register - Filter Hit Number
    CiVEC_TXCODE       = CiVEC+2,      //!< Interrupt Code Register - Transmit Interrupt Flag Code
    CiVEC_RXCODE       = CiVEC+3,      //!< Interrupt Code Register - Receive Interrupt Flag Code
    CiINT              = 0x01C,                     //!< Interrupt Register
    CiINT_FLAG         = CiINT+0,      //!< Interrupt Register - Interrupts flags
    CiINT_CONFIG       = CiINT+2,      //!< Interrupt Register - Interrupts enable
    CiRXIF             = 0x020,                     //!< Receive Interrupt Status Register
    CiTXIF             = 0x024,                     //!< Receive Overflow Interrupt Status Register
    CiRXOVIF           = 0x028,                     //!< Transmit Interrupt Status Register
    CiTXATIF           = 0x02C,                     //!< Transmit Attempt Interrupt Status Register
    CiTXREQ            = 0x030,                     //!< Transmit Request Register
    CiTREC             = 0x034,                     //!< Transmit/Receive Error Count Register
    CiTREC_REC         = CiTREC+0,     //!< Transmit/Receive Error Count Register - Receive Error Counter
    CiTREC_TEC         = CiTREC+1,     //!< Transmit/Receive Error Count Register - Transmit Error Counter
    CiTREC_STATUS      = CiTREC+2,     //!< Transmit/Receive Error Count Register - Error Status
    CiBDIAG0           = 0x038,                     //!< Bus Diagnostic Register 0
    CiBDIAG0_NRERRCNT  = CiBDIAG0+0,   //!< Bus Diagnostic Register 0 - Nominal Bit Rate Receive Error Counter
    CiBDIAG0_NTERRCNT  = CiBDIAG0+1,   //!< Bus Diagnostic Register 0 - Nominal Bit Rate Transmit Error Counter
    CiBDIAG0_DRERRCNT  = CiBDIAG0+2,   //!< Bus Diagnostic Register 0 - Data Bit Rate Receive Error Counter
    CiBDIAG0_DTERRCNT  = CiBDIAG0+3,   //!< Bus Diagnostic Register 0 - Data Bit Rate Transmit Error Counter
    CiBDIAG1           = 0x03C,                     //!< Bus Diagnostic Register 1
    CiBDIAG1_EFMSGCNT  = CiBDIAG1+0,   //!< Bus Diagnostic Register 1 - Error Free Message Counter
    CiTEFCON           = 0x040,                     //!< Transmit Event FIFO Control Register
    CiTEFCON_CONFIG    = CiTEFCON+0,   //!< Transmit Event FIFO Control Register - Interrupt configuration
    CiTEFCON_CONTROL   = CiTEFCON+1,   //!< Transmit Event FIFO Control Register - TEF Control
    CiTEFSTA           = 0x044,                     //!< Transmit Event FIFO Status Register
    CiTEFSTA_FLAGS     = CiTEFSTA+0,   //!< Transmit Event FIFO Status Register - Flags
    CiTEFUA            = 0x048,                     //!< Transmit Event FIFO User Address Register
    Reserved4C         = 0x04C,                     //!< Reserved Register
    // Transmit Queue Registers
    CiTXQCON           = 0x050,                     //!< Transmit Queue Control Register
    CiTXQCON_CONFIG    = CiTXQCON+0,   //!< Transmit Queue Control Register - Interrupt configuration
    CiTXQCON_CONTROL   = CiTXQCON+1,   //!< Transmit Queue Control Register - TXQ Control
    CiTXQSTA           = 0x054,                     //!< Transmit Queue Status Register
    CiTXQSTA_FLAGS     = CiTXQSTA+0,   //!< Transmit Queue Status Register - Flags
    CiTXQSTA_TXQCI     = CiTXQSTA+1,   //!< Transmit Queue Status Register - Transmit Queue Message Index
    CiTXQUA            = 0x058,                     //!< Transmit Queue User Address Register
    // FIFOs Registers
    CiFIFOCONm         = 0x05C,                     //!< FIFO Control Register m, (m = 1 to 31)
    CiFIFOCONm_CONFIG  = CiFIFOCONm+0, //!< FIFO Control Register m, (m = 1 to 31) - Interrupt configuration
    CiFIFOCONm_CONTROL = CiFIFOCONm+1, //!< FIFO Control Register m, (m = 1 to 31) - FIFO Control
    CiFIFOSTAm         = 0x060,                     //!< FIFO Status Register m, (m = 1 to 31)
    CiFIFOSTAm_FLAGS   = CiFIFOSTAm+0, //!< FIFO Status Register m, (m = 1 to 31) - Flags
    CiFIFOSTAm_FIFOCI  = CiFIFOSTAm+1, //!< FIFO Status Register m, (m = 1 to 31) - FIFO Message Index
    CiFIFOUAm          = 0x064,                     //!< FIFO User Address Register m, (m = 1 to 31)
    CiFIFOCON1         = 0x05C,                     //!< FIFO Control Register 1
    CiFIFOSTA1         = 0x060,                     //!< FIFO Status Register 1
    CiFIFOUA1          = 0x064,                     //!< FIFO User Address Register 1
    CiFIFOCON2         = 0x068,                     //!< FIFO Control Register 2
    CiFIFOSTA2         = 0x06C,                     //!< FIFO Status Register 2
    CiFIFOUA2          = 0x070,                     //!< FIFO User Address Register 2
    CiFIFOCON3         = 0x074,                     //!< FIFO Control Register 3
    CiFIFOSTA3         = 0x078,                     //!< FIFO Status Register 3
    CiFIFOUA3          = 0x07C,                     //!< FIFO User Address Register 3
    CiFIFOCON4         = 0x080,                     //!< FIFO Control Register 4
    CiFIFOSTA4         = 0x084,                     //!< FIFO Status Register 4
    CiFIFOUA4          = 0x088,                     //!< FIFO User Address Register 4
    CiFIFOCON5         = 0x08C,                     //!< FIFO Control Register 5
    CiFIFOSTA5         = 0x090,                     //!< FIFO Status Register 5
    CiFIFOUA5          = 0x094,                     //!< FIFO User Address Register 5
    CiFIFOCON6         = 0x098,                     //!< FIFO Control Register 6
    CiFIFOSTA6         = 0x09C,                     //!< FIFO Status Register 6
    CiFIFOUA6          = 0x0A0,                     //!< FIFO User Address Register 6
    CiFIFOCON7         = 0x0A4,                     //!< FIFO Control Register 7
    CiFIFOSTA7         = 0x0A8,                     //!< FIFO Status Register 7
    CiFIFOUA7          = 0x0AC,                     //!< FIFO User Address Register 7
    CiFIFOCON8         = 0x0B0,                     //!< FIFO Control Register 8
    CiFIFOSTA8         = 0x0B4,                     //!< FIFO Status Register 8
    CiFIFOUA8          = 0x0B8,                     //!< FIFO User Address Register 8
    CiFIFOCON9         = 0x0BC,                     //!< FIFO Control Register 9
    CiFIFOSTA9         = 0x0C0,                     //!< FIFO Status Register 9
    CiFIFOUA9          = 0x0C4,                     //!< FIFO User Address Register 9
    CiFIFOCON10        = 0x0C8,                     //!< FIFO Control Register 10
    CiFIFOSTA10        = 0x0CC,                     //!< FIFO Status Register 10
    CiFIFOUA10         = 0x0D0,                     //!< FIFO User Address Register 10
    CiFIFOCON11        = 0x0D4,                     //!< FIFO Control Register 11
    CiFIFOSTA11        = 0x0D8,                     //!< FIFO Status Register 11
    CiFIFOUA11         = 0x0DC,                     //!< FIFO User Address Register 11
    CiFIFOCON12        = 0x0E0,                     //!< FIFO Control Register 12
    CiFIFOSTA12        = 0x0E4,                     //!< FIFO Status Register 12
    CiFIFOUA12         = 0x0E8,                     //!< FIFO User Address Register 12
    CiFIFOCON13        = 0x0EC,                     //!< FIFO Control Register 13
    CiFIFOSTA13        = 0x0F0,                     //!< FIFO Status Register 13
    CiFIFOUA13         = 0x0F4,                     //!< FIFO User Address Register 13
    CiFIFOCON14        = 0x0F8,                     //!< FIFO Control Register 14
    CiFIFOSTA14        = 0x0FC,                     //!< FIFO Status Register 14
    CiFIFOUA14         = 0x100,                     //!< FIFO User Address Register 14
    CiFIFOCON15        = 0x104,                     //!< FIFO Control Register 15
    CiFIFOSTA15        = 0x108,                     //!< FIFO Status Register 15
    CiFIFOUA15         = 0x10C,                     //!< FIFO User Address Register 15
    CiFIFOCON16        = 0x110,                     //!< FIFO Control Register 16
    CiFIFOSTA16        = 0x114,                     //!< FIFO Status Register 16
    CiFIFOUA16         = 0x118,                     //!< FIFO User Address Register 16
    CiFIFOCON17        = 0x11C,                     //!< FIFO Control Register 17
    CiFIFOSTA17        = 0x120,                     //!< FIFO Status Register 17
    CiFIFOUA17         = 0x124,                     //!< FIFO User Address Register 17
    CiFIFOCON18        = 0x128,                     //!< FIFO Control Register 18
    CiFIFOSTA18        = 0x12C,                     //!< FIFO Status Register 18
    CiFIFOUA18         = 0x130,                     //!< FIFO User Address Register 18
    CiFIFOCON19        = 0x134,                     //!< FIFO Control Register 19
    CiFIFOSTA19        = 0x138,                     //!< FIFO Status Register 19
    CiFIFOUA19         = 0x13C,                     //!< FIFO User Address Register 19
    CiFIFOCON20        = 0x140,                     //!< FIFO Control Register 20
    CiFIFOSTA20        = 0x144,                     //!< FIFO Status Register 20
    CiFIFOUA20         = 0x148,                     //!< FIFO User Address Register 20
    CiFIFOCON21        = 0x14C,                     //!< FIFO Control Register 21
    CiFIFOSTA21        = 0x150,                     //!< FIFO Status Register 21
    CiFIFOUA21         = 0x154,                     //!< FIFO User Address Register 21
    CiFIFOCON22        = 0x158,                     //!< FIFO Control Register 22
    CiFIFOSTA22        = 0x15C,                     //!< FIFO Status Register 22
    CiFIFOUA22         = 0x160,                     //!< FIFO User Address Register 22
    CiFIFOCON23        = 0x164,                     //!< FIFO Control Register 23
    CiFIFOSTA23        = 0x168,                     //!< FIFO Status Register 23
    CiFIFOUA23         = 0x16C,                     //!< FIFO User Address Register 23
    CiFIFOCON24        = 0x170,                     //!< FIFO Control Register 24
    CiFIFOSTA24        = 0x174,                     //!< FIFO Status Register 24
    CiFIFOUA24         = 0x178,                     //!< FIFO User Address Register 24
    CiFIFOCON25        = 0x17C,                     //!< FIFO Control Register 25
    CiFIFOSTA25        = 0x180,                     //!< FIFO Status Register 25
    CiFIFOUA25         = 0x184,                     //!< FIFO User Address Register 25
    CiFIFOCON26        = 0x188,                     //!< FIFO Control Register 26
    CiFIFOSTA26        = 0x18C,                     //!< FIFO Status Register 26
    CiFIFOUA26         = 0x190,                     //!< FIFO User Address Register 26
    CiFIFOCON27        = 0x194,                     //!< FIFO Control Register 27
    CiFIFOSTA27        = 0x198,                     //!< FIFO Status Register 27
    CiFIFOUA27         = 0x19C,                     //!< FIFO User Address Register 27
    CiFIFOCON28        = 0x1A0,                     //!< FIFO Control Register 28
    CiFIFOSTA28        = 0x1A4,                     //!< FIFO Status Register 28
    CiFIFOUA28         = 0x1A8,                     //!< FIFO User Address Register 28
    CiFIFOCON29        = 0x1AC,                     //!< FIFO Control Register 29
    CiFIFOSTA29        = 0x1B0,                     //!< FIFO Status Register 29
    CiFIFOUA29         = 0x1B4,                     //!< FIFO User Address Register 29
    CiFIFOCON30        = 0x1B8,                     //!< FIFO Control Register 30
    CiFIFOSTA30        = 0x1BC,                     //!< FIFO Status Register 30
    CiFIFOUA30         = 0x1C0,                     //!< FIFO User Address Register 30
    CiFIFOCON31        = 0x1C4,                     //!< FIFO Control Register 31
    CiFIFOSTA31        = 0x1C8,                     //!< FIFO Status Register 31
    CiFIFOUA31         = 0x1CC,                     //!< FIFO User Address Register 31
    // Filters Registers
    CiFLTCONm          = 0x1D0,                     //!< Filter Control Register m, (m = 0 to 31)
    CiFLTCON0          = 0x1D0,                     //!< Filter  0 to  3 Control Register
    CiFLTCON0_FILTER0  = CiFLTCON0+0,  //!< Filter  0 to  3 Control Register - Filter 0
    CiFLTCON0_FILTER1  = CiFLTCON0+1,  //!< Filter  0 to  3 Control Register - Filter 1
    CiFLTCON0_FILTER2  = CiFLTCON0+2,  //!< Filter  0 to  3 Control Register - Filter 2
    CiFLTCON0_FILTER3  = CiFLTCON0+3,  //!< Filter  0 to  3 Control Register - Filter 3
    CiFLTCON1          = 0x1D4,                     //!< Filter  4 to  7 Control Register
    CiFLTCON1_FILTER4  = CiFLTCON1+0,  //!< Filter  4 to  7 Control Register - Filter 4
    CiFLTCON1_FILTER5  = CiFLTCON1+1,  //!< Filter  4 to  7 Control Register - Filter 5
    CiFLTCON1_FILTER6  = CiFLTCON1+2,  //!< Filter  4 to  7 Control Register - Filter 6
    CiFLTCON1_FILTER7  = CiFLTCON1+3,  //!< Filter  4 to  7 Control Register - Filter 7
    CiFLTCON2          = 0x1D8,                     //!< Filter  8 to 11 Control Register
    CiFLTCON2_FILTER8  = CiFLTCON2+0,  //!< Filter  8 to 11 Control Register - Filter 8
    CiFLTCON2_FILTER9  = CiFLTCON2+1,  //!< Filter  8 to 11 Control Register - Filter 9
    CiFLTCON2_FILTER10 = CiFLTCON2+2,  //!< Filter  8 to 11 Control Register - Filter 10
    CiFLTCON2_FILTER11 = CiFLTCON2+3,  //!< Filter  8 to 11 Control Register - Filter 11
    CiFLTCON3          = 0x1DC,                     //!< Filter 12 to 15 Control Register
    CiFLTCON3_FILTER12 = CiFLTCON3+0,  //!< Filter 12 to 15 Control Register - Filter 12
    CiFLTCON3_FILTER13 = CiFLTCON3+1,  //!< Filter 12 to 15 Control Register - Filter 13
    CiFLTCON3_FILTER14 = CiFLTCON3+2,  //!< Filter 12 to 15 Control Register - Filter 14
    CiFLTCON3_FILTER15 = CiFLTCON3+3,  //!< Filter 12 to 15 Control Register - Filter 15
    CiFLTCON4          = 0x1E0,                     //!< Filter 16 to 19 Control Register
    CiFLTCON4_FILTER16 = CiFLTCON4+0,  //!< Filter 16 to 19 Control Register - Filter 16
    CiFLTCON4_FILTER17 = CiFLTCON4+1,  //!< Filter 16 to 19 Control Register - Filter 17
    CiFLTCON4_FILTER18 = CiFLTCON4+2,  //!< Filter 16 to 19 Control Register - Filter 18
    CiFLTCON4_FILTER19 = CiFLTCON4+3,  //!< Filter 16 to 19 Control Register - Filter 19
    CiFLTCON5          = 0x1E4,                     //!< Filter 20 to 23 Control Register
    CiFLTCON5_FILTER20 = CiFLTCON5+0,  //!< Filter 20 to 23 Control Register - Filter 20
    CiFLTCON5_FILTER21 = CiFLTCON5+1,  //!< Filter 20 to 23 Control Register - Filter 21
    CiFLTCON5_FILTER22 = CiFLTCON5+2,  //!< Filter 20 to 23 Control Register - Filter 22
    CiFLTCON5_FILTER23 = CiFLTCON5+3,  //!< Filter 20 to 23 Control Register - Filter 23
    CiFLTCON6          = 0x1E8,                     //!< Filter 24 to 27 Control Register
    CiFLTCON6_FILTER24 = CiFLTCON6+0,  //!< Filter 24 to 27 Control Register - Filter 24
    CiFLTCON6_FILTER25 = CiFLTCON6+1,  //!< Filter 24 to 27 Control Register - Filter 25
    CiFLTCON6_FILTER26 = CiFLTCON6+2,  //!< Filter 24 to 27 Control Register - Filter 26
    CiFLTCON6_FILTER27 = CiFLTCON6+3,  //!< Filter 24 to 27 Control Register - Filter 27
    CiFLTCON7          = 0x1EC,                     //!< Filter 28 to 31 Control Register
    CiFLTCON7_FILTER28 = CiFLTCON7+0,  //!< Filter 28 to 31 Control Register - Filter 28
    CiFLTCON7_FILTER29 = CiFLTCON7+1,  //!< Filter 28 to 31 Control Register - Filter 29
    CiFLTCON7_FILTER30 = CiFLTCON7+2,  //!< Filter 28 to 31 Control Register - Filter 30
    CiFLTCON7_FILTER31 = CiFLTCON7+3,  //!< Filter 28 to 31 Control Register - Filter 31
    CiFLTOBJm          = 0x1F0,                     //!< Filter Object Register m, (m = 0 to 31)
    CiMASKm            = 0x1F4,                     //!< Filter Mask Register m, (m = 0 to 31)
    CiFLTOBJ0          = 0x1F0,                     //!< Filter Object Register 0
    CiMASK0            = 0x1F4,                     //!< Filter Mask Register 0
    CiFLTOBJ1          = 0x1F8,                     //!< Filter Object Register 1
    CiMASK1            = 0x1FC,                     //!< Filter Mask Register 1
    CiFLTOBJ2          = 0x200,                     //!< Filter Object Register 2
    CiMASK2            = 0x204,                     //!< Filter Mask Register 2
    CiFLTOBJ3          = 0x208,                     //!< Filter Object Register 3
    CiMASK3            = 0x20C,                     //!< Filter Mask Register 3
    CiFLTOBJ4          = 0x210,                     //!< Filter Object Register 4
    CiMASK4            = 0x214,                     //!< Filter Mask Register 4
    CiFLTOBJ5          = 0x218,                     //!< Filter Object Register 5
    CiMASK5            = 0x21C,                     //!< Filter Mask Register 5
    CiFLTOBJ6          = 0x220,                     //!< Filter Object Register 6
    CiMASK6            = 0x224,                     //!< Filter Mask Register 6
    CiFLTOBJ7          = 0x228,                     //!< Filter Object Register 7
    CiMASK7            = 0x22C,                     //!< Filter Mask Register 7
    CiFLTOBJ8          = 0x230,                     //!< Filter Object Register 8
    CiMASK8            = 0x234,                     //!< Filter Mask Register 8
    CiFLTOBJ9          = 0x238,                     //!< Filter Object Register 9
    CiMASK9            = 0x23C,                     //!< Filter Mask Register 9
    CiFLTOBJ10         = 0x240,                     //!< Filter Object Register 10
    CiMASK10           = 0x244,                     //!< Filter Mask Register 10
    CiFLTOBJ11         = 0x248,                     //!< Filter Object Register 11
    CiMASK11           = 0x24C,                     //!< Filter Mask Register 11
    CiFLTOBJ12         = 0x250,                     //!< Filter Object Register 12
    CiMASK12           = 0x254,                     //!< Filter Mask Register 12
    CiFLTOBJ13         = 0x258,                     //!< Filter Object Register 13
    CiMASK13           = 0x25C,                     //!< Filter Mask Register 13
    CiFLTOBJ14         = 0x260,                     //!< Filter Object Register 14
    CiMASK14           = 0x264,                     //!< Filter Mask Register 14
    CiFLTOBJ15         = 0x268,                     //!< Filter Object Register 15
    CiMASK15           = 0x26C,                     //!< Filter Mask Register 15
    CiFLTOBJ16         = 0x270,                     //!< Filter Object Register 16
    CiMASK16           = 0x274,                     //!< Filter Mask Register 16
    CiFLTOBJ17         = 0x278,                     //!< Filter Object Register 17
    CiMASK17           = 0x27C,                     //!< Filter Mask Register 17
    CiFLTOBJ18         = 0x280,                     //!< Filter Object Register 18
    CiMASK18           = 0x284,                     //!< Filter Mask Register 18
    CiFLTOBJ19         = 0x288,                     //!< Filter Object Register 19
    CiMASK19           = 0x28C,                     //!< Filter Mask Register 19
    CiFLTOBJ20         = 0x290,                     //!< Filter Object Register 20
    CiMASK20           = 0x294,                     //!< Filter Mask Register 20
    CiFLTOBJ21         = 0x298,                     //!< Filter Object Register 21
    CiMASK21           = 0x29C,                     //!< Filter Mask Register 21
    CiFLTOBJ22         = 0x2A0,                     //!< Filter Object Register 22
    CiMASK22           = 0x2A4,                     //!< Filter Mask Register 22
    CiFLTOBJ23         = 0x2A8,                     //!< Filter Object Register 23
    CiMASK23           = 0x2AC,                     //!< Filter Mask Register 23
    CiFLTOBJ24         = 0x2B0,                     //!< Filter Object Register 24
    CiMASK24           = 0x2B4,                     //!< Filter Mask Register 24
    CiFLTOBJ25         = 0x2B8,                     //!< Filter Object Register 25
    CiMASK25           = 0x2BC,                     //!< Filter Mask Register 25
    CiFLTOBJ26         = 0x2C0,                     //!< Filter Object Register 26
    CiMASK26           = 0x2C4,                     //!< Filter Mask Register 26
    CiFLTOBJ27         = 0x2C8,                     //!< Filter Object Register 27
    CiMASK27           = 0x2CC,                     //!< Filter Mask Register 27
    CiFLTOBJ28         = 0x2D0,                     //!< Filter Object Register 28
    CiMASK28           = 0x2D4,                     //!< Filter Mask Register 28
    CiFLTOBJ29         = 0x2D8,                     //!< Filter Object Register 29
    CiMASK29           = 0x2DC,                     //!< Filter Mask Register 29
    CiFLTOBJ30         = 0x2E0,                     //!< Filter Object Register 30
    CiMASK30           = 0x2E4,                     //!< Filter Mask Register 30
    CiFLTOBJ31         = 0x2E8,                     //!< Filter Object Register 31
    CiMASK31           = 0x2EC,                     //!< Filter Mask Register 31

    // MCP251XFD Specific Registers
    OSC                = 0xE00,                     //!< Oscillator Control Register
    OSC_CONFIG         = OSC+0,        //!< Oscillator Control Register - Configuration register
    OSC_CHECK          = OSC+1,        //!< Oscillator Control Register - Check frequency configuration register
    #if defined(_MSC_VER) || (defined(__cplusplus) && (__cplusplus >= 201103L/*C++11*/)) || (!defined(__cplusplus))
    IOCON              = 0xE04,                     //!< Input/Output Control Register
                                                                //!< @deprecated Use IOCON_x subregisters with MCP251XFD_ReadSFR8() and MCP251XFD_WriteSFR8(). Follows datasheets errata for: Writing multiple bytes to the IOCON register using one SPI WRITE instruction may overwrite LAT0 and LAT1
    #endif
    IOCON_DIRECTION    = 0xE04+0,                   //!< Input/Output Control Register - Pins direction
    IOCON_OUTLEVEL     = 0xE04+1,                   //!< Input/Output Control Register - Pin output level
    IOCON_INLEVEL      = 0xE04+2,                   //!< Input/Output Control Register - Pin input level
    IOCON_PINMODE      = 0xE04+3,                   //!< Input/Output Control Register - Pin mode
    CRC                = 0xE08,                     //!< CRC Register
    CRC_CRC            = CRC+0,        //!< CRC Register - Last CRC mismatch
    CRC_FLAGS          = CRC+2,        //!< CRC Register - Status flags
    CRC_CONFIG         = CRC+3,        //!< CRC Register - Interrupts enable
    ECCCON             = 0xE0C,                     //!< ECC Control Register
    ECCCON_ENABLE      = ECCCON+0,     //!< ECC Control Register - Interrupt and ECC enable
    ECCCON_PARITY      = ECCCON+1,     //!< ECC Control Register - Fixed parity value
    ECCSTAT            = 0xE10,                     //!< ECC Status Register
    ECCSTAT_FLAGS      = ECCSTAT+0,    //!< ECC Status Register - Status flags
    ECCSTAT_ERRADDR    = ECCSTAT+2,    //!< ECC Status Register - ECC error address
    DEVID              = 0xE14,                     //!< Device ID Register
};

struct [[nodiscard]] ChecksumBuilder final{
    static constexpr uint16_t TABLE[] = {
        0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
        0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
        0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
        0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
        0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
        0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
        0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
        0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
        0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
        0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
        0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
        0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
        0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
        0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
        0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
        0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
        0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
    };

    uint16_t val;

    static constexpr ChecksumBuilder from_default(){
        return ChecksumBuilder{.val = 0xffff};
    }

    constexpr ChecksumBuilder push_byte(const uint8_t byte) const{
        ChecksumBuilder self = *this;

        self.val = static_cast<uint16_t>(
            (self.val << 8) 
            ^ TABLE[static_cast<size_t>(self.val >> 8) ^ static_cast<size_t>(byte)]
        );

        return self;
    }

    constexpr ChecksumBuilder push_bytes(const std::span<const uint8_t> bytes) const {
        ChecksumBuilder self = *this;

        for(size_t i = 0; i < bytes.size(); i++){
            self = self.push_byte(bytes[i]);
        }
    }

    constexpr uint16_t finalize() const {
        return val;
    }
};

};
}