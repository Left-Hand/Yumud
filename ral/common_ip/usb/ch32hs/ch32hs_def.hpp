#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


namespace ymd::ral::usb::ch32hs{
/******************************************************************************************
 * 22.2.1 全局寄存器（基地址 0x40023400）
 ******************************************************************************************/

/**
 * @brief USB控制寄存器 (R8_USB_CTRL)
 * @note 偏移地址: 0x00 | 复位值: 0x06 | 8位有效，高24位保留
 * @details 控制USB工作模式、速率、复位、DMA使能、上拉电阻等核心功能
 */
struct R32_USB_CTRL {
    // 使能USB的DMA，正常传输需置1：1-使能DMA及DMA中断；0-关闭DMA
    uint32_t RB_UC_DMA_EN:1;
    // USB的FIFO和中断标志清零：1-清空，需软件清零；0-不清空
    uint32_t RB_UC_CLR_ALL:1;
    // USB协议处理器软件复位：1-强制复位SIE，需软件清零；0-不复位（清零后PB6/PB7为USBIO）
    uint32_t RB_UC_RST_SIE:1;
    // 传输完成中断未清零前自动暂停：1-暂停，设备NAK/主机停传；0-不暂停
    uint32_t RB_UC_INT_BUSY:1;
    // 设备模式下USB使能+内部上拉电阻：1-使能传输并启用上拉；0-不启用
    uint32_t RB_UC_DEV_PU_EN:1;
    // USB总线速率选择：00-全速；01-高速；10-低速
    uint32_t RB_UC_SPEED_TYPE:2;
    // USB工作模式选择：1-主机模式(HOST)；0-设备模式(DEVICE)
    uint32_t RB_UC_HOST_MODE:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 设置USB工作模式
     * @param is_host true-主机模式；false-设备模式
     */
    void set_work_mode(bool is_host) {
        RB_UC_HOST_MODE = is_host ? 1 : 0;
    }

    /**
     * @brief 设置USB总线速率
     * @param speed 0-全速；1-高速；2-低速
     */
    void set_speed(uint8_t speed) {
        RB_UC_SPEED_TYPE = (speed & 0x03) <= 2 ? (speed & 0x03) : 0;
    }

    /**
     * @brief 设备模式下启用USB及内部上拉电阻
     */
    void dev_enable_pullup() {
        RB_UC_DEV_PU_EN = 1;
    }

    /**
     * @brief 软件复位USB协议处理器SIE
     */
    void reset_sie() {
        RB_UC_RST_SIE = 1;
    }

    /**
     * @brief 清除SIE复位，使能USBIO口
     */
    void clear_sie_reset() {
        RB_UC_RST_SIE = 0;
    }

    /**
     * @brief 使能USB DMA功能（正常传输必须置1）
     */
    void enable_dma() {
        RB_UC_DMA_EN = 1;
    }

    /**
     * @brief 清空USB所有中断标志和FIFO
     */
    void clear_all_fifo_irq() {
        RB_UC_CLR_ALL = 1;
    }

    /**
     * @brief 检查是否为主机模式
     * @return true-主机模式；false-设备模式
     */
    constexpr bool is_host_mode() const {
        return RB_UC_HOST_MODE == 1;
    }

    /**
     * @brief 检查DMA是否使能
     * @return true-DMA使能；false-DMA关闭
     */
    constexpr bool is_dma_enabled() const {
        return RB_UC_DMA_EN == 1;
    }
};
VALIDATE_R32(R32_USB_CTRL);

/**
 * @brief USB中断使能寄存器 (R8_USB_INT_EN)
 * @note 偏移地址: 0x02 | 复位值: 0x00 | 8位有效，高24位保留
 * @details 使能/禁止USB各类中断，主机/设备模式部分中断功能复用
 */
struct R8_USB_INT_EN {
    // 总线复位/设备检测中断使能：1-使能；0-禁止（设备:总线复位；主机:设备插拔）
    uint8_t RB_UIE_BUS_RST:1;
    // 传输完成中断使能（不含SETUP）：1-使能；0-禁止
    uint8_t RB_UIE_TRANSFER:1;
    // 挂起/唤醒事件中断使能：1-使能；0-禁止
    uint8_t RB_UIE_SUSPEND:1;
    // SOF中断使能：1-使能；0-禁止（设备:收SOF触发；主机:SOF定时触发）
    uint8_t RB_UIE_SOF_ACT:1;
    // FIFO溢出中断使能：1-使能；0-禁止
    uint8_t RB_UIE_FIFO_OV:1;
    // SETUP事务完成中断使能：1-使能；0-禁止
    uint8_t RB_UIE_SETUP_ACT:1;
    // 同步传输收发中断使能：1-使能；0-禁止
    uint8_t RB_UIE_ISO_ACT:1;
    // 设备模式NAK接收中断使能：1-使能；0-禁止
    uint8_t RB_UIE_DEV_NAK:1;

    /**
     * @brief 使能指定中断
     * @param irq_mask 中断掩码（对应各位1=使能）
     */
    void enable_irq(uint8_t irq_mask) {
        uint8_t reg_val = std::bit_cast<uint8_t>(*this);
        reg_val |= (irq_mask & 0xFF);
        *this = std::bit_cast<R8_USB_INT_EN>(reg_val);
    }

    /**
     * @brief 禁止指定中断
     * @param irq_mask 中断掩码（对应各位1=禁止）
     */
    void disable_irq(uint8_t irq_mask) {
        uint8_t reg_val = std::bit_cast<uint8_t>(*this);
        reg_val &= ~(irq_mask & 0xFF);
        *this = std::bit_cast<R8_USB_INT_EN>(reg_val);
    }

    /**
     * @brief 使能所有中断
     */
    void enable_all_irq() {
        enable_irq(0xFF);
    }

    /**
     * @brief 禁止所有中断
     */
    void disable_all_irq() {
        disable_irq(0xFF);
    }

    /**
     * @brief 检查传输完成中断是否使能
     * @return true-使能；false-禁止
     */
    constexpr bool is_transfer_irq_en() const {
        return RB_UIE_TRANSFER == 1;
    }
};
VALIDATE_R8(R8_USB_INT_EN);

/**
 * @brief USB设备地址寄存器 (R8_USB_DEV_AD)
 * @note 偏移地址: 0x03 | 复位值: 0x00 | 7位有效，bit7+高24位保留
 * @details 主机/设备模式复用，存储设备地址/HUB地址
 */
struct R32_USB_DEV_AD {
    // USB地址[6:0]：设备-自身地址；主机-目标设备/HUB地址（0~127）
    uint32_t RB_MASK_USB_ADDR:7;
    // 保留位，只读，值为0
    uint32_t RESERVED:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 设置USB地址
     * @param addr 地址值（0~127）
     */
    void set_dev_addr(uint8_t addr) {
        RB_MASK_USB_ADDR = addr & 0x7F;
    }

    /**
     * @brief 获取当前USB地址
     * @return 地址值（0~127）
     */
    constexpr uint8_t get_dev_addr() const {
        return static_cast<uint8_t>(RB_MASK_USB_ADDR & 0x7F);
    }
};
VALIDATE_R32(R32_USB_DEV_AD);

/**
 * @brief USB帧号寄存器 (R16_USB_FRAME_NO)
 * @note 偏移地址: 0x04 | 复位值: 0x0000 | 16位有效，高16位保留
 * @details 存储SOF帧号+高速微帧号，主机/设备模式实时更新
 */
struct R32_USB_FRAME_NO {
    // SOF帧号[10:0]：有效帧号，主机发送/设备接收（0~2047）
    uint32_t FRAME_NO:11;
    // 高速模式微帧号[2:0]：高3位，标识当前微帧（0~7）
    uint32_t MICRO_FRAME:3;
    // 保留位，只读，值为0
    uint32_t __RESV__:18;

    /**
     * @brief 获取当前SOF帧号
     * @return 帧号（0~2047）
     */
    constexpr uint16_t get_frame_num() const {
        return static_cast<uint16_t>(FRAME_NO & 0x7FF);
    }

    /**
     * @brief 获取高速模式微帧号
     * @return 微帧号（0~7）
     */
    constexpr uint8_t get_micro_frame() const {
        return static_cast<uint8_t>(MICRO_FRAME & 0x07);
    }

    /**
     * @brief 检查是否为高速模式微帧
     * @return true-高速微帧；false-全速/低速帧
     */
    constexpr bool is_highspeed_micro() const {
        return MICRO_FRAME != 0;
    }
};
VALIDATE_R32(R32_USB_FRAME_NO);

/**
 * @brief USB挂起控制寄存器 (R8_USB_SUSPEND)
 * @note 偏移地址: 0x06 | 复位值: 0x00 | 8位有效，高24位保留
 * @details 检测挂起/唤醒状态、PHY线状态，主机模式测试模式配置
 */
struct R32_USB_SUSPEND {
    // 主机模式测试模式[1:0]：00-正常模式，其他为测试模式
    uint32_t RB_USB_SYS_MOD:2;
    // 挂起状态下唤醒标志：1-检测到唤醒信号；0-无（退出挂起自动清0）
    uint32_t RB_USB_WAKEUP_ST:1;
    // 保留位，只读，值为0
    uint32_t RESERVED:1;
    // PHY的Linestate信号[1:0]：实时反映PHY总线线状态
    uint32_t RB_USB_LINESTATE:2;
    // 保留位，只读，值为0
    uint32_t RESERVED2:2;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 设置主机模式测试模式
     * @param test_mod 测试模式值（0~3）
     */
    void set_host_test_mod(uint8_t test_mod) {
        RB_USB_SYS_MOD = test_mod & 0x03;
    }

    /**
     * @brief 检查是否检测到唤醒信号
     * @return true-有唤醒信号；false-无
     */
    constexpr bool is_wakeup_detected() const {
        return RB_USB_WAKEUP_ST == 1;
    }

    /**
     * @brief 获取PHY线状态
     * @return 线状态值（0~3）
     */
    constexpr uint8_t get_phy_linestate() const {
        return static_cast<uint8_t>(RB_USB_LINESTATE & 0x03);
    }
};
VALIDATE_R32(R32_USB_SUSPEND);

/**
 * @brief USB当前速度类型寄存器 (R8_USB_SPEED_TYPE)
 * @note 偏移地址: 0x08 | 复位值: 0x00 | 2位有效，高30位保留
 * @details 只读实际工作速率（区别于USB_CTRL的期望速率），主机/设备模式复用
 */
struct R32_USB_SPEED_TYPE {
    // 实际工作速率[1:0]：00-全速；01-高速；10-低速（只读）
    uint32_t RB_USB_SPEED_TYPE:2;
    // 保留位，只读，值为0
    uint32_t __RESV__:30;

    /**
     * @brief 检查当前是否为高速模式
     * @return true-高速(480Mbps)；false-全速/低速
     */
    constexpr bool is_highspeed() const {
        return RB_USB_SPEED_TYPE == 1;
    }

    /**
     * @brief 检查当前是否为全速模式
     * @return true-全速(12Mbps)；false-高速/低速
     */
    constexpr bool is_fullspeed() const {
        return RB_USB_SPEED_TYPE == 0;
    }

    /**
     * @brief 检查当前是否为低速模式
     * @return true-低速(1.5Mbps)；false-高速/全速
     */
    constexpr bool is_lowspeed() const {
        return RB_USB_SPEED_TYPE == 2;
    }

    /**
     * @brief 获取当前速率类型
     * @return 0-全速；1-高速；2-低速
     */
    constexpr uint8_t get_speed_type() const {
        return static_cast<uint8_t>(RB_USB_SPEED_TYPE & 0x03);
    }
};
VALIDATE_R32(R32_USB_SPEED_TYPE);

/**
 * @brief USB杂项状态寄存器 (R8_USB_MIS_ST)
 * @note 偏移地址: 0x09 | 复位值: 0x28(xx101000b) | 8位有效，高24位保留
 * @details 只读USB各类实时状态：SIE忙闲、FIFO就绪、总线复位、设备插拔等
 */
struct R32_USB_MIS_ST {
    // 主机模式SPLIT包发送允许：1-允许；0-禁止（只读）
    uint32_t RB_UMS_SPLIT_CAN:1;
    // 主机模式设备连接状态：1-已连接；0-未连接（只读）
    uint32_t RB_UMS_DEV_ATTACH:1;
    // USB总线挂起状态：1-挂起；0-正常（只读）
    uint32_t RB_UMS_SUSPEND:1;
    // USB总线复位状态：1-复位中；0-正常（只读）
    uint32_t RB_UMS_BUS_RST:1;
    // 接收FIFO数据就绪：1-FIFO非空；0-FIFO空（只读）
    uint32_t RB_UMS_R_FIFO_RDY:1;
    // SIE协议处理器空闲状态：1-空闲；0-忙（只读）
    uint32_t RB_UMS_SIE_FREE:1;
    // 主机模式SOF包传输中：1-发送SOF；0-空闲/完成（只读）
    uint32_t RB_UMS_SOF_ACT:1;
    // 主机模式SOF包预示：1-即将发送SOF；0-无（只读）
    uint32_t RB_UMS_SOF_PRES:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 检查SIE协议处理器是否空闲
     * @return true-空闲；false-忙（传输中）
     */
    constexpr bool is_sie_free() const {
        return RB_UMS_SIE_FREE == 1;
    }

    /**
     * @brief 检查接收FIFO是否有数据
     * @return true-FIFO非空；false-FIFO空
     */
    constexpr bool is_rx_fifo_ready() const {
        return RB_UMS_R_FIFO_RDY == 1;
    }

    /**
     * @brief 检查USB总线是否处于复位状态
     * @return true-复位中；false-正常
     */
    constexpr bool is_bus_rst() const {
        return RB_UMS_BUS_RST == 1;
    }

    /**
     * @brief 主机模式检查是否有设备连接
     * @return true-设备已连接；false-无设备
     */
    constexpr bool is_dev_attached() const {
        return RB_UMS_DEV_ATTACH == 1;
    }
};
VALIDATE_R32(R32_USB_MIS_ST);

/**
 * @brief USB中断标志寄存器 (R8_USB_INT_FG)
 * @note 偏移地址: 0x0A | 复位值: 0x00 | 8位有效，高24位保留
 * @details 标识USB各类中断触发状态，写1清零，主机/设备模式部分位复用
 */
struct R8_USB_INT_FG {
    // 总线复位/设备检测中断标志：1-触发；0-无（写1清零）
    // 设备:总线复位；主机:设备插拔
    uint32_t RB_UIF_BUS_RST:1;
    // 传输完成中断标志：1-触发；0-无（写1清零，不含SETUP）
    uint32_t RB_UIF_TRANSFER:1;
    // 挂起/唤醒事件中断标志：1-触发；0-无（写1清零）
    uint32_t RB_UIF_SUSPEND:1;
    // 主机模式SOF定时中断标志：1-触发；0-无（写1清零）
    uint32_t RB_UIF_HST_SOF:1;
    // FIFO溢出中断标志：1-触发；0-无（写1清零）
    uint32_t RB_UIF_FIFO_OV:1;
    // SETUP事务完成中断标志：1-触发；0-无（写1清零）
    uint32_t RB_UIF_SETUP_ACT:1;
    // 同步传输收发中断标志：1-触发；0-无（写1清零）
    uint32_t RB_UIF_ISO_ACT:1;
    // 保留位，只读，值为0
    uint32_t RESERVED:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 清零指定中断标志
     * @param flag_mask 标志掩码（对应各位1=清零）
     */
    void clear_flag(uint8_t flag_mask) {
        // 写1清零，直接置位对应位
        uint8_t reg_val = std::bit_cast<uint8_t>(*this);
        reg_val |= (flag_mask & 0x7F);
        *this = std::bit_cast<R8_USB_INT_FG>(reg_val);
    }

    /**
     * @brief 清零所有中断标志
     */
    void clear_all_flag() {
        clear_flag(0x7F);
    }

    /**
     * @brief 检查传输完成中断是否触发
     * @return true-已触发；false-未触发
     */
    constexpr bool is_transfer_flag() const {
        return RB_UIF_TRANSFER == 1;
    }

    /**
     * @brief 检查SETUP事务完成中断是否触发
     * @return true-已触发；false-未触发
     */
    constexpr bool is_setup_flag() const {
        return RB_UIF_SETUP_ACT == 1;
    }

    /**
     * @brief 检查FIFO溢出中断是否触发
     * @return true-已触发；false-未触发
     */
    constexpr bool is_fifo_ov_flag() const {
        return RB_UIF_FIFO_OV == 1;
    }
};
VALIDATE_R32(R8_USB_INT_FG);

/**
 * @brief USB中断状态寄存器 (R8_USB_INT_ST)
 * @note 偏移地址: 0x0B | 复位值: 0x00(00xxxxxxb) | 8位有效，高24位保留
 * @details 只读中断详细状态，主机/设备模式位功能完全复用
 */
struct R32_USB_INT_ST {
    // 主机:应答PID标识[3:0]；设备:当前传输端点号[3:0]（只读）
    uint32_t MASK_UIS_H_RES_ENDP:4;
    // 设备:令牌PID标识[1:0]；主机:保留（只读）00-OUT 01-SOF 10-IN 11-SETUP
    uint32_t MASK_UIS_TOKEN:2;
    // Toggle匹配状态：1-匹配；0-不匹配（只读）
    uint32_t RB_UIS_TOG_OK:1;
    // 设备模式NAK响应状态：1-回应NAK；0-无（只读）
    uint32_t RB_UIS_IS_NAK:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 设备模式获取当前传输端点号
     * @return 端点号（0~15）
     */
    constexpr uint8_t dev_get_ep_id() const {
        return static_cast<uint8_t>(MASK_UIS_H_RES_ENDP & 0x0F);
    }

    /**
     * @brief 设备模式获取当前令牌PID类型
     * @return 0-OUT 1-SOF 2-IN 3-SETUP
     */
    constexpr uint8_t dev_get_token_pid() const {
        return static_cast<uint8_t>(MASK_UIS_TOKEN & 0x03);
    }

    /**
     * @brief 检查Toggle是否匹配
     * @return true-匹配；false-不匹配
     */
    constexpr bool is_tog_ok() const {
        return RB_UIS_TOG_OK == 1;
    }

    /**
     * @brief 设备模式检查是否回应了NAK
     * @return true-已NAK；false-无
     */
    constexpr bool dev_is_nak_resp() const {
        return RB_UIS_IS_NAK == 1;
    }

    /**
     * @brief 主机模式获取设备应答PID
     * @return 应答PID标识（0~15，0=无应答/超时）
     */
    constexpr uint8_t host_get_resp_pid() const {
        return static_cast<uint8_t>(MASK_UIS_H_RES_ENDP & 0x0F);
    }
};
VALIDATE_R32(R32_USB_INT_ST);

/**
 * @brief USB接收长度寄存器 (R16_USB_RX_LEN)
 * @note 偏移地址: 0x0C | 复位值: 0xXXXX | 16位有效，高16位保留
 * @details 只读当前端点实际接收的数据字节数，主机/设备模式复用
 */
struct R32_USB_RX_LEN {
    // 实际接收数据字节数[15:0]：只读，标识最新一次接收的字节数
    uint32_t R16_USB_RX_LEN:16;
    // 保留位，只读，值为0
    uint32_t __RESV__:16;

    /**
     * @brief 获取实际接收的数据字节数
     * @return 字节数（0~65535）
     */
    constexpr uint16_t get_rx_len() const {
        return static_cast<uint16_t>(R16_USB_RX_LEN & 0xFFFF);
    }
};
VALIDATE_R32(R32_USB_RX_LEN);

/******************************************************************************************
 * 22.2.2 设备模式寄存器（基地址 0x40023410）
 ******************************************************************************************/

/**
 * @brief USB端点使能配置寄存器 (R32_UEP_CONFIG)
 * @note 偏移地址: 0x10 | 复位值: 0x00000000
 * @details 配置端点1~15的收发使能，端点0收发始终使能
 */
struct R32_UEP_CONFIG {
    // 保留位，只读，值为0
    uint32_t RESERVED0:1;
    // 端点1~15发送使能[15:1]：1-使能；0-禁止（1位对应1个端点）
    uint32_t RB_UEP_T_EN:15;
    // 保留位，只读，值为0
    uint32_t RESERVED1:1;
    // 端点1~15接收使能[31:17]：1-使能；0-禁止（1位对应1个端点）
    uint32_t RB_UEP_R_EN:15;

    /**
     * @brief 使能指定端点的发送功能
     * @param ep_id 端点号（1~15）
     */
    void enable_ep_tx(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_T_EN |= (1 << (ep_id - 1));
        }
    }

    /**
     * @brief 禁止指定端点的发送功能
     * @param ep_id 端点号（1~15）
     */
    void disable_ep_tx(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_T_EN &= ~(1 << (ep_id - 1));
        }
    }

    /**
     * @brief 使能指定端点的接收功能
     * @param ep_id 端点号（1~15）
     */
    void enable_ep_rx(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_R_EN |= (1 << (ep_id - 1));
        }
    }

    /**
     * @brief 禁止指定端点的接收功能
     * @param ep_id 端点号（1~15）
     */
    void disable_ep_rx(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_R_EN &= ~(1 << (ep_id - 1));
        }
    }

    /**
     * @brief 检查指定端点发送是否使能
     * @param ep_id 端点号（1~15）
     * @return true-使能；false-禁止
     */
    constexpr bool is_ep_tx_en(uint8_t ep_id) const {
        return (ep_id >=1 && ep_id <=15) ? ((RB_UEP_T_EN & (1 << (ep_id - 1))) != 0) : false;
    }

    /**
     * @brief 检查指定端点接收是否使能
     * @param ep_id 端点号（1~15）
     * @return true-使能；false-禁止
     */
    constexpr bool is_ep_rx_en(uint8_t ep_id) const {
        return (ep_id >=1 && ep_id <=15) ? ((RB_UEP_R_EN & (1 << (ep_id - 1))) != 0) : false;
    }
};
VALIDATE_R32(R32_UEP_CONFIG);

/**
 * @brief USB端点类型配置寄存器 (R32_UEP_TYPE)
 * @note 偏移地址: 0x14 | 复位值: 0x00000000
 * @details 配置端点1~15的收发传输类型，1=同步传输 0=非同步传输
 */
struct R32_UEP_TYPE {
    // 保留位，只读，值为0
    uint32_t RESERVED0:1;
    // 端点1~15IN方向类型[15:1]：1-同步；0-非同步（1位对应1个端点）
    uint32_t RB_UEP_T_TYPE:15;
    // 保留位，只读，值为0
    uint32_t RESERVED1:1;
    // 端点1~15OUT方向类型[31:17]：1-同步；0-非同步（1位对应1个端点）
    uint32_t RB_UEP_R_TYPE:15;

    /**
     * @brief 设置指定端点IN方向为同步传输
     * @param ep_id 端点号（1~15）
     */
    void set_ep_tx_iso(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_T_TYPE |= (1 << (ep_id - 1));
        }
    }

    /**
     * @brief 设置指定端点IN方向为非同步传输
     * @param ep_id 端点号（1~15）
     */
    void set_ep_tx_noniso(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_T_TYPE &= ~(1 << (ep_id - 1));
        }
    }

    /**
     * @brief 设置指定端点OUT方向为同步传输
     * @param ep_id 端点号（1~15）
     */
    void set_ep_rx_iso(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_R_TYPE |= (1 << (ep_id - 1));
        }
    }

    /**
     * @brief 设置指定端点OUT方向为非同步传输
     * @param ep_id 端点号（1~15）
     */
    void set_ep_rx_noniso(uint8_t ep_id) {
        if (ep_id >=1 && ep_id <=15) {
            RB_UEP_R_TYPE &= ~(1 << (ep_id - 1));
        }
    }

    /**
     * @brief 检查指定端点IN方向是否为同步传输
     * @param ep_id 端点号（1~15）
     * @return true-同步；false-非同步
     */
    constexpr bool is_ep_tx_iso(uint8_t ep_id) const {
        return (ep_id >=1 && ep_id <=15) ? ((RB_UEP_T_TYPE & (1 << (ep_id - 1))) != 0) : false;
    }
};
VALIDATE_R32(R32_UEP_TYPE);

/**
 * @brief USB端点缓冲区模式寄存器 (R32_UEP_BUF_MOD)
 * @note 偏移地址: 0x18 | 复位值: 0x00000000
 * @details 配置端点单/双缓冲模式、同步端点缓冲区自动切换模式
 */
struct R32_UEP_BUF_MOD {
    // 端点0~15缓冲区模式[15:0]：1-双缓冲；0-单缓冲（1位对应1个端点）
    uint32_t RB_UEP_BUF_MOD:16;
    // 同步端点0~15缓冲区自动切换[31:16]：1-使能；0-禁止（1位对应1个端点）
    uint32_t RB_UEP_ISO_BUF_MOD:16;

    /**
     * @brief 设置指定端点为双缓冲模式（仅单方向传输有效）
     * @param ep_id 端点号（0~15）
     */
    void set_ep_double_buf(uint8_t ep_id) {
        if (ep_id >=0 && ep_id <=15) {
            RB_UEP_BUF_MOD |= (1 << ep_id);
        }
    }

    /**
     * @brief 设置指定端点为单缓冲模式
     * @param ep_id 端点号（0~15）
     */
    void set_ep_single_buf(uint8_t ep_id) {
        if (ep_id >=0 && ep_id <=15) {
            RB_UEP_BUF_MOD &= ~(1 << ep_id);
        }
    }

    /**
     * @brief 使能同步端点缓冲区自动切换功能
     * @param ep_id 同步端点号（0~15）
     */
    void enable_iso_buf_auto(uint8_t ep_id) {
        if (ep_id >=0 && ep_id <=15) {
            RB_UEP_ISO_BUF_MOD |= (1 << ep_id);
        }
    }

    /**
     * @brief 检查指定端点是否为双缓冲模式
     * @param ep_id 端点号（0~15）
     * @return true-双缓冲；false-单缓冲
     */
    constexpr bool is_ep_double_buf(uint8_t ep_id) const {
        return (ep_id >=0 && ep_id <=15) ? ((RB_UEP_BUF_MOD & (1 << ep_id)) != 0) : false;
    }
};
VALIDATE_R32(R32_UEP_BUF_MOD);

/**
 * @brief 端点0缓冲区DMA地址寄存器 (R32_UEP0_DMA)
 * @note 偏移地址: 0x1C | 复位值: 0xXXXXXXXX
 * @details 配置端点0收发共用缓冲区起始地址，**必须4字节对齐**
 */
struct R32_UEP0_DMA {
    // 端点0缓冲区起始地址[31:0]：4字节对齐，收发共用
    uint32_t R32_UEP0_DMA:32;

    /**
     * @brief 设置端点0缓冲区DMA地址（自动4字节对齐）
     * @param addr 缓冲区起始地址
     */
    void set_dma_addr(uint32_t addr) {
        R32_UEP0_DMA = addr & 0xFFFFFFFC; // 屏蔽低2位，强制4字节对齐
    }

    /**
     * @brief 获取端点0缓冲区DMA地址
     * @return 32位对齐地址
     */
    constexpr uint32_t get_dma_addr() const {
        return R32_UEP0_DMA;
    }
};
VALIDATE_R32(R32_UEP0_DMA);

/**
 * @brief 端点n接收缓冲区DMA地址寄存器 (R32_UEPn_RX_DMA)
 * @note 偏移地址: 0x20+4*(n-1) | 复位值: 0xXXXXXXXX (n=1~15)
 * @details 配置端点1~15接收缓冲区起始地址，**必须4字节对齐**
 */
struct R32_UEPn_RX_DMA {
    // 端点n接收缓冲区起始地址[31:0]：4字节对齐
    uint32_t R32_UEPn_RX_DMA:32;

    /**
     * @brief 设置接收缓冲区DMA地址（自动4字节对齐）
     * @param addr 缓冲区起始地址
     */
    void set_dma_addr(uint32_t addr) {
        R32_UEPn_RX_DMA = addr & 0xFFFFFFFC;
    }

    /**
     * @brief 获取接收缓冲区DMA地址
     * @return 32位对齐地址
     */
    constexpr uint32_t get_dma_addr() const {
        return R32_UEPn_RX_DMA;
    }
};
VALIDATE_R32(R32_UEPn_RX_DMA);

/**
 * @brief 端点n发送缓冲区DMA地址寄存器 (R32_UEPn_TX_DMA)
 * @note 偏移地址: 0x5C+4*(n-1) | 复位值: 0xXXXXXXXX (n=1~15)
 * @details 配置端点1~15发送缓冲区起始地址
 */
struct R32_UEPn_TX_DMA {
    // 端点n发送缓冲区起始地址[31:0]
    uint32_t R32_UEPn_TX_DMA:32;

    /**
     * @brief 设置发送缓冲区DMA地址
     * @param addr 缓冲区起始地址
     */
    void set_dma_addr(uint32_t addr) {
        R32_UEPn_TX_DMA = addr;
    }

    /**
     * @brief 获取发送缓冲区DMA地址
     * @return 32位地址
     */
    constexpr uint32_t get_dma_addr() const {
        return R32_UEPn_TX_DMA;
    }
};
VALIDATE_R32(R32_UEPn_TX_DMA);

/**
 * @brief 端点n最大包长度寄存器 (R16_UEPn_MAX_LEN)
 * @note 偏移地址: 0x98+4*n | 复位值: 0xXXXX (n=0~15) | 11位有效
 * @details 配置端点最大接收包长度，超出部分将被丢弃
 */
struct R32_UEPn_MAX_LEN {
    // 最大接收包长度[10:0]：0~2047（端点0最大64，其他最大1024）
    uint32_t UEPn_MAX_LEN:11;
    // 保留位，只读，值为0
    uint32_t __RESV__:21;

    /**
     * @brief 设置端点最大接收包长度
     * @param len 长度（端点0:0~64；其他:0~1024）
     */
    void set_max_len(uint16_t len) {
        UEPn_MAX_LEN = len & 0x7FF;
    }

    /**
     * @brief 获取端点最大接收包长度
     * @return 长度值（0~2047）
     */
    constexpr uint16_t get_max_len() const {
        return static_cast<uint16_t>(UEPn_MAX_LEN & 0x7FF);
    }
};
VALIDATE_R32(R32_UEPn_MAX_LEN);

/**
 * @brief 端点n发送长度寄存器 (R16_UEPn_T_LEN)
 * @note 偏移地址: 0xD8+4*n | 复位值: 0xXXXX (n=0~15) | 11位有效
 * @details 配置端点准备发送的数据字节数，端点0低7位有效
 */
struct R32_UEPn_T_LEN {
    // 发送数据字节数[10:0]：端点0低7位有效（0~63），其他0~1023
    uint32_t UEPn_T_LEN:11;
    // 保留位，只读，值为0
    uint32_t __RESV__:21;

    /**
     * @brief 设置端点发送数据长度
     * @param len 长度；ep0:0~63；其他:0~1023
     * @param is_ep0 是否为端点0
     */
    void set_tx_len(uint16_t len, bool is_ep0 = false) {
        if (is_ep0) {
            UEPn_T_LEN = len & 0x3F; // 端点0低7位有效
        } else {
            UEPn_T_LEN = len & 0x3FF;
        }
    }

    /**
     * @brief 获取端点发送数据长度
     * @return 长度值
     */
    constexpr uint16_t get_tx_len() const {
        return static_cast<uint16_t>(UEPn_T_LEN & 0x7FF);
    }
};
VALIDATE_R32(R32_UEPn_T_LEN);

/**
 * @brief 端点n发送控制寄存器 (R8_UEPn_TX_CTRL)
 * @note 偏移地址: 0xDA+4*n | 复位值: 0x00 (n=0~15) | 8位有效，高24位保留
 * @details 配置端点发送同步触发位、自动翻转、IN事务响应方式
 */
struct R32_UEPn_TX_CTRL {
    // IN事务响应控制[1:0]：00-就绪等ACK 10-应答NAK 11-应答STALL
    uint32_t MASK_UEP_T_RES:2;
    // 保留位，只读，值为0
    uint32_t RESERVED:1;
    // 发送同步触发位[1:0]：00-DATA0 01-DATA1 10-DATA2 11-MDATA
    uint32_t MASK_UEP_T_TOG:2;
    // 同步触发位自动翻转：1-使能（非同步翻转/同步自减）0-禁止（端点0保留）
    uint32_t RB_UEP_T_TOG_AUTO:1;
    // 保留位，只读，值为0
    uint32_t RESERVED2:2;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    // 发送响应枚举
    enum class TxResp : uint8_t {
        ACK_READY = 0x00, // 数据就绪，期望ACK
        NAK_BUSY  = 0x02, // 应答NAK或忙
        STALL_ERR = 0x03  // 应答STALL或错误
    };

    // 发送同步触发位枚举
    enum class TxTog : uint8_t {
        DATA0 = 0x00,
        DATA1 = 0x01,
        DATA2 = 0x02,
        MDATA = 0x03
    };

    /**
     * @brief 设置IN事务响应方式
     * @param resp 响应类型
     */
    void set_tx_resp(TxResp resp) {
        MASK_UEP_T_RES = static_cast<uint8_t>(resp);
    }

    /**
     * @brief 设置发送同步触发位
     * @param tog 触发位类型
     */
    void set_tx_tog(TxTog tog) {
        MASK_UEP_T_TOG = static_cast<uint8_t>(tog);
    }

    /**
     * @brief 使能发送同步触发位自动翻转（非端点0）
     */
    void enable_tog_auto() {
        RB_UEP_T_TOG_AUTO = 1;
    }

    /**
     * @brief 禁止发送同步触发位自动翻转
     */
    void disable_tog_auto() {
        RB_UEP_T_TOG_AUTO = 0;
    }

    /**
     * @brief 检查是否使能自动翻转
     * @return true-使能；false-禁止
     */
    constexpr bool is_tog_auto_en() const {
        return RB_UEP_T_TOG_AUTO == 1;
    }
};
VALIDATE_R32(R32_UEPn_TX_CTRL);

/**
 * @brief 端点n接收控制寄存器 (R8_UEPn_RX_CTRL)
 * @note 偏移地址: 0xDB+4*n | 复位值: 0x00 (n=0~15) | 8位有效，高24位保留
 * @details 配置端点接收同步触发位、自动翻转、OUT事务响应方式
 */
struct R32_UEPn_RX_CTRL {
    // OUT事务响应控制[1:0]：00-就绪等ACK 01-NYET 10-NAK 11-STALL
    uint32_t MASK_UEP_R_RES:2;
    // 保留位，只读，值为0
    uint32_t RESERVED:1;
    // 接收同步触发位[1:0]：00-DATA0 01-DATA1 10-DATA2 11-MDATA（同步传输无效）
    uint32_t MASK_UEP_R_TOG:2;
    // 同步触发位自动翻转：1-使能 0-禁止（端点0保留，只读）
    uint32_t RB_UEP_R_TOG_AUTO:1;
    // 保留位，只读，值为0
    uint32_t RESERVED2:2;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    // 接收响应枚举
    enum class RxResp : uint8_t {
        ACK_READY = 0x00, // 数据就绪，期望ACK
        NYET      = 0x01, // 应答NYET
        NAK_BUSY  = 0x02, // 应答NAK或忙
        STALL_ERR = 0x03  // 应答STALL或错误
    };

    // 接收同步触发位枚举
    enum class RxTog : uint8_t {
        DATA0 = 0x00,
        DATA1 = 0x01,
        DATA2 = 0x02,
        MDATA = 0x03
    };

    /**
     * @brief 设置OUT事务响应方式
     * @param resp 响应类型
     */
    void set_rx_resp(RxResp resp) {
        MASK_UEP_R_RES = static_cast<uint8_t>(resp);
    }

    /**
     * @brief 设置接收同步触发位
     * @param tog 触发位类型
     */
    void set_rx_tog(RxTog tog) {
        MASK_UEP_R_TOG = static_cast<uint8_t>(tog);
    }

    /**
     * @brief 检查接收同步触发位是否自动翻转
     * @return true-使能；false-禁止
     */
    constexpr bool is_tog_auto_en() const {
        return RB_UEP_R_TOG_AUTO == 1;
    }
};
VALIDATE_R32(R32_UEPn_RX_CTRL);

/******************************************************************************************
 * 22.2.3 主机模式寄存器（基地址 0x40023401/0x40023410）
 ******************************************************************************************/

/**
 * @brief USB主机控制寄存器 (R8_UHOST_CTRL)
 * @note 偏移地址: 0x01 | 复位值: 0x00 | 8位有效，高24位保留
 * @details 主机模式核心控制寄存器，含远程唤醒、端口控制等
 */
struct R32_UHOST_CTRL {
    // 主机模式控制位[7:0]：含远程唤醒、端口使能等（手册未详细定义位段，预留）
    uint32_t UHOST_CTRL:8;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 触发远程唤醒（拉高再拉低对应位）
     * @param wakeup_bit 远程唤醒位（手册指定位）
     */
    void trigger_remote_wakeup(uint8_t wakeup_bit) {
        UHOST_CTRL |= (1 << wakeup_bit);
        UHOST_CTRL &= ~(1 << wakeup_bit);
    }

    /**
     * @brief 设置主机控制位
     * @param mask 控制位掩码
     */
    void set_ctrl(uint8_t mask) {
        UHOST_CTRL |= mask;
    }

    /**
     * @brief 清除主机控制位
     * @param mask 控制位掩码
     */
    void clear_ctrl(uint8_t mask) {
        UHOST_CTRL &= ~mask;
    }
};
VALIDATE_R32(R32_UHOST_CTRL);

/******************************************************************************************
 * 寄存器基地址与映射定义（工程直接使用）
 ******************************************************************************************/
static constexpr uint32_t USBHS_BASE            = 0x40023400;
// 全局寄存器映射
#define USBHS_CTRL            (*((volatile R32_USB_CTRL*)(USBHS_BASE + 0x00)))
#define USBHS_UHOST_CTRL      (*((volatile R32_UHOST_CTRL*)(USBHS_BASE + 0x01)))
#define USBHS_INT_EN          (*((volatile R8_USB_INT_EN*)(USBHS_BASE + 0x02)))
#define USBHS_DEV_AD          (*((volatile R32_USB_DEV_AD*)(USBHS_BASE + 0x03)))
#define USBHS_FRAME_NO        (*((volatile R32_USB_FRAME_NO*)(USBHS_BASE + 0x04)))
#define USBHS_SUSPEND         (*((volatile R32_USB_SUSPEND*)(USBHS_BASE + 0x06)))
#define USBHS_SPEED_TYPE      (*((volatile R32_USB_SPEED_TYPE*)(USBHS_BASE + 0x08)))
#define USBHS_MIS_ST          (*((volatile R32_USB_MIS_ST*)(USBHS_BASE + 0x09)))
#define USBHS_INT_FG          (*((volatile R8_USB_INT_FG*)(USBHS_BASE + 0x0A)))
#define USBHS_INT_ST          (*((volatile R32_USB_INT_ST*)(USBHS_BASE + 0x0B)))
#define USBHS_RX_LEN          (*((volatile R32_USB_RX_LEN*)(USBHS_BASE + 0x0C)))

// 设备模式寄存器映射
#define USBHS_UEP_CONFIG      (*((volatile R32_UEP_CONFIG*)(USBHS_BASE + 0x10)))
#define USBHS_UEP_TYPE        (*((volatile R32_UEP_TYPE*)(USBHS_BASE + 0x14)))
#define USBHS_UEP_BUF_MOD     (*((volatile R32_UEP_BUF_MOD*)(USBHS_BASE + 0x18)))
#define USBHS_UEP0_DMA        (*((volatile R32_UEP0_DMA*)(USBHS_BASE + 0x1C)))
// 端点n(1~15)RX/TX DMA地址映射宏
#define USBHS_UEPn_RX_DMA(n)  (*((volatile R32_UEPn_RX_DMA*)(USBHS_BASE + 0x20 + 4*((n)-1))))
#define USBHS_UEPn_TX_DMA(n)  (*((volatile R32_UEPn_TX_DMA*)(USBHS_BASE + 0x5C + 4*((n)-1))))
// 端点n(0~15)最大长度/发送长度/收发控制映射宏
#define USBHS_UEPn_MAX_LEN(n) (*((volatile R32_UEPn_MAX_LEN*)(USBHS_BASE + 0x98 + 4*(n))))
#define USBHS_UEPn_T_LEN(n)   (*((volatile R32_UEPn_T_LEN*)(USBHS_BASE + 0xD8 + 4*(n))))
}