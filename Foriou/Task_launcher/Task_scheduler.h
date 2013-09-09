#ifndef YRUI_TASK_SCHEDULER_H_
#define YRUI_TASK_SCHEDULER_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/archive/text_wiarchive.hpp>
#include <boost/archive/text_woarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <string>
#include <memory>
#include <vector>
#include <taskschd.h>
#include <atlbase.h>
#include <comdef.h>
#include "Util/Utility.h"
#include "Task_launcher/Task_scheduler_fwd.h"

namespace Foriou {

using namespace boost::posix_time;

class Action {
public:
    Action() {}

    explicit Action(const std::wstring& exe, const std::wstring& arg = L"")
        : exe_(exe), arg_(arg)
    {
    }

    const std::wstring& exe() const { return exe_; }
    const std::wstring& arg() const { return arg_; }

private:
    std::wstring exe_;
    std::wstring arg_;

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar & exe_ & arg_;
    }
};

//! define some common action generator.
namespace Aux {

inline Action close_computer()
{
    return Action(L"shutdown", L"-s -t 0");
}

inline Action open_file(const std::wstring& path)
{
    return Action(path);
}

}

class Trigger {
public:
    enum Frequency {
        once,
        daily,
        weekly,
        monthly
    };

    Trigger() {}

    explicit Trigger(ptime time, Frequency freq = once)
        : time_(time), freq_(once)
    {
    }

    ptime time() const { return time_; }
    Frequency  freq() const { return freq_; }

private:
    ptime time_;
    Frequency  freq_;

private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive& ar, const unsigned int) const
    {
        ar & to_iso_string(time_) & freq_;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int)
    {
        std::string time;
        ar & time & freq_;
        time_ = from_iso_string(time);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

class Task {
public:
    Task() {}
    Task(const std::wstring& name,
         const Action& action,
         const Trigger& trigger,
         const std::wstring& brief = L"")
        : name_(name),
          brief_(brief),
          action_(action),
          trigger_(trigger)
    {
    }

    const std::wstring& name() const { return name_; }
    const std::wstring& brief() const { return brief_; }
    const Action& action() const { return action_; }
    const Trigger& trigger() const { return trigger_; }

private:
    std::wstring name_;
    std::wstring brief_;
    Action action_;
    Trigger trigger_;

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar & name_ & brief_ & action_ & trigger_;
    }
};

namespace Detail {

/*!
 * \brief   store task info in the disk.
 */
class Task_saver {
public:
    typedef std::vector<Task> Task_vec;

    explicit Task_saver(const std::wstring& file) : file_(file) {}

    /*! get from disk. */
    Task_vec get();

    /*! put to disk. */
    void put(const Task_vec& vec);

private:
    std::wstring file_;
};

class Dirty_checker {
public:
    explicit Dirty_checker(bool is_dirty = false) : is_dirty_(is_dirty) {}

    bool is_dirty() const { return is_dirty_; }

    void clear() { is_dirty_ = false; }
    void dirt()  { is_dirty_ = true; }

private:
    bool is_dirty_;
};

}  // of namespace Detail

/*!
 * \brief   Singleton.
 */
class Task_scheduler : private Unique_object {
public:
    typedef std::vector<Task>::iterator iterator;
    typedef std::vector<Task>::const_iterator const_iterator;

public:
    static Task_scheduler* get();

    ~Task_scheduler();

public:
    void attach(Scheduler_view* view);

    void create_task(const Task& t);
    void create_task(const Task& t, boost::system::error_code& ec);
    void delete_task(const std::wstring& task_name);

    /*!
     * \brief   get task with the name \a task_name.
     * \return  nullptr if task do not exist.
     */
    const Task* get_task(const std::wstring& task_name);

    /*!
     * This is called by the host, the scheduler has no notion
     * of the change of the status of tasks, thereby client must
     * call this to check the status.
     */
    void update();

    /*!
     * \brief   if !this->is_usable(), the object cannot be used.
     */
    bool is_usable() const { return is_ok_; }

public:
    iterator begin() { return tasks_.begin(); }
    iterator end()   { return tasks_.end(); }
    const_iterator begin() const { return tasks_.begin(); }
    const_iterator end()   const { return tasks_.end(); }

    std::size_t size() const { return tasks_.size(); }

private:
    Task_scheduler();

    /*! synchronize with the underlying mechanism. */
    void synchronize_sys_();
    void synchronize_disk_();

    void notify_();

private:
    bool is_ok_;

    std::vector<Task> tasks_;
    Scheduler_view* view_;

    CComPtr<ITaskService> service_;
    CComPtr<ITaskFolder> folder_;

    Detail::Task_saver saver_;
    Detail::Dirty_checker dirty_;

private:
    static std::unique_ptr<Task_scheduler> inst_;
};

}

#endif // YRUI_TASK_SCHEDULER_H_
