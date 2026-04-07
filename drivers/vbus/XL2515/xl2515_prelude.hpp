#pragma once

#include <cstdint>
//参考资料
// https://www.sekorm.com/news/76608264.html
namespace ymd::drivers{

struct [[nodiscard]] CommandFactory final{
public:
    CommandFactory& operator=(CommandFactory&&) = delete;

    static constexpr uint8_t CAN_RESET          = 0xC0;
    static constexpr uint8_t CAN_READ           = 0x03;
    static constexpr uint8_t CAN_WRITE          = 0x02;
    static constexpr uint8_t CAN_RTS            = 0x80;
    static constexpr uint8_t CAN_RTS_TXB0       = 0x81;
    static constexpr uint8_t CAN_RTS_TXB1       = 0x82;
    static constexpr uint8_t CAN_RTS_TXB2       = 0x84;
    static constexpr uint8_t CAN_RD_STATUS      = 0xA0;
    static constexpr uint8_t CAN_BIT_MODIFY     = 0x05;
    static constexpr uint8_t CAN_RX_STATUS      = 0xB0;
    static constexpr uint8_t CAN_RD_RX_BUFF     = 0x90;
    static constexpr uint8_t CAN_LOAD_TX        = 0x40;


    /**
     * @details 根据传入的缓冲器使能标志，生成对应的RTS指令
     * @param txb0 使能TXB0缓冲器发送
     * @param txb1 使能TXB1缓冲器发送
     * @param txb2 使能TXB2缓冲器发送
     * @return uint8_t 组合后的RTS指令值
     * @note 支持同时使能多个缓冲器，指令值为对应位的或运算结果
     */
    static constexpr uint8_t request_to_send(const bool txb0, const bool txb1, const bool txb2) noexcept
    {
        uint8_t cmd = CAN_RTS;
        if (txb0) cmd |= 0x01;
        if (txb1) cmd |= 0x02;
        if (txb2) cmd |= 0x04;
        return cmd;
    }
};
}