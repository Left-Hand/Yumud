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

#ifdef CAN1_PRESENT
extern "C"{
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void);

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void);

__interrupt
void CAN1_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN1_SCE_IRQHandler(void);
#endif
}
#endif

#ifdef CAN2_PRESENT
extern "C"{
__interrupt
void CAN2_TX_IRQHandler(void);

__interrupt
void CAN2_RX0_IRQHandler(void);

__interrupt
void CAN2_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN2_SCE_IRQHandler(void);
#endif
}
#endif

#ifdef CAN3_PRESENT
extern "C"{
__interrupt
void CAN3_TX_IRQHandler(void);

__interrupt
void CAN3_RX0_IRQHandler(void);

__interrupt
void CAN3_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN3_SCE_IRQHandler(void);
#endif
}
#endif

namespace ymd::hal{

class Gpio;

struct CanFilter;

struct Can;

struct CanInterruptDispatcher{
    static void isr_tx(Can & can);
    static void isr_rx(Can & can, volatile uint32_t & rfifo_reg, const CanFifoIndex fifo_idx);
    static void isr_rx0(Can & can);
    static void isr_rx1(Can & can);
    static void isr_sce(Can & can);
};

class [[nodiscard]] Can final{
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
    explicit Can(void * inst):inst_(inst){;}
    Can(const Can & other) = delete;
    Can(Can && other) = delete;

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

    //获取硬件发送错误计数器的计数
    [[nodiscard]] uint8_t get_tx_errcnt();

    //获取硬件接收错误计数器的计数
    [[nodiscard]] uint8_t get_rx_errcnt();

    //获取[可能的]发生的错误
    [[nodiscard]] Option<CanError> last_error();

    //总线是否已经离线
    [[nodiscard]] bool is_busoff();

    template<typename Fn>
    void set_event_callback(Fn && cb){
        event_callback_ = std::forward<Fn>(cb);
    }


    template<size_t I>
    requires (I < 14)
    CanFilter filters() const {
        return CanFilter(this->inst_, Nth(I));
    }

private:
    void * inst_;
    
    #ifndef CAN_BUFFERED_QUEUE_SIZE
    static constexpr size_t CAN_BUFFERED_QUEUE_SIZE = 8;
    #endif

    RingBuf<BxCanFrame, CAN_BUFFERED_QUEUE_SIZE> rx_queue_;
    RingBuf<BxCanFrame, CAN_BUFFERED_QUEUE_SIZE> tx_queue_;

    EventCallback event_callback_ = nullptr;

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



#ifdef CAN1_PRESENT
extern Can can1;
#endif

#ifdef CAN2_PRESENT
extern Can can2;
#endif

#ifdef CAN3_PRESENT
extern Can can3;
#endif

}