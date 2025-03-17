#pragma once

#include <coroutine>
#include <exception>
#include <type_traits>
#include <functional>
#include <optional>


namespace ymd::coro{
// class Task{
// public:
//     struct promise_type;
//     using TaskHd1 = std::coroutine_handle<promise_type>;

// private:
//     TaskHd1  hd1_;

// public:
//     Task(auto h) : hd1_{h} {}
//     ~Task() {
//         if (hd1_) { hd1_.destroy(); }
//     }

//     Task(const Task&) = delete;
//     Task& operator=(const Task&) = delete;

//     bool resume() {
//         if (!hd1_ || hd1_.done())
//             return false;
//         hd1_.resume();
//         return true;
//     }

// public:
//     struct promise_type {
//         /* data */
//         auto get_return_object() {
//             return Task{TaskHd1::from_promise(*this)};
//         }
//         auto initial_suspend() { return std::suspend_always{};}
//         void unhandled_exception() { std::terminate();}
//         void return_void() {}
//         auto final_suspend() noexcept { return std::suspend_always{}; }
//     };
// };



template<typename T>
struct Future {
    struct promise_type {
        Future get_return_object() { 
            return Future{std::coroutine_handle<promise_type>::from_promise(*this)}; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_value(T value) { this->value = std::move(value); }
        T value;
    };

    template<typename Func>
    auto then(Func&& func);

    using handle_type = std::coroutine_handle<promise_type>;

    Future(handle_type h) : coro(h) {}
    ~Future() {
        if (coro) {
            coro.destroy();
            coro = nullptr;
        }
    }

    bool await_ready() const { return false; } // 示例实现
    void await_suspend(handle_type h) { /* 挂起逻辑 */ }
    // T await_resume() { return value; } // 返回结果

    T get() {
        if (!coro.done()) coro.resume();
        return std::move(coro.promise().value);
    }

    handle_type coro;
};

// 特化 Future<void>
template<>
struct Future<void> {
    struct promise_type {
        Future<void> get_return_object() { 
            return Future<void>{std::coroutine_handle<promise_type>::from_promise(*this)}; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}  
        void unhandled_exception() { std::terminate(); }
    };

    template<typename Func>
    auto then(Func&& func);

    using handle_type = std::coroutine_handle<promise_type>;

    Future(handle_type h) : coro(h) {}
    ~Future() {
        if (coro) {
            coro.destroy();
            coro = nullptr;
        }
    }

    void get() {
        if (!coro.done()) coro.resume();
        coro = nullptr; // 释放资源
    }

    handle_type coro;
};

template<typename T>
struct Promise {
    struct awaitable {
        Future<T> fut;
        bool await_ready() const noexcept {
            return fut.coro.done();
        }
        void await_suspend(std::coroutine_handle<> h) noexcept {
            fut.coro.promise().continuation = h;
            fut.coro.resume();
        }
        void await_resume() const noexcept {}
    };

    awaitable operator co_await() {
        return {std::move(*static_cast<Future<T>*>(this))};
    }
};



template<typename T>
template<typename Func>
auto Future<T>::then(Func&& func) {
    using NextT = decltype(func(std::declval<T>()));

    // 修改：创建目标类型Future<NextT>的协程承诺和句柄
    auto new_promise = new typename Future<NextT>::promise_type();
    auto new_coro = std::coroutine_handle<typename Future<NextT>::promise_type>::from_promise(*new_promise);

    new_coro.promise().value = std::move(coro.promise().value);
    new_coro.resume();

    return Future<NextT>{new_coro};
}

template<typename Func>
auto Future<void>::then(Func&& func) {
    using NextT = decltype(func());

    // 修改：创建目标类型Future<NextT>的协程承诺和句柄
    auto new_promise = new typename Future<NextT>::promise_type();
    auto new_coro = std::coroutine_handle<typename Future<NextT>::promise_type>::from_promise(*new_promise);

    new_coro.resume();

    return Future<NextT>{new_coro};
}
}
