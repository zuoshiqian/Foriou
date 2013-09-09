#include "Shell/Timed_dialog.h"

namespace Foriou { namespace Detail {

LRESULT Timed_dialog::HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm)
{
    switch (uMsg) {
    case WM_SYSCOMMAND:
        switch (wpm) {
        case SC_CLOSE:
            this->AnimatedHideWindow(500, AW_BLEND);
            return 0;
        }
        break;

    case WM_TIMER:
        if (wpm == dialog_timer) {
            _Elapse();
            if (_Has_expired()) {
                _Stop_timing();
                this->Close(0);
            }
        }
        break;

    case WM_MOUSEMOVE:
        _Reset_elapse();
        break;
    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

CControlUI* Plugin_view::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(_Xml.c_str(), 0, nullptr, &this->GetManager());
    if (!root) {
        ::MessageBox(NULL, _Xml.c_str(), L"Bad xml file", MB_OK | MB_ICONERROR);
    }

    return root;
}

void Timed_dialog::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"click") {
        auto sender = msg.pSender;
        auto& name = sender->GetName();
        if (name == L"close_win") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
    }
}

UINT Timed_dialog::ShowModal(int timeout_second)
{
    if (timeout_second <== 0) return 0;

    _Timeout_second = timeout_second;
    _Elapse_second = 0;

    return _Start_timing()? Win_base::ShowModal(): 0;
}

bool Timed_dialog::_Start_timing()
{
    assert(::IsWindow(*this));

    return ::SetTimer(*this, dialog_timer, elapse_unit, nullptr) != 0;
}

void Timed_dialog::_Stop_timing()
{
    ::KillTimer(*this, dialog_timer);
}

}}  // of namespace Foriou::Detail

//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-05
//////////////////////////////////////////
