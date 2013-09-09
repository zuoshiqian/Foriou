#ifndef FORIOU_PLUGIN_MAIL_CHECKER_VIEW_H_
#define FORIOU_PLUGIN_MAIL_CHECKER_VIEW_H_

#include "DirectUI/UiLib.h"
#include "Util/Win_base.h"
#include "Mail_checker/Mail_checker.h"

namespace Foriou {

using namespace UiLib;

class Plugin_app;

class Plugin_view : public Win_base {
public:
    Plugin_view(Plugin_app* app, Logger lg);
    virtual ~Plugin_view() throw();

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;
    virtual void Notify(TNotifyUI& notifier) override;

private:
    virtual CControlUI* Init() override;

private:
    enum { mail_check_timer = 1 };
    enum { mail_check_interval = 15 * 1000 };

    bool is_checking_;

    void start_check_();
    void stop_check_();

private:
    void clear_all_input_();

private:
    Plugin_app* plugin_;
    Mail_checker app_;
};

}  // of namespace Foriou

#endif // !FORIOU_PLUGIN_MAIL_CHECKER_VIEW_H_
