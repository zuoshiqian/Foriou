#include "Task_launcher/Plugin_view.h"
#include "Task_launcher/Task_scheduler.h"
#include <boost/log/trivial.hpp>
#include <ShlObj.h>
#include <comdef.h>
#include <atlbase.h>

namespace Foriou {

namespace {

inline const wchar_t* task_name(const Task& t)
{
    return t.name().c_str();
}

inline const wchar_t* task_time(const Task& t)
{
    return to_simple_wstring(t.trigger().time()).c_str();
}

inline const wchar_t* task_freq(const Task& t)
{
    switch (t.trigger().freq()) {
    case Trigger::once: return L"once";
    case Trigger::daily: return L"daily";
    case Trigger::weekly: return L"weekly";
    case Trigger::monthly: return L"monthly";
    }
    assert(!"Bad Task Frequency");
    return L"";
}

inline const wchar_t* task_action(const Task& t)
{
    return t.action().exe().c_str();
}

inline const wchar_t* task_brief(const Task& t)
{
    return t.brief().c_str();
}

bool check_freq(const std::wstring& freq)
{
    return freq == L"once" ||
            freq == L"daily" ||
            freq == L"weekly" ||
            freq == L"monthly";
}

inline void throw_if_failed(HRESULT hr)
{
    if (FAILED(hr)) throw _com_error(hr);
}

std::wstring select_exe(HWND parent)
try {
    CComPtr<IFileOpenDialog> open_dlg;
    throw_if_failed(open_dlg.CoCreateInstance(__uuidof(FileOpenDialog)));
    throw_if_failed(open_dlg->Show(parent));

    CComPtr<IShellItem> item;
    throw_if_failed(open_dlg->GetResult(&item));

    PWSTR path = nullptr;
    throw_if_failed(item->GetDisplayName(SIGDN_FILESYSPATH, &path));

    std::wstring res(path);
    ::CoTaskMemFree(path);

    return res;
}
catch (_com_error) {
    return L"";
}

}  // of namespace unnamed

Plugin_view::Plugin_view(Logger logger)
    : Win_base(logger), list_(), detail_()
{
    assert(Task_scheduler::get()->is_usable());
}

LRESULT Plugin_view::HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm)
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

    case WM_TIMER:
        if (wpm == update_scheduler && ::IsWindowVisible(*this)) {
            Task_scheduler::get()->update();
        }
        break;
    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Plugin_view::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"windowinit") {
        list_ = static_cast<CListUI*>(this->GetManager().FindControl(L"win_list"));
        list_->SetTextCallback(this);

        detail_ = this->GetManager().FindControl(L"detail");

        enable_createion_(false);

        Task_scheduler::get()->attach(this);

        SetTimer(*this, update_scheduler, update_duration, nullptr);
    }
    else if (msg.sType == L"click") {
        if (msg.pSender->GetName() == L"sys_close") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (msg.pSender->GetName() == L"sys_min") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        else if (msg.pSender->GetName() == L"new") {
            this->DoLog(trace, L"Click New");
            prepare_creation_();
        }
        else if (msg.pSender->GetName() == L"create") {
            this->DoLog(trace, L"Click Create");
            if (create_task_()) {
                end_creation_();
            }
        }
        else if (msg.pSender->GetName() == L"del") {
            this->DoLog(trace, L"Click Del");
            auto select = list_->GetCurSel();
            if (select != -1) {
                auto item = list_->GetItemAt(select);
                assert(item != nullptr);
                std::wstring name = this->GetItemText(item, 0, 0);

                this->DoLog(info, L"Delete task " + name);
                Task_scheduler::get()->delete_task(name);
                clear_task_detail_();
            }
        }
        else if (msg.pSender->GetName() == L"quit") {
            this->DoLog(trace, L"Click Quit");
            end_creation_();
        }
        else if (msg.pSender->GetName() == L"select_exe") {
            this->DoLog(trace, L"Select exe");
            auto exe = select_exe(*this);
            if (!exe.empty()) {
                this->DoLog(trace, exe + L" has been selected");
                this->GetManager().FindControl(L"action_content")->SetText(exe.c_str());
            }
        }
    }
    else if (msg.sType == L"itemclick") {
        auto sender = msg.pSender;
        auto name = this->GetItemText(sender, 0, 0);
        this->DoLog(trace, std::wstring(L"Select item: ") + name);
        if (auto task = Task_scheduler::get()->get_task(name)) {
            show_task_detail_(*task);
        }
    }
}

LPCTSTR Plugin_view::GetItemText(CControlUI* control, int, int)
{
    return control->GetUserData();
}

CControlUI* Plugin_view::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"task.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        this->DoLog(error, L"Bad xml file");
        ::MessageBox(NULL, L"Bad xml file", L"Task scheduler", MB_OK | MB_ICONERROR);
    }

    return root;
}

void Plugin_view::update(Task_scheduler* ts)
{
    this->DoLog(trace, L"Update task list");

    assert(ts && list_);

    if (ts->size() == list_->GetCount() && ts->size() == 0) return;
    assert(ts->size() != list_->GetCount());

    list_->RemoveAll();
    for (auto& task: *ts) {
        auto item = new CListTextElementUI;
        item->SetUserData(task.name().c_str());
        list_->Add(item);
    }
}

void Plugin_view::clear_task_detail_()
{
    this->DoLog(info, L"Clear task detail");

    auto& mgr = this->GetManager();

    auto name = static_cast<CEditUI*>(mgr.FindControl(L"name_content"));
    auto time = static_cast<CEditUI*>(mgr.FindControl(L"time_content"));
    auto freq = static_cast<CComboUI*>(mgr.FindControl(L"freq"));
    auto action = static_cast<CEditUI*>(mgr.FindControl(L"action_content"));
    auto brief = mgr.FindControl(L"brief_content");

    /// pre-conditions
    /// requires check in release
    /// [
    assert(name && !name->IsEnabled());
    assert(time && !time->IsEnabled());
    assert(freq && !freq->IsEnabled());
    assert(action && !action->IsEnabled());
    assert(brief && !brief->IsEnabled());

    this->Require(MAKE_REQUIRE_ARGUMENT(name && !name->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(time && !time->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(freq && !freq->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(action && !action->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(brief && !brief->IsEnabled()));
    /// ]

    /// do the work
    /// [
    this->DoLog(trace, L"Do the clean");
    name->SetText(nullptr);
    time->SetText(nullptr);
    freq->SelectItem(0);        // option => once
    action->SetText(nullptr);
    brief->SetText(nullptr);
    /// ]
}

void Plugin_view::show_task_detail_(const Task& t)
{
    this->DoLog(trace, L"Show task detail");

    auto& mgr = this->GetManager();

    auto name = static_cast<CEditUI*>(mgr.FindControl(L"name_content"));
    auto time = static_cast<CEditUI*>(mgr.FindControl(L"time_content"));
    auto freq = static_cast<CComboUI*>(mgr.FindControl(L"freq"));
    auto freq_sel = mgr.FindControl(task_freq(t));
    auto action = static_cast<CEditUI*>(mgr.FindControl(L"action_content"));
    auto brief = mgr.FindControl(L"brief_content");

    /// pre-conditions
    /// [
    assert(name && !name->IsEnabled());
    assert(time && !time->IsEnabled());
    assert(freq && !freq->IsEnabled());
    assert(freq_sel);
    assert(action && !action->IsEnabled());
    assert(brief && !brief->IsEnabled());

    this->Require(MAKE_REQUIRE_ARGUMENT(name && !name->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(time && !time->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(freq && !freq->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(freq_sel));
    this->Require(MAKE_REQUIRE_ARGUMENT(action && !action->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(brief && !brief->IsEnabled()));
    /// ]

    /// [
    this->DoLog(trace, L"Do the display");
    name->SetText(task_name(t));
    time->SetText(task_time(t));
    freq->SelectItem(freq->GetItemIndex(freq_sel));
    action->SetText(task_action(t));
    brief->SetText(task_brief(t));
    /// ]
}

void Plugin_view::prepare_creation_()
{
    this->DoLog(trace, L"Prepare creation");

    assert(list_->IsVisible());
    assert(list_->GetWidth() != 0);

    using namespace Animation;
    list_->SetUserData(std::to_wstring(list_->GetWidth()).c_str());
    animator(this).animate(
        "list_width",
        0,
        1000,
        [this]{ creation_ready_(); }
        );
}

void Plugin_view::creation_ready_()
{
    this->DoLog(trace, L"Creation ready");

    list_->SetVisible(false);

    auto& mgr = this->GetManager();

    auto footer1 = mgr.FindControl(L"footer");
    auto footer2 = mgr.FindControl(L"footer2");

    assert(footer1 && footer2);
    assert(footer1->IsVisible());
    assert(!footer2->IsVisible());

    this->Require(MAKE_REQUIRE_ARGUMENT(footer1 && footer2));
    this->Require(MAKE_REQUIRE_ARGUMENT(footer1->IsVisible()));
    this->Require(MAKE_REQUIRE_ARGUMENT(!footer2->IsVisible()));

    footer1->SetVisible(false);
    footer2->SetVisible();

    enable_createion_();
}

void Plugin_view::end_creation_()
{
    this->DoLog(trace, L"End creation");

    using namespace Animation;

    list_->SetVisible();
    animator(this).animate(
        "list_width",
        std::stoi(std::wstring(list_->GetUserData())),
        1000,
        [this]{
            auto& mgr = this->GetManager();
            auto footer1 = mgr.FindControl(L"footer");
            auto footer2 = mgr.FindControl(L"footer2");

            assert(footer1 && footer2);
            assert(!footer1->IsVisible());
            assert(footer2->IsVisible());

            this->Require(MAKE_REQUIRE_ARGUMENT(footer1 && footer2));
            this->Require(MAKE_REQUIRE_ARGUMENT(!footer1->IsVisible()));
            this->Require(MAKE_REQUIRE_ARGUMENT(footer2->IsVisible()));

            footer1->SetVisible();
            footer2->SetVisible(false);

            enable_createion_(false);
        }
    );
}

void Plugin_view::enable_createion_(bool enable)
{
    this->DoLog(trace, enable? L"Enable creation": L"Disable creation");

    auto& mgr = this->GetManager();

    auto name = static_cast<CEditUI*>(mgr.FindControl(L"name_content"));
    auto time = static_cast<CEditUI*>(mgr.FindControl(L"time_content"));
    auto freq = static_cast<CComboUI*>(mgr.FindControl(L"freq"));
    auto action = static_cast<CEditUI*>(mgr.FindControl(L"action_content"));
    auto select_exe = static_cast<CButtonUI*>(mgr.FindControl(L"select_exe"));
    auto brief = mgr.FindControl(L"brief_content");

    assert(name);
    assert(time);
    assert(freq);
    assert(action);
    assert(select_exe);
    assert(brief);

    name->SetEnabled(enable);
    time->SetEnabled(enable);
    freq->SetEnabled(enable);
    action->SetEnabled(enable);
    select_exe->SetEnabled(enable);
    brief->SetEnabled(enable);

    name->SetText(nullptr);
    time->SetText(nullptr);
    freq->SelectItem(0);
    action->SetText(nullptr);
    brief->SetText(nullptr);

    if (enable) {
        name->SetTipValue(L"The name of task");
        time->SetTipValue(L"The time of task");
        action->SetTipValue(L"The action of task");
    }
}

bool Plugin_view::create_task_()
try {
    auto& mgr = this->GetManager();

    auto name = static_cast<CEditUI*>(mgr.FindControl(L"name_content"));
    auto time = static_cast<CEditUI*>(mgr.FindControl(L"time_content"));
    auto freq = static_cast<CComboUI*>(mgr.FindControl(L"freq"));
    auto action = static_cast<CEditUI*>(mgr.FindControl(L"action_content"));
    auto brief = mgr.FindControl(L"brief_content");

    // pre-conditions
    assert(name && name->IsEnabled());
    assert(time && time->IsEnabled());
    assert(freq && freq->IsEnabled());
    assert(action && action->IsEnabled());
    assert(brief && brief->IsEnabled());

    this->Require(MAKE_REQUIRE_ARGUMENT(name && name->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(time && time->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(freq && freq->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(action && action->IsEnabled()));
    this->Require(MAKE_REQUIRE_ARGUMENT(brief && brief->IsEnabled()));


    auto ts = Task_scheduler::get();
    auto error = [](std::string&& s){ throw std::runtime_error(std::move(s)); };

    // check task name
    if (name->GetText().IsEmpty()) error("invalid name");
    if (ts->get_task(name->GetText().GetStringW())) error("task name exits");

    const wchar_t* time_str = time->GetText();
    auto temp_time = std::string(time_str, time_str + time->GetText().GetLength());
    if (time_from_string(temp_time).is_special()) error("invalid time");
    //if (!check_freq(freq->GetText().GetStringW())) error("invalid frequency");
    if (action->GetText().IsEmpty()) error("invalid action");

    auto task_action = Aux::open_file(action->GetText().GetStringW());
    Trigger trigger(time_from_string(temp_time));
    ts->create_task(Task(
        name->GetText().GetStringW(),
        task_action,
        trigger,
        brief->GetText().GetStringW()
        )
    );

    this->DoLog(trace, L"Create task");
    return true;
}
catch (std::exception& e) {
    ::MessageBoxA(*this, e.what(), "Task Creation Error", MB_OK);
    return false;
}

//-------------------------------------------------------------
// Implement the animation interface
bool Plugin_view::has_attr(const std::string& name)
{
    return name == "list_width";
}

int Plugin_view::get_attr(const std::string& name)
{
    assert(list_);
    return list_->GetWidth();
}

void Plugin_view::set_attr(const std::string& name, int value)
{
    assert(list_);
    auto rect = list_->GetPos();
    rect.right = rect.left + value;
    list_->SetPos(rect);
}

}  // of namespace Foriou
//-------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------
