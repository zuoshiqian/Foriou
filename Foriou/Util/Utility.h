#ifndef FORIOU_UTILITY_H_
#define FORIOU_UTILITY_H_

#include <windows.h>
#include <functional>
#include <map>
#include <cassert>

namespace Foriou {

class Noncopyable {
protected:
    Noncopyable() {};

private:
    Noncopyable(const Noncopyable&);
    Noncopyable& operator=(const Noncopyable&);
};

class Nonmovable {
protected:
    Nonmovable() {};

private:
    Nonmovable(Nonmovable&&);
    Nonmovable& operator=(Nonmovable&&);
};

class Unique_object : private Noncopyable, private Nonmovable {};

class Event_target : private Unique_object {
public:
    typedef std::function<int(WPARAM, LPARAM)> Handler;
    virtual ~Event_target() {};

    Handler get_handler(UINT msg);

protected:
    Event_target() {};

    void add_handler(UINT msg, Handler h);

private:
    std::map<UINT, Handler> handlers_;
};

LRESULT CALLBACK event_router(HWND hwnd, UINT msg, WPARAM wpm, LPARAM lpm);

}  // of namespace Foriou

#endif // !FORIOU_UTILITY_H_
