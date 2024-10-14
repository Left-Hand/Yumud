#include "tb.h"

#include "src/app/gui/coro_task.hpp"
// #include 
using Task = coro::Task;

// Task hello(int max) {
//     DEBUGGER << "hello world\n";
//     for (int i = 0; i < max; ++i) {
//         DEBUGGER << "hello " << i << "\n";
//         co_await std::suspend_always{};
//     }

//     DEBUGGER << "hello end\n";
// }




Task hello(int max) {
    DEBUGGER << "hello start\n";
    for (int i = 0; i < max; ++i) {
        DEBUGGER << "hello " << i << "\n";
        co_await std::suspend_always{};
    }
    DEBUGGER << "hello end\n";
}

Task world(int max) {
    DEBUGGER << "world start\n";
    for (int i = 0; i < max; ++i) {
        DEBUGGER << "world " << i << "\n";
        co_await std::suspend_always{};
    }
    DEBUGGER << "world end\n";
}

void coro_tb() {
    DEBUGGER.init(576000, CommMethod::Dma);
    DEBUGGER.setEps(4);
    DEBUGGER << "Coroutine test bench started.\n";
    
    // 创建两个协程实例
    Task co_hello = hello(3);
    Task co_world = world(3);

    while (co_hello.resume()) {
        while(co_world.resume()){
            DEBUGGER << "hello coroutine suspend\n";
        }
    }
}

// void coro_tb() {


//     auto co = hello(3);
//     while (co.resume()) {
//         DEBUGGER << "hello coroutine suspend\n";
//     }
// }