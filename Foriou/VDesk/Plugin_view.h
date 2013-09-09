#ifndef FORIOU_PLUGIN_VIRTUAL_DESKTOP_VIEW_H_
#define FORIOU_PLUGIN_VIRTUAL_DESKTOP_VIEW_H_

#include "DirectUI/UiLib.h"
#include "Util/Win_base.h"
#include "VDesk/VDesk.h"
#include <array>

namespace Foriou {

using namespace UiLib;

class Plugin_view : public Win_base {
public:
    Plugin_view(Logger logger);
    virtual ~Plugin_view() throw();

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;
    virtual void Notify(TNotifyUI& notifier) override;

private:
    virtual CControlUI* Init() override;

    void register_hotkeys_();
    void unregister_hotkeys_();

    void highlight_(int i);

    void switch_to_(int i);

private:
    std::array<ATOM, 4> hotkeys_;
};

}  // of namespace Foriou

#endif // !FORIOU_PLUGIN_VIRTUAL_DESKTOP_VIEW_H_
