#include "Core/App_config.h"
#include "Task_launcher/Plugin_app.h"
#include "Task_launcher/Plugin_view.h"
#include "Task_launcher/Task_scheduler.h"
#include <boost/log/trivial.hpp>

namespace Foriou {

const std::wstring Plugin_app::name_(L"Task_launcher");

// do nothing meaningful
Plugin_app::Plugin_app(IService* app)
    : app_(app)
{
    assert(app_);
    this->log(trace, L"Constructor");
}

Plugin_app::~Plugin_app() throw()
{
    // it should be destroyed in unload
    this->log(trace, L"Destructor");
    if (app_) {
        this->unload();
    }
    assert(app_ == nullptr);
    assert(view_ == nullptr);
}

const std::wstring& Plugin_app::name()
{
    return name_;
}

bool Plugin_app::load()
{
    this->log(trace, L"Load");

    using namespace std::placeholders;
    view_.reset(new Plugin_view(std::bind(&Plugin_app::log, this, _1, _2)));
    view_->Create();
    if (*view_ == NULL) view_.reset();

    auto data_file = App_config::Path::data_resource + std::wstring(L"task_scheduler.dat");
    return *view_ != NULL && Task_scheduler::create(data_file);
}

void Plugin_app::unload()
{
    this->log(trace, L"Unload");

    view_.reset();
    app_ = nullptr;

    assert(view_ == nullptr);
    assert(app_ == nullptr);
}

Message Plugin_app::request(const Message& msg)
{
    this->log(trace, L"Recieve request");

    assert(view_);
    assert(msg.state() == todo);

    switch (msg.type()) {
    case on_service:
        this->log(info, L"On service");
        view_->ShowWindow(true);
        break;

    case on_end:
        view_->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
        break;

    default:
        return Message(msg.type(), no_content);
    }

    return Message(msg.type(), done);
}

void Plugin_app::log(Log_level level, const std::wstring& content)
{
    if (app_) {
        app_->log(this, level, content);
    }
}

}  // of namespace Foriou

Foriou::IPlugin* create_plugin(Foriou::IService* app)
{
    static Foriou::Plugin_app plugin(app);
    return &plugin;
}

//-----------------------------------------------------------
// End of File
//-----------------------------------------------------------
