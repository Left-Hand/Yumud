#pragma once

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"

#include <coroutine>

namespace ymd::async{

template<typename T>
struct Poll;

template<typename T>
struct Ready{
    constexpr explicit Ready(T&& val):
        value_(std::move<T>(val)){}

    constexpr explicit Ready(const T & val):
        value_((val)){}

    constexpr Poll<T> poll() const {
        return Ready(value_);
    }
private:
    T value_;
};

template<typename T>
Ready(T&& val) -> Ready<std::decay_t<T>>;

struct Pending{};

template<typename T>
struct Poll {
public:

    constexpr Poll() = default;
    constexpr Poll(Ready<T>&& ready):
        state_(std::move(ready)){}
    constexpr Poll(const Ready<T>& ready):
        state_(ready){}
    constexpr Poll(Pending pending):
        state_(pending){}

    constexpr bool is_ready() const {
        return std::holds_alternative<Ready<T>>(state_);
    }

    constexpr bool is_pending() const {
        return std::holds_alternative<Pending>(state_);
    }

    constexpr const T & unwrap() const {
        if(!std::holds_alternative<Ready<T>>(state_)) __builtin_abort();
        return std::get<Ready<T>>(state_).value_;
    }

private:
    std::variant<Ready<T>, Pending> state_;
};

template<typename T, typename E>
class Promise;

struct Context {
    void (*wake_fn)(void*);
    void* data;
    
    void wake() const {
        if(wake_fn) wake_fn(data);
    }
};

template<typename T, typename E>
class Future {
public:

    struct promise_type {
        Future<T,E> get_return_object() { /*...*/ }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(Result<T,E> val) { /*...*/ }
        void unhandled_exception() { /*...*/ }
    };

    struct FutureState {

        constexpr bool is_ready() const {
            return result_.is_some();
        }

        constexpr bool is_pending() const {
            return !result_.is_some();
        }

        constexpr const Result<T, E> & unwrap() const {
            return result_.unwrap();
        }

        constexpr const Result<T, E> take_result(){
            if(!result_.is_some()) __builtin_abort();
            const auto res = result_.unwrap();
            result_ = None;
            return res;
        }
    private:
        Option<Result<T, E>> result_ = None;

        constexpr void set_result(Result<T, E> && res){
            result_ = Some(std::move(res));
        }

        friend class Promise<T, E>;
    };
    
    FutureState & state_;
    
public:
    constexpr Future (FutureState & state): state_(state){}
    Poll<Result<T, E>> poll(Context& cx) noexcept {
        if(state_.is_ready()) {
            return Ready(std::move(state_).unwrap());
        }
        return Pending();
    }


    template<typename Fn>
    constexpr Future<T, E> map(Fn&& fn) const {
        if(state_.is_ready()) {
            return Ready(fn(state_.unwrap()));
        }
        return Pending();
    }
};

// Helper implementations
template<typename T, typename E>
std::pair<Promise<T,E>, Future<T,E>> make_promise() {
    Promise<T,E> p;
    return {std::move(p), p.get_future()};
}

template<typename T, typename E>
class Promise {
    Future<T, E>::FutureState state_;
    
public:
    Future<T, E> get_future() {
        return Future<T, E>{state_};
    }
    
    void set_result(Result<T, E> && res) {
        state_.set_result(std::move(res));
    }
};



// Demo 1: Basic async operation
Future<int, const char*> create_async_operation(Promise<int, const char*>& promise) {
    // Simulate async operation (e.g., IO)
    clock::delay(1000ms); // Simulate delay
    const auto val = 42;
    if(val < 100) {
        promise.set_result(Ok(val));
    } else {
        promise.set_result(Err("Operation failed"));
    }
    return promise.get_future();
}

// // Demo 2: Chained operations
// Future<int, const char*> demo_chain_operations() {
//     auto [promise, future] = make_promise<int, const char*>();
    
//     create_async_operation(promise)
//     .map([](int val) {  // Transform result
//         return val * 2; 
//     })
//     .then([](Result<int, const char*> res) { // Final handling
//         if(res.is_ok()) {
//             log("Result: {}", res.unwrap());
//         } else {
//             log_error("Error: {}", res.unwrap_err());
//         }
//         return Ok();
//     });
//     Ready(1)
//     return future;
// }

// // Demo 3: Full workflow
// void run_async_demo() {
//     // Create promise/future pair
//     auto [promise, future] = make_promise<int, const char*>();
    
//     // Setup async processing
//     auto processed = future
//         .map([](int val) {
//             return val + 100;
//         })
//         .then([](Result<int, const char*> res) {
//             // Handle final result
//             return res.map([](int val) {
//                 log("Final value: {}", val);
//                 return val;
//             });
//         });

//     // Start async operation
//     simulate_async_work(promise);
    
//     // Run in executor
//     get_executor().spawn(std::move(processed));
//     get_executor().run();
// }



// // Simple executor implementation
// class DemoExecutor {
//     std::vector<Future<void, void>> tasks;
    
// public:
//     template<typename F>
//     void spawn(F&& fut) {
//         tasks.emplace_back(fut);
//     }

//     void run() {
//         Context ctx{nullptr, nullptr};
//         while(!tasks.empty()) {
//             auto it = tasks.begin();
//             while(it != tasks.end()) {
//                 if(it->poll(ctx).is_ready()) {
//                     it = tasks.erase(it);
//                 } else {
//                     ++it;
//                 }
//             }
//             yield_thread(); // Simulate cooperative scheduling
//         }
//     }
// };

// // Usage example
// int mm_async_main() {
//     // Simple async demo
//     {
//         auto [promise, future] = make_promise<int, const char*>();
//         get_executor().spawn(
//             future.map([](int val) {
//                 log("Received value: {}", val);
//                 return val + 10;
//             })
//         );
        
//         promise.set_result(Ok(100));
//         get_executor().run();
//     }

//     // Error handling demo
//     {
//         auto [promise, future] = make_promise<int, const char*>();
//         get_executor().spawn(
//             future.then([](Result<int, const char*> res) {
//                 if(res.is_err()) {
//                     log("Handled error: {}", res.unwrap_err());
//                 }
//                 return Ok();
//             })
//         );
        
//         promise.set_result(Err("Connection timeout"));
//         get_executor().run();
//     }
    
//     return 0;
// }


// class Executor {
//     using TaskList = IntrusiveList<FutureVTable>;
    
// public:
//     template<typename F>
//     void spawn(F&& fut) {
//         tasks.push_back(fut);
//     }
    
//     void run() {
//         while(!tasks.empty()) {
//             auto& task = tasks.front();
//             Context ctx{&wake_stater, &task};
            
//             if(task.poll(ctx).is_ready()) {
//                 tasks.pop_front();
//             }
//         }
//     }
    
// private:
//     static void wake_stater(void* task) {
//         static_cast<TaskList::Node*>(task)->unlink();
//         tasks.push_back(*static_cast<TaskList::Node*>(task));
//     }
// };

// // Scheduler integration (basic example)
// class Scheduler {
// public:
//     template<typename F, typename... Args>
//     static auto spawn(F&& func, Args&&... args) -> Future</* deduced type */> {
//         // Implementation would manage async execution
//     }
// };


// template<typename F>
// auto map(F&& f) -> Future<std::invoke_result_t<F, T>, E> {
//     struct MapFuture {
//         Future<T, E> state;
//         F mapper;
        
//         Poll<Result<std::invoke_result_t<F, T>, E>> poll(Context& cx) {
//             auto res = state.poll(cx);
//             if(res.is_ready()) {
//                 return res.result().map(std::forward<F>(mapper));
//             }
//             return Pending{};
//         }
//     };
//     return MapFuture{*this, std::forward<F>(f)};
// }

// template<typename F>
// auto then(F&& f) -> Future<std::invoke_result_t<F, Result<T, E>>, E> {
//     struct ThenFuture {
//         Future<T, E> state;
//         F callback;
        
//         Poll<Result<std::invoke_result_t<F, Result<T, E>>, E>> poll(Context& cx) {
//             auto res = state.poll(cx);
//             if(res.is_ready()) {
//                 return Ready(std::forward<F>(callback)(res.result()));
//             }
//             return Pending{};
//         }
//     };
//     return ThenFuture{*this, std::forward<F>(f)};
// }

// class Executor {
//     struct Task {
//         virtual ~Task() = default;
//         virtual Poll<Result<void, void>> poll(Context&) = 0;
//         IntrusiveListNode<Task> node;
//     };

//     IntrusiveList<Task, &Task::node> tasks;

// public:
//     template<typename Fut>
//     void spawn(Fut&& fut) {
//         struct WrappedTask : Task {
//             Fut future;
            
//             WrappedTask(Fut&& f) : future(std::move(f)) {}
            
//             Poll<Result<void, void>> poll(Context& cx) override {
//                 return future.poll(cx).map([](auto&&){ return Ok(); });
//             }
//         };
        
//         tasks.push_back(*new WrappedTask(std::move(fut)));
//     }

//     void run() {
//         while(!tasks.empty()) {
//             auto& task = tasks.front();
//             Context ctx{&wake_stater, &task};
            
//             if(task.poll(ctx).is_ready()) {
//                 delete &task;
//                 tasks.pop_front();
//             }
//         }
//     }

// private:
//     static void wake_stater(void* task) {
//         auto& t = *static_cast<Task*>(task);
//         // Implementation depends on your IntrusiveList
//         // Typically would move task to front of queue
//     }
// };

// template<typename F>
// auto async_task(F&& func) -> Future<std::invoke_result_t<F>, void> {
//     struct AsyncFuture {
//         F func;
//         Option<std::invoke_result_t<F>> result;
        
//         Poll<Result<std::invoke_result_t<F>, void>> poll(Context&) {
//             if(!result) {
//                 result = Some(func());
//             }
//             return Ready(Ok(*result));
//         }
//     };
//     return AsyncFuture{std::forward<F>(func)};
// }

// auto async_io() -> Future<int, IOError> {
//     return []() -> Poll<Result<int, IOError>> {
//         if(io_ready()) {
//             return Ready(Ok(read_io()));
//         }
//         return Pending{};
//     };
// }

// void example_usage() {
//     Executor ex;
    
//     ex.spawn(
//         async_io()
//         .map([](int val){ return val * 2; })
//         .then([](Result<int, IOError> res) {
//             if(res.is_ok()) {
//                 log("Got value: {}", res.unwrap());
//             }
//             return Ok();
//         })
//     );
    
//     ex.run();
// }
}