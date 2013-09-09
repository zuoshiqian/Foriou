#ifndef YRUI_TASK_SCHEDULER_ERROR_
#define YRUI_TASK_SCHEDULER_ERROR_

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace Foriou { namespace Error {

enum Task_error {
    unknown = 1,
    task_exists,
    bad_action,
    bad_trigger
};

namespace Detail {

class Task_category : public boost::system::error_category {
public:
    virtual const char* name() const override
    {
        return "Foriou.Task_scheduler";
    }

    virtual std::string message(int value) const override
    {
        switch (value) {
        case task_exists: return "Task exists";
        case bad_action: return "Bad action";
        case bad_trigger: return "Bad trigger";
        default: return "Task error";
        }
    }
};

}  // of namespace Foriou::Error::Detail

inline const boost::system::error_category&
    get_task_category()
{
    static Detail::Task_category inst_;
    return inst_;
}

}}  // of namespace Foriou::Error

namespace boost { namespace system {

template <> struct is_error_code_enum<Foriou::Error::Task_error> {
    static const bool value = true;
};

}}  // of namespace boost::system

namespace Foriou { namespace Error {

//! use object generator to hide the detail.
inline boost::system::error_code make_error_code(Task_error e)
{
    return boost::system::error_code(e, get_task_category());
}

inline void check_error_code(const boost::system::error_code& ec)
{
    if (ec) {
        throw boost::system::system_error(ec);
    }
}

}}  // of namespace Foriou::Error

#endif // YRUI_TASK_SCHEDULER_ERROR_
