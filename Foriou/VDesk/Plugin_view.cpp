#include "VDesk/Plugin_view.h"
#include <functional>
#include <boost/log/trivial.hpp>

namespace Foriou {

namespace {

const wchar_t* hotkeys[] = {
    L"VDESK_HOTKEY_1",
    L"VDESK_HOTKEY_2",
    L"VDESK_HOTKEY_3",
    L"VDESK_HOTKEY_4"
};

class Transaction_guard {
public:
    Transaction_guard(function<void()> handler)
        : has_commit_(), handler_(handler)
    {
    }

    ~Transaction_guard() throw()
    {
        if (!has_commit_) try {
            handler_();
        }
        catch (...) {
            // ignore all
        }
    }

    void commit() { has_commit_ = true; }

private:
    typedef Transaction_guard self_type;

    Transaction_guard(const self_type&);
    Transaction_guard(self_type&&);
    self_type& operator=(const self_type&);
    self_type& operator=(self_type&&);

private:
    bool has_commit_;
    std::function<void()> handler_;
};

}  // of namespace unnamed

using Plugin::VDesk;

Plugin_view::Plugin_view(Logger lg)
    : Win_base(lg)
{
    hotkeys_.fill(0);
}

Plugin_view::~Plugin_view() throw()
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

    case WM_HOTKEY:
        {
            this->DoLog(trace, L"Hotkey");
            auto iter = std::find(
                            hotkeys_.begin(),
                            hotkeys_.end(),
                            static_cast<int>(wpm)
                        );

            if (iter != hotkeys_.end()) {
                int i = iter - hotkeys_.begin();
                switch_to_(i);
            }
        }
        break;

    case WM_DESTROY:
        unregister_hotkeys_();
        break;
    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Plugin_view::Notify(TNotifyUI& msg)
{
    if (msg.sType == L"windowinit") {
        register_hotkeys_();
    }
    else if (msg.sType == L"click") {
        auto sender = msg.pSender;
        auto& name = sender->GetName();
        if (name == L"close_win") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (name == L"vdesk1") {
            switch_to_(0);
        }
        else if (name == L"vdesk2") {
            switch_to_(1);
        }
        else if (name == L"vdesk3") {
            switch_to_(2);
        }
        else if (name == L"vdesk4") {
            switch_to_(3);
        }
    }
}


CControlUI* Plugin_view::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"vdesk.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        this->DoLog(error, L"Bad xml file");
        ::MessageBox(NULL, L"Bad xml file", L"Virtual Desktop", MB_OK | MB_ICONERROR);
    }

    return root;
}

void Plugin_view::register_hotkeys_()
{
    assert(::IsWindow(*this));

    Transaction_guard guard([this]{
        ::MessageBox(NULL, L"Cannot create hotkeys", L"Virtual Desktop", MB_OK);
        unregister_hotkeys_();
    });

    for (int i = 0; i < hotkeys_.size(); ++i) {
        hotkeys_[i] = ::GlobalAddAtom(hotkeys[i]);
        if (!hotkeys_[i]) {
            return;
        }

        if (!::RegisterHotKey(*this, hotkeys_[i], MOD_CONTROL, VK_F1 + i)) {
            return;
        }
    }

    guard.commit();
}

void Plugin_view::unregister_hotkeys_()
{
    for (auto& hotkey: hotkeys_) {
        if (!hotkey) {
            assert(::IsWindow(*this));
            ::UnregisterHotKey(*this, hotkey);
            ::GlobalDeleteAtom(hotkey);
            hotkey = 0;
        }
    }
}

void Plugin_view::switch_to_(int i)
{
    this->DoLog(info, L"Switch to " + std::to_wstring(i));

    VDesk::get()->switch_to(i);
    highlight_(i);
}

void Plugin_view::highlight_(int i)
{
    auto tile = static_cast<CTileLayoutUI*>(this->GetManager().FindControl(L"tile"));
    assert(tile);

    CControlUI* c = tile->GetItemAt(i);
    for (int i = 0; i < Plugin::VDesk::desk_limit; ++i) {
        auto btn = tile->GetItemAt(i);
        assert(btn);

        CDuiString text;
        text.Format(L"Desktop %d", i + 1);
        btn->SetText(text);
    }

    c->SetText(c->GetText() + L" selected");
}

}  // of namespace Foriou
//-------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------
