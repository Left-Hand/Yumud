#pragma once

#include <cstdint>


namespace ymd::drivers::sja1000{
struct [[nodiscard]] RegSubAddr{
    enum class [[nodiscard]] Kind:uint8_t{
        CONTROL       = 0x00,       //内部控制寄存器
        COMMAND       = 0x01,       //命令寄存器
        STATUS        = 0x02,       //状态寄存器
        INTERRUPT     = 0x03,       //中断寄存器
        INTENABLE     = 0x04,       //中断使能寄存器
        BTR0          = 0x06,       //总线定时寄存器0
        BTR1          = 0x07,       //总线定时寄存器1
        OCR           = 0x08,       //输出控制寄存器
        TEST          = 0x09,       //测试寄存器

        RESVER1       = 0x0A,       //保留1
        ARBITRATE     = 0x0B,       //仲裁丢失捕捉
        ERRCATCH      = 0x0C,       //错误代码捕捉
        ERRLIMIT      = 0x0D,       //错误报警限额

        RXERR         = 0x0E,         //接收错误计数器
        TXERR         = 0x0F,         //发送错误计数器

        ACR1          = 0x10,       //验收代码寄存器
        ACR2          = 0x11,       //验收代码寄存器
        ACR3          = 0x12,       //验收代码寄存器
        ACR4          = 0x13,       //验收代码寄存器
        AMR1          = 0x14,       //验收屏蔽寄存器
        AMR2          = 0x15,       //验收屏蔽寄存器
        AMR3          = 0x16,       //验收屏蔽寄存器
        AMR4          = 0x17,       //验收屏蔽寄存器
    };

    Kind kind_;
};

struct [[nodiscard]] RegAddr final{
    RegSubAddr sub_addr;
    uint8_t addr;
};

}