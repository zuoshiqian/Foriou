#ifndef YRUI_TASK_SCHEDULER_FWD_H_
#define YRUI_TASK_SCHEDULER_FWD_H_

namespace Foriou {

class Action;
class Trigger;
class Task;
class Task_scheduler;

class Scheduler_view {
public:
    virtual ~Scheduler_view() {}

    virtual void update(Task_scheduler*) = 0;
};

}

#endif // YRUI_TASK_SCHEDULER_FWD_H_
