#ifndef FORIOU_PLUGIN_WIN_TOPER_VIEW_H_
#define FORIOU_PLUGIN_WIN_TOPER_VIEW_H_

#include "DirectUI/UiLib.h"
#include "Util/Win_base.h"
#include "Win_top/Win_top.h"

namespace Foriou {

using namespace UiLib;

class Plugin_view : public Win_base {
public:
    Plugin_view(Logger logger);

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;
    virtual void Notify(TNotifyUI& notifier) override;

private:
    virtual CControlUI* Init() override;

    void display_win_info_(const std::wstring& info);
    void clear_win_info_();

    bool is_capturing_;
    HWND captured_;
    void on_capture_window_(HWND hwnd);
    void enable_pin_(bool enable = true);

private:
    bool is_pined_;
    void pin_self_(bool pin = true);

private:
    Plugin::Win_toper app_;
};

}  // of namespace Foriou

#endif // !FORIOU_PLUGIN_WIN_TOPER_VIEW_H_
