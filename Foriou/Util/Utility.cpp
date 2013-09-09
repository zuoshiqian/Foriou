#include "Util/Utility.h"
#include <cassert>
#include <iostream>

namespace Foriou {

void Event_target::add_handler(UINT msg, Event_target::Handler h)
{
    assert(h != nullptr && "Event_target: handlers must be non-null");
    handlers_.insert(std::make_pair(msg, h));
}

Event_target::Handler Event_target::get_handler(UINT msg)
{
    auto h = handlers_.find(msg);
    return (h == handlers_.end())? nullptr: h->second;
}

//------------------------------------------------------------------------
LRESULT CALLBACK event_router(HWND hwnd, UINT msg, WPARAM wpm, LPARAM lpm)
{
    Event_target* target = nullptr;

    if (msg == WM_NCCREATE) {
        auto data = reinterpret_cast<CREATESTRUCT*>(lpm);
        target = reinterpret_cast<Event_target*>(data->lpCreateParams);
        SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<LONG>(target));
    }
    else {
        target = reinterpret_cast<Event_target*>(GetWindowLong(hwnd, GWL_USERDATA));
        if (msg == WM_NCDESTROY) {
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0L);
        }
    }

    if (target) {
        // 存在消息处理器，则调用处理器
        if (auto handler = target->get_handler(msg)) {
            return handler(wpm, lpm);
        }
    }

    return DefWindowProc(hwnd, msg, wpm, lpm);
}

}  // of namespace Foriou

//-------------------------------------------------------
// End Of File
//-------------------------------------------------------
