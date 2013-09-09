#include "Core/Menu.h"
#include <string>

namespace Foriou { namespace Detail {

CControlUI* Menu::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"menu.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        ::MessageBox(NULL, L"Bad xml file", L"Shell", MB_OK);
    }

    return root;
}

void Menu::adjust_position(POINT pt)
{
    CDuiRect rcWnd;
    GetWindowRect(m_hWnd, &rcWnd);
    int nWidth = rcWnd.GetWidth();
    int nHeight = rcWnd.GetHeight();
    rcWnd.left = pt.x;
    rcWnd.top = pt.y;
    rcWnd.right = rcWnd.left + nWidth;
    rcWnd.bottom = rcWnd.top + nHeight;
    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
    CDuiRect rcWork = oMonitor.rcWork;

    if( rcWnd.bottom > rcWork.bottom ) {
        if( nHeight >= rcWork.GetHeight() ) {
            rcWnd.top = 0;
            rcWnd.bottom = nHeight;
        }
        else {
            rcWnd.bottom = rcWork.bottom;
            rcWnd.top = rcWnd.bottom - nHeight;
        }
    }
    if( rcWnd.right > rcWork.right ) {
        if( nWidth >= rcWork.GetWidth() ) {
            rcWnd.left = 0;
            rcWnd.right = nWidth;
        }
        else {
            rcWnd.right = rcWork.right;
            rcWnd.left = rcWnd.right - nWidth;
        }
    }
    ::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Menu::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"itemclick") {
        auto& item = msg.pSender->GetName();
        std::wstring userdata = msg.pSender->GetUserData();

        // try conversion
        if (!userdata.empty()) try {
            invoke_handler_(std::stoi(userdata));
        }
        catch (std::exception) {
            // ignored
        }
    }
    else if (msg.sType == L"itemselect") {
        this->hide();
    }
}

LRESULT Menu::HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm)
{
    switch (uMsg) {
    case WM_KEYDOWN:
        if (wpm == VK_ESCAPE) this->hide();
        break;

    case WM_KILLFOCUS:
        if (*this != reinterpret_cast<HWND>(wpm)) this->hide();
        break;
    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Menu::show(POINT pt)
{
    this->adjust_position(pt);
    this->ShowWindow(true);
    ::SetWindowPos(*this, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void Menu::hide()
{
    ::ShowWindow(*this, SW_HIDE);
    auto menu = static_cast<CListUI*>(this->GetManager().FindControl(L"menu"));
    auto item = menu->GetItemAt(menu->GetCurSel());
    if (item) {
        auto list_item = static_cast<IListItemUI*>(item->GetInterface(L"ListItem"));
        if (list_item) {
            list_item->Select(false);
        }
    }
}

void Menu::invoke_handler_(Menu::Item_id item)
{
    auto iter = items_.find(item);
    if (iter != items_.end()) {
        iter->second();
    }
}

void Menu::create_self_()
{
    CWindowWnd::CreateDuiWindow(
        NULL,
        nullptr,
        WS_POPUPWINDOW,
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST
    );
}

}}  // of namespace Foriou::Detail

//--------------------------------------------------------
// End of File
//--------------------------------------------------------
