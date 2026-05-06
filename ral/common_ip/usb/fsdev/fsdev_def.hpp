#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


namespace ymd::ral::usb::fsdev{


// 寄存器大小校验宏，确保结构体为32位（CH32寄存器按32位访问）
#define VALIDATE_R32(T) static_assert(sizeof(T) == sizeof(uint32_t), #T " must be 32-bit")

/******************************************************************************************
 * 21.3 通用类寄存器（基地址 0x40005C00）
 ******************************************************************************************/

/**
 * @brief USB控制寄存器 (USBD_CNTR)
 * @note 偏移地址: 0x40 | 复位值: 0x00000003
 * @details 控制USB模块中断使能、低功耗、复位、唤醒等核心功能
 */
struct R32_USBD_CNTR {
    // 强制USB复位控制位: 1-强制复位USB模块; 0-清除USB复位
    uint32_t FRES:1;
    // 断电模式控制位: 1-进入断电模式(关闭USB模块); 0-退出断电模式
    uint32_t PDWN:1;
    // 低功耗模式控制位: 1-低功耗模式(挂起时降功耗); 0-非低功耗模式
    uint32_t LPMODE:1;
    // 屏蔽挂起检测控制位: 1-屏蔽挂起检测; 0-开启挂起检测
    uint32_t FSUSP:1;
    // 唤醒请求控制位: 1-输出唤醒信号(1~15ms); 0-空闲(仅FSUSP=1时可置1)
    uint32_t RESUME:1;
    // 保留位，只读，值为0
    uint32_t __RESV1__:3;
    // 定时帧首丢失中断使能位: 1-使能ESOF中断; 0-禁止ESOF中断
    uint32_t ESOFM:1;
    // 帧首(SOF)中断使能位: 1-使能SOF中断; 0-禁止SOF中断
    uint32_t SOFM:1;
    // USB复位中断使能位: 1-使能复位中断; 0-禁止复位中断
    uint32_t RSTM:1;
    // 挂起(SUSP)中断使能位: 1-使能挂起中断; 0-禁止挂起中断
    uint32_t SUSPM:1;
    // 唤醒中断使能位: 1-使能唤醒中断; 0-禁止唤醒中断
    uint32_t WKUPM:1;
    // 出错中断使能位: 1-使能出错中断; 0-禁止出错中断
    uint32_t ERRM:1;
    // 分组缓冲区溢出中断使能位: 1-使能PMAOVR中断; 0-禁止PMAOVR中断
    uint32_t PMAOVRM:1;
    // 正确传输中断使能位: 1-使能CTR中断; 0-禁止CTR中断
    uint32_t CTRM:1;
    // 保留位，只读，值为0
    uint32_t __RESV2__:16;

    /**
     * @brief 使能所有USB中断
     * @return 无返回值，直接修改寄存器
     */
    constexpr void enable_all_irq() {
        auto & self = (*this);
        self.CTRM = 1; self.PMAOVRM = 1; self.ERRM = 1;
        self.WKUPM = 1; self.SUSPM = 1; self.RSTM = 1;
        self.SOFM = 1; self.ESOFM = 1;
    }

    /**
     * @brief 进入USB低功耗挂起模式
     * @return 无返回值，直接修改寄存器
     */
    constexpr void enter_suspend_lp() {
        auto & self = (*this);
        self.FSUSP = 1;
        self.LPMODE = 1;
    }

    /**
     * @brief 触发USB唤醒序列（1~15ms后需软件清0 RESUME）
     * @return 无返回值，直接修改寄存器
     */
    constexpr void trigger_wakeup() {
        auto & self = (*this);
        if (self.FSUSP) self.RESUME = 1;
    }

    /**
     * @brief 检查USB是否处于强制复位状态
     * @return true-强制复位中; false-未复位
     */
    constexpr bool is_forced_reset() const noexcept {
        return FRES == 1;
    }
};
VALIDATE_R32(R32_USBD_CNTR);

/**
 * @brief USB中断状态寄存器 (USBD_ISTR)
 * @note 偏移地址: 0x44 | 复位值: 0x00000000
 * @details 标识USB各类中断/事件的状态，部分位为写0清除、写1无效
 */
struct R32_USBD_ISTR {
    // 端点号[3:0]: 硬件写入优先级最高的中断端点号，端点0优先级最高
    uint32_t EP_ID:4;
    // 传输方向标志: 0-IN事务(设备→主机); 1-OUT事务(主机→设备)
    uint32_t DIR:1;
    // 保留位，只读，值为0
    uint32_t __RESV1__:3;
    // 定时帧首丢失标志: 1-未按时收到SOF包; 0-正常(写0清除)
    uint32_t ESOF:1;
    // 帧首(SOF)标志: 1-检测到SOF包; 0-无(写0清除)
    uint32_t SOF:1;
    // USB复位标志: 1-检测到总线/强制复位; 0-无(写0清除)
    uint32_t RST:1;
    // 总线挂起标志: 1-总线3ms无活动; 0-无(写0清除)
    uint32_t SUSP:1;
    // 唤醒信号标志: 1-检测到唤醒信号; 0-无(写0清除)
    uint32_t WKUP:1;
    // 出错标志: 1-发生CRC/位填充/帧格式等错误; 0-无(写0清除)
    uint32_t ERR:1;
    // 分组缓冲区溢出标志: 1-缓冲区溢出; 0-无(写0清除)
    uint32_t PMAOVR:1;
    // 正确传输状态标志: 1-端点完成正确传输; 0-无(只读)
    uint32_t CTR:1;
    // 保留位，只读，值为0
    uint32_t __RESV2__:16;

    /**
     * @brief 清除所有可写清除的中断标志
     * @return 无返回值，直接修改寄存器
     */
    constexpr void clear_all_flag() {
        auto & self = (*this);
        self.ESOF = 0; self.SOF = 0; self.RST = 0;
        self.SUSP = 0; self.WKUP = 0; self.ERR = 0;
        self.PMAOVR = 0;
    }

    /**
     * @brief 检查是否存在高优先级中断（同步/双缓冲批量传输）
     * @param ep_type 端点类型(0-批量,1-控制,2-同步,3-中断)
     * @return true-高优先级中断; false-非高优先级
     */
    constexpr bool is_high_prio_irq(uint8_t ep_type) const noexcept {
        return CTR == 1 && (ep_type == 0 || ep_type == 2);
    }

    /**
     * @brief 获取中断对应的端点号
     * @return 端点号(0~7)
     */
    constexpr uint8_t get_ep_id() const noexcept {
        return static_cast<uint8_t>(EP_ID & 0x07);
    }

    /**
     * @brief 检查是否为OUT事务中断
     * @return true-OUT事务; false-IN事务
     */
    constexpr bool is_out_trans() const noexcept {
        return DIR == 1 && CTR == 1;
    }
};
VALIDATE_R32(R32_USBD_ISTR);

/**
 * @brief USB帧编号寄存器 (USBD_FNR)
 * @note 偏移地址: 0x48 | 复位值: 0x00000XXX
 * @details 存储USB帧编号、数据线电平、SOF包锁定状态等
 */
struct R32_USBD_FNR {
    // 帧编号[10:0]: 最新收到的SOF包11位帧编号，主机每帧自增
    uint32_t FN:11;
    // 帧首丢失标志位[1:0]: 丢失的SOF包数目，收到SOF包自动清除
    uint32_t LSOF:2;
    // SOF包计数停止锁定位: 1-锁定(连续收到2个SOF包); 0-未锁定
    uint32_t LCK:1;
    // D-数据线电平状态: 只读，实时反映D-引脚电平
    uint32_t RXDM:1;
    // D+数据线电平状态: 只读，实时反映D+引脚电平
    uint32_t RXDP:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:16;

    /**
     * @brief 获取当前USB帧编号
     * @return 帧编号(0~2047)
     */
    constexpr uint16_t get_frame_num() const noexcept {
        return static_cast<uint16_t>(FN & 0x7FF);
    }

    /**
     * @brief 检查SOF计数是否锁定
     * @return true-锁定; false-未锁定
     */
    constexpr bool is_lck() const noexcept {
        return LCK == 1;
    }

    /**
     * @brief 检查USB总线是否处于复位状态（D+D-均为0且>10ms）
     * @return true-总线复位中; false-非复位
     */
    constexpr bool is_bus_reset() const noexcept {
        return RXDP == 0 && RXDM == 0;
    }

    /**
     * @brief 获取丢失的SOF包数目
     * @return 丢失数目(0~3)
     */
    constexpr uint8_t get_lost_sof() const noexcept {
        return static_cast<uint8_t>(LSOF & 0x03);
    }
};
VALIDATE_R32(R32_USBD_FNR);

/**
 * @brief USB设备地址寄存器 (USBD_DADDR)
 * @note 偏移地址: 0x4C | 复位值: 0x00000000
 * @details 配置USB设备地址和功能使能位
 */
struct R32_USBD_DADDR {
    // USB设备地址[6:0]: 主机枚举分配的地址，范围0~127
    uint32_t ADD:7;
    // USB功能使能位: 1-使能USB设备功能; 0-关闭USB设备功能
    uint32_t EF:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:24;

    /**
     * @brief 设置USB设备地址并使能设备
     * @param addr 设备地址(0~127)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_dev_addr(uint8_t addr) {
        auto & self = (*this);
        self.ADD = addr & 0x7F;
        self.EF = 1;
    }

    /**
     * @brief 禁用USB设备功能
     * @return 无返回值，直接修改寄存器
     */
    constexpr void disable_usb() {
        auto & self = (*this);
        self.EF = 0;
    }

    /**
     * @brief 检查USB设备是否已使能
     * @return true-已使能; false-未使能
     */
    constexpr bool is_usb_enable() const noexcept {
        return EF == 1;
    }

    /**
     * @brief 获取当前USB设备地址
     * @return 设备地址(0~127)
     */
    constexpr uint8_t get_dev_addr() const noexcept {
        return static_cast<uint8_t>(ADD & 0x7F);
    }
};
VALIDATE_R32(R32_USBD_DADDR);

/**
 * @brief USB分组缓冲区描述表地址寄存器 (USBD_BTABLE)
 * @note 偏移地址: 0x50 | 复位值: 0x00000000
 * @details 配置缓冲区描述表的基地址，按8字节对齐（最低3位为0）
 */
struct R32_USBD_BTABLE {
    // 保留位，只读，值为0（地址按8字节对齐，最低3位必须为0）
    uint32_t __RESV1__:3;
    // 缓冲表基地址[15:3]: 描述表在SRAM中的基地址，范围0x40006000~0x400061FF
    uint32_t BTABLE:13;
    // 保留位，只读，值为0
    uint32_t __RESV2__:16;

    /**
     * @brief 设置缓冲区描述表基地址（自动按8字节对齐）
     * @param addr 基地址(必须为0x40006000~0x400061FF且8字节对齐)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_btable_addr(uint32_t addr) {
        auto & self = (*this);
        self.BTABLE = (addr & 0x1FF8) >> 3; // 屏蔽低3位，右移3位存入
    }

    /**
     * @brief 获取缓冲区描述表实际基地址
     * @return 32位基地址
     */
    constexpr uint32_t get_btable_addr() const noexcept {
        return static_cast<uint32_t>(BTABLE << 3) | 0x40006000;
    }

    /**
     * @brief 检查地址是否按8字节对齐
     * @return true-对齐; false-未对齐
     */
    constexpr bool is_addr_align() const noexcept {
        return (get_btable_addr() & 0x07) == 0;
    }
};
VALIDATE_R32(R32_USBD_BTABLE);

/******************************************************************************************
 * 21.3 端点类寄存器（基地址 0x40005C00，偏移0x00~0x1C，共8个端点EP0~EP7）
 ******************************************************************************************/

/**
 * @brief USB端点配置寄存器 (USBD_EPRx)
 * @note 偏移地址: 0x00+4*x | 复位值: 0x00000000 (x=0~7，对应EP0~EP7)
 * @details 配置端点类型、收发状态、数据PID、中断标志等，双向端点配置核心寄存器
 */
struct R32_USBD_EPRx {
    // 端点地址域[3:0]: 配置端点号，范围0~7
    uint32_t EA:4;
    // 发送状态位[1:0]: 00-禁用;01-STALL;10-NAK;11-ACK(写0无效，写1翻转)
    uint32_t STAT_TX:2;
    // 发送数据PID: 0-DATA0;1-DATA1(写0无效，写1翻转)
    uint32_t DTOG_TX:1;
    // 正确发送标志位: 1-完成IN事务;0-无(写0清除，写1无效)
    uint32_t CTR_TX:1;
    // 端点特殊类型控制位: 批量-双缓冲;控制-STATUS_OUT;同步/中断-未使用
    uint32_t EP_KIND:1;
    // 传输端点类型[1:0]: 00-批量;01-控制;10-同步;11-中断
    uint32_t EP_TYPE:2;
    // SETUP事务标志: 1-SETUP事务;0-OUT事务(只读，CTR_RX=0时更新)
    uint32_t SETUP:1;
    // 接收状态位[1:0]: 00-禁用;01-STALL;10-NAK;11-ACK(写0无效，写1翻转)
    uint32_t STAT_RX:2;
    // 接收数据PID: 0-DATA0;1-DATA1(写0无效，写1翻转)
    uint32_t DTOG_RX:1;
    // 正确接收标志位: 1-完成OUT/SETUP事务;0-无(写0清除，写1无效)
    uint32_t CTR_RX:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:16;

    // 端点状态枚举
    enum class EpState : uint8_t {
        DISABLED = 0x00, // 禁用
        STALL    = 0x01, // 应答STALL
        NAK      = 0x02, // 应答NAK
        ACK      = 0x03  // 应答ACK
    };

    // 端点类型枚举
    enum class EpType : uint8_t {
        BULK     = 0x00, // 批量端点
        CONTROL  = 0x01, // 控制端点
        ISO      = 0x02, // 同步端点
        INTERRUPT= 0x03  // 中断端点
    };

    /**
     * @brief 初始化端点（类型+地址+使能）
     * @param type 端点类型
     * @param ep_id 端点号(0~7)
     * @param is_double_buf 是否双缓冲(仅批量端点有效)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void init_ep(EpType type, uint8_t ep_id, bool is_double_buf = false) {
        auto & self = (*this);
        self.EA = ep_id & 0x07;
        self.EP_TYPE = static_cast<uint8_t>(type);
        self.EP_KIND = (type == EpType::BULK && is_double_buf) ? 1 : 0;
        self.STAT_RX = static_cast<uint8_t>(EpState::ACK);
        self.STAT_TX = static_cast<uint8_t>(EpState::ACK);
    }

    /**
     * @brief 清除端点收发中断标志
     * @return 无返回值，直接修改寄存器
     */
    constexpr void clear_ep_flag() {
        auto & self = (*this);
        self.CTR_RX = 0;
        self.CTR_TX = 0;
    }

    /**
     * @brief 设置端点接收状态
     * @param state 接收状态
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_rx_state(EpState state) {
        auto & self = (*this);
        self.STAT_RX = static_cast<uint8_t>(state);
    }

    /**
     * @brief 设置端点发送状态
     * @param state 发送状态
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_tx_state(EpState state) {
        auto & self = (*this);
        self.STAT_TX = static_cast<uint8_t>(state);
    }

    /**
     * @brief 翻转接收数据PID
     * @return 无返回值，直接修改寄存器
     */
    constexpr void toggle_dog_rx() {
        auto & self = (*this);
        self.DTOG_RX = 1; // 写1翻转
    }

    /**
     * @brief 翻转发送数据PID
     * @return 无返回值，直接修改寄存器
     */
    constexpr void toggle_dog_tx() {
        auto & self = (*this);
        self.DTOG_TX = 1; // 写1翻转
    }

    /**
     * @brief 检查是否为SETUP事务
     * @return true-SETUP事务; false-OUT事务
     */
    constexpr bool is_setup_trans() {
        return CTR_RX == 1 && SETUP == 1;
    }

    /**
     * @brief 检查端点是否为双缓冲模式
     * @return true-双缓冲; false-单缓冲
     */
    constexpr bool is_double_buf() {
        return EP_KIND == 1 && EP_TYPE == static_cast<uint8_t>(EpType::BULK);
    }
};
VALIDATE_R32(R32_USBD_EPRx);

/******************************************************************************************
 * 21.3 缓冲区描述类寄存器（基地址 0x40006000 + USBD_BTABLE）
 ******************************************************************************************/

/**
 * @brief 端点发送缓冲区地址寄存器 (USBD_ADDRx_TX)
 * @note 偏移地址: BTABLE + 16*x + 0 | 复位值: 0x00000000 (x=0~7)
 * @details 配置端点发送数据缓冲区起始地址，按2字节对齐（最低1位为0）
 */
struct R32_USBD_ADDRx_TX {
    // 保留位，只读，值为0（地址按2字节对齐，最低1位必须为0）
    uint32_t __RESV1__:1;
    // 发送缓冲区地址[15:1]: 发送数据缓冲区起始地址，范围0x40006000~0x400061FF
    uint32_t ADDRx_TX:15;
    // 保留位，只读，值为0
    uint32_t __RESV2__:16;

    /**
     * @brief 设置发送缓冲区地址（自动按2字节对齐）
     * @param addr 缓冲区地址(2字节对齐，0x40006000~0x400061FF)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_tx_buf_addr(uint32_t addr) {
        auto & self = (*this);
        self.ADDRx_TX = (addr & 0x1FFE) >> 1; // 屏蔽低1位，右移1位存入
    }

    /**
     * @brief 获取发送缓冲区实际地址
     * @return 32位缓冲区地址
     */
    constexpr uint32_t get_tx_buf_addr() const noexcept {
        return static_cast<uint32_t>(ADDRx_TX << 1) | 0x40006000;
    }
};
VALIDATE_R32(R32_USBD_ADDRx_TX);

/**
 * @brief 端点发送数据字节数寄存器 (USBD_COUNTx_TX)
 * @note 偏移地址: BTABLE + 16*x + 4 | 复位值: 0x00000000 (x=0~7)
 * @details 配置端点发送数据的字节数，范围0~1023
 */
struct R32_USBD_COUNTx_TX {
    // 发送数据字节数[9:0]: 待发送数据长度，范围0~1023
    uint32_t COUNTx_TX:10;
    // 保留位，只读，值为0
    uint32_t __RESV__:22;

    /**
     * @brief 设置发送数据字节数
     * @param len 数据长度(0~1023)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_tx_len(uint16_t len) {
        auto & self = (*this);
        self.COUNTx_TX = len & 0x3FF;
    }

    /**
     * @brief 获取当前设置的发送数据长度
     * @return 数据长度(0~1023)
     */
    constexpr uint16_t get_tx_len() const noexcept {
        return static_cast<uint16_t>(COUNTx_TX & 0x3FF);
    }
};
VALIDATE_R32(R32_USBD_COUNTx_TX);

/**
 * @brief 端点接收缓冲区地址寄存器 (USBD_ADDRx_RX)
 * @note 偏移地址: BTABLE + 16*x + 8 | 复位值: 0x00000000 (x=0~7)
 * @details 配置端点接收数据缓冲区起始地址，按2字节对齐（最低1位为0）
 */
struct R32_USBD_ADDRx_RX {
    // 保留位，只读，值为0（地址按2字节对齐，最低1位必须为0）
    uint32_t __RESV1__:1;
    // 接收缓冲区地址[15:1]: 接收数据缓冲区起始地址，范围0x40006000~0x400061FF
    uint32_t ADDRx_RX:15;
    // 保留位，只读，值为0
    uint32_t __RESV2__:16;

    /**
     * @brief 设置接收缓冲区地址（自动按2字节对齐）
     * @param addr 缓冲区地址(2字节对齐，0x40006000~0x400061FF)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void set_rx_buf_addr(uint32_t addr) {
        auto & self = (*this);
        self.ADDRx_RX = (addr & 0x1FFE) >> 1; // 屏蔽低1位，右移1位存入
    }

    /**
     * @brief 获取接收缓冲区实际地址
     * @return 32位缓冲区地址
     */
    constexpr uint32_t get_rx_buf_addr() const noexcept {
        return static_cast<uint32_t>(ADDRx_RX << 1) | 0x40006000;
    }
};
VALIDATE_R32(R32_USBD_ADDRx_RX);

/**
 * @brief 端点接收数据字节数寄存器 (USBD_COUNTx_RX)
 * @note 偏移地址: BTABLE + 16*x + 12 | 复位值: 0x00000XXX (x=0~7)
 * @details 配置接收缓冲区大小、存储区块参数，只读实际接收字节数
 */
struct R32_USBD_COUNTx_RX {
    // 实际接收字节数[9:0]: 硬件写入，标识OUT/SETUP事务实际接收的字节数(只读)
    uint32_t COUNTx_RX:10;
    // 存储区块数目[4:0]: 配合BL_SIZE配置接收缓冲区大小
    uint32_t NUM_BLOCK:5;
    // 存储区块大小: 0-2字节/块;1-32字节/块
    uint32_t BL_SIZE:1;
    // 保留位，只读，值为0
    uint32_t __RESV__:16;

    // 缓冲区块大小枚举
    enum class BlkSize : uint8_t {
        SIZE_2B  = 0x00, // 2字节/块
        SIZE_32B = 0x01  // 32字节/块
    };

    /**
     * @brief 配置接收缓冲区大小
     * @param blk_size 块大小
     * @param blk_num 块数目(0~31，对应大小参考手册表20-8)
     * @return 无返回值，直接修改寄存器
     */
    constexpr void config_rx_buf(BlkSize blk_size, uint8_t blk_num) {
        auto & self = (*this);
        self.BL_SIZE = static_cast<uint8_t>(blk_size);
        self.NUM_BLOCK = blk_num & 0x1F;
    }

    /**
     * @brief 获取实际接收的字节数
     * @return 实际接收长度(0~1023)
     */
    constexpr uint16_t get_actual_rx_len() const noexcept {
        return static_cast<uint16_t>(COUNTx_RX & 0x3FF);
    }

    /**
     * @brief 计算配置的接收缓冲区总大小
     * @return 缓冲区大小(字节)
     */
    constexpr uint16_t get_rx_buf_size() const noexcept {
        uint16_t blk = BL_SIZE == 0 ? 2 : 32;
        return static_cast<uint16_t>(blk * (NUM_BLOCK + 1));
    }

    /**
     * @brief 检查接收是否溢出（实际长度≥配置大小）
     * @return true-溢出; false-正常
     */
    constexpr bool is_rx_overflow() const noexcept {
        return get_actual_rx_len() >= get_rx_buf_size();
    }
};
VALIDATE_R32(R32_USBD_COUNTx_RX);

static constexpr uint32_t USBD_BASE        = 0x40005C00;
static constexpr uint32_t USBD_BUF_BASE    = 0x40006000;
#define R32_USBD_CNTR_REG (*((volatile R32_USBD_CNTR*)(USBD_BASE + 0x40)))
#define R32_USBD_ISTR_REG (*((volatile R32_USBD_ISTR*)(USBD_BASE + 0x44)))
#define R32_USBD_FNR_REG  (*((volatile R32_USBD_FNR*)(USBD_BASE + 0x48)))
#define R32_USBD_DADDR_REG (*((volatile R32_USBD_DADDR*)(USBD_BASE + 0x4C)))
#define R32_USBD_BTABLE_REG (*((volatile R32_USBD_BTABLE*)(USBD_BASE + 0x50)))
#define R32_USBD_EPRx_REG(x) (*((volatile R32_USBD_EPRx*)(USBD_BASE + 0x00 + 4*(x)))) // x=0~7

}