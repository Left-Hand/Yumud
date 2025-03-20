#include "src/testbench/tb.h"
#include "sys/debug/debug.hpp"

#include "sys/math/real.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "utils/Result.hpp"
#include <bitset>
#include <ranges>


using namespace ymd;

#define UART uart2

struct FoundInfo{
    uint8_t addr;
    uint max_bbaud;
};

Result<void, BusError> make_result(const BusError res){
    if(res.ok()) return Ok();
    else return Err(res); 
}

#include <coroutine>

// 协程任务模板
template<typename T>

struct [[nodiscard]] Task {
    struct promise_type {
        Result<T, BusError> result;
        std::coroutine_handle<> continuation;

        Task get_return_object() { return Task{Handle::from_promise(*this)}; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(Result<T, BusError> res) { result = res; }
        void unhandled_exception() noexcept {}
    };

    using Handle = std::coroutine_handle<promise_type>;

    explicit Task(Handle h) : handle(h) {}
    ~Task() { if (handle) handle.destroy(); }

    struct Awaiter {
        Handle handle;
        
        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> cont) noexcept {
            handle.promise().continuation = cont;
            handle.resume();
        }
        Result<T, BusError> await_resume() noexcept {
            return handle.promise().result;
        }
    };

    Awaiter operator co_await() noexcept {
        return Awaiter{handle};
    }

    Handle handle;
};

struct I2cTester{
    static constexpr uint start_freq = 200_KHz;
    static constexpr auto grow_scale = 2;
    
    // __pure
    // __attribute__((const))
    static Result<uint, BusError> getMaxBaudRate(I2c & i2c, const uint8_t read_addr){
        auto i2c_drv = hal::I2cDrv{i2c, read_addr};
        // if(auto err = i2c_drv.verify(); err.wrong()){
        //     return err; 
        // }
        const uint max_baud = [&]{
            uint baud = start_freq;
            while(baud < 10_MHz){
                // i2c_drv.set_baudrate(uint(baud * grow_scale));
                i2c.set_baudrate(uint(baud * grow_scale));
                const auto err = i2c_drv.verify();
                if(err.wrong()) break;

                baud = baud + (baud >> 1);
                delay(2);
            }

            return baud;
        }();

        // DEBUG_PRINTLN("??");
        uart2.set_rx_strategy(CommStrategy::Blocking);

        return Ok{max_baud};
    }
    static Result<void, BusError> verify(I2c & i2c, const uint8_t read_addr, const uint bbaud = start_freq){
        return make_result(hal::I2cDrv{i2c, read_addr}.verify());
    }

};


void i2c_scanner_functional(){

    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);

    // log("i2c_scanner_main") | log("1");
    
    I2cSw i2c = {portA[12], portA[15]};
    i2c.init(100_KHz);

    // const uint8_t read_addr = 0x08;

    
    // I2cTester::getMaxBaudRate(i2c, read_addr)
    //     .then([](uint baud) -> Result<uint, BusError>{ 
    //         if (baud > 100_KHz) return Ok(baud);
    //         else return Err(BusError::OCCUPIED); 
    //     })
        ;
}


template<typename ... Args>
auto log(Args && ... args) {
    return std::views::transform(
        [&args...](auto&& value) {
            DEBUG_PRINTS(std::forward<Args>(args)..., value);
            return std::forward<decltype(value)>(value);
        }
    );
}

// template<typename T>
// struct A;

// template<typename T>
// struct B;

// template<typename T, typename U>
// struct C{
//     std::variant<A<T>, B<U>> value;
// };

// template<typename T>
// struct A{
//     T value;
// };

// template<typename T>
// struct B{
//     T value;
// };

// C<int, int> test(int v){
//     return v > 0 ? A<int>{v} : B<int>{v};
// }
void test_result(){
    // DEBUG_SOURCE("hahah");
    // while(1);
    // using MyResult = Result<void, BusError>;
    // auto ok = Ok<void>{};
    // auto ok = Ok();
    // MyResult res = {ok};
    // DEBUG_PRINTLN("before");
    // auto res = Result<uint, BusError>(Err(BusError(BusError::NO_ACK)));
    // res.loc().expect("Device is not responding", "nmd");
    // DEBUG_PRINTLN("after");



    // if(res.ok()){
        //     DEBUG_PRINTLN("impossiable", res.unwrap());
    // res.loc().expect("Device is not responding", "nmd", micros());
    // }else{
    //     DEBUG_PRINTLN("unreachable");
    // }
    // DEBUG_PRINTLN("after");
    while(true);
}
void i2c_scanner_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.forceSync();
    
    // test_result();
    I2cSw i2c = {portB[6], portB[7]};
    i2c.init(100_KHz);
    
    // auto data = std::vector{1, 2, 3};
    // 遍历时打印每个元素
    // for (int n : data | log("Value: ")) { /* ... */ }
    // 输出：
    // Value: 1
    // Value: 2
    // Value: 3

    // namespace views = std::views;
    // std::vector<int> data { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    // auto result { data
    //    | views::filter([](const auto& value) { return value % 2 == 0; })/* 2 4 6 8 10 */
    //    | views::transform([](const auto& value) { return value * 2.0; })/* 4 8 12 16 20 */
    //    | views::drop(2)                                                 /* 12 16 20 */
    //    | views::reverse                                                 /* 20 16 12 */
    // //    | views::transform([](int i) { return to_string(i); })           /* "20" "16" "12" */
    // };

    // DEBUG_PRINTLN();


    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Scan Start");


    {
        std::vector<FoundInfo> founded_devices;


        for(uint8_t i = 0; i < 128; i++){
            const uint8_t read_addr = i << 1;
            I2cTester::verify(i2c, read_addr)
                .if_ok([&]{
                    const auto result = I2cTester::getMaxBaudRate(i2c, read_addr);
                    founded_devices.emplace_back(
                        read_addr, 
                        result.loc().expect("unknown bug")
                    );
                });

            // I2cTester::verify(i2c, read_addr)
            //     .and_then([&i2c, read_addr]() -> Result<FoundInfo, BusError> {
            //         return I2cTester::getMaxBaudRate(i2c, read_addr)
            //             .map([read_addr](uint baud) {
            //                 return FoundInfo{read_addr, baud};
            //             });
            //     })
            //     .match(
            //         [&founded_devices](FoundInfo info) { // 成功分支
            //             founded_devices.emplace_back(info);
            //         },
            //         [](BusError err) { // 失败分支
            //             PANIC("Address", read_addr, "failed:", err.code());
            //         }
            //     );


            // I2cTester::verify(i2c, read_addr)
            // .and_then([&i2c, read_addr] {
            //     return I2cTester::getMaxBaudRate(i2c, read_addr)
            //         .map([read_addr](uint max_baud) {
            //             return FoundInfo{read_addr, max_baud};
            //         });
            // })
            // .map([&founded_devices](auto&& info) {
            //     founded_devices.push_back(std::forward<decltype(info)>(info));
            // });

            delay(1);
        }

        delay(10);

        if(founded_devices.size()){
            DEBUG_PRINTS("Found", founded_devices.size(), " devices" );
            DEBUG_PRINTLN("---------");
            {
                const auto guard = DEBUGGER.createGuard();
                DEBUGGER.setIndent(1);
                for(auto & [read_addr, baud] : founded_devices){
                    DEBUG_PRINTS(
                        std::bitset<8>(read_addr), '\t',
                        "|  Max baud is: ", baud, "Hz"
                    );

                    delay(1);
                }
                // const Result res = Err{0};
                // res.loc().expect("unknown bug");

            }
            DEBUG_PRINTLN("---------");
        }else{
            DEBUG_PRINTLN("No device found");
        }

    }

    DEBUG_PRINTLN("Scan done, Click reset to restart");

    exit(0);
}






//     async_task<void> scan_devices() {
//     for (uint8_t i : std::views::iota(0, 128)) {
//         co_await i2c.acquire();
//         auto result = co_await I2cTester::verify(i2c, i<<1);
//         if (result) {
//             auto baud = co_await I2cTester::getMaxBaudRate(i2c, i<<1);
//             founded_devices.emplace_back(i<<1, baud);
//         }
//     }
// }