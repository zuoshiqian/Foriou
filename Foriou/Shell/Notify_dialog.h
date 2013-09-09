#ifndef FORIOU_SHELL_NOTIFY_DIALOG_H_
#define FORIOU_SHELL_NOTIFY_DIALOG_H_

#include "Shell/Timed_dialog.h"

namespace Foriou { namespace Detail {

class Notify_dialog : public Timed_dialog {
public:
    Notify_dialog();

    void DoModal(int timeout);

public:
    virtual void Notify(TNotifyUI& notifier) override;
};

}}  // of namespace Foriou::Detail

#endif // !FORIOU_SHELL_NOTIFY_DIALOG_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-05
//////////////////////////////////////////
