#ifndef __TASKER_HPP__
#define __TASKER_HPP__

#include "sys/clock/clock.h"
#include <cstdint>
#include <functional>
#include <vector>

enum class TaskerStatus:uint8_t {
    OK,
    Error
};

enum class TaskStatus:uint8_t {
    Ready,
    Blocked,
    Terminated
};

using CallbackFunction = std::function<void()>;

class Task {
    private:

        CallbackFunction callback;

        uint8_t ID = 0;
        uint32_t timePiece = 0;
        int32_t timeoutTick = 0;

        TaskStatus state = TaskStatus::Ready;

        friend class Tasker;

    public:

        Task(const CallbackFunction & _callback, const uint32_t _timePiece):
                callback(_callback), timePiece(_timePiece), timeoutTick(_timePiece){;}

        void setPiece(const uint32_t _timePiece){timePiece = _timePiece;}
        TaskerStatus delay(uint32_t ms);
        TaskerStatus terminate();
        TaskerStatus rework();

};

class Tasker{
    private:
        uint32_t tick;

        std::vector<Task *>taskptrLists;

        friend class Task;

    public:
        uint32_t task_cnt;
        uint32_t getTick(void);
        void handle(void);
        TaskerStatus addTask(Task & task);
        TaskerStatus addTasks(std::vector<Task>& tasks);
        TaskerStatus delay(uint32_t ms);

};

#endif //__TASKER_HPP__