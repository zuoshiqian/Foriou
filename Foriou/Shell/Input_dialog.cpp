#include "Shell/Input_dialog.h"

namespace Foriou { namespace Detail {

boost::optional<std::wstring>
    Input_dialog::DoModal(const std::wstring& prompt, int timeout)
{
    boost::optional<std::wstring> ret;

    auto prompt_control = this->GetManager().FindControl(L"prompt");
    auto input_content = this->GetManager().FindControl(L"input");

    if (prompt_control && input_content) {
        prompt_control->SetText(prompt.c_str());

        if (auto exit_code = this->ShowModalImp(timeout)) {
            if (exit_code == confirmed) {
                ret = input_content->GetText().GetStringW();
            }
        }
    }

    return ret;
}

void Input_dialog::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"click") {
        auto sender = msg.pSender;
        auto& name = sender->GetName();
        if (name == L"ok") {
            this->Close(confirmed);
        }
        else if (name == L"cancel") {
            this->Close(canceled);
        }
    }

    Timed_dialog::Notify(msg);
}

}} // of namespace Foriou::Detail
