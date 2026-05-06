#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


namespace ymd::ral::usb::ch32fs{
// 基地址定义（与参考代码一致）
static constexpr uint32_t USBFS_BASE = 0x50000000U;

/******************************************************************************************
 * 23.2.1 全局寄存器（Global Registers）
 ******************************************************************************************/

/**
 * @brief USB控制寄存器 (R8_USB_CTRL)
 * @details 控制USB工作模式、速率、复位、DMA使能、上拉电阻等核心功能
 * @note 偏移地址: 0x00 | 复位值: 0x06 | 8位有效，高24位保留
 * @see 表23-1、表23-2
 */
struct R8_USB_CTRL {
    /**
     * @brief USB的DMA和DMA中断控制位
     * @details 1:使能DMA功能和DMA中断；0:关闭DMA（正常传输需置1）
     */
    uint8_t RB_UC_DMA_EN:1;

    /**
     * @brief USB的FIFO和中断标志清零
     * @details 1:强制清空FIFO和中断标志（需软件清零）；0:不清空
     */
    uint8_t RB_UC_CLR_ALL:1;

    /**
     * @brief USB协议处理器软件复位控制位
     * @details 1:强制复位USB协议处理器(SIE)（需软件清零）；0:不复位（清零后PB6/PB7切换为USBIO模式）
     */
    uint8_t RB_UC_RST_SIE:1;

    /**
     * @brief 传输完成中断未清零前自动暂停使能位
     * @details 1:中断标志UIF_TRANSFER未清零前自动暂停（设备NAK/主机停传）；0:不暂停
     */
    uint8_t RB_UC_INT_BUSY:1;

    /**
     * @brief USB系统控制组合[1]（配合RB_UC_HOST_MODE）
     * @details 与bit5组成MASK_UC_SYS_CTRL，配置USB设备功能和上拉电阻
     */
    uint8_t MASK_UC_SYS_CTRL1:1;

    /**
     * @brief USB系统控制组合[0]（配合RB_UC_HOST_MODE）
     * @details 与bit4组成MASK_UC_SYS_CTRL，配置USB设备功能和上拉电阻
     */
    uint8_t MASK_UC_SYS_CTRL0:1;

    /**
     * @brief USB总线信号传输速率选择位
     * @details 1:1.5Mbps（低速）；0:12Mbps（全速）
     */
    uint8_t RB_UC_LOW_SPEED:1;

    /**
     * @brief USB工作模式选择位
     * @details 1:主机模式(HOST)；0:设备模式(DEVICE)
     */
    uint8_t RB_UC_HOST_MODE:1;


    /**
     * @brief 设置USB工作模式
     * @param is_host true-主机模式；false-设备模式
     */
    void set_work_mode(bool is_host) {
        RB_UC_HOST_MODE = is_host ? 1U : 0U;
    }

    /**
     * @brief 设置USB传输速率
     * @param is_low_speed true-1.5Mbps（低速）；false-12Mbps（全速）
     */
    void set_speed(bool is_low_speed) {
        RB_UC_LOW_SPEED = is_low_speed ? 1U : 0U;
    }

    /**
     * @brief 配置USB系统控制组合（设备模式）
     * @param enable_dev true-使能USB设备功能；false-禁止
     * @param use_internal_pullup true-启用内部1.5K上拉；false-禁用（需外部上拉）
     */
    void config_device_sys(bool enable_dev, bool use_internal_pullup) {
        set_work_mode(false); // 切换为设备模式
        if (!enable_dev) {
            MASK_UC_SYS_CTRL1 = 0U;
            MASK_UC_SYS_CTRL0 = 0U;
        } else if (!use_internal_pullup) {
            MASK_UC_SYS_CTRL1 = 0U;
            MASK_UC_SYS_CTRL0 = 1U;
        } else {
            MASK_UC_SYS_CTRL1 = 1U;
            MASK_UC_SYS_CTRL0 = 0U;
        }
    }

    /**
     * @brief 配置USB系统控制组合（主机模式）
     * @param mode 0-正常工作；1-强制DP/DM输出SE0；2-强制输出J状态；3-强制输出K状态/唤醒
     */
    void config_host_sys(uint8_t mode) {
        set_work_mode(true); // 切换为主机模式
        MASK_UC_SYS_CTRL1 = (mode >> 1) & 1U;
        MASK_UC_SYS_CTRL0 = mode & 1U;
    }

    /**
     * @brief 使能DMA功能
     */
    void enable_dma() {
        RB_UC_DMA_EN = 1U;
    }

    /**
     * @brief 禁用DMA功能
     */
    void disable_dma() {
        RB_UC_DMA_EN = 0U;
    }

    /**
     * @brief 软件复位USB协议处理器(SIE)
     */
    void reset_sie() {
        RB_UC_RST_SIE = 1U;
    }

    /**
     * @brief 清除SIE复位状态
     */
    void clear_sie_reset() {
        RB_UC_RST_SIE = 0U;
    }

    /**
     * @brief 清空所有FIFO和中断标志
     */
    void clear_all_fifo_irq() {
        RB_UC_CLR_ALL = 1U;
    }

    /**
     * @brief 检查是否为主机模式
     * @return true-主机模式；false-设备模式
     */
    [[nodiscard]] constexpr bool is_host_mode() const noexcept {
        return RB_UC_HOST_MODE == 1U;
    }

    /**
     * @brief 检查是否为低速模式
     * @return true-1.5Mbps；false-12Mbps
     */
    [[nodiscard]] constexpr bool is_low_speed() const noexcept {
        return RB_UC_LOW_SPEED == 1U;
    }

    /**
     * @brief 检查DMA是否使能
     * @return true-DMA使能；false-DMA禁用
     */
    [[nodiscard]] constexpr bool is_dma_enabled() const noexcept {
        return RB_UC_DMA_EN == 1U;
    }
};
VALIDATE_R8(R8_USB_CTRL);

/**
 * @brief USB中断使能寄存器 (R8_USB_INT_EN)
 * @details 使能/禁止USB各类中断，主机/设备模式部分中断功能复用
 * @note 偏移地址: 0x02 | 复位值: 0x00 | 8位有效，高24位保留
 * @see 表23-1
 */
struct R8_USB_INT_EN {
    /**
     * @brief 总线复位/设备检测中断使能
     * @details 设备模式: USB总线复位中断；主机模式: USB设备连接/断开中断；1-使能；0-禁止
     */
    uint8_t RB_UIE_BUS_RST_DETECT:1;

    /**
     * @brief USB传输完成中断使能（不含SETUP事务）
     * @details 1-使能；0-禁止
     */
    uint8_t RB_UIE_TRANSFER:1;

    /**
     * @brief USB总线挂起或唤醒事件中断使能
     * @details 1-使能；0-禁止
     */
    uint8_t RB_UIE_SUSPEND:1;

    /**
     * @brief USB主机模式SOF定时中断使能
     * @details 1-使能；0-禁止
     */
    uint8_t RB_UIE_HST_SOF:1;

    /**
     * @brief FIFO溢出中断使能
     * @details 1-使能；0-禁止
     */
    uint8_t RB_UIE_FIFO_OV:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED:1;

    /**
     * @brief USB设备模式NAK接收中断使能
     * @details 1-使能；0-禁止
     */
    uint8_t RB_UIE_DEV_NAK:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED2:1;

    /**
     * @brief 使能指定中断
     * @param irq_mask 中断掩码（对应位为1表示使能）
     */
    void enable_irq(uint8_t irq_mask) {
        uint8_t reg_val = std::bit_cast<uint8_t>(*this);
        reg_val |= (irq_mask & 0xFFU);
        *this = std::bit_cast<R8_USB_INT_EN>(reg_val);
    }

    /**
     * @brief 禁止指定中断
     * @param irq_mask 中断掩码（对应位为1表示禁止）
     */
    void disable_irq(uint8_t irq_mask) {
        uint8_t reg_val = std::bit_cast<uint8_t>(*this);
        reg_val &= ~(irq_mask & 0xFFU);
        *this = std::bit_cast<R8_USB_INT_EN>(reg_val);
    }

    /**
     * @brief 使能所有中断
     */
    void enable_all_irq() {
        enable_irq(0xFFU);
    }

    /**
     * @brief 禁止所有中断
     */
    void disable_all_irq() {
        disable_irq(0xFFU);
    }

    /**
     * @brief 检查传输完成中断是否使能
     * @return true-使能；false-禁止
     */
    [[nodiscard]] constexpr bool is_transfer_irq_enabled() const noexcept {
        return RB_UIE_TRANSFER == 1U;
    }

    /**
     * @brief 检查总线复位/设备检测中断是否使能
     * @return true-使能；false-禁止
     */
    [[nodiscard]] constexpr bool is_bus_rst_detect_irq_enabled() const noexcept {
        return RB_UIE_BUS_RST_DETECT == 1U;
    }
};
VALIDATE_R8(R8_USB_INT_EN);

/**
 * @brief USB设备地址寄存器 (R8_USB_DEV_AD)
 * @details 主机/设备模式复用，存储设备地址或HUB地址
 * @note 偏移地址: 0x03 | 复位值: 0x00 | 8位有效，高24位保留
 * @see 表23-1
 */
struct R8_USB_DEV_AD {
    /**
     * @brief USB地址[6:0]
     * @details 设备模式: 自身地址；主机模式: 目标设备/HUB地址（范围0~127）
     */
    uint8_t MASK_USB_ADDR:7;

    /**
     * @brief USB通用标志位
     * @details 用户自定义，无硬件关联
     */
    uint8_t RB_UDA_GP_BIT:1;

    /**
     * @brief 设置USB地址
     * @param addr 地址值（0~127）
     */
    void set_dev_addr(uint8_t addr) {
        MASK_USB_ADDR = addr & 0x7FU;
    }

    /**
     * @brief 设置通用标志位
     * @param val 标志值（0或1）
     */
    void set_gp_bit(bool val) {
        RB_UDA_GP_BIT = val ? 1U : 0U;
    }

    /**
     * @brief 获取当前USB地址
     * @return 地址值（0~127）
     */
    [[nodiscard]] constexpr uint8_t get_dev_addr() const noexcept {
        return static_cast<uint8_t>(MASK_USB_ADDR & 0x7FU);
    }

    /**
     * @brief 获取通用标志位值
     * @return 标志值（0或1）
     */
    [[nodiscard]] constexpr bool get_gp_bit() const noexcept {
        return RB_UDA_GP_BIT == 1U;
    }
};
VALIDATE_R8(R8_USB_DEV_AD);

/**
 * @brief USB杂项状态寄存器 (R8_USB_MIS_ST)
 * @details 只读USB各类实时状态：SIE忙闲、FIFO就绪、总线复位、设备插拔等
 * @note 偏移地址: 0x05 | 复位值: 0xXX | 8位有效，高24位保留
 * @see 表23-1
 */
struct R8_USB_MIS_ST {
    /**
     * @brief USB主机模式下设备连接状态位
     * @details 1:端口已连接USB设备；0:无设备连接（只读）
     */
    uint8_t RB_UMS_DEV_ATTACH:1;

    /**
     * @brief USB主机模式下DM引脚电平状态
     * @details 1:高电平（低速设备）；0:低电平（全速设备）（只读）
     */
    uint8_t RB_UMS_DM_LEVEL:1;

    /**
     * @brief USB挂起状态位
     * @details 1:USB总线挂起（无活动）；0:正常工作（只读）
     */
    uint8_t RB_UMS_SUSPEND:1;

    /**
     * @brief USB总线复位状态位
     * @details 1:总线处于复位态；0:正常状态（只读）
     */
    uint8_t RB_UMS_BUS_RST:1;

    /**
     * @brief USB接收FIFO数据就绪状态位
     * @details 1:FIFO非空；0:FIFO为空（只读）
     */
    uint8_t RB_UMS_R_FIFO_RDY:1;

    /**
     * @brief USB协议处理器空闲状态位
     * @details 1:SIE空闲；0:忙（传输中）（只读）
     */
    uint8_t RB_UMS_SIE_FREE:1;

    /**
     * @brief USB主机模式SOF包传输状态位
     * @details 1:正在发送SOF包；0:发送完成/空闲（只读）
     */
    uint8_t RB_UMS_SOF_ACT:1;

    /**
     * @brief USB主机模式SOF包预示状态位
     * @details 1:即将发送SOF包（其他数据包延后）；0:无SOF包发送（只读）
     */
    uint8_t RB_UMS_SOF_PRES:1;


    /**
     * @brief 检查USB协议处理器是否空闲
     * @return true-空闲；false-忙
     */
    [[nodiscard]] constexpr bool is_sie_free() const noexcept {
        return RB_UMS_SIE_FREE == 1U;
    }

    /**
     * @brief 检查接收FIFO是否有数据
     * @return true-FIFO非空；false-FIFO为空
     */
    [[nodiscard]] constexpr bool is_rx_fifo_ready() const noexcept {
        return RB_UMS_R_FIFO_RDY == 1U;
    }

    /**
     * @brief 检查USB总线是否处于复位状态
     * @return true-复位中；false-正常
     */
    [[nodiscard]] constexpr bool is_bus_reset() const noexcept {
        return RB_UMS_BUS_RST == 1U;
    }

    /**
     * @brief 检查USB总线是否挂起
     * @return true-挂起；false-正常
     */
    [[nodiscard]] constexpr bool is_bus_suspended() const noexcept {
        return RB_UMS_SUSPEND == 1U;
    }

    /**
     * @brief 主机模式检查是否有设备连接
     * @return true-设备已连接；false-无设备
     */
    [[nodiscard]] constexpr bool is_device_attached() const noexcept {
        return RB_UMS_DEV_ATTACH == 1U;
    }
};
VALIDATE_R8(R8_USB_MIS_ST);

/**
 * @brief USB中断标志寄存器 (R8_USB_INT_FG)
 * @details 标识USB各类中断触发状态，写1清零，主机/设备模式部分位复用
 * @note 偏移地址: 0x06 | 复位值: 0x20 | 8位有效，高24位保留
 * @see 表23-1
 */
struct R8_USB_INT_FG {
    /**
     * @brief 总线复位/设备检测中断标志
     * @details 设备模式: USB总线复位触发；主机模式: 设备连接/断开触发；写1清零
     */
    uint8_t RB_UIF_BUS_RST_DETECT:1;

    /**
     * @brief USB传输完成中断标志（不含SETUP事务）
     * @details 1:传输完成触发；0:无事件；写1清零
     */
    uint8_t RB_UIF_TRANSFER:1;

    /**
     * @brief USB总线挂起或唤醒事件中断标志
     * @details 1:挂起/唤醒触发；0:无事件；写1清零
     */
    uint8_t RB_UIF_SUSPEND:1;

    /**
     * @brief USB主机模式SOF定时中断标志
     * @details 1:SOF传输完成触发；0:无事件；写1清零
     */
    uint8_t RB_UIF_HST_SOF:1;

    /**
     * @brief FIFO溢出中断标志
     * @details 1:FIFO溢出触发；0:无事件；写1清零
     */
    uint8_t RB_UIF_FIFO_OV:1;

    /**
     * @brief USB协议处理器空闲状态位
     * @details 1:SIE空闲；0:忙（传输中）（只读）
     */
    uint8_t RB_U_SIE_FREE:1;

    /**
     * @brief USB传输DATA0/1同步标志匹配状态位
     * @details 1:同步；0:不同步（只读）
     */
    uint8_t RB_U_TOG_OK:1;

    /**
     * @brief USB设备模式NAK响应状态位
     * @details 1:传输中回应NAK；0:无NAK响应（只读）
     */
    uint8_t RB_U_IS_NAK:1;


    /**
     * @brief 清零指定中断标志
     * @param flag_mask 标志掩码（对应位为1表示清零）
     */
    void clear_flag(uint8_t flag_mask) {
        uint8_t reg_val = std::bit_cast<uint8_t>(*this);
        reg_val |= (flag_mask & 0xFFU); // 写1清零
        *this = std::bit_cast<R8_USB_INT_FG>(reg_val);
    }

    /**
     * @brief 清零所有可清除的中断标志
     */
    void clear_all_flag() {
        clear_flag(0x1FU); // 仅清零可写位（bit0~bit4）
    }

    /**
     * @brief 检查传输完成中断是否触发
     * @return true-已触发；false-未触发
     */
    [[nodiscard]] constexpr bool is_transfer_flag_set() const noexcept {
        return RB_UIF_TRANSFER == 1U;
    }

    /**
     * @brief 检查FIFO溢出中断是否触发
     * @return true-已触发；false-未触发
     */
    [[nodiscard]] constexpr bool is_fifo_overflow_flag_set() const noexcept {
        return RB_UIF_FIFO_OV == 1U;
    }

    /**
     * @brief 检查同步标志是否匹配
     * @return true-匹配；false-不匹配
     */
    [[nodiscard]] constexpr bool is_tog_ok() const noexcept {
        return RB_U_TOG_OK == 1U;
    }
};
VALIDATE_R8(R8_USB_INT_FG);

/**
 * @brief USB中断状态寄存器 (R8_USB_INT_ST)
 * @details 只读中断详细状态，区分端点号、令牌PID、同步匹配结果
 * @note 偏移地址: 0x07 | 复位值: 0xXX | 8位有效，高24位保留
 * @see 表23-1
 */
struct R8_USB_INT_ST {
    /**
     * @brief 设备模式: 当前传输端点号；主机模式: 应答PID标识
     * @details 设备模式: 0~15；主机模式: 0000=无应答/超时，其他=应答PID（只读）
     */
    uint8_t MASK_UIS_ENDP_H_RES:4;

    /**
     * @brief 设备模式: 当前传输令牌PID标识
     * @details 00-OUT包；01-保留；10-IN包；11-SETUP包（主机模式保留，只读）
     */
    uint8_t MASK_UIS_TOKEN:2;

    /**
     * @brief 传输DATA0/1同步标志匹配状态位
     * @details 1:同步；0:不同步（只读）
     */
    uint8_t RB_UIS_TOG_OK:1;

    /**
     * @brief USB设备模式NAK响应状态位
     * @details 1:传输中回应NAK；0:无NAK响应（只读）
     */
    uint8_t RB_UIS_IS_NAK:1;


    /**
     * @brief 设备模式获取当前传输端点号
     * @return 端点号（0~15）
     */
    [[nodiscard]] constexpr uint8_t dev_get_endpoint_id() const noexcept {
        return static_cast<uint8_t>(MASK_UIS_ENDP_H_RES & 0x0FU);
    }

    /**
     * @brief 设备模式获取当前传输令牌PID类型
     * @return 0-OUT包；1-保留；2-IN包；3-SETUP包
     */
    [[nodiscard]] constexpr uint8_t dev_get_token_pid() const noexcept {
        return static_cast<uint8_t>(MASK_UIS_TOKEN & 0x03U);
    }

    /**
     * @brief 主机模式获取设备应答PID
     * @return 应答PID标识（0=无应答/超时，1~15=对应应答）
     */
    [[nodiscard]] constexpr uint8_t host_get_response_pid() const noexcept {
        return static_cast<uint8_t>(MASK_UIS_ENDP_H_RES & 0x0FU);
    }

    /**
     * @brief 检查同步标志是否匹配
     * @return true-匹配；false-不匹配
     */
    [[nodiscard]] constexpr bool is_tog_ok() const noexcept {
        return RB_UIS_TOG_OK == 1U;
    }

    /**
     * @brief 设备模式检查是否回应NAK
     * @return true-已回应NAK；false-无
     */
    [[nodiscard]] constexpr bool dev_is_nak_responded() const noexcept {
        return RB_UIS_IS_NAK == 1U;
    }

    /**
     * @brief 设备模式检查是否为SETUP事务
     * @return true-SETUP事务；false-其他事务
     */
    [[nodiscard]] constexpr bool dev_is_setup_transaction() const noexcept {
        return dev_get_token_pid() == 3U;
    }
};
VALIDATE_R8(R8_USB_INT_ST);

/**
 * @brief USB接收长度寄存器 (R16_USB_RX_LEN)
 * @details 只读当前端点实际接收的数据字节数
 * @note 偏移地址: 0x08 | 复位值: 0x0XXX | 10位有效，高22位保留
 * @see 表23-1
 */
struct R32_USB_RX_LEN {
    /**
     * @brief 当前USB端点接收的数据字节数
     * @details 范围0~1023（只读）
     */
    uint32_t R16_USB_RX_LEN:10;

    /**
     * @brief 保留位，只读，值为0
     */
    uint32_t __RESV__:22;

    /**
     * @brief 获取实际接收的数据字节数
     * @return 字节数（0~1023）
     */
    [[nodiscard]] constexpr uint16_t get_rx_length() const noexcept {
        return static_cast<uint16_t>(R16_USB_RX_LEN & 0x3FFU);
    }
};
VALIDATE_R32(R32_USB_RX_LEN);

/**
 * @brief USB OTG控制寄存器 (R32_USB_OTG_CR)
 * @details 配置OTG功能使能、阈值电压、ID引脚上拉、VBUS充放电
 * @note 偏移地址: 0x54 | 复位值: 0x00000000 | 仅适用于CH32V305/V307/F205/F207
 * @see 表23-1
 */
struct R32_USB_OTG_CR {
    /**
     * @brief OTG VBUS放电使能
     * @details 1:使能VBUS放电；0:禁止
     */
    uint32_t RB_CR_DISCHAR_VBUS:1;

    /**
     * @brief OTG VBUS充电使能
     * @details 1:使能VBUS充电；0:禁止
     */
    uint32_t RB_CR_CHARGE_VBUS:1;

    /**
     * @brief USB_OTG_ID引脚上拉使能
     * @details 1:使能ID引脚上拉；0:禁止
     */
    uint32_t RB_CR_IDPU:1;

    /**
     * @brief OTG功能使能
     * @details 1:使能OTG功能；0:禁止
     */
    uint32_t RB_CR_OTG_EN:1;

    /**
     * @brief OTG VBUS阈值电压设置
     * @details 1:VBUS_VLD电平为4.4V；0:VBUS_VLD电平为4.8V
     */
    uint32_t RB_CR_VBUS_VTH:1;

    /**
     * @brief OTG会话有效阈值电压设置
     * @details 1:SESS_VLD电平为1.4V；0:SESS_VLD电平为0.8V
     */
    uint32_t RB_CR_SESS_VTH:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint32_t __RESV__:26;

    /**
     * @brief 使能OTG功能
     */
    void enable_otg() {
        RB_CR_OTG_EN = 1U;
    }

    /**
     * @brief 禁用OTG功能
     */
    void disable_otg() {
        RB_CR_OTG_EN = 0U;
    }

    /**
     * @brief 使能ID引脚上拉
     */
    void enable_id_pullup() {
        RB_CR_IDPU = 1U;
    }

    /**
     * @brief 禁用ID引脚上拉
     */
    void disable_id_pullup() {
        RB_CR_IDPU = 0U;
    }

    /**
     * @brief 使能VBUS充电
     */
    void enable_vbus_charge() {
        RB_CR_CHARGE_VBUS = 1U;
    }

    /**
     * @brief 使能VBUS放电
     */
    void enable_vbus_discharge() {
        RB_CR_DISCHAR_VBUS = 1U;
    }

    /**
     * @brief 检查OTG功能是否使能
     * @return true-已使能；false-未使能
     */
    [[nodiscard]] constexpr bool is_otg_enabled() const noexcept {
        return RB_CR_OTG_EN == 1U;
    }
};
VALIDATE_R32(R32_USB_OTG_CR);

/**
 * @brief USB OTG状态寄存器 (R32_USB_OTG_SR)
 * @details 只读OTG工作状态：ID标志、会话状态、VBUS电平
 * @note 偏移地址: 0x58 | 复位值: 0x0000000X | 仅适用于CH32V305/V307/F205/F207
 * @see 表23-1
 */
struct R32_USB_OTG_SR {
    /**
     * @brief OTG VBUS输入电平
     * @details 1:VBUS电压大于阈值电压；0:小于阈值电压（只读）
     */
    uint32_t RB_SR_VBUS_VLD:1;

    /**
     * @brief OTG会话有效标志
     * @details 1:会话有效（电平大于阈值）；0:无效（只读）
     */
    uint32_t RB_SR_SESS_VLD:1;

    /**
     * @brief OTG会话结束有效标志
     * @details 1:有效；0:无效（只读）
     */
    uint32_t RB_SR_SESS_END:1;

    /**
     * @brief OTG ID标志
     * @details 1:B设备；0:A设备（只读）
     */
    uint32_t RB_SR_ID_DIG:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint32_t __RESV__:28;

    /**
     * @brief 检查是否为B设备
     * @return true-B设备；false-A设备
     */
    [[nodiscard]] constexpr bool is_b_device() const noexcept {
        return RB_SR_ID_DIG == 1U;
    }

    /**
     * @brief 检查VBUS电压是否有效
     * @return true-电压有效；false-无效
     */
    [[nodiscard]] constexpr bool is_vbus_valid() const noexcept {
        return RB_SR_VBUS_VLD == 1U;
    }

    /**
     * @brief 检查OTG会话是否有效
     * @return true-会话有效；false-无效
     */
    [[nodiscard]] constexpr bool is_session_valid() const noexcept {
        return RB_SR_SESS_VLD == 1U;
    }
};
VALIDATE_R32(R32_USB_OTG_SR);

/******************************************************************************************
 * 23.2.2 设备模式寄存器（Device Mode Registers）
 ******************************************************************************************/

/**
 * @brief USB设备物理端口控制寄存器 (R8_UDEV_CTRL)
 * @details 控制设备端口引脚状态、速率、使能
 * @note 偏移地址: 0x01 | 复位值: 0xX0 | 8位有效，高24位保留
 * @see 表23-3
 */
struct R8_UDEV_CTRL {
    /**
     * @brief USB设备物理端口使能位
     * @details 1:使能物理端口；0:禁用物理端口
     */
    uint8_t RB_UD_PORT_EN:1;

    /**
     * @brief USB设备模式通用标志位
     * @details 用户自定义，无硬件关联
     */
    uint8_t RB_UD_GP_BIT:1;

    /**
     * @brief USB设备物理端口低速模式使能位
     * @details 1:1.5Mbps（低速）；0:12Mbps（全速）
     */
    uint8_t RB_UD_LOW_SPEED:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED:1;

    /**
     * @brief 当前UD-引脚状态
     * @details 1:高电平；0:低电平（只读）
     */
    uint8_t RB_UD_DM_PIN:1;

    /**
     * @brief 当前UD+引脚状态
     * @details 1:高电平；0:低电平（只读）
     */
    uint8_t RB_UD_DP_PIN:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED2:1;

    /**
     * @brief USB设备端口UD+/UD-引脚内部下拉电阻控制位
     * @details 1:禁用内部下拉；0:使能内部下拉（可用于GPIO模式）
     */
    uint8_t RB_UD_PD_DIS:1;

    /**
     * @brief 使能设备物理端口
     */
    void enable_port() {
        RB_UD_PORT_EN = 1U;
    }

    /**
     * @brief 禁用设备物理端口
     */
    void disable_port() {
        RB_UD_PORT_EN = 0U;
    }

    /**
     * @brief 设置设备端口速率
     * @param is_low_speed true-低速（1.5Mbps）；false-全速（12Mbps）
     */
    void set_port_speed(bool is_low_speed) {
        RB_UD_LOW_SPEED = is_low_speed ? 1U : 0U;
    }

    /**
     * @brief 禁用UD+/UD-引脚内部下拉电阻
     */
    void disable_internal_pull_down() {
        RB_UD_PD_DIS = 1U;
    }

    /**
     * @brief 检查设备端口是否使能
     * @return true-已使能；false-未使能
     */
    [[nodiscard]] constexpr bool is_port_enabled() const noexcept {
        return RB_UD_PORT_EN == 1U;
    }

    /**
     * @brief 获取UD+引脚当前状态
     * @return true-高电平；false-低电平
     */
    [[nodiscard]] constexpr bool get_dp_pin_state() const noexcept {
        return RB_UD_DP_PIN == 1U;
    }
};
VALIDATE_R8(R8_UDEV_CTRL);

/**
 * @brief 端点1(9)/4(8/12)模式控制寄存器 (R8_UEP4_1_MOD)
 * @details 控制端点1、4的收发使能和缓冲区模式
 * @note 偏移地址: 0x0C | 复位值: 0x00 | 8位有效，高24位保留
 * @see 表23-3、表23-4
 */
struct R8_UEP4_1_MOD {
    /**
     * @brief 端点4(8/12)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲（注：为1时收发不能同时使能）
     */
    uint8_t RB_UEP4_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED:1;

    /**
     * @brief 端点4(8/12)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP4_TX_EN:1;

    /**
     * @brief 端点4(8/12)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP4_RX_EN:1;

    /**
     * @brief 端点1(9)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲（注：为1时收发不能同时使能）
     */
    uint8_t RB_UEP1_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED2:1;

    /**
     * @brief 端点1(9)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP1_TX_EN:1;

    /**
     * @brief 端点1(9)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP1_RX_EN:1;


    /**
     * @brief 配置端点1(9)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲（收发不能同时使能）
     */
    void config_ep1(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP1_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP1_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP1_BUF_MOD = is_double_buf ? 1U : 0U;
    }

    /**
     * @brief 配置端点4(8/12)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲（收发不能同时使能）
     */
    void config_ep4(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP4_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP4_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP4_BUF_MOD = is_double_buf ? 1U : 0U;
    }

    /**
     * @brief 检查端点1是否使能接收
     * @return true-使能；false-禁止
     */
    [[nodiscard]] constexpr bool is_ep1_rx_enabled() const noexcept {
        return RB_UEP1_RX_EN == 1U;
    }
};
VALIDATE_R8(R8_UEP4_1_MOD);

/**
 * @brief 端点2(10)/3(11)模式控制寄存器 (R8_UEP2_3_MOD)
 * @details 控制端点2、3的收发使能和缓冲区模式
 * @note 偏移地址: 0x0D | 复位值: 0x00 | 8位有效，高24位保留
 * @see 表23-3、表23-4
 */
struct R8_UEP2_3_MOD {
    /**
     * @brief 端点2(10)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲（注：为1时收发不能同时使能）
     */
    uint8_t RB_UEP2_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED:1;

    /**
     * @brief 端点2(10)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP2_TX_EN:1;

    /**
     * @brief 端点2(10)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP2_RX_EN:1;

    /**
     * @brief 端点3(11)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲（注：为1时收发不能同时使能）
     */
    uint8_t RB_UEP3_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED2:1;

    /**
     * @brief 端点3(11)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP3_TX_EN:1;

    /**
     * @brief 端点3(11)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP3_RX_EN:1;


    /**
     * @brief 配置端点2(10)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲（收发不能同时使能）
     */
    void config_ep2(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP2_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP2_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP2_BUF_MOD = is_double_buf ? 1U : 0U;
    }

    /**
     * @brief 配置端点3(11)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲（收发不能同时使能）
     */
    void config_ep3(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP3_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP3_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP3_BUF_MOD = is_double_buf ? 1U : 0U;
    }
};
VALIDATE_R8(R8_UEP2_3_MOD);

/**
 * @brief 端点5(13)/6(14)模式控制寄存器 (R8_UEP5_6_MOD)
 * @details 控制端点5、6的收发使能和缓冲区模式
 * @note 偏移地址: 0x0E | 复位值: 0x00 | 8位有效，高24位保留
 * @see 表23-3、表23-4
 */
struct R8_UEP5_6_MOD {
    /**
     * @brief 端点5(13)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲（注：为1时收发不能同时使能）
     */
    uint8_t RB_UEP5_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED:1;

    /**
     * @brief 端点5(13)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP5_TX_EN:1;

    /**
     * @brief 端点5(13)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP5_RX_EN:1;

    /**
     * @brief 端点6(14)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲（注：为1时收发不能同时使能）
     */
    uint8_t RB_UEP6_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED2:1;

    /**
     * @brief 端点6(14)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP6_TX_EN:1;

    /**
     * @brief 端点6(14)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP6_RX_EN:1;


    /**
     * @brief 配置端点5(13)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲（收发不能同时使能）
     */
    void config_ep5(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP5_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP5_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP5_BUF_MOD = is_double_buf ? 1U : 0U;
    }

    /**
     * @brief 配置端点6(14)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲（收发不能同时使能）
     */
    void config_ep6(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP6_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP6_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP6_BUF_MOD = is_double_buf ? 1U : 0U;
    }
};
VALIDATE_R8(R8_UEP5_6_MOD);

/**
 * @brief 端点7(15)模式控制寄存器 (R8_UEP7_MOD)
 * @details 控制端点7的收发使能和缓冲区模式
 * @note 偏移地址: 0x0F | 复位值: 0x00 | 8位有效，高24位保留
 * @see 表23-3、表23-4
 */
struct R8_UEP7_MOD {
    /**
     * @brief 端点7(15)数据缓冲区模式控制位
     * @details 1:双缓冲；0:单缓冲
     */
    uint8_t RB_UEP7_BUF_MOD:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t RESERVED:1;

    /**
     * @brief 端点7(15)发送使能
     * @details 1:使能发送(IN)；0:禁止发送
     */
    uint8_t RB_UEP7_TX_EN:1;

    /**
     * @brief 端点7(15)接收使能
     * @details 1:使能接收(OUT)；0:禁止接收
     */
    uint8_t RB_UEP7_RX_EN:1;

    /**
     * @brief 保留位，只读，值为0
     */
    uint8_t __RESV__:4;

    /**
     * @brief 配置端点7(15)模式
     * @param enable_rx 接收使能
     * @param enable_tx 发送使能
     * @param is_double_buf 是否双缓冲
     */
    void config_ep7(bool enable_rx, bool enable_tx, bool is_double_buf) {
        RB_UEP7_RX_EN = enable_rx ? 1U : 0U;
        RB_UEP7_TX_EN = enable_tx ? 1U : 0U;
        RB_UEP7_BUF_MOD = is_double_buf ? 1U : 0U;
    }
};
VALIDATE_R8(R8_UEP7_MOD);

/**
 * @brief 端点n缓冲区起始地址寄存器 (R32_UEPn_DMA)
 * @details 配置端点n的缓冲区起始地址（4字节对齐）
 * @note 偏移地址: 0x10+4*n (n=0~7) | 复位值: 0xXXXXXXXX
 * @see 表23-3
 */
template<uint8_t N>
struct R32_UEPn_DMA {
    static_assert(N <= 7U, "Endpoint number must be 0~7");

    /**
     * @brief 端点n缓冲区起始地址
     * @details 地址必须4字节对齐（低2位为0）
     */
    uint32_t R32_UEPn_DMA:32;

    /**
     * @brief 设置缓冲区起始地址（自动4字节对齐）
     * @param addr 缓冲区地址
     */
    void set_dma_addr(uint32_t addr) {
        R32_UEPn_DMA = addr & 0xFFFFFFFCU;
    }

    /**
     * @brief 获取缓冲区起始地址
     * @return 32位对齐地址
     */
    [[nodiscard]] constexpr uint32_t get_dma_addr() const noexcept {
        return R32_UEPn_DMA;
    }

    /**
     * @brief 检查地址是否4字节对齐
     * @return true-对齐；false-未对齐
     */
    [[nodiscard]] constexpr bool is_addr_aligned() const noexcept {
        return (R32_UEPn_DMA & 0x03U) == 0U;
    }
};

}