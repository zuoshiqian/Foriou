#include "Task_launcher/Task_error.h"
#include "Task_launcher/Task_scheduler.h"
#include <cassert>
#include <algorithm>
#include <fstream>
#include <boost/archive/text_wiarchive.hpp>
#include <boost/archive/text_woarchive.hpp>
#include <boost/serialization/vector.hpp>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

namespace Foriou {

namespace Detail {

const auto task_folder = L"Foriou";
const auto task_saver = L"task_sched.dat";

inline void throw_if_failed(HRESULT hr)
{
    if (FAILED(hr)) throw _com_error(hr);
}

inline auto
find(const std::vector<Task>& ts, const std::wstring& name)->decltype(ts.begin())
{
    return std::find_if(std::begin(ts), std::end(ts), [&](const Task& t){
                return t.name() == name;
            });
}

template <int Freq>
void create_trigger_imp(CComPtr<ITriggerCollection> ts, Trigger t)
{
    assert(!"Not implemented yet");
}


inline CComBSTR to_bstr(ptime t)
{
    return to_iso_extended_wstring(t).c_str();
}

inline CComBSTR to_bstr(const std::wstring& str)
{
    return str.c_str();
}

template <>
void create_trigger_imp<Trigger::once>(CComPtr<ITriggerCollection> ts, Trigger t)
{
    CComPtr<ITrigger> trigger;
    throw_if_failed(ts->Create(TASK_TRIGGER_TIME, &trigger));

    CComQIPtr<ITimeTrigger> timer(trigger);
    timer->put_StartBoundary(to_bstr(t.time()));
    timer->put_EndBoundary(to_bstr(t.time() + seconds(1)));
}

inline void create_trigger(CComPtr<ITriggerCollection> ts, Trigger t)
{
    switch (t.freq()) {
    case Trigger::once: return create_trigger_imp<Trigger::once>(ts, t);
    case Trigger::daily: return create_trigger_imp<Trigger::daily>(ts, t);
    case Trigger::weekly: return create_trigger_imp<Trigger::weekly>(ts, t);
    case Trigger::monthly: return create_trigger_imp<Trigger::monthly>(ts, t);
    }
    assert(!"Bad Trigger::Frequency");
}

}  // of namespace Detail

using Detail::throw_if_failed;

std::unique_ptr<Task_scheduler> Task_scheduler::inst_;

Task_scheduler* Task_scheduler::get()
{
    return inst_? inst_.get(): nullptr;
}

Task_scheduler* Task_scheduler::create(const std::wstring& data_file)
{
    if (!inst_) {
        inst_.reset(new Task_scheduler(data_file));
    }
    return inst_.get();
}

Task_scheduler::Task_scheduler(const std::wstring& data_file)
    : is_ok_(), view_(), saver_(data_file)
{
    try {
        throw_if_failed(::CoInitialize(nullptr));
        throw_if_failed(service_.CoCreateInstance(__uuidof(TaskScheduler)));
        throw_if_failed(
            service_->Connect(
                CComVariant(),
                CComVariant(),
                CComVariant(),
                CComVariant()
                )
        );

        throw_if_failed(service_->GetFolder(CComBSTR(L"\\"), &folder_));
        CComPtr<ITaskFolder> subfolder;
        auto hr = folder_->CreateFolder(
            CComBSTR(Detail::task_folder),
            CComVariant(),
            &subfolder
            );
        if (hr != 0x800700b7) throw_if_failed(hr);
        if (!subfolder) {
            folder_->GetFolder(CComBSTR(Detail::task_folder), &subfolder);
        }

        folder_ = subfolder;

        tasks_ = saver_.get();
        synchronize_sys_();
        synchronize_disk_();

        is_ok_ = true;
        assert(service_);
        assert(folder_);
        assert(!dirty_.is_dirty());
    }
    catch (_com_error&) {
        // do nothing
    }
}

Task_scheduler::~Task_scheduler()
{
    synchronize_disk_();

    view_ = nullptr;
    is_ok_ = false;

    folder_.Release();
    service_.Release();

    ::CoUninitialize();
}

void Task_scheduler::attach(Scheduler_view* view)
{
    assert(this->is_usable());

    view_ = view;
    notify_();
}

void Task_scheduler::create_task(const Task& t)
{
    boost::system::error_code ec;
    this->create_task(t, ec);
    Error::check_error_code(ec);
}

//! \todo   fix this: backend may contain items which were not
//!         serialized.
void Task_scheduler::synchronize_sys_()
{
    CComPtr<IRegisteredTaskCollection> tasks;
    throw_if_failed(folder_->GetTasks(0, &tasks));

    LONG count = 0;
    throw_if_failed(tasks->get_Count(&count));

    if (count == tasks_.size()) return;

    dirty_.dirt();

    // clear existing
    std::vector<Task> backup;
    std::swap(backup, tasks_);
    for (LONG i = 1; i <= count; ++i) {
        CComPtr<IRegisteredTask> task;
        CComBSTR name;
        TASK_STATE state;
        throw_if_failed(tasks->get_Item(CComVariant(i), &task));
        throw_if_failed(task->get_Name(&name));
        throw_if_failed(task->get_State(&state));
        if (state != TASK_STATE_DISABLED) {
            std::wstring nm(name);
            auto iter = Detail::find(backup, nm);
            if (iter != std::end(backup)) {
                tasks_.push_back(*iter);
            }
        }
    }
}

void Task_scheduler::synchronize_disk_()
{
    if (dirty_.is_dirty()) {
        saver_.put(tasks_);
        dirty_.clear();
    }

    assert(!dirty_.is_dirty());
}

void Task_scheduler::notify_()
{
    assert(this->is_usable());
    if (view_) view_->update(this);
}

void Task_scheduler::update()
{
    assert(this->is_usable());

    synchronize_sys_();
    if (dirty_.is_dirty()) {
        synchronize_disk_();
        notify_();
    }

    assert(!dirty_.is_dirty());
}

void Task_scheduler::create_task(const Task& t, boost::system::error_code& ec)
try {
    assert(this->is_usable());
    // check argument
    if (Detail::find(tasks_, t.name()) != std::end(tasks_)) {
        ec = Error::task_exists;
        return;
    }

    // create definition
    CComPtr<ITaskDefinition> task_def;
    throw_if_failed(service_->NewTask(0, &task_def));

    // create action
    CComPtr<IActionCollection> actions;
    throw_if_failed(task_def->get_Actions(&actions));

    CComPtr<IAction> action;
    throw_if_failed(actions->Create(TASK_ACTION_EXEC, &action));

    CComQIPtr<IExecAction> exec(action);
    throw_if_failed(exec->put_Path(Detail::to_bstr(t.action().exe())));
    throw_if_failed(exec->put_Arguments(Detail::to_bstr(t.action().arg().c_str())));

    // create trigger
    CComPtr<ITriggerCollection> triggers;
    throw_if_failed(task_def->get_Triggers(&triggers));

    Detail::create_trigger(triggers, t.trigger());

    // settings
    CComPtr<ITaskSettings> settings;
    throw_if_failed(task_def->get_Settings(&settings));
    throw_if_failed(settings->put_DeleteExpiredTaskAfter(L"PT5S"));

    // register
    assert(folder_);
    CComPtr<IRegisteredTask> registered_task;
    throw_if_failed(folder_->RegisterTaskDefinition(
            CComBSTR(t.name().c_str()),
            task_def,
            TASK_CREATE_OR_UPDATE,
            CComVariant(),
            CComVariant(),
            TASK_LOGON_INTERACTIVE_TOKEN,
            CComVariant(L""),
            &registered_task
        )
    );

    tasks_.push_back(t);
    dirty_.dirt();
    synchronize_disk_();
    notify_();
}
catch (_com_error& e) {
    ec = Error::unknown;
}

void Task_scheduler::delete_task(const std::wstring& name)
{
    assert(this->is_usable());
    auto iter = Detail::find(tasks_, name);
    if (iter != std::end(tasks_)) {
        tasks_.erase(iter);
        // I do not care the result
        folder_->DeleteTask(Detail::to_bstr(name), 0);
        synchronize_disk_();
        notify_();
    }
}

const Task* Task_scheduler::get_task(const std::wstring& name)
{
    auto iter = Detail::find(tasks_, name);
    return iter == std::end(tasks_)? nullptr: &*iter;
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
namespace Detail {

//! \todo   fix these: wrong serialization and deserialization.
//! @{
Task_saver::Task_vec Task_saver::get()
{
    std::wifstream ifs(Detail::task_saver);
    Task_vec vec;
    if (ifs) {
        boost::archive::text_wiarchive in(ifs);
        in >> vec;
    }
    //! \bug    always return empty vec.
    return vec;
}

void Task_saver::put(const Task_saver::Task_vec& v)
{
    std::wofstream ofs(file_);
    boost::archive::text_woarchive out(ofs);
    out << v;
}
//! @}

}  // of namespace Detail
}  // of namespace Foriou
