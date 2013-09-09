#ifndef FORIOU_SHELL_QUERY_DIALOG_H_
#define FORIOU_SHELL_QUERY_DIALOG_H_

#include <boost/optional.hpp>
#include "Shell/Timed_dialog.h"

namespace Foriou { namespace Detail {

class Query_dialog : public Timed_dialog {
public:
    Query_dialog();

    boost::optional<bool> DoModal(int timeout);

public:
    virtual void Notify(TNotifyUI& notifier) override;
};

}}  // of namespace Foriou::Detail

#endif // !FORIOU_SHELL_QUERY_DIALOG_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-05
//////////////////////////////////////////
