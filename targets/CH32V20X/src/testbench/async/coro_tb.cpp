#include "src/testbench/tb.h"
#include "sys/debug/debug.hpp"

#include "src/app/gui/coro_task.hpp"


using namespace ymd::coro;
// using Task = coro::Task;

#define UART hal::uart2

Future<void> hello(int max) {
    DEBUGGER << "hello start\n";
    for (int i = 0; i < max; ++i) {
        DEBUGGER << "hello " << i << "\n";
        co_await std::suspend_always{};
    }
    DEBUGGER << "hello end\n";
}

Future<void> world(int max) {
    DEBUGGER << "world start\n";
    for (int i = 0; i < max; ++i) {
        DEBUGGER << "world " << i << "\n";
        co_await std::suspend_always{};
    }
    DEBUGGER << "world end\n";
}

// 将lambda改为命名协程函数
Future<void> dummyCoroutine() {
    // 模拟异步操作
    co_return;
}


Future<void> coro_tb() {

    
    // Task co_hello = hello(3);
    // Task co_world = world(3);

    // while (co_hello.resume()) {
    //     while(co_world.resume()){
    //         DEBUGGER << "hello coroutine suspend\n";
    //     }
    // }

    // 链式调用：hello → world → 完成处理
    // auto task = hello(3).then([](auto) {
    //     return world(3);
    // }).then([](auto) {
    //     DEBUGGER << "All tasks completed!\n";
    //     return Future<void>{}; // 终止链
    // });

    // 使用协程链式调用
    auto future = dummyCoroutine();  // 直接获取协程句柄

    auto nextFuture = future.then([]() {
        return 42;
    });

    // int result = co_await nextFuture;  // 确保协程完成后再获取结果
    // DEBUGGER << "Final result: " << result << "\n";

    while(true);
}

void coro_main(){
    UART.init(576000, CommMethod::Dma);
    DEBUGGER.retarget(&UART);
    DEBUGGER.setEps(4);
    DEBUGGER << "Coroutine test bench started.\n";

    auto tb_coro = coro_tb();  // 获取协程句柄

    while(true);
}