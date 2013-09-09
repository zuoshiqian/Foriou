#include "Win_top/Plugin_view.h"
#include "Win_top/Win_top.h"
#include <boost/log/trivial.hpp>

namespace Foriou {
namespace {

inline HWND current_window()
{
    POINT pt;
    GetCursorPos(&pt);
    return WindowFromPoint(pt);
}

}

Plugin_view::Plugin_view(Logger logger)
    : Win_base(logger),
      is_capturing_(),
      captured_(),
      is_pined_()
{
}

LRESULT Plugin_view::HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm)
{
    switch (uMsg) {
    case WM_SYSCOMMAND:
        switch (wpm) {
        case SC_CLOSE:
            this->AnimatedHideWindow(500, AW_BLEND);
            return 0;
        }
        break;

    case WM_RBUTTONDOWN:
        assert(!is_capturing_);
        SetCapture(*this);
        SetCursor(LoadCursor(NULL, IDC_CROSS));
        is_capturing_ = true;
        this->DoLog(trace, L"Start capturing window");
        break;

    case WM_RBUTTONUP:
        if (is_capturing_) {
            auto cur_win = current_window();
            if (cur_win != captured_) on_capture_window_(cur_win);
            ReleaseCapture();
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            is_capturing_ = false;
            this->DoLog(trace, L"End capturing window");
        }
        break;

    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Plugin_view::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"windowinit") {
        clear_win_info_();
        enable_pin_(false);
    }
    else if (msg.sType == L"click") {
        auto sender = msg.pSender;
        auto& name = sender->GetName();
        if (name == L"close_win") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (name == L"pin_win") {
            pin_self_(!is_pined_);
        }
        else if (name == L"pin" && !app_.is_top(captured_)) {
            enable_pin_(false);
            app_.top(captured_);
        }
        else if (name == L"unpin" && app_.is_top(captured_)) {
            app_.untop(captured_);
            enable_pin_(true);
        }
    }
}

void Plugin_view::enable_pin_(bool enable)
{
    auto pin = this->GetManager().FindControl(L"pin");
    auto unpin = this->GetManager().FindControl(L"unpin");

    assert(pin && unpin);

    pin->SetEnabled(enable);
    unpin->SetEnabled(!enable && captured_);
}

void Plugin_view::on_capture_window_(HWND hwnd)
{
    assert(::IsWindow(hwnd));
    assert(hwnd != captured_);

    display_win_info_(app_.description(hwnd));
    enable_pin_(!app_.is_top(hwnd));

    captured_ = hwnd;
}

void Plugin_view::pin_self_(bool pin)
{
    assert(pin != is_pined_);

    auto pin_btn = this->GetManager().FindControl(L"pin_win");
    assert(pin_btn);
    pin_btn->SetText(pin? L"T": L"U");
    pin_btn->SetToolTip(pin? L"The window is pined": L"The window it not pined");
    pin? app_.top(*this): app_.untop(*this);

    is_pined_ = pin;
}

void Plugin_view::clear_win_info_()
{
    auto info = this->GetManager().FindControl(L"info_screen");
    assert(info);
    info->SetText(L"No window selected");
}

void Plugin_view::display_win_info_(const std::wstring& info)
{
    auto screen = this->GetManager().FindControl(L"info_screen");
    assert(screen);
    screen->SetText(info.empty()? L"No description": info.c_str());
}

CControlUI* Plugin_view::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"win_top_ui.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        this->DoLog(error, L"Bad xml file");
        ::MessageBox(NULL, L"Bad xml file", L"Win top", MB_OK | MB_ICONERROR);
    }

    return root;
}

}  // of namespace Foriou
//-------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------
