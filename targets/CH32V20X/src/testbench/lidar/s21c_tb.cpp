#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/default.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

enum class [[nodiscard]] State:uint8_t{
    Idle,
    Measuring,
    Received,
};


struct S21C_RxActivity{
    struct Config{
        hal::Gpio rx_pin;
    };

    explicit S21C_RxActivity(
        const Config& cfg
    ):
        // tx_pin_(cfg.tx_pin),
        rx_pin_(cfg.rx_pin){;}
    
    void reset(){
        state_ = State::Idle;
        begin_us_ = 0us;
        end_us_ = 0us;
    }

    void resume(){
        switch(state_){
            case State::Idle:
                if(rx_pin_.read() == HIGH){
                    begin_us_ = clock::micros();
                    state_ = State::Measuring;
                }
                break;
            case State::Measuring:
                if(rx_pin_.read() == LOW){
                    end_us_ = clock::micros();
                    state_ = State::Received;
                }
                break;
            case State::Received: 
                break;
        }
    }
    State state() const{ return state_; }

    bool has_received() const{ 
        return state_ == State::Received;
    }

    Option<Microseconds> elapsed() const {
        if(state_ != State::Received) return None;
        return Some(Microseconds{end_us_ - begin_us_});
    }
private:
    // hal::Gpio tx_pin_;
    hal::Gpio rx_pin_;
    State state_;
    Microseconds begin_us_ = 0us;
    Microseconds end_us_ = 0us;
};

static constexpr size_t LIDAR_SCAN_FREQ = 20'000;

void s21c_main(){
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&hal::usart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.force_sync(EN);

    clock::delay(2ms);
    auto & timer = hal::timer1;

    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(LIDAR_SCAN_FREQ), 
        .count_mode = hal::TimerCountMode::Up
    }).unwrap()
        .dont_alter_to_pins()
    ;

    // Microseconds exe_us = 0us;
    static uint32_t ticks_cnt_ = 0;

    timer.register_nvic<hal::TimerIT::Update>({0, 0}, EN);

    auto s21c_tx_pin_ = hal::PC<2>();
    auto s21c_tx_pin2_ = hal::PC<3>();
    auto s21c_rx_pin_ = hal::PC<8>();

    s21c_tx_pin_.outod();
    s21c_tx_pin2_.outod();
    s21c_rx_pin_.inpu();

    S21C_RxActivity s21c_({s21c_rx_pin_});

    static constexpr Milliseconds TX_BOARDCASTING_MS = 10ms;
    static constexpr Milliseconds TX_DURATION_MS = 100ms;

    static constexpr uint32_t TX_BOARDCASTING_TICKS = TX_BOARDCASTING_MS.count() * LIDAR_SCAN_FREQ / 1000;
    static constexpr uint32_t TX_DURATION_TICKS = TX_DURATION_MS.count() * LIDAR_SCAN_FREQ / 1000;

    Microseconds elapsed_us_ = 0us;
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            // const auto now_secs = clock::time();
            // // const auto level = BoolLevel::from(
            // //     math::frac(math::frac(now_secs) * 10)  > 0.1_uq16);
            // s21c_tx_pin_ = level;

            const auto now_ticks = ticks_cnt_ ;
            const auto frac_now_ticks = now_ticks % TX_DURATION_TICKS;

            const bool is_emitting = frac_now_ticks < TX_BOARDCASTING_TICKS;
            if(is_emitting){
                s21c_tx_pin_ = LOW;
                s21c_tx_pin2_ = LOW;
                s21c_.reset();
            }else{
                s21c_tx_pin_ = HIGH;
                s21c_tx_pin2_ = HIGH;
            }

            s21c_.resume();

            if(const auto may_elapsed_us = s21c_.elapsed(); may_elapsed_us.is_some()){
                elapsed_us_ = may_elapsed_us.unwrap();
            }
            ticks_cnt_++;
            
        }
        default: break;
        }
    });

    timer.enable_interrupt<hal::TimerIT::Update>(EN);

    timer.start();

    while(true){
        DEBUG_PRINTLN(
            s21c_tx_pin_.read().to_bool(),
            s21c_rx_pin_.read().to_bool(),
            elapsed_us_.count(),
            iq16(uint32_t(elapsed_us_.count()) * uq12(340.0 / 1000.0 / 2) / 1000)
        );
    }
}