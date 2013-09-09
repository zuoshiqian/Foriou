
#ifndef FORIOU_SHELL_INPUT_DIALOG_H_
#define FORIOU_SHELL_INPUT_DIALOG_H_

#include <boost/optional.hpp>
#include "Shell/Timed_dialog.h"

namespace Foriou { namespace Detail {

class Input_dialog : public Timed_dialog {
public:
    Input_dialog() : Timed_dialog(L"input_dialog.xml") {}

    boost::optional<std::wstring>
        DoModal(const std::wstring& prompt, int timeout);

public:
    virtual void Notify(TNotifyUI& notifier) override;

private:
    enum Dialog_option {
        canceled,
        confirmed = 0x1000
    };
};

}}  // of namespace Foriou::Detail

#endif // !FORIOU_SHELL_INPUT_DIALOG_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-05
//////////////////////////////////////////
