#ifndef FORIOU_TRAY_ICON_H_
#define FORIOU_TRAY_ICON_H_

#include "Util/Utility.h"
#include <Windows.h>
#include <Shellapi.h>
#include <string>
#include <memory>

namespace Foriou {

class ITray_action {
public:
    virtual ~ITray_action() {}

    virtual void on_click() = 0;
    virtual void on_right_click() = 0;
    virtual void on_dbclick() = 0;
};

class Tray_icon : private Event_target {
public:
    /*!
     * \brief   create a tray using \a icon.
     * \throw   std::runtime_error if creation failed.
     */
    Tray_icon(HICON icon);

    ~Tray_icon() throw();

    void set_action(ITray_action* action) { action_.reset(action); }

    void show_notification(const std::wstring& notify);

private:
    enum { WM_SHELL_TRAY = WM_USER + 1 };

private:
    void register_handlers_();

    int on_destroy_(WPARAM, LPARAM);
    int on_refresh_(WPARAM, LPARAM);
    int on_notify_(WPARAM, LPARAM);

private:
    void create_tray_();

private:
    HWND win_;
    NOTIFYICONDATA tray_;
    HICON icon_;
    std::unique_ptr<ITray_action> action_;
};

}

#endif // !FORIOU_TRAY_ICON_H_
