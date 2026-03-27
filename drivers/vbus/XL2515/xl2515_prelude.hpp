#pragma once

#include <cstdint>
//参考资料
// https://www.sekorm.com/news/76608264.html
namespace ymd::drivers{



/**
 * @brief MCP2515 SPI 指令工厂类
 * @details 该类为 MCP2515 CAN 控制器提供所有 SPI 指令的获取接口，
 * 采用静态类设计，不允许实例化，所有指令均为编译期常量
 */
struct [[nodiscard]] CommandFactory final{
public:
    CommandFactory& operator=(CommandFactory&&) = delete;

    /// @brief 复位指令：将内部寄存器复位为缺省状态，器件设定为配置模式
    static constexpr uint8_t CAN_RESET          = 0xC0;
    /// @brief 读指令：从指定地址起始的寄存器读取数据
    static constexpr uint8_t CAN_READ           = 0x03;
    /// @brief 写指令：将数据写入指定地址起始的寄存器
    static constexpr uint8_t CAN_WRITE          = 0x02;
    /// @brief RTS基础指令：请求发送报文的基地址
    static constexpr uint8_t CAN_RTS            = 0x80;
    /// @brief RTS_TXB0指令：请求发送TXB0缓冲器中的报文
    static constexpr uint8_t CAN_RTS_TXB0       = 0x81;
    /// @brief RTS_TXB1指令：请求发送TXB1缓冲器中的报文
    static constexpr uint8_t CAN_RTS_TXB1       = 0x82;
    /// @brief RTS_TXB2指令：请求发送TXB2缓冲器中的报文
    static constexpr uint8_t CAN_RTS_TXB2       = 0x84;
    /// @brief 读状态指令：快速查询发送和接收功能的相关状态
    static constexpr uint8_t CAN_RD_STATUS      = 0xA0;
    /// @brief 位修改指令：将特殊寄存器的单独位置1或清零（不适用于所有寄存器）
    static constexpr uint8_t CAN_BIT_MODIFY     = 0x05;
    /// @brief RX状态指令：确定匹配的滤波器和接收报文的类型（标准帧、扩展帧、远程帧）
    static constexpr uint8_t CAN_RX_STATUS      = 0xB0;
    /// @brief 读RX缓冲器指令：n,m指示四个地址之一放置地址指针，CS拉高后CANINT.RXIF清零
    static constexpr uint8_t CAN_RD_RX_BUFF     = 0x90;
    /// @brief 装载TX缓冲器指令：a,b,c指示六个地址之一放置地址指针，减轻写命令开销
    static constexpr uint8_t CAN_LOAD_TX        = 0x40;

    /**
     * @brief 获取复位指令
     * @return uint8_t 复位指令值 0xC0
     */
    static constexpr uint8_t reset() noexcept
    {
        return CAN_RESET;
    }

    /**
     * @brief 获取读指令
     * @return uint8_t 读指令值 0x03
     */
    static constexpr uint8_t read() noexcept
    {
        return CAN_READ;
    }

    /**
     * @brief 获取写指令
     * @return uint8_t 写指令值 0x02
     */
    static constexpr uint8_t write() noexcept
    {
        return CAN_WRITE;
    }

    /**
     * @brief 获取请求发送指令
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

    /**
     * @brief 获取读状态指令
     * @return uint8_t 读状态指令值 0xA0
     */
    static constexpr uint8_t read_status() noexcept
    {
        return CAN_RD_STATUS;
    }

    /**
     * @brief 获取位修改指令
     * @return uint8_t 位修改指令值 0x05
     */
    static constexpr uint8_t bit_modify() noexcept
    {
        return CAN_BIT_MODIFY;
    }

    /**
     * @brief 获取RX状态指令
     * @return uint8_t RX状态指令值 0xB0
     */
    static constexpr uint8_t rx_status() noexcept
    {
        return CAN_RX_STATUS;
    }

    /**
     * @brief 获取读RX缓冲器指令
     * @return uint8_t 读RX缓冲器指令值 0x90
     */
    static constexpr uint8_t read_rx_buffer() noexcept
    {
        return CAN_RD_RX_BUFF;
    }

    /**
     * @brief 获取装载TX缓冲器指令
     * @return uint8_t 装载TX缓冲器指令值 0x40
     */
    static constexpr uint8_t load_tx_buffer() noexcept
    {
        return CAN_LOAD_TX;
    }
};
}