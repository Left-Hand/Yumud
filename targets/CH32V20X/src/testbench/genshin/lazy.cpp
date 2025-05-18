
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
// #include "rust_enum.hpp"
#include <coroutine>
#include <vector>
#include <chrono>
#include <ratio>


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
            //DEBUG_PRINTS("--->Started the coroutine, put the brakes on!");
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

    void resume(){
        if (!coro.done()) {
            coro.resume();
        }
    }
    T get()
    {
        while (!coro.done()) {
            coro.resume();
        }
        return coro.promise().value;
    }
    struct promise_type
    {
        // uint32_t start_time = 0;
        // uint32_t delay_ms = 0;
        T value;
        promise_type()
        {
        }
        ~promise_type()
        {
        }

        std::suspend_always yield_value(auto v) {
            // val.emplace(std::move(v));
            return {};
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
    DEBUG_PRINTS("Reading data...");

    co_return "billion$!";
}

Deferred<int> write_data(CustomAllocator* alloc)
{
    DEBUG_PRINTS("Write data...");

    co_return 12;
}

class Millis{
public: 

private:
    uint32_t ms_;
};

// struct CoroutineManager {
//     using DeferredPromiseType = Deferred<int>::promise_type;
//     std::vector<std::coroutine_handle<DeferredPromiseType>> active;

//     void add(std::coroutine_handle<DeferredPromiseType> h) {
//         active.push_back(h);
//     }

//     void process() {
//         auto now = clock::millis();
//         for (auto it = active.begin(); it != active.end();) {
//             auto& handle = *it;
//             if (handle.done()) {
//                 it = active.erase(it);
//             } else {
//                 if (handle.promise().start_time + handle.promise().delay_ms <= now) {
//                     handle.resume();
//                 }
//                 ++it;
//             }
//         }
//     }
// };

class CheckLevel final{
public:
    consteval explicit CheckLevel(size_t val) : val_(val) {}

    // 使用 spaceship operator 自动生成所有比较运算符
    consteval auto operator<=>(const CheckLevel&) const noexcept = default;

    consteval CheckLevel operator +(const size_t val){
        return CheckLevel(val_ + val);
    }

    consteval CheckLevel operator -(const size_t val){
        return CheckLevel(val_ - val);
    }
private:    
    size_t val_;
};


struct Hardening final{
    #ifndef ALLOCATOR_HARDENING
    static constexpr CheckLevel ALLOCATOR_HARDENING = CheckLevel{3};
    #endif
    static constexpr CheckLevel ALLOCATOR_HARDENING_ROBUST = CheckLevel{2};
    static constexpr bool INCLUDE_ROBUST_CHECKS =
        ALLOCATOR_HARDENING >= ALLOCATOR_HARDENING_ROBUST;


    template<typename T, typename U>
    struct [[nodiscard]] Multiply{
        constexpr Multiply(T& value, U increment, std::source_location loc = std::source_location::current()){
            if constexpr (Hardening::INCLUDE_ROBUST_CHECKS) {
                ASSERT<bool>(!__builtin_mul_overflow(value, increment, &value), loc);
            } else {
                value *= increment;
            }
        }
    };

    template<typename T, typename U>
    struct [[nodiscard]] Increment{
        constexpr Increment(T& value, U increment, std::source_location loc = std::source_location::current()){
            if constexpr (Hardening::INCLUDE_ROBUST_CHECKS) {
                ASSERT<bool>(!__builtin_add_overflow(value, increment, &value), loc);
            } else {
                value += increment;
            }
        }
    };

    template<typename T, typename U>
    struct [[nodiscard]] Decrement{
        constexpr Decrement(T& value, U increment, std::source_location loc = std::source_location::current()){
            if constexpr (Hardening::INCLUDE_ROBUST_CHECKS) {
                ASSERT<bool>(!__builtin_add_overflow(value, increment, &value), loc);
            } else {
                value += increment;
            }
        }
    };
};

// Helper variables to determine when a template parameter is an array type.
// Based on the sample implementation found at
// https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique.
template <typename>
constexpr bool is_unbounded_array_v = false;

template <typename T>
constexpr bool is_unbounded_array_v<T[]> = true;

template <typename>
constexpr bool is_bounded_array_v = false;

template <typename T, size_t kN>
constexpr bool is_bounded_array_v<T[kN]> = true;

/// Describes the layout of a block of memory.
///
/// Layouts are passed to allocators, and consist of a (possibly padded) size
/// and a power-of-two alignment no larger than the size. Layouts can be
/// constructed for a type `T` using `Layout::Of`.
///
/// Example:
///
/// @code{.cpp}
///    struct MyStruct {
///      uint8_t field1[3];
///      uint32_t field2[3];
///    };
///    constexpr Layout layout_for_struct = Layout::Of<MyStruct>();
/// @endcode
class Layout {
public:
    constexpr Layout() : Layout(0) {}
    constexpr explicit Layout(size_t size)
        : Layout(size, alignof(std::max_align_t)) {}
    constexpr Layout(size_t size, size_t alignment)
        : size_(size), alignment_(alignment) {}

    /// Creates a Layout for the given type.
    template <typename T>
    static constexpr std::enable_if_t<!std::is_array_v<T>, Layout> Of() {
        return Layout(sizeof(T), alignof(T));
    }

    /// Creates a Layout for the given bounded array type, e.g. Foo[kN].
    template <typename T>
    requires is_bounded_array_v<T>
    static constexpr Layout Of() {
        return Layout(sizeof(T), alignof(std::remove_extent_t<T>));
    }

    /// Creates a Layout for the given array type, e.g. Foo[].
    template <typename T>
    requires is_unbounded_array_v<T>
    static constexpr Layout Of(size_t count) {
        using U = std::remove_extent_t<T>;
        size_t size = sizeof(U);
        Hardening::Multiply(size, count);
        return Layout(size, alignof(U));
    }

    // /// If the result is okay, returns its contained layout; otherwise, returns a
    // /// default layout.
    // static constexpr Layout Unwrap(const Result<Layout>& result) {
    //     return result.ok() ? (*result) : Layout();
    // }

    // constexpr Layout Extend(size_t size) const {
    //     Hardening::Increment(size, size_);
    //     return Layout(size, alignment_);
    // }

    constexpr Layout Align(size_t alignment) const {
        return Layout(size_, std::max(alignment, alignment_));
    }

    constexpr size_t size() const { return size_; }
    constexpr size_t alignment() const { return alignment_; }

    private:
    size_t size_;
    size_t alignment_;
};

inline bool operator==(const Layout& lhs, const Layout& rhs) {
    return lhs.size() == rhs.size() && lhs.alignment() == rhs.alignment();
}

inline bool operator!=(const Layout& lhs, const Layout& rhs) {
    return !(lhs == rhs);
}

class Allocator{
public: 

};
/// Context required for creating and executing coroutines.
class CoroContext final{
public:
    /// Creates a `CoroContext` which will allocate coroutine state using
    /// `alloc`.
    explicit CoroContext(Allocator& alloc) : alloc_(alloc) {}
    Allocator& alloc() const { return alloc_; }

private:
    Allocator& alloc_;
};

template<typename T>
struct Singleton final{
    static T & get(){
        static T instance;
        return instance;
    }
};



// Utility for async delay
// Deferred<int> sleep(uint32_t ms) {
auto sleep(uint32_t ms) {
    struct Awaiter {
        Milliseconds start_time;
        Milliseconds delay_ms;

        Awaiter(uint32_t ms) : start_time(clock::millis()), delay_ms(ms) {}

        bool await_ready() {
            DEBUG_PRINTLN("mill", clock::millis(), start_time);
            return (clock::millis() - start_time) >= delay_ms;
        }

        void await_suspend(std::coroutine_handle<> awaiting) {
        //     this->handle = awaiting; // Save the handle for later resuming
        }

        void await_resume() {}

        void reset(){
            start_time = clock::millis();
        }

        // static void update() {
        // // Called periodically from main loop to resume timed-out coroutines
        // for (auto& awaiter : active_awaiters) {
        //     if ((clock::millis() - awaiter.start_time) >= awaiter.delay_ms) {
        //         awaiter.h.resume();
        //     }
        // }
    };
    // co_await Awaiter{ms};
    return Awaiter{ms};
}



// Mock sensor interface
struct Sensor {
    Sensor(int value) : value_(value) {}
    Deferred<int> read_data() const {
        co_await sleep(100);
        co_return value_;
    }
private:
    int value_;
};



// Step 1: Read raw sensor data
Deferred<int> read_sensor(const Sensor & sensor) {
    DEBUG_PRINTS("Reading sensor data...");
    int raw_value = co_await sensor.read_data();
    DEBUG_PRINTS("Raw value read: ", raw_value);
    co_return raw_value;
}

// Step 2: Examine data validity
Deferred<std::optional<int>> examine_data(int raw_value) {
    DEBUG_PRINTS("Examining data validity...");
    
    if (raw_value < 0) {
        DEBUG_PRINTS("Invalid data detected!");
        co_return std::nullopt; // Early return on error
    }
    
    // Data is invalid, proceed to processing
    co_return raw_value;
}

// Step 3: Process the data
int process_data(int value) {
    DEBUG_PRINTS("Processing data...");
    return value * 2; // Example processing
}

// Full pipeline as coroutine
Deferred<std::optional<int>> sensor_pipeline(const Sensor & sensor) {
    DEBUG_PRINTS("Starting sensor pipeline...");
    
    // Read sensor data
    int raw_value = co_await read_sensor(sensor);
    
    // Examine data validity
    auto validated = co_await examine_data(raw_value);
    if (!validated) {
        DEBUG_PRINTS("Pipeline terminated early");
        co_return std::nullopt;
    }
    
    // Process data and return result
    int processed_value = process_data(*validated);
    DEBUG_PRINTS("Final processed value: ", processed_value);
    co_return processed_value;
}

// Test the coroutine
void test_pipeline() {
    //并发开始两个获取请求
    DEBUG_PRINTS("=== Test Valid Case ===");
    auto task1 = sensor_pipeline(Sensor(23));
    DEBUG_PRINTS("\n=== Test Error Case ===");
    auto task2 = sensor_pipeline(Sensor(-2));
    
    //等待请求结束并获取结果
    auto result1 = task1.get();
    auto result2 = task2.get();

    DEBUG_PRINTS("Result: ", result1);
    DEBUG_PRINTS("Result: ", result2);
}


#define UART DEBUGGER_INST

void lazy_main() {

    UART.init(576000 * 10 / 9);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync();

    auto & led = portD[0];
    // auto & led = portB[8];
    led.outpp();
    while(true){
        led = HIGH;
        clock::delay(2ms);
        led = LOW;
        clock::delay(1ms);
        DEBUG_PRINTLN(clock::millis(), UART.available());
        // while(UART.available()){
        //     char chr;
        //     UART.read1(chr);
        //     DEBUG_PRINTLN(chr);
        // }
    }


    portC[13].outpp();
    portC[13].write(HIGH);  

    // while(true){
    //     DEBUG_PRINTS(clock::millis());
    // }

    // const auto res = reply(42).get();

    // CustomAllocator my_allocator; // Pre-allocated allocator

    
    // auto res = reply(&my_allocator, 42);
    // test_pipeline();
    // while(true);

    auto blink = [] -> Deferred<int> {
        // auto sleep_task = sleep(200);
        while(true){
            portC[13].write(HIGH);
            co_await (sleep(200));
            // co_await sleep_task;
            // sleep_task.reset();
            // clock::delay(200ms);
            portC[13].write(LOW);
            // co_await sleep_task;
            // sleep_task.reset();
            co_await (sleep(200));
            // co_await sleep(200);
            // clock::delay(200ms);
            // co_await 0;
            co_yield 0;
        }
    };

    auto task = blink();
    while(true){
        // task.get();
        task.resume();
        DEBUG_PRINTLN(clock::millis());
    }

    // start_blink(); // Initialize blinking
    
    // // Main event loop
    // while(true) {
    //     Singleton<CoroutineManager>::get().process();
    //     clock::delay(1ms); // Prevent CPU spinning
    // }

    // while (true) {
    //     uint32_t now = clock::millis();

    //     for (auto it = g_suspended_coroutines.begin(); it != g_suspended_coroutines.end();) {
    //         auto& coro = *it;
    //         if (coro.resume()) {
    //             // Coro is done, remove it
    //             it = g_suspended_coroutines.erase(it);
    //         } else {
    //             ++it;
    //         }
    //     }

    //     clock::delay(1ms); // Prevent tight loop from consuming all CPU
    // }
    while(true);
}