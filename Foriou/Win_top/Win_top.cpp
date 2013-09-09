#include "Win_top/Win_top.h"
#include <algorithm>
#include <cassert>

namespace Helper = Foriou::Plugin;

/**
 * @pre-condition   none
 * @post-condition  all the windows which have been set is untopmost
 */
Helper::Win_toper::~Win_toper()
{
    this->untop_all();
}

/**
 * @pre-condition   win is a window regardless its state
 * @post-condition  win is topmost
 */
void Helper::Win_toper::top(HWND hwnd)
{
    auto win = this->top_level(hwnd);
    if (std::find(top_windows_.begin(), top_windows_.end(), win) ==
        top_windows_.end()) {
        if (::IsWindow(win) && !this->filter(hwnd)) {
            ::SetWindowPos(
                win,
                HWND_TOPMOST,
                0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE
            );
            top_windows_.push_back(win);
        }
    }
}

/**
 * @pre-condition   win is a window regardless its state
 * @post-condition  win is no longer topmost
 */
void Helper::Win_toper::untop(HWND hwnd)
{
    auto win = this->top_level(hwnd);
    auto iter = std::find(top_windows_.begin(), top_windows_.end(), win);
    if (iter != top_windows_.end()) {
        ::SetWindowPos(
            *iter,
            HWND_NOTOPMOST,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE
        );
        top_windows_.erase(iter);
    }
}

/**
 * @pre-condition   none
 * @post-condition  all the topmost windows have been canceled
 */
void Helper::Win_toper::untop_all()
{
    std::for_each(top_windows_.begin(), top_windows_.end(), [](HWND win){
        ::SetWindowPos(
            win,
            HWND_NOTOPMOST,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE
        );
    });

    top_windows_.clear();
}

/**
 * @pre-condition   win is a window regardless its state
 * @post-condition  return the state of win
 */
bool Helper::Win_toper::is_top(HWND hwnd) const
{
    auto win = this->top_level(hwnd);
    return std::find(top_windows_.begin(), top_windows_.end(), win) !=
           top_windows_.end();
}

/**
 * @pre-condition   hwnd is a window handle
 * @post-condition  string_t is not empty
 */
Helper::Win_toper::string_t
    Helper::Win_toper::description(HWND hwnd) const
{
    if (!::IsWindow(hwnd)) return L"";

    auto win = this->top_level(hwnd);

    WCHAR title[MAX_PATH] = {};
    ::GetWindowText(win, title, MAX_PATH);

    return string_t(title);
}

/**
 * @pre-condition   the hwnd is a window
 * @post-condition  return the top-level window
 */
HWND Helper::Win_toper::top_level(HWND hwnd) const
{
    HWND top_level = hwnd;
    while (auto parent =
           reinterpret_cast<HWND>(::GetWindowLong(top_level, GWL_HWNDPARENT))
           )
    {
        top_level = parent;
    }

    return top_level;
}

/**
 * @pre-condition   hwnd is valid
 * @post-condition  return true of false
 */
bool Helper::Win_toper::filter(HWND hwnd) const
{
    assert(::IsWindow(hwnd));

    // filter all toolbar windows which has no icon in
    // the taskbar.
    return ::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW;
}
