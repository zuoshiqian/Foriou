#ifndef FORIOU_SHELL_TIMED_DIALOG_H_
#define FORIOU_SHELL_TIMED_DIALOG_H_

#include "Util/Win_base.h"

namespace Foriou { namespace Detail {

/*!
 * common base for all the dialogs, providing the functionality
 * of timing.
 */
class Timed_dialog : public Win_base {
public:
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;

public:
    virtual void Notify(TNotifyUI& notifier) override;

protected:
    Timed_dialog(const std::wstring& xml)
        : Win_base(nullptr), _Xml(xml)
    {
    }

    virtual ~Timed_dialog() {}

    UINT ShowModalImp(int timeout_second);
    //void RecoverTimeout() { _Reset_elapse(); }

private:
    virtual CControlUI* Init() override;

//! \name   timing implementation
//! @{
private:
    enum { dialog_timer = 1 };
    enum { elapse_unit = 1000 /* 1 second */ };

    bool _Start_timing();
    void _Stop_timing();

    void _Elapse() { ++_Elapse_second; }
    void _Reset_elapse() { _Elapse_second = 0; }
    bool _Has_expired() const { return _Elapse_second >= _Timeout_second; }

private:
    int _Timeout_second;
    int _Elapse_second;
//! @}

private:
    std::wstring _Xml;
};

}}  // of namespace Foriou::Detail

#endif // !FORIOU_SHELL_TIMED_DIALOG_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-05
//////////////////////////////////////////
