#ifndef FORIOU_PLUGIN_HANDPAD_VIEW_H_
#define FORIOU_PLUGIN_HANDPAD_VIEW_H_

#include "DirectUI/UiLib.h"
#include "Util/Win_base.h"
#include "Handpad/Plugin_app.h"
#include "Handpad/Handpad.h"

namespace Foriou {

using namespace UiLib;

class Plugin_view : public Win_base, public IListCallbackUI{
public:
    Plugin_view(Logger logger);

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;
    virtual void Notify(TNotifyUI& notifier) override;
    virtual LPCTSTR GetItemText(CControlUI* control, int index, int subitem) override;

    bool invariant() const;

private:
    virtual CControlUI* Init() override;

private:
    void del_note_(const std::wstring& filename);
    void save_note_();
    void create_note_();

    void clear_notepad_();
    void update_list_();

private:
    Handpad pad_;

    CListUI* list_;
    CEditUI* title_;
    CRichEditUI* content_;

    bool is_dirty_;
};

}  // of namespace Foriou

#endif // !FORIOU_PLUGIN_HANDPAD_VIEW_H_
