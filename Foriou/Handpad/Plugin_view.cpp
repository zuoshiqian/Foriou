#include "Handpad/Plugin_view.h"
#include "Handpad/Handpad.h"
#include <boost/log/trivial.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Foriou {

namespace {

template <class Callable>
inline void repeat(size_t n, Callable fn)
{
    while (n-- > 0) fn();
}

inline std::wstring generate_note_name()
{
    using namespace boost::posix_time;
    return to_iso_wstring(second_clock::local_time()).substr(0, 14);
}

}  // of namespace unnamed

Plugin_view::Plugin_view(Logger logger)
    : Win_base(logger),
      pad_(L"Notes"),
      list_(),
      title_(),
      content_(),
      is_dirty_()
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

    }

    return Win_base::HandleMessage(uMsg, wpm, lpm);
}

void Plugin_view::Notify(TNotifyUI& msg)
{
    auto sender = msg.pSender;
    auto& type = msg.sType;
    auto& name = sender->GetName();
    if (type == L"windowinit") {
        auto& mgr = this->GetManager();
        list_ = static_cast<CListUI*>(mgr.FindControl(L"note_list"));
        title_ = static_cast<CEditUI*>(mgr.FindControl(L"title"));
        content_ = static_cast<CRichEditUI*>(mgr.FindControl(L"content"));

        list_->SetTextCallback(this);

        assert(list_);
        assert(title_);
        assert(content_);

        update_list_();
    }
    else if (type == L"click") {
        if (name == L"close_win") {
            ::SendMessage(*this, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (name == L"add") {
            this->DoLog(trace, L"Click Add");
            if (is_dirty_) {
                this->DoLog(info, L"Save the old one");
                save_note_();
                update_list_();
            }
            create_note_();
        }
        else if (name == L"delete") {
            this->DoLog(trace, L"Click Delete");
            auto select = list_->GetCurSel();
            if (select != -1) {
                auto item = list_->GetItemAt(select);
                if (item != nullptr) {
                    this->DoLog(trace, L"Do deletion");
                    del_note_(this->GetItemText(item, select, 0));
                    clear_notepad_();
                    update_list_();
                }
            }
        }
        else if (name == L"save") {
            this->DoLog(trace, L"Click Save");
            if (is_dirty_) {
                this->DoLog(info, L"Do save");
                save_note_();
                update_list_();
            }
        }
    }
    else if (type == L"itemclick") {
        this->DoLog(trace, L"Select item");

        if (is_dirty_) {
            save_note_();
            update_list_();
        }

        auto note_name = this->GetItemText(sender, list_->GetItemIndex(sender), 0);
        this->DoLog(info, L"Select " + std::wstring(note_name));

        title_->SetText(note_name);
        content_->SetText(pad_.load(note_name).c_str());

        assert(!is_dirty_);
    }
    else if (type == L"killfocus" && sender == content_) {
        this->DoLog(trace, L"Text changed");
        is_dirty_ = true;
    }
}

bool Plugin_view::invariant() const
{
    return list_ && title_ && content_;
}

CControlUI* Plugin_view::Init()
{
    CDialogBuilder builder;
    auto root = builder.Create(L"handpad_ui.xml", 0, nullptr, &this->GetManager());
    if (!root) {
        this->DoLog(error, L"Bad xml file");
        ::MessageBox(NULL, L"Bad xml file", L"Handpad", MB_OK | MB_ICONERROR);
    }

    return root;
}

LPCTSTR Plugin_view::GetItemText(CControlUI* control, int index, int)
{
    assert(index < pad_.size());
    return pad_[index].c_str();
}

//-----------------------------------------------------------------
//
void Plugin_view::update_list_()
{
    this->DoLog(trace, L"Update note list");

    assert(this->invariant());

    if (list_->GetCount() != pad_.size()) {
        list_->RemoveAll();
        repeat(pad_.size(), [this]{ list_->Add(new CListTextElementUI); });
    }
}

void Plugin_view::clear_notepad_()
{
    this->DoLog(trace, L"Clear notepad display");

    assert(this->invariant());

    title_->SetText(nullptr);
    content_->SetText(nullptr);
    is_dirty_ = false;
}

void Plugin_view::del_note_(const std::wstring& note)
{
    this->DoLog(info, L"Delete note " + note);

    assert(this->invariant());

    pad_.del_note(note);
}

void Plugin_view::create_note_()
{
    this->DoLog(info, L"Create new note");

    assert(this->invariant());
    assert(!is_dirty_);

    clear_notepad_();
}

void Plugin_view::save_note_()
{
    this->DoLog(info, L"Save note");

    assert(this->invariant());
    if (is_dirty_) {
        if (!content_->GetText().IsEmpty()) {
            std::wstring title = title_->GetText().IsEmpty()?
                generate_note_name():
                title_->GetText().GetStringW();
            //BOOST_LOG_TRIVIAL(trace) << "Saving: title -> " << title;
            //BOOST_LOG_TRIVIAL(trace) << "Saving: content -> " << content_->GetText();
            pad_.save(title, make_string_ref(content_->GetText().GetData()));

            this->DoLog(trace, title + L" Saved");
        }

        is_dirty_ = false;
    }

    assert(!is_dirty_);
}

}  // of namespace Foriou
//-------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------
