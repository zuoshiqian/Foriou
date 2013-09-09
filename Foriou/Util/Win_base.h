#ifndef FORIOU_WIN_BASE_H_
#define FORIOU_WIN_BASE_H_

#include "Core/IPlugin.h"
#include "DirectUI/UiLib.h"
#include "Util/Utility.h"
#include <functional>
#include <boost/preprocessor/wstringize.hpp>

#define MAKE_REQUIRE_ARGUMENT(exp)  exp, BOOST_PP_WSTRINGIZE(exp)

namespace Foriou {

using namespace UiLib;

typedef std::function<void(Log_level, const std::wstring&)> Logger;

class Win_base : public CWindowWnd, public INotifyUI, private Unique_object {
public:
    explicit Win_base(Logger logger) : logger_(logger)
    {
    }

    virtual ~Win_base() { ::DestroyWindow(*this); }

public:
    void Create();
    bool AnimatedShowWindow(std::size_t time, int flags);
    bool AnimatedHideWindow(std::size_t time, int flags);

public:
    virtual PCWSTR GetWindowClassName() const override;
    virtual UINT   GetClassStyle() const override;

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm) override;

protected:
    CPaintManagerUI& GetManager() { return ui_; }

    void DoLog(Log_level level, const std::wstring& content);
    void Require(bool condition, const std::wstring& errinfo);

private:
    virtual CControlUI* Init() = 0;

private:
    void DoPaintImp_(HDC hdc);
    void FixRoundCorner_();

private:
    CPaintManagerUI ui_;
    Logger logger_;
};

}

#endif // FORIOU_WIN_BASE_H_
