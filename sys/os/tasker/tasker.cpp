
#include "Tasker.hpp"
#include "clock/clock.h"

uint32_t Tasker::getTick(void) {
    return millis();
}

// void Tasker::update(uint32_t period) {
//     tick += period;
// }

TaskerStatus Tasker::addTask(Task & task) {
    auto targptr = &task;

    for (auto it = taskptrLists.begin(); it != taskptrLists.end(); it++) {
        if (*it == targptr) {
            return TaskerStatus::Error;
        }
    }

    task.ID = task_cnt++;
    taskptrLists.push_back(targptr);

    return TaskerStatus::OK;
}

TaskerStatus Tasker::addTasks(std::vector<Task>& tasks){
    TaskerStatus taskerStatus = TaskerStatus::OK;

    for(auto it = tasks.begin(); it != tasks.end(); it++){
        taskerStatus = static_cast<TaskerStatus>(static_cast<bool>(addTask(*it)) | static_cast<bool>(taskerStatus));
    }

    return taskerStatus;
}

// TaskerStatus Tasker::addTasks(const std::array<Task>& tasks){
//     TaskerStatus taskerStatus = TaskerStatus::OK;

//     for(auto it = tasks.begin(); it != tasks.end(); it++){
//         taskerStatus = static_cast<TaskerStatus>(static_cast<bool>(addTask(*it))
//              | static_cast<bool>(taskerStatus));
//     }

//     return taskerStatus;
// }

void Tasker::handle(void) {
    for (Task * taskptr : taskptrLists) {
        Task & task = *taskptr;
        switch (task.state) {
            case TaskStatus::Ready:
                task.timeoutTick--;
                if (task.timeoutTick == 0) {
                    task.timeoutTick = task.timePiece;
                    if (task.callback != nullptr) {
                        task.callback();
                    }
                }
                break;
            case TaskStatus::Blocked:
                task.timeoutTick++;
                if (task.timeoutTick == 0) {
                    task.rework();
                }
                break;
            case TaskStatus::Terminated:
                break;
            default:
                break;
        }
    }
}

TaskerStatus Tasker::delay(uint32_t ms) {
    for (Task * taskptr : taskptrLists) {
        Task & task = *taskptr;
        task.delay(ms);
    }
    return TaskerStatus::OK;
}

TaskerStatus Task::delay(uint32_t ms) {
    this->state = TaskStatus::Blocked;

    if(this->timeoutTick < 0){
        this->timeoutTick -= ms;
    }else{
        this->timeoutTick = -ms;
    }
    return TaskerStatus::OK;
}

TaskerStatus Task::terminate() {
    this->state = TaskStatus::Terminated;
    this->timeoutTick = 0;
    return TaskerStatus::OK;
}

TaskerStatus Task::rework() {
    this->state = TaskStatus::Ready;
    this->timeoutTick =this->timePiece;
    return TaskerStatus::OK;
}