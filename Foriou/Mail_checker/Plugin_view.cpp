#include "Mail_checker/Plugin_view.h"
#include "Mail_checker/Plugin_app.h"
#include "Mail_checker/Mail_checker.h"
#include <boost/log/trivial.hpp>

namespace Foriou {

Plugin_view::Plugin_view(Plugin_app* app, Logger lg)
    : Win_base(lg),
      is_checking_(),
      plugin_(app),
      app_()
{
}

Plugin_view::~Plugin_view()
{
    plugin_ = nullptr;
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

    case WM_TIMER:
        if (wpm == mail_check_timer) {
            this->DoLog(info, L"Check new emails");
            assert(app_.is_login());
            if (app_.has_new_email()) {
                this->DoLog(info, L"New email detected");
                plugin_->notify(L"There is a new email");
            }
        }
        break;
    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Plugin_view::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"windowinit") {

    }
    else if (msg.sType == L"click") {
        auto sender = msg.pSender;
        auto& name = sender->GetName();
        if (name == L"close_win") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (name == L"disable") {
            stop_check_();
            clear_all_input_();

            this->DoLog(info, L"Disable email check");
        }
        else if (name == L"cancel") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (name == L"confirm") {
            auto& mgr = this->GetManager();
            auto user = mgr.FindControl(L"name_content");
            auto pswd = mgr.FindControl(L"passwd_content");
            assert(user && pswd);
            if (user->GetText().IsEmpty()) {
                ::MessageBox(*this, L"Please input username", L"Mail checker", MB_OK);
            }
            else if (pswd->GetText().IsEmpty()) {
                ::MessageBox(*this, L"Please input password", L"Mail checker", MB_OK);
            }
            else try {
                auto list = static_cast<CComboUI*>(mgr.FindControl(L"candidate"));
                assert(list);

                auto select = list->GetItemAt(list->GetCurSel());
                assert(select);

                std::string server = CDuiString(select->GetUserData()).GetStringA();
                std::string username = user->GetText().GetStringA();
                std::string password = pswd->GetText().GetStringA();

                app_.connect_server(server);
                app_.login(username, password);

                start_check_();
                ::MessageBox(*this, L"Set OK", L"Mail checker", MB_OK);
                ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);

                this->DoLog(info, L"Setting ok");
            }
            catch (std::exception& e) {
                ::MessageBoxA(*this, e.what(), "Mail checker", MB_OK | MB_ICONERROR);
            }
        }
    }
}

CControlUI* Plugin_view::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"mail_checker.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        this->DoLog(error, L"Bad xml file");
        ::MessageBox(NULL, L"Bad xml file", L"Mail checker", MB_OK);
    }

    return root;
}

void Plugin_view::clear_all_input_()
{
    auto& mgr = this->GetManager();
    auto user = mgr.FindControl(L"name_content");
    auto pswd = mgr.FindControl(L"passwd_content");

    assert(user && pswd);

    user->SetText(nullptr);
    pswd->SetText(nullptr);
}

void Plugin_view::start_check_()
{
    if (!is_checking_) {
        this->DoLog(trace, L"Start check");
        SetTimer(*this, mail_check_timer, mail_check_interval, nullptr);
        is_checking_ = true;
    }

    assert(is_checking_);
}

void Plugin_view::stop_check_()
{
    if (is_checking_) {
        this->DoLog(trace, L"Stop check");
        ::KillTimer(*this, mail_check_timer);
        is_checking_ = false;
    }

    assert(!is_checking_);
}

}  // of namespace Foriou
//-------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------
