#include "Shell/Main_win.h"
#include "Shell/Detail/Shell_imp.h"

namespace Foriou {

LRESULT Main_win::HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm)
{
    switch (uMsg) {
    case WM_SYSCOMMAND:
        switch (wpm) {
        case SC_CLOSE:
            this->AnimatedHideWindow(500, AW_BLEND);
            return 0;

        case SC_MINIMIZE:
            this->AnimatedHideWindow(500, AW_CENTER);
            ::ShowWindow(*this, SW_MINIMIZE);
            return 0;
        }
        break;
    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Main_win::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"click") {
        if (msg.pSender->GetName() == L"sys_close") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (msg.pSender->GetName() == L"sys_min") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        else if (msg.pSender->GetName() == L"task") {
            shell_->on_select(1);
        }
        else if (msg.pSender->GetName() == L"win_top") {
            shell_->on_select(2);
        }
        else if (msg.pSender->GetName() == L"handpad") {
            shell_->on_select(3);
        }
        else if (msg.pSender->GetName() == L"vdesk") {
            shell_->on_select(4);
        }
        else if (msg.pSender->GetName() == L"checker") {
            shell_->on_select(5);
        }
    }
}

CControlUI* Main_win::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"main_win.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        this->DoLog(error, L"Bad xml file");
        ::MessageBox(NULL, L"Bad xml file", L"Main win", MB_OK | MB_ICONERROR);
    }
    return root;
}

//--------------------------------------------------------------------------------
}  // of namespace Foriou

//---------------------------------------------------------------
// End of File
//---------------------------------------------------------------
