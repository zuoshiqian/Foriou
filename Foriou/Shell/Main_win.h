#ifndef FORIOU_MAIN_WIN_H_
#define FORIOU_MAIN_WIN_H_

#include "Util/Win_base.h"

namespace Foriou {

namespace Detail {

class Shell_imp;

}

using namespace UiLib;

class Main_win : public Win_base {
public:
    explicit Main_win(Detail::Shell_imp* imp, Logger lg)
        : Win_base(lg),
          shell_(imp)
    {
        assert(shell_ != nullptr);
    }

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;

public:
    virtual void Notify(TNotifyUI& notifier) override;

private:
    virtual CControlUI* Init() override;

private:
    Detail::Shell_imp* shell_;
};

}  // of namespace Foriou

#endif // FORIOU_MAIN_WIN_H_
