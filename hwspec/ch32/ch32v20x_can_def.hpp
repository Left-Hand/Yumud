#pragma once

#include <cstdint>

namespace hwspec::CH32V20x{
    namespace CAN_Regs{
        struct CTLR_Reg{
            uint32_t INRQ:1;
            uint32_t SLEEP:1;
            uint32_t TXFP:1;
            uint32_t RFLM:1;

            uint32_t NART:1;
            uint32_t AWUM:1;
            uint32_t ABOM:1;
            uint32_t TTCM:1;

            uint32_t :7;
            uint32_t RST:1;

            uint32_t DBF:1;
            uint32_t :15;
        };

        struct STATR_Reg{
            uint32_t INAK:1;
            uint32_t SLAK:1;
            uint32_t ERRI:1;
            uint32_t WKUI:1;

            uint32_t SLAKI:1;
            uint32_t :3;

            uint32_t TXM:1;
            uint32_t RXM:1;
            uint32_t SAMP:1;
            uint32_t RX:1;

            uint32_t :20;

        };

        struct TSTATR_Reg{
            uint32_t RQCP0:1;
            uint32_t TXOK0:1;
            uint32_t ALST0:1;
            uint32_t TERR0:1;

            uint32_t :3;
            uint32_t ABRQ0:1;


            uint32_t RQCP1:1;
            uint32_t TXOK1:1;
            uint32_t ALST1:1;
            uint32_t TERR1:1;

            uint32_t :3;
            uint32_t ABRQ1:1;


            uint32_t RQCP2:1;
            uint32_t TXOK2:1;
            uint32_t ALST2:1;
            uint32_t TERR2:1;

            uint32_t :3;
            uint32_t ABRQ2:1;


            uint32_t CODE:2;
            uint32_t TME0:1;
            uint32_t TME1:1;

            uint32_t TME2:1;
            uint32_t LOW0:1;
            uint32_t LOW1:1;
            uint32_t LOW2:1;
        };

        struct RFIFO_Reg{
            uint32_t FMP0:2;
            uint32_t :1;
            uint32_t FULL:1;

            uint32_t FOVR:1;
            uint32_t :26;
        };

        struct INTEN_Reg{
            uint32_t TMEIE:1;
            uint32_t FMPIE0:1;
            uint32_t FFIE0:1;
            uint32_t FOVIE0:1;

            uint32_t FMPIE1:1;
            uint32_t FFIE1:1;
            uint32_t FOVIE1:1;
            uint32_t :1;

            uint32_t EWGIE:1;
            uint32_t EPVIE:1;
            uint32_t BOFIE:1;
            uint32_t LECIE:1;

            uint32_t :3;
            uint32_t ERRIE:1;

            uint32_t WKUIE:1;
            uint32_t SLKIE:1;

            uint32_t :14;
        };


        struct ERRSR_Reg{
            // 错误警告标志位。
            // 当收发错误计数器达到警告阈值时，即大于
            // 等于 96 时，硬件置 1。
            uint32_t EWGF:1;

            uint32_t EPVF:1;
            uint32_t BOFF:1;
            uint32_t :1;


            // 上次错误代号。
            // 检测到 CAN 总线上发送错误时，控制器根据
            // 出错情况设置，当正确收发报文时，置 000b。
            // 000：无错误；
            // 001：位填充错误；
            // 010：FORM 格式错误；
            // 011：ACK 确认错误；
            // 100：隐性位错误；
            // 101：显性位错误；
            // 110：CRC 错误；
            // 111：软件设置。
            // 通常应用软件读取到错误时，把代号设置为
            // 111b，可以检测到代号更新。
            uint32_t LEC:3;
            uint32_t :9;

            // 发送错误计数器。
            // 当 CAN 发送出错时，根据出错条件，该计数
            // 器加 1 或 8；发送成功后，该计数器减 1 或
            // 设为 120(错误计数值大于 127)。计数器值超
            // 过 127 时，CAN 进入错误被动状态。
            uint32_t TEC:8;

            // 接收错误计数器。
            // 当 CAN 接收出错时，根据出错条件，该计数
            // 器加 1 或 8；接收成功后，该计数器减 1 或
            // 设为 120(错误计数值大于 127)。计数器值超
            // 过 127 时，CAN 进入错误被动状态。
            uint32_t REC:8;

        };


        struct BTIMR_Reg{
            uint32_t BRP:10;
            uint32_t :6;

            uint32_t TS1:4;
            uint32_t TS2:3;
            uint32_t :1;

            uint32_t SJW:2;
            uint32_t :4;
            uint32_t LBKM:1;
            uint32_t SILM:1;
        };

        struct TTCTLR_Reg{
            uint32_t TIMCMV:16;
            uint32_t TIMRST:1;
            uint32_t MODE:1;
            uint32_t :14;
        };

        struct TTCNT_Reg{
            uint32_t TIMCNT:16;
            uint32_t :16;
        };

        struct TXMIR_Reg{
            uint32_t TXRQ:1;
            uint32_t RTR:1;
            uint32_t IDE:1;
            union{
                uint32_t EXID:29;
                struct{
                    uint32_t :18;
                    uint32_t STID:11;
                };
            };
            
        };

        struct TXMDTR_Reg{
            uint32_t DLC:4;
            uint32_t :4;

            uint32_t TGT:1;
            uint32_t :7;
            uint32_t TIME:16;
        };

        struct TXMDLR_Reg{
            uint8_t DATA[4];
        };

        struct TXMDHR_Reg{
            uint8_t DATA[4];
        };

        struct RXMIR_Reg{
            uint32_t :1;
            uint32_t RTR:1;
            uint32_t IDE:1;
            union{
                uint32_t EXID:29;
                struct{
                    uint32_t :18;
                    uint32_t STID:11;
                };
            };
        };

        struct RXMDTR_Reg{
            uint32_t DLC:4;
            uint32_t :4;
            uint32_t FMI:8;
            uint32_t TIME:16;
        };

        struct RXMDLR_Reg{
            uint8_t DATA[4];
        };

        struct RXMDHR_Reg{
            uint8_t DATA[4];
        };

        struct FCTLR_Reg{
            // 过滤器初始化模式使能标志位。
            // 1：过滤器组为初始化模式；
            // 0：过滤器组为正常模式。
            uint32_t FINIT:1;
            uint32_t :7;

            //CAN2 过滤器开始组（取值范围 1-27）
            uint32_t CAN2SB:6;
        };

        struct FMCFGR_Reg{
            uint32_t FBM0:1;
            uint32_t FBM1:1;
            uint32_t FBM2:1;
            uint32_t FBM3:1;
            uint32_t FBM4:1;
            uint32_t FBM5:1;
            uint32_t FBM6:1;
            uint32_t FBM7:1;
            uint32_t FBM8:1;
            uint32_t FBM9:1;

            uint32_t FBM10:1;
            uint32_t FBM11:1;
            uint32_t FBM12:1;
            uint32_t FBM13:1;
            uint32_t FBM14:1;
            uint32_t FBM15:1;
            uint32_t FBM16:1;
            uint32_t FBM17:1;
            uint32_t FBM18:1;
            uint32_t FBM19:1;

            uint32_t FBM20:1;
            uint32_t FBM21:1;
            uint32_t FBM22:1;
            uint32_t FBM23:1;
            uint32_t FBM24:1;
            uint32_t FBM25:1;
            uint32_t FBM26:1;
            uint32_t FBM27:1;
            uint32_t FBM28:1;

            uint32_t :4;
        };

        struct FSCFGR_Reg{
            uint32_t FSC0:1;
            uint32_t FSC1:1;
            uint32_t FSC2:1;
            uint32_t FSC3:1;
            uint32_t FSC4:1;
            uint32_t FSC5:1;
            uint32_t FSC6:1;
            uint32_t FSC7:1;
            uint32_t FSC8:1;
            uint32_t FSC9:1;

            uint32_t FSC10:1;
            uint32_t FSC11:1;
            uint32_t FSC12:1;
            uint32_t FSC13:1;
            uint32_t FSC14:1;
            uint32_t FSC15:1;
            uint32_t FSC16:1;
            uint32_t FSC17:1;
            uint32_t FSC18:1;
            uint32_t FSC19:1;

            uint32_t FSC20:1;
            uint32_t FSC21:1;
            uint32_t FSC22:1;
            uint32_t FSC23:1;
            uint32_t FSC24:1;
            uint32_t FSC25:1;
            uint32_t FSC26:1;
            uint32_t FSC27:1;
            uint32_t FSC28:1;

            uint32_t :4;
        };

        struct FAFIFOR_Reg{
            uint32_t FFA0:1;
            uint32_t FFA1:1;
            uint32_t FFA2:1;
            uint32_t FFA3:1;
            uint32_t FFA4:1;
            uint32_t FFA5:1;
            uint32_t FFA6:1;
            uint32_t FFA7:1;
            uint32_t FFA8:1;
            uint32_t FFA9:1;

            uint32_t FFA10:1;
            uint32_t FFA11:1;
            uint32_t FFA12:1;
            uint32_t FFA13:1;
            uint32_t FFA14:1;
            uint32_t FFA15:1;
            uint32_t FFA16:1;
            uint32_t FFA17:1;
            uint32_t FFA18:1;
            uint32_t FFA19:1;

            uint32_t FFA20:1;
            uint32_t FFA21:1;
            uint32_t FFA22:1;
            uint32_t FFA23:1;
            uint32_t FFA24:1;
            uint32_t FFA25:1;
            uint32_t FFA26:1;
            uint32_t FFA27:1;
            uint32_t FFA28:1;

            uint32_t :4;
        };

        struct FWR_Reg{
            uint32_t FACT0:1;
            uint32_t FACT1:1;
            uint32_t FACT2:1;
            uint32_t FACT3:1;
            uint32_t FACT4:1;
            uint32_t FACT5:1;
            uint32_t FACT6:1;
            uint32_t FACT7:1;
            uint32_t FACT8:1;
            uint32_t FACT9:1;

            uint32_t FACT10:1;
            uint32_t FACT11:1;
            uint32_t FACT12:1;
            uint32_t FACT13:1;
            uint32_t FACT14:1;
            uint32_t FACT15:1;
            uint32_t FACT16:1;
            uint32_t FACT17:1;
            uint32_t FACT18:1;
            uint32_t FACT19:1;

            uint32_t FACT20:1;
            uint32_t FACT21:1;
            uint32_t FACT22:1;
            uint32_t FACT23:1;
            uint32_t FACT24:1;
            uint32_t FACT25:1;
            uint32_t FACT26:1;
            uint32_t FACT27:1;
            uint32_t FACT28:1;

            uint32_t :4;
        };

        struct FiR_Reg{
            uint32_t FB0:1;
            uint32_t FB1:1;
            uint32_t FB2:1;
            uint32_t FB3:1;
            uint32_t FB4:1;
            uint32_t FB5:1;
            uint32_t FB6:1;
            uint32_t FB7:1;
            uint32_t FB8:1;
            uint32_t FB9:1;

            uint32_t FB10:1;
            uint32_t FB11:1;
            uint32_t FB12:1;
            uint32_t FB13:1;
            uint32_t FB14:1;
            uint32_t FB15:1;
            uint32_t FB16:1;
            uint32_t FB17:1;
            uint32_t FB18:1;
            uint32_t FB19:1;

            uint32_t FB20:1;
            uint32_t FB21:1;
            uint32_t FB22:1;
            uint32_t FB23:1;
            uint32_t FB24:1;
            uint32_t FB25:1;
            uint32_t FB26:1;
            uint32_t FB27:1;
            uint32_t FB28:1;

            uint32_t :4;
        };

        struct CAN_Def{
            CTLR_Reg CTLR;
            STATR_Reg STATR;
            TSTATR_Reg TSTATR;
            RFIFO_Reg RFIFO[2];
            INTEN_Reg INTENR;
            ERRSR_Reg ERRSR;
            BTIMR_Reg BTIMR;
            TTCTLR_Reg TTCTLR;
            TTCNT_Reg TTCNT;

            uint32_t __RESV1__[0x40006580 - (0x40006424+4)];

            TXMIR_Reg TXMIR[3];//0x40006580
            TXMDTR_Reg TXMDTR[3];
        };

        struct CAN_Filt_Def{
        private:
            struct CAN_Filt_Pair{
                FiR_Reg FIR[2];
            };
        public:
            //0x40006600
            volatile FCTLR_Reg FCTLR;

            volatile FMCFGR_Reg FMCFGR;
            volatile FSCFGR_Reg FSCFGR;
            volatile FAFIFOR_Reg FAFIFOR;
            volatile FWR_Reg FWR;

            volatile CAN_Filt_Pair FILTER[28];
        };


        static inline CAN_Def * CAN1 = (CAN_Def *)(0x40006400);
        static inline CAN_Def * CAN2 = (CAN_Def *)(0x40006800);
        static inline CAN_Filt_Def * CAN_Filt = (CAN_Filt_Def *)(0x40006600);

    }
}