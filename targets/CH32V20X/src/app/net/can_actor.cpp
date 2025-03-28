// #include "core/debug/debug.hpp"

// #include "hal/bus/can/can.hpp"
// #include "hal/timer/instance/timer_hw.hpp"

// #include "coroutine"

// using namespace ymd;
// using Can = hal::Can;


// void init_can(Can & can){
//     can.init(1_MHz, Can::Mode::Internal);

//     can[0].mask(
//         hal::CanID16{0x200, hal::CanRemote::Any}, hal::CanID16::IGNORE_LOW(7, hal::CanRemote::Any),
//         hal::CanID16{0x000, hal::CanRemote::Any}, hal::CanID16::IGNORE_LOW(7, hal::CanRemote::Any));
// }

// class CanDrv{
// public:
//     CanDrv(hal::Can & can):can_(can){;}

//     void write(const hal::CanMsg & msg){
//         can_.write(msg);
//     }

//     void read(hal::CanMsg & msg){
//         msg = can_.read();
//     }

// private:
//     hal::Can & can_;
// };

// class CanActor{
//     void send(const hal::CanMsg& msg) {
//         // 封装发送操作为异步任务
//         co_await [this, msg]() -> std::future<void> {
//             std::promise<void> p;
//             can_.write(msg);
//             p.set_value(); // 发送完成后通知
//             return p.get_future();
//         };
//     }
// };

// #define DBG_UART hal::uart2

// void can_actor_main(){
//     DBG_UART.init(576000);
//     DEBUGGER.retarget(&DBG_UART);
//     DEBUGGER.set_eps(4);
//     DEBUGGER.set_splitter(",");

//     auto & can = hal::can1;

//     init_can(can);

//     while(true){
//     }
// }