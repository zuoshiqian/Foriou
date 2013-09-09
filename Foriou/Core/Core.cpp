#include "Core/Core.h"
#include "Core/Log.h"
#include "Core/App_config.h"
#include "Core/Error_handle.h"
#include "Core/Common_def.h"
#include "Shell/Shell.h"
#include "Shell/Main_win.h"
#include "DirectUI/UiLib.h"
#include <locale>

namespace Foriou {
namespace {

std::string WcsToMbs( const std::wstring& wcs )
{
    int lengthOfMbs = WideCharToMultiByte( CP_ACP, 0, wcs.c_str(), -1, NULL, 0, NULL, NULL);
    char* mbs = new char[ lengthOfMbs ];
    WideCharToMultiByte( CP_ACP, 0, wcs.c_str(), -1, mbs, lengthOfMbs, NULL, NULL);
    std::string result = mbs;
    delete mbs;
    mbs = NULL;
    return result;
}

//! \name   log related stuff.
//! @{
inline void do_log(IPlugin* p, Log_level level, const std::wstring& content)
{
    do_log(p->name(), level, content);
}

template <class String_type>
inline void do_log(Core*, Log_level level, String_type&& content)
{
    do_log(L"Core", level, std::forward<String_type>(content));
}
//! @}

//! check whether the program is normally executed.
template <class String_type>
inline void app_require(bool condition, String_type&& error)
{
    if (!condition) {
        do_log(L"Core", fatal, std::forward<String_type>(error));
        std::terminate();
    }
}

}  // of namespace unnamed

//! \name   callback actions
//! @{
class Tray_action : public ITray_action {
public:
    Tray_action(Core* core) : core_(core) {}

    virtual void on_click() override
    {
        do_log(core_, trace, "Click the tray");
        assert(::IsWindow(core_->win_));
        auto shell = core_->shell_.get();
        shell->is_minimal()? shell->restore(): shell->minimize();
    }

    virtual void on_dbclick() override
    {
        do_log(core_, trace, "Double click the tray");
        core_->quit();
    }

    virtual void on_right_click() override
    {
        do_log(core_, trace, "Right click the tray");
        POINT pt = {};
        ::GetCursorPos(&pt);
        core_->menu_->show(pt);
    }

    virtual ~Tray_action() { core_ = nullptr; }

private:
    Core* core_;
};

class Shell_action : public IShell_action {
public:
    Shell_action(Core* core) : core_(core) {}

    virtual void on_click() override
    {
        do_log(core_, trace, "Click the Shell");
    }

    virtual void on_dbclick() override
    {
        do_log(core_, trace, "Double click the Shell");
    }

    virtual void on_right_click() override
    {
        do_log(core_, trace, "Right click the Shell");
        POINT pt = {};
        ::GetCursorPos(&pt);
        core_->menu_->show(pt);
    }

    virtual void on_drag() override
    {
        do_log(core_, trace, "Drag the Shell");
    }

    virtual void on_drag_end() override
    {
        do_log(core_, trace, "End drag the Shell");
    }

    virtual void on_select(Plugin_id id) override
    {
        do_log(core_, trace, "Request plugin");
        auto plugin = core_->mgr_.get_plugin(id);
        plugin->request(Message(on_service));
    }

    virtual ~Shell_action() { core_ = nullptr; }

private:
    Core* core_;
};
//! @}

using namespace UiLib;
using namespace App_config;

//! do nothing meaningful.
Core::Core(HINSTANCE hinst)
    : hinst_(hinst), win_(), gdiplusToken_(), mgr_(this)
{
    std::locale::global(std::locale(""));
    std::set_terminate(Error::terminate_handler);
    init_log(WcsToMbs(Path::log_file));
}

//! \brief  init the application.
//! \post   all components are properly initialized and the tool window is created, or
//!         the app remains initialized.
bool Core::init_app()
{
    do_log(this, trace, "Check existing instance");
    if (has_another_app_()) return false;

    do_log(this, trace, "Init DirectUI");
    CPaintManagerUI::SetInstance(this->app_handle());
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + Path::ui_resource);

    do_log(this, trace, "Init Gdiplus and COM");
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, NULL);

    if (FAILED(::CoInitialize(NULL))) return false;

    register_app_();
    register_handlers_();

    do_log(this, trace, "Create core window");
    win_ = CreateWindowEx(WS_EX_TOOLWINDOW,
                          Path::app_window_class,
                          nullptr,
                          WS_POPUP | WS_DISABLED,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          hinst_,
                          this
                          );
    if (win_) try {
        /// note that all resource are managed by RAII, if one of them
        /// failed, an exception will be throwed, I will simply notify the user
        /// and return false signaling failure of initialization.
        /// [
        do_log(this, trace, "Create window success");
        do_log(this, trace, "Init tray");
        tray_.reset(new Tray_icon(::LoadIcon(NULL, IDI_WINLOGO)));
        tray_->set_action(new Tray_action(this));

        shell_.reset(new Shell(WcsToMbs(Path::shell_config)));
        shell_->register_action(new Shell_action(this));

        register_menus_();

        do_log(this, trace, "Load plugins");
        load_plugin_(L"Task_launcher.dll");
        load_plugin_(L"Win_top.dll");
        load_plugin_(L"Handpad.dll");
        load_plugin_(L"VDesk.dll");
        load_plugin_(L"Mail_checker.dll");
        /// ]

        ::PostMessage(win_, WM_FORIOU_INIT, 0, 0);
        do_log(this, info, "Initialization all success");
    }
    catch (std::exception& e) {
        do_log(this, fatal, e.what());
        ::MessageBoxA(NULL, e.what(), "Foriou Initialization", MB_ICONERROR | MB_OK);
        return false;
    }

    return win_ != NULL;
}

Core::~Core()
{
    do_log(this, trace, "Destroy Core");
    if (::IsWindow(win_)) {
        /// as a matter of fact, this clause is entered if the message loop
        /// is abnormally stoped, therefore I simply destroy the window as well
        /// as release resource by hand.
        /// [
        do_log(this, trace, "Destruct all in destructor");
        ::DestroyWindow(win_);
        /// ]
    }

    assert(win_ == NULL);
    assert(shell_ == nullptr);
    assert(tray_ == nullptr);
    assert(menu_ == nullptr);

    app_require(win_ == NULL, "core window is not destroyed");
    app_require(shell_ == nullptr, "shell is not destroyed");
    app_require(tray_ == nullptr, "tray is not destroyed");
    app_require(menu_ == nullptr, "menu is not destroyed");

    /// these functions have no side effect for redundant invocation.
    /// [
    do_log(this, trace, "Finalize all components");
    unregister_app_();
    ::CoUninitialize();
    if (gdiplusToken_) ::GdiplusShutdown(gdiplusToken_);
    /// ]

    hinst_ = NULL;
    gdiplusToken_ = 0;

    do_log(this, trace, "Quit the application");
}

void Core::load_plugin_(const std::wstring& plugin)
{
    auto plugin_path = Path::plugin + plugin;
    if (mgr_.load_plugin(plugin_path) == 0) {
        auto pname = WcsToMbs(plugin);
        auto error_info = "Cannot load plugin " + pname;
        throw std::runtime_error(error_info);
    }
}

bool Core::has_another_app_()
{
    return ::FindWindow(Path::app_window_class, nullptr) != NULL;
}

void Core::register_app_()
{
    WNDCLASS     wndclass = {};

    wndclass.lpfnWndProc   = event_router;
    wndclass.hInstance     = hinst_;
    wndclass.lpszClassName = Path::app_window_class;

    ::RegisterClass(&wndclass);
}

void Core::unregister_app_()
{
    // ignore the result
    ::UnregisterClass(Path::app_window_class, hinst_);
}

void Core::register_menus_()
{
    std::map<
        Detail::Menu::Item_id,
        Detail::Menu::Item_handler>
        items;

    items[menu_top] = [this]{
        do_log(this, trace, "Menu: top");
    };

    items[menu_about] = [this]{
        do_log(this, trace, "Menu: about");
    };

    items[menu_quit] = [this]{
        do_log(this, trace, "Menu: quit");
        this->quit();
    };

    menu_.reset(new Detail::Menu(std::move(items)));
}

void Core::register_handlers_()
{
    this->add_handler(
        WM_CLOSE,
        [&](WPARAM wpm, LPARAM lpm){ return on_close_(wpm, lpm); }
        );
    this->add_handler(
        WM_DESTROY,
        [&](WPARAM wpm, LPARAM lpm){ return on_destroy_(wpm, lpm); }
        );
    this->add_handler(
        WM_FORIOU_INIT,
        [&](WPARAM wpm, LPARAM lpm){ return on_init_(wpm, lpm); }
        );
}

int Core::exec()
{
    assert(::IsWindow(win_));

    shell_->show();

    do_log(this, trace, "Start message loop");
    CPaintManagerUI::MessageLoop();
    do_log(this, trace, "End message loop");

    return 0;
}

void Core::quit()
{
    do_log(this, info, "Signal to quit");

    ::PostMessage(win_, WM_CLOSE, 0, 0);
    //::PostQuitMessage(0);
}

//-------------------------------------------------------
int Core::on_close_(WPARAM wpm, LPARAM lpm)
{
    do_log(this, trace, "Close Window, namely prepare to quit the app");

    do_log(this, trace, "Notify plugins on_end");
    for (auto& plugin: mgr_) {
        plugin->request(Message(on_end));
    }

    // close shell
    shell_->hide();
    menu_->hide();

    ::DestroyWindow(win_);
    ::PostQuitMessage(0);

    return 0;
}

int Core::on_destroy_(WPARAM wpm, LPARAM lpm)
{
    do_log(this, trace, "Destroy Window");
    tray_.reset();
    shell_.reset();
    menu_.reset();
    mgr_.unload_all();
    win_ = NULL;

    assert(win_ == NULL);
    assert(shell_ == nullptr);
    assert(tray_ == nullptr);
    assert(menu_ == nullptr);
    assert(mgr_.size() == 0);

    return 0;
}

int Core::on_init_(WPARAM, LPARAM)
{
    do_log(this, trace, "On_init Foriou");
    for (auto& plugin: mgr_) {
        plugin->request(Message(on_init));
    }
    return 0;
}
//--------------------------------------------------------

Message Core::send_msg(const Message& msg)
{
    assert(msg.state() == todo);

    switch (msg.type()) {
    case on_notify:
        ::MessageBox(NULL, msg.info().c_str(), Path::app_name, MB_OK);
        break;

    default:
        return Message(msg.type(), no_content);
    }

    return Message(msg.type(), done);
}

void Core::config(IPlugin* plugin, const std::wstring& name, const std::wstring& val)
{

}

std::wstring Core::config(IPlugin* plugin, const std::wstring& name)
{
    return L"";
}

void Core::log(IPlugin* p, Log_level l, const std::wstring& c)
{
    do_log(p, l, c);
}

HINSTANCE Core::app_handle()
{
    assert(hinst_ != NULL);
    return hinst_;
}

}  // of namespace Foriou

//-----------------------------------------------------------
// End of File
//-----------------------------------------------------------
