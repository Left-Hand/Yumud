#pragma once


#include "core/utils/Result.hpp"
#include "core/container/ringbuf.hpp"
#include "ral/can.hpp"
#include "can_layout.hpp"

#include "primitive/can/bxcan_frame.hpp"
#include "primitive/can/can_event.hpp"

#include "can_filter.hpp"



//#region switches

#define CAN_SCE_ENABLED

//#endregion switches


namespace ymd::hal{

class Gpio;


struct alignas(4) [[nodiscard]] BareCanSharedState final{
    
};

struct BareCan final{
public:
public:
    using BaudRate = CanBaudrate;
    using WiringMode = CanWiringMode;
    using Error = CanError;
    using LibError = CanLibError;

    using EventCallback = std::function<void(CanEvent)>;


    struct [[nodiscard]] Config{
        CanRemap remap;
        WiringMode wiring_mode;
        CanNominalBitTimming bit_timming;
    };

public:
    explicit BareCan(void * inst):inst_(inst){;}
    BareCan(const BareCan & other) = delete;
    BareCan(BareCan && other) = delete;

    void init(const Config & cfg);
    void deinit();


    // 尝试写入一个报文 如果没有可用的空间将返回错误
    [[nodiscard]] Result<void, CanLibError> try_write(const BxCanFrame & frame);

    // 尝试读取一个报文 如果没有可读的报文将返回空
    [[nodiscard]] Option<BxCanFrame> try_read();

    // 读取一个报文 如果没有可读的报文时立即终止程序
    [[nodiscard]] BxCanFrame read();

    // 可以继续写入的CAN报文数量
    [[nodiscard]] size_t free_capacity();

    // 已经收到的CAN报文的数量
    [[nodiscard]] size_t available();



    //获取硬件发送错误计数器的计数
    [[nodiscard]] uint8_t get_tx_errcnt();

    //获取硬件接收错误计数器的计数
    [[nodiscard]] uint8_t get_rx_errcnt();

    //获取[可能的]发生的错误
    [[nodiscard]] Option<CanError> last_error();

    //总线是否已经离线
    [[nodiscard]] bool is_busoff();


    // 是否正在进行发送
    [[nodiscard]] bool is_tranmitting();

    // 是否正在进行接收
    [[nodiscard]] bool is_receiving();

    // 使能硬件重传机制
    // 建议不要开启 开启后一旦目标掉线失去应答会疯狂往总线填写报文，造成总线堵塞
    void enable_hw_retransmit(const Enable en);

    // 结束指定邮箱的发送
    void abort_transmit(const CanMailboxIndex mbox_idx);

    // 结束所有邮箱的发送
    void abort_all_transmits();

    // 当接收 FIFO 溢出时，已接收邮箱报文未读
    // 出，邮箱未释放时，新接收到的报文被丢弃；
    void enable_rxfifo_lock(const Enable en);

    //使能报文索引优先级 开启前顺序发送报文 开启后按报文标识符优先级发送
    void enable_index_priority(const Enable en);
private:
    void * inst_;

    void alter_to_pins(const CanRemap remap);
    void enable_rcc(const Enable en);
    void set_remap(const CanRemap remap);

    void init_interrupts();

    //在指定的邮箱填写报文
    void transmit(const BxCanFrame & frame, const CanMailboxIndex mbox_idx);

    void poll_backup_fifo();

    //在指定的fifo读取报文
    [[nodiscard]] BxCanFrame receive(const CanFifoIndex fifo_idx);

    
    [[nodiscard]] uint32_t get_aligned_bus_clk_freq();

    friend class CanFilter;
    friend class CanInterruptDispatcher;
};

struct [[nodiscard]] BareCanSender final{

};



struct [[nodiscard]] BareCanReceiver final{

};




}