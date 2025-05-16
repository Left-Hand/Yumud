
#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "hal/bus/can/can_msg.hpp" 
#include "hal/bus/uart/uarthw.hpp" 

#include "core/utils/typetraits/size_traits.hpp"
#include "core/utils/typetraits/function_traits.hpp"
// #include "core/utils/typetraits/typetraits_details.hpp"
#include "core/utils/typetraits/serialize_traits.hpp"
#include "core/utils/typetraits/enum_traits.hpp"
#include "rust_enum.hpp"
#include <coroutine>


// https://zplutor.github.io/2022/03/25/cpp-coroutine-beginner/

using namespace ymd;


struct CustomAllocator {
    void* allocate(size_t size){
        return new char[size];
    }
    void deallocate(void* ptr, size_t size){
        delete reinterpret_cast<uint8_t*>(ptr);
    }
};

template <typename T>
struct Eager
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    Eager(handle_type h)
        : coro(h){
    }
    Eager(const Eager &) = delete;
    Eager(Eager &&s)
        : coro(s.coro)
    {
        s.coro = nullptr;
    }
    ~Eager()
    {

        if (coro)
            coro.destroy();
    }
    Eager &operator=(const Eager &) = delete;
    Eager &operator=(Eager &&s)
    {
        coro = s.coro;
        s.coro = nullptr;
        return *this;
    }

    T get()
    {
        return coro.promise().value;
    }
    struct promise_type
    {
        T value;
        promise_type()
        {
        }
        ~promise_type()
        {

        }

        auto get_return_object()
        {

            return Eager<T>{handle_type::from_promise(*this)};
        }
        auto initial_suspend()
        {
    
            return std::suspend_never{};
            //DEBUGGER << "--->Started the coroutine, put the brakes on!" << std::endl;
            //return std::suspend_always{};
        }
        auto return_value(T v)
        {
            value = v;
            return std::suspend_never{};
        }
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }
        void unhandled_exception()
        {
            std::exit(1);
        }
    };
};

template <typename T>
struct Deferred
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    Deferred(handle_type h)
        : coro(h)
    {
    }
    Deferred(const Deferred &) = delete;
    Deferred(Deferred &&s)
        : coro(s.coro)
    {

        s.coro = nullptr;
    }
    ~Deferred()
    {

        if (coro)
            coro.destroy();
    }
    Deferred &operator=(const Deferred &) = delete;
    Deferred &operator=(Deferred &&s)
    {
        coro = s.coro;
        s.coro = nullptr;
        return *this;
    }

    T get()
    {
        return coro.promise().value;
    }
    struct promise_type
    {
        T value;
        promise_type()
        {
        }
        ~promise_type()
        {
        }

        auto get_return_object()
        {
            return Deferred<T>{handle_type::from_promise(*this)};
        }
        auto initial_suspend()
        {
            return std::suspend_always{};
        }
        auto return_value(T v)
        {
            value = v;
            return std::suspend_never{};
        }
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }
        void unhandled_exception()
        {
            std::exit(1);
        }
        
    };
    bool await_ready()
    {
        const auto ready = this->coro.done();
        return ready;
    }
    void await_suspend(std::coroutine_handle<> awaiting)
    {
        {
            this->coro.resume();
        }

        awaiting.resume();
    }
    auto await_resume()
    {
        const auto r = this->coro.promise().value;
        return r;
    }
};
Deferred<std::string> read_data(CustomAllocator* alloc)
{
    DEBUGGER << "Reading data..." << std::endl;

    co_return "billion$!";
}

Deferred<int> write_data(CustomAllocator* alloc)
{
    DEBUGGER << "Write data..." << std::endl;

    co_return 12;
}

void wait(const uint32_t ms){
    delay(ms);
}

Eager<int> reply(CustomAllocator* alloc, int i)
{
    DEBUGGER << "Started await_answer" << std::endl;

    auto fn = [&] -> Deferred<int> {
        auto a = co_await read_data(alloc);
        wait(100);
        portC[13].clr();
        DEBUGGER << "Data we got is " << a  << std::endl;
        auto v = co_await write_data(alloc);
        wait(100);
        portC[13].set();
        DEBUGGER << "write result is " << v << std::endl;
        co_return 1;
    };

    co_await fn();
    co_await fn();
    // co_return bool(hal::portC[1].read());
    co_return sys::chip::get_chip_id();
}



// Mock sensor interface
struct Sensor {
    static inline bool invalid = false;
    
    static int read_raw_data() {
        if (invalid) {
            return -1; // Simulate failed read
        }
        return 42; // Simulated invalid sensor data
    }
};

// Utility for async delay
Deferred<int> sleep_for(int ms) {
    struct Awaiter {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) {
            // In real code, would use hardware timer interrupt
            // Here we just resume immediately (simulated async)
            h.resume();
        }
        void await_resume() {}
    };
    co_await Awaiter{};
}

// Step 1: Read raw sensor data
Deferred<int> read_sensor() {
    DEBUGGER << "Reading sensor data..." << std::endl;
    co_await sleep_for(100); // Simulate async read delay
    
    int raw_value = Sensor::read_raw_data();
    DEBUGGER << "Raw value read: " << raw_value << std::endl;
    co_return raw_value;
}

// Step 2: Examine data validity
Deferred<std::optional<int>> examine_data(int raw_value) {
    DEBUGGER << "Examining data validity..." << std::endl;
    
    if (raw_value < 0) {
        DEBUGGER << "Invalid data detected!" << std::endl;
        co_return std::nullopt; // Early return on error
    }
    
    // Data is invalid, proceed to processing
    co_return raw_value;
}

// Step 3: Process the data
int process_data(int value) {
    DEBUGGER << "Processing data..." << std::endl;
    return value * 2; // Example processing
}

// Full pipeline as coroutine
Eager<std::optional<int>> sensor_pipeline() {
    DEBUGGER << "Starting sensor pipeline..." << std::endl;
    
    // Read sensor data
    int raw_value = co_await read_sensor();
    
    // Examine data validity
    auto validated = co_await examine_data(raw_value);
    if (!validated) {
        DEBUGGER << "Pipeline terminated early" << std::endl;
        co_return std::nullopt;
    }
    
    // Process data and return result
    int processed_value = process_data(*validated);
    DEBUGGER << "Final processed value: " << processed_value << std::endl;
    co_return processed_value;
}

// Test the coroutine
void test_pipeline() {
    DEBUGGER << "=== Test Valid Case ===" << std::endl;
    auto result1 = sensor_pipeline().get();
    DEBUGGER << "Result: " << (result1 ? std::to_string(*result1) : "nullopt") << std::endl;
    DEBUGGER << "\n=== Test Error Case ===" << std::endl;
    Sensor::invalid = true;
    auto result2 = sensor_pipeline().get();
    // PANIC();
    portC[13].toggle();
    DEBUGGER << "Result: " << (result2 ? std::to_string(*result2) : "nullopt") << std::endl;
    portC[13].toggle();
    // PANIC("Result: ", (result2 ? std::to_string(*result2) : "nullopt"));
}

#define UART DEBUGGER_INST

void lazy_main() {

    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync();

    portC[13].outpp();
    portC[13].write(HIGH);  

    // while(true){
    //     DEBUG_PRINTLN(millis());
    // }

    // const auto res = reply(42).get();

    // CustomAllocator my_allocator; // Pre-allocated allocator

    
    // auto res = reply(&my_allocator, 42);

    // DEBUG_PRINTLN("final res is ", res.get());
    test_pipeline();

    while(true);
}