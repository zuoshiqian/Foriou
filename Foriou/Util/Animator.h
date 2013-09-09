#ifndef FORIOU_ANIMATOR_H_
#define FORIOU_ANIMATOR_H_

#include <functional>
#include <map>
#include <stdexcept>
#include <string>

namespace Foriou { namespace Animation {

class Animation_error : public std::runtime_error {
public:
    template <class Msg>
    Animation_error(Msg&& msg)
        : std::runtime_error(std::forward<Msg>(msg))
    {
    }
};

class Attr_error : public Animation_error {
public:
    Attr_error(const std::string& attr)
        : Animation_error("bad attribute: " + attr)
    {
    }
};

/*!
 * \brief   Interface of the system.
 */
class IAnimator {
public:
    virtual ~IAnimator() {}

public:
    int attr(const std::string& name);
    void attr(const std::string& name, int value);

private:
    virtual bool has_attr(const std::string& name) = 0;
    virtual void set_attr(const std::string& name, int val) = 0;
    virtual int get_attr(const std::string& name) = 0;
};

class Animator_initiator {
public:
    Animator_initiator(IAnimator* x) : fx_(x) {}

    Animator_initiator& animate(
        const std::string& name,
        int value,
        std::size_t duration,
        std::function<void()> callback = nullptr
        )
    {
        std::map<std::string, int> p;
        p.insert(std::make_pair(name, value));
        return animate(p, duration, callback);
    }

    Animator_initiator& animate(
        const std::map<std::string, int>& properties,
        std::size_t duration,
        std::function<void()> callback = nullptr
        );

    /*!
     * \brief   stop any exsting animation.
     */
    Animator_initiator& stop();

private:
    IAnimator* fx_;
};

/*! Converter (analog to jQuery $) */
Animator_initiator animator(IAnimator* x);

inline Animator_initiator animator(IAnimator& x)
{
    return animator(&x);
}
// not supported
namespace Easing {


}  // of namespace Easing

}}  // of namespace Foriou

#endif // !FORIOU_ANIMATOR_H_
