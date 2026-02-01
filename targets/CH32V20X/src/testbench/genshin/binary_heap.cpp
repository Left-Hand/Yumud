
#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "hal/bus/uart/hw_singleton.hpp" 

#include "core/tmp/bits/width.hpp"
#include "core/tmp/functor.hpp"
#include "core/container/heapless_binaryheap.hpp"

using namespace ymd;

namespace{
struct Task {
    int priority;
    std::string name;
    
    bool operator<(const Task& other) const {
        return priority < other.priority; // 优先级值大的更重要
    }
    
    friend OutputStream & operator<<(OutputStream& os, const Task& task) {
        return os << task.name << "(" << task.priority << ")";
    }
};
}


void binary_heap_tb(){
    
    using namespace heapless;
    
    DEBUGGER << "=== 测试最大堆（默认） ===" << std::endl;
    BinaryHeap<int, 10> max_heap;
    
    // 插入元素
    max_heap.push(5).examine();
    max_heap.push(2).examine();
    max_heap.push(8).examine();
    max_heap.push(1).examine();
    max_heap.push(10).examine();
    
    DEBUGGER << "堆顶元素: " << max_heap.peek() << std::endl;
    DEBUGGER << "当前大小: " << max_heap.length() << std::endl;
    DEBUGGER << "是否已满: " << max_heap.is_full() << std::endl;
    
    DEBUGGER << "\n堆中元素: ";
    for (const auto& item : max_heap) {
        DEBUGGER << item << " ";
    }
    DEBUGGER << std::endl;
    
    
    DEBUGGER << "\n=== 测试最小堆 ===" << std::endl;
    BinaryHeap<int, 10, std::greater<int>> min_heap;
    
    min_heap.push(5).examine();
    min_heap.push(2).examine();
    min_heap.push(8).examine();
    min_heap.push(1).examine();
    min_heap.push(10).examine();
    
    DEBUGGER << "最小堆顶: " << min_heap.peek() << std::endl;
    DEBUGGER << "当前大小: " << max_heap.length() << std::endl;
    DEBUGGER << "是否已满: " << max_heap.is_full() << std::endl;
    

    DEBUGGER << "\n=== 测试 try_push 和 pop ===" << std::endl;
    BinaryHeap<std::string, 3> string_heap;
    
    DEBUGGER << "插入 'hello': " << string_heap.try_push("hello") << std::endl;
    DEBUGGER << "插入 'world': " << string_heap.try_push("world") << std::endl;
    DEBUGGER << "插入 'foo': " << string_heap.try_push("foo") << std::endl;
    DEBUGGER << "插入 'bar' (应该失败): " << string_heap.try_push("bar") << std::endl;
    
    // std::string value;
    // while (string_heap.pop(value)) {
    //     DEBUGGER << "弹出: " << value << std::endl;
    // }
    
    DEBUGGER << "\n=== 测试自定义对象 ===" << std::endl;

    
    BinaryHeap<Task, 5> task_heap;
    task_heap.emplace(3, "低优先级").examine();
    task_heap.emplace(1, "最低优先级").examine();
    task_heap.emplace(5, "最高优先级").examine();
    task_heap.emplace(4, "高优先级").examine();
    task_heap.emplace(2, "中优先级").examine();
    
    DEBUGGER << "按优先级处理任务: " << std::endl;
    while (!task_heap.is_empty()) {
        auto task = task_heap.pop().unwrap();
        DEBUGGER << "处理: " << task << std::endl;
    }
}
