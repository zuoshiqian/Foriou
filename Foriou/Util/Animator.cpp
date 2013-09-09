#include "Util/Animator.h"
#include <cassert>
#include <queue>
#include <chrono>
#include <windows.h>

//! \todo remove this
#include <iostream>

namespace Foriou { namespace Animation {
namespace Detail {

class Animation_param {
    std::map<std::string, int> properties_;
    std::size_t duration_;
    std::function<void()> callback_;

public:
    template <class Property>
    Animation_param(Property&& p, std::size_t duration, std::function<void()> cb)
        : properties_(std::forward<Property>(p)),
          duration_(duration),
          callback_(cb)
      {
      }

    std::map<std::string, int>& property() { return properties_; }
    std::size_t duration() const { return duration_; }
    void callback() { if (callback_) callback_(); }
};

class Animator {
public:
    template <class Property>
    Animator(IAnimator* x, Property&& p);

    // show a frame of animation.
    void step();

    bool is_done() const { return is_done_; }

    // call the callback
    //! \pre    is_done().
    void notify_complete();

    IAnimator* animator() { return fx_; }

private:
    typedef std::chrono::system_clock clock_type;
    typedef std::chrono::time_point<clock_type> time_type;
    IAnimator* fx_;
    time_type start_time_;
    Animation_param param_;
    std::map<std::string, int> start_;
    bool is_done_;
};


/*!
 * \brief   Singleton.
 */
class Timeline_t {
public:
    ~Timeline_t() { stop_(); };

    template <class Fx>
    void add_animator(Fx&& animator);

    bool remove_animator(IAnimator* fx);

private:
    enum { tick_unit = 13 };

    void start_();
    void stop_();
    void tick_();

    bool is_work() const { return timer_id_ != 0; }

private:
    friend Timeline_t* timeline();

    Timeline_t() : timer_id_() {}

private:
    Timeline_t(const Timeline_t&);
    Timeline_t(Timeline_t&&);
    Timeline_t& operator=(const Timeline_t&);
    Timeline_t& operator=(Timeline_t&&);

private:
    UINT timer_id_;
    std::map<IAnimator*, std::queue<Animator>> actors_;
    static VOID CALLBACK timer_proc(HWND hwnd, UINT msg, UINT id, DWORD time);
};

Timeline_t* timeline()
{
    static Timeline_t timer;
    return &timer;
}


//! \name   Impl
//! @{
template <class Property>
Animator::Animator(IAnimator* x, Property&& p)
    : fx_(x),
      param_(std::forward<Property>(p)),
      start_time_(clock_type::now()),
      is_done_()
{
    for (auto& elem: param_.property()) {
        std::string name = elem.first;
        //! \note   if the attr is not support, a exception will
        //!         be throwed.
        int value = fx_->attr(name);
        start_.insert(std::make_pair(name, value));
    }
}

void Animator::notify_complete()
{
    if (this->is_done()) {
        param_.callback();
    }
}

void Animator::step()
{
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    // check completeness
    std::map<std::string, int> update_values;
    auto now = clock_type::now();
    auto elapse = duration_cast<milliseconds>(now - start_time_);
    if (elapse.count() > param_.duration()) {
        is_done_ = true;
        // fix the attribute values
        update_values = param_.property();
    }
    else {
        double ratio = double(elapse.count()) / param_.duration();
        for (auto& elem: param_.property()) {
            auto& name = elem.first;
            int start = start_[name];
            //! \todo   Add easing functions
            int new_value = static_cast<int>(start + (elem.second - start) * ratio);
            update_values.insert(std::make_pair(name, new_value));
        }
    }

    // do animation
    //      update all the values
    for (auto& elem: update_values) {
        fx_->attr(elem.first, elem.second);
    }
}


template <class Fx>
void Timeline_t::add_animator(Fx&& animator)
{
    IAnimator* fx = animator.animator();
    actors_[fx].push(std::forward<Fx>(animator));
    if (!is_work()) start_();
}

void Timeline_t::start_()
{
    assert(!is_work());
    timer_id_ = ::SetTimer(NULL, 0, tick_unit, timer_proc);
    if (timer_id_ == 0) {
        Animation_error("Animation: timer was not usable");
    }
}

void Timeline_t::stop_()
{
    // assert(is_work());
    if (is_work()) {
        ::KillTimer(NULL, timer_id_);
        timer_id_ = 0;
    }
    assert(!is_work());
}

void Timeline_t::tick_()
{
    assert(is_work());

    for (auto& actor: actors_) {
        IAnimator* animator = actor.first;
        auto& fx_queue = actor.second;
        auto& fx = fx_queue.front();
        fx.step();
        if (fx.is_done()) {
            try {
                fx.notify_complete();
            }
            catch (...) {
                // ignore all exceptions
            }
            fx_queue.pop();
        }
    }

    // delete all empty queues
    auto iter = actors_.begin();
    while (iter != actors_.end()) {
        if (iter->second.empty()) {
            iter = actors_.erase(iter);
        }
        else {
            ++iter;
        }
    }

    if (actors_.empty()) stop_();
}

bool Timeline_t::remove_animator(IAnimator* fx)
{
    if (actors_.erase(fx)) {
        if (actors_.empty()) stop_();
        return true;
    }
    return false;
}

VOID CALLBACK Timeline_t::timer_proc(HWND hwnd, UINT msg, UINT id, DWORD time)
{
    assert(timeline()->timer_id_ == id);
    timeline()->tick_();
}
//! @}

}  // of namespace Detail

int IAnimator::attr(const std::string& name)
{
    if (!this->has_attr(name)) throw Attr_error(name);
    return this->get_attr(name);
}

void IAnimator::attr(const std::string& name, int value)
{
    if (!this->has_attr(name)) throw Attr_error("bad attr " + name);
    return this->set_attr(name, value);
}

Animator_initiator animator(IAnimator* x)
{
    return Animator_initiator(x);
}

Animator_initiator& Animator_initiator::stop()
{
    Detail::timeline()->remove_animator(fx_);
    return *this;
}

Animator_initiator& Animator_initiator::animate(
    const std::map<std::string, int>& properties,
    std::size_t duration,
    std::function<void()> callback
    )
{
    if (!properties.empty()) {
        Detail::Animation_param par(properties, duration, callback);
        Detail::Animator animator(fx_, std::move(par));
        Detail::timeline()->add_animator(std::move(animator));
    }
    else {
        // if no property is specified, invoke the callback directly.
        if (callback) callback();
    }
    return *this;
}
}}  // of namespace Foriou::Animation

//----------------------------------------------------------------
// End of File
//----------------------------------------------------------------
