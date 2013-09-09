#include "Core/Tray_icon.h"
#include <cstring>
#include <cwchar>

namespace Foriou {

namespace {

const auto tray_title = L"Foriou";
const auto tray_class = L"FORIOU_TRAY_CLASS";
const UINT WM_TASKBARCREATED = ::RegisterWindowMessage(L"TaskbarCreated");

}

Tray_icon::Tray_icon(HICON icon)
    : win_(), icon_(icon), tray_(), action_()
{
	WNDCLASS wc = {};
	wc.lpfnWndProc = event_router;
	wc.hInstance = ::GetModuleHandle(nullptr);
	wc.lpszClassName = tray_class;

	RegisterClass(&wc);

	win_ = CreateWindowEx(
		WS_EX_TOOLWINDOW,
        tray_class,
		nullptr,
		WS_POPUP | WS_DISABLED,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		wc.hInstance,
		this
		);

    if (!win_) {
        throw std::runtime_error("cannot create tray");
    }

    register_handlers_();
    create_tray_();
}

Tray_icon::~Tray_icon() throw()
{
    if (::IsWindow(win_)) {
        ::DestroyWindow(win_);
    }

    assert(win_ == NULL);
    assert(icon_ == NULL);
    assert(action_ == NULL);
}

void Tray_icon::show_notification(const std::wstring& notify)
{
    assert(::IsWindow(win_));

    NOTIFYICONDATA nid = {sizeof(NOTIFYICONDATA)};

    nid.hWnd = win_;
    nid.uFlags = NIF_INFO;
    nid.uTimeout = 30000;
    nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
    nid.hBalloonIcon = icon_;

    std::wcscpy(nid.szInfoTitle, tray_title);
    std::wcscpy(nid.szInfo, notify.c_str());

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void Tray_icon::create_tray_()
{
    assert(icon_ != NULL);
    assert(win_ != NULL);

	tray_.cbSize = sizeof(NOTIFYICONDATA);
	tray_.hWnd = win_;
	tray_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tray_.uCallbackMessage = WM_SHELL_TRAY;
	tray_.hIcon = icon_;
	std::wcscpy(tray_.szTip, tray_title);

	Shell_NotifyIcon(NIM_ADD, &tray_);
}

void Tray_icon::register_handlers_()
{
    this->add_handler(
        WM_DESTROY,
        [&](WPARAM w, LPARAM l){ return on_destroy_(w, l); }
        );
    this->add_handler(
        WM_TASKBARCREATED,
        [&](WPARAM w, LPARAM l){ return on_refresh_(w, l); }
        );
    this->add_handler(
        WM_SHELL_TRAY,
        [&](WPARAM w, LPARAM l){ return on_notify_(w, l); }
        );
}

int Tray_icon::on_destroy_(WPARAM, LPARAM)
{
    ::Shell_NotifyIcon(NIM_DELETE, &tray_);
    ::DestroyIcon(icon_);

    win_ = NULL;
    icon_ = NULL;
    action_ = nullptr;

    return 0;
}

int Tray_icon::on_refresh_(WPARAM, LPARAM)
{
    // remove existing one
    ::Shell_NotifyIcon(NIM_DELETE,&tray_);
    create_tray_();

    return 0;
}

int Tray_icon::on_notify_(WPARAM w, LPARAM l)
{
    if (action_) {
        auto mouse_msg = static_cast<UINT>(l);
        switch (mouse_msg) {
        case WM_LBUTTONDOWN:
            action_->on_click();
            break;

        case WM_LBUTTONDBLCLK:
            action_->on_dbclick();
            break;

        case WM_RBUTTONDOWN:
            action_->on_right_click();
            break;
        }
    }
    return 0;
}

}  // of namespace Foriou

//------------------------------------------------------
// End of File
//------------------------------------------------------
