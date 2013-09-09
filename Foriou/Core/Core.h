#ifndef FORIOU_CORE_H_
#define FORIOU_CORE_H_

#include "Core/IPlugin.h"
#include "Core/Plugin_manager.h"
#include "Core/Tray_icon.h"
#include "Core/Menu.h"
#include "Shell/Shell.h"
#include "Util/Utility.h"
#include <memory>
#include <map>
#include <Windows.h>

namespace Foriou {

class Core : public Event_target, public IService {
public:
    Core(HINSTANCE hinst);
    virtual ~Core() throw();

public:
    bool init_app();

    // main loop
    int exec();

    void quit();

//! \name   IService interface
//! @{
public:
    virtual Message send_msg(const Message&) override;

    virtual void config(IPlugin*, const std::wstring& name, const std::wstring& val) override;
    virtual std::wstring config(IPlugin*, const std::wstring& name) override;

    virtual void log(IPlugin*, Log_level, const std::wstring&) override;

    virtual HINSTANCE app_handle() override;
//! @}

private:
    void register_handlers_();
    void register_menus_();

private:
    enum { WM_FORIOU_INIT = WM_USER + 2 };

    int on_init_(WPARAM, LPARAM);

    int on_close_(WPARAM, LPARAM);
    int on_destroy_(WPARAM, LPARAM);

private:
    //! \brief  load plugin.
    //! \throw  std::runtime_error if plugin cannot be load.
    void load_plugin_(const std::wstring& name);

private:
    // app related
    bool has_another_app_();
    void register_app_();
    void unregister_app_();

private:
    HINSTANCE hinst_;
    HWND win_;
    ULONG_PTR gdiplusToken_;

//! \name   resource
//! @{
private:
    std::unique_ptr<Tray_icon> tray_;
    std::unique_ptr<Shell> shell_;
    std::unique_ptr<Detail::Menu> menu_;

    Detail::Plugin_manager mgr_;
//! @}

private:
    friend class Tray_action;
    friend class Shell_action;
};

}  // of namespace Foriou

#endif // !FORIOU_CORE_H_
