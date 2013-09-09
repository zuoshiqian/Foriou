#include "Util/Win_base.h"
#include <exception>

namespace Foriou {

PCWSTR Win_base::GetWindowClassName() const
{
    return L"FORIOU_WIN_BASE";
}

UINT Win_base::GetClassStyle() const
{
    return UI_CLASSSTYLE_DIALOG;
}

LRESULT Win_base::HandleMessage(UINT uMsg, WPARAM wpm, LPARAM lpm)
{
    switch (uMsg) {
    case WM_CREATE:
        {
        ui_.Init(*this);

        auto root = this->Init();
        if (!root) {
            return 1;
        }

        ui_.AttachDialog(root);
        ui_.AddNotifier(this);

        return 0;
        }

    case WM_NCHITTEST:
        {
        POINT pt;
        pt.x = GET_X_LPARAM(lpm); pt.y = GET_Y_LPARAM(lpm);
	    ::ScreenToClient(*this, &pt);

	    RECT rcClient;
	    ::GetClientRect(*this, &rcClient);

	    if(!::IsZoomed(*this)) {
		    RECT rcSizeBox = ui_.GetSizeBox();
		    if( pt.y < rcClient.top + rcSizeBox.top ) {
			    if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
			    if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
			    return HTTOP;
		    }
		    else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
			    if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
			    if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
			    return HTBOTTOM;
		    }
		    if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
		    if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
	    }

	    RECT rcCaption = ui_.GetCaptionRect();
	    if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		    && pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
			    CControlUI* pControl = static_cast<CControlUI*>(ui_.FindControl(pt));
			    if( pControl && _tcsicmp(pControl->GetClass(), _T("ButtonUI")) != 0 &&
				    _tcsicmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
				    _tcsicmp(pControl->GetClass(), _T("TextUI")) != 0 &&
				    _tcsicmp(pControl->GetClass(), _T("FadeButtonUI")) != 0)
				    return HTCAPTION;
	    }

	    return HTCLIENT;
        }

    case WM_PRINT:
        DoPaintImp_(reinterpret_cast<HDC>(wpm));
        break;

    case WM_SIZE:
        FixRoundCorner_();
        return 0;

    case WM_DESTROY:
        logger_ = nullptr;
        break;
    }

    LRESULT lRes = 0;
    if (ui_.MessageHandler(uMsg, wpm, lpm, lRes)) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wpm, lpm);
}

//--------------------------------------------------------------------------------
bool Win_base::AnimatedShowWindow(std::size_t time, int flags)
{
    return AnimateWindow(*this, time, flags | AW_ACTIVATE) != 0;
}

bool Win_base::AnimatedHideWindow(std::size_t time, int flags)
{
    return AnimateWindow(*this, time, flags | AW_HIDE) != 0;
}

void Win_base::Create()
{
    CWindowWnd::CreateDuiWindow(NULL, nullptr, WS_POPUPWINDOW);
    CWindowWnd::CenterWindow();
}

//--------------------------------------------------------------
void Win_base::DoPaintImp_(HDC hdc)
{
    ::InvalidateRect(*this, nullptr, TRUE);
    ::UpdateWindow(*this);
}

void Win_base::FixRoundCorner_()
{
    SIZE round = ui_.GetRoundCorner();
    if(!::IsIconic(*this) && (round.cx != 0 || round.cy != 0)) {
        CDuiRect rcWnd;
        ::GetWindowRect(*this, &rcWnd);
        rcWnd.Offset(-rcWnd.left, -rcWnd.top);
        rcWnd.right++;
		rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(
            rcWnd.left, rcWnd.top,
            rcWnd.right, rcWnd.bottom,
            round.cx, round.cy
            );
        ::SetWindowRgn(*this, hRgn, TRUE);
        ::DeleteObject(hRgn);
    }
}

void Win_base::DoLog(Log_level level, const std::wstring& content)
{
    if (logger_) {
        logger_(level, content);
    }
}

void Win_base::Require(bool condition, const std::wstring& errinfo)
{
#ifndef NRTCHECK
    if (!condition) {
        this->DoLog(fatal, errinfo);
        std::terminate();
    }
#endif  //!NRTCHECK
}

}
//------------------------------------------------------------
// End of File
//------------------------------------------------------------
