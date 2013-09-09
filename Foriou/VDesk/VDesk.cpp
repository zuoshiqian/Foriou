#include "VDesk.h"
#include <cassert>
#include <string>
#include <algorithm>

namespace Foriou { namespace Plugin {

namespace Detail {

namespace {

std::vector<HWND> desktop_windows()
{
    std::vector<HWND> wins;

    auto deskHwnd = GetDesktopWindow();
    auto hwnd     = GetWindow(deskHwnd, GW_CHILD);

    while (hwnd) {
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        if ((::GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE) &&
			!(::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) {
            wins.push_back(hwnd);
        }
    }

    return wins;
}

UINT remove_win_from_desktop(HWND hwnd)
{
    assert(::GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE);
    //assert(::GetParent(hwnd) == ::GetDesktopWindow());

	WINDOWPLACEMENT place = { sizeof(WINDOWPLACEMENT) };
	::GetWindowPlacement(hwnd, &place);
	::ShowWindow(hwnd, SW_HIDE);

    assert(!(::GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE));
	return place.showCmd;
}

void restore_win_to_desktop(HWND hwnd, UINT showcmd)
{
    assert(!(::GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE));
    //assert(::GetParent(hwnd) == ::GetDesktopWindow());

	::ShowWindow(hwnd, showcmd);

    assert(::GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE);
}

void destroy_win_process(HWND hwnd)
{
    DWORD pid = 0;
    ::GetWindowThreadProcessId(hwnd, &pid);
    if (auto handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid)) {
        if (::TerminateProcess(handle, 0)) return;
    }
    throw std::runtime_error("cannot terminate process");
}

std::wstring win_class_name(HWND hwnd)
{
    wchar_t buffer[100] = {};
    ::GetClassName(hwnd, buffer, 100);
    return buffer;
}

std::wstring win_name(HWND hwnd)
{
    wchar_t buffer[100] = {};
    ::GetWindowText(hwnd, buffer, 100);
    return buffer;
}

}  // private implementation

bool VDesktop_state::save_and_close()
{
    if (this->is_active()) {
        // close previous state
        assert(state_.empty());

        // enumerate all windows on the desktop, and filter out
        // system windows, save and close reminda.
        for (auto win: desktop_windows()) {
			if (!is_filtered_(win)) {
				auto showcmd = remove_win_from_desktop(win);
                state_.push_back(Win_state(win, showcmd));
            }
        }

        is_active_ = false;

        return true;
    }

    assert(!this->is_active());
    return false;
}

bool VDesktop_state::restore()
{
    if (!this->is_active()) {
        // restore windows in memory
        for (auto win: state_) {
			restore_win_to_desktop(win.hwnd, win.show_state);
        }

        state_.clear();
        is_active_ = true;

        return true;
    }

    assert(this->is_active());
    return false;
}

void VDesktop_state::destroy_desktop()
{
    assert(!this->is_active());

    if (!state_.empty()) {
        for (auto win: state_) {
			destroy_win_process(win.hwnd);
        }

        state_.clear();
    }

    assert(state_.empty());
    assert(!this->is_active());
}

bool VDesktop_state::is_filtered_(HWND hwnd)
{
    if (filter_) {
        return filter_(hwnd);
    }
    return false;
}

// fitlers
bool filter_current_process(HWND hwnd)
{
    DWORD pid = 0;
    ::GetWindowThreadProcessId(hwnd, &pid);
    return ::GetCurrentProcessId() == pid;
}

}  // Detail implementation

VDesk::VDesk()
	: current_(initial_desk)
{
	using namespace Detail;

	// initialize the predefined four desktops
	desks_.reserve(4);
	desks_.push_back(VDesktop_state(filter_current_process, true));
	desks_.resize(4, VDesktop_state(filter_current_process));
}

/*!
 * switch back to the initial desktop and destroy all extra desktop.
 */
VDesk::~VDesk() throw()
{
	// change back to the initial desktop
	switch_to(initial_desk);

	try {
		// destroy extra desktops
		std::for_each(
			desks_.begin() + 1,		// exclude the initial desktop
			desks_.end(),
			[](Detail::VDesktop_state& vd){
				assert(!vd.is_active());
				vd.destroy_desktop();
		});
	}
	catch (...) {
		// ignore any exceptions
	}
}

void VDesk::switch_to(const int i)
{
	assert(0 <= i && i < desk_limit);

	if (current_ != i) {
		assert(desks_[current_].is_active());
		assert(!desks_[i].is_active());

		desks_[current_].save_and_close();
		desks_[i].restore();

		current_ = i;
	}

	assert(current_ == i && desks_[i].is_active());
}

}}  // of namespace Foriou::Plugin

//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-01
//////////////////////////////////////////
