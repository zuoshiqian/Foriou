#ifndef FORIOU_PLUGIN_TASK_LAUNCHER_VIEW_H_
#define FORIOU_PLUGIN_TASK_LAUNCHER_VIEW_H_

#include "DirectUI/UiLib.h"
#include "Util/Animator.h"
#include "Util/Win_base.h"
#include "Task_launcher/Task_scheduler_fwd.h"

namespace Foriou {

using namespace UiLib;

class Plugin_view :
    public Win_base,
    public Scheduler_view,
    public IListCallbackUI,
    public Animation::IAnimator {
public:
    Plugin_view(Logger logger);

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;
    virtual void Notify(TNotifyUI& notifier) override;
    virtual LPCTSTR GetItemText(CControlUI* control, int index, int subitem) override;

    virtual void update(Task_scheduler*) override;

private:
    // button callbacks
    void prepare_creation_();
    void creation_ready_();
    void end_creation_();

    void enable_createion_(bool enable = true);

    /*!
     * \brief   display task info at the right subpage.
     */
    void show_task_detail_(const Task& t);
    void clear_task_detail_();

    bool create_task_();

private:
    virtual CControlUI* Init() override;

    enum { update_scheduler = 1 };
    enum { update_duration = 10000 };

private:
    virtual bool has_attr(const std::string& name) override;
    virtual void set_attr(const std::string& name, int val) override;
    virtual int get_attr(const std::string& name) override;

private:
    CListUI* list_;
    CControlUI* detail_;
};

}  // of namespace Foriou

#endif // !FORIOU_PLUGIN_TASK_LAUNCHER_VIEW_H_
