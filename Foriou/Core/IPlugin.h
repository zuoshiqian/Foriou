#ifndef FORIOU_IPLUGIN_H_
#define FORIOU_IPLUGIN_H_

#ifndef FY_PLUGIN_EXPORTS
#define FYAPI extern "C" __declspec(dllimport)
#else
#define FYAPI extern "C" __declspec(dllexport)
#endif

#include <Windows.h>
#include <string>

/*!
 * \file    IPlugin.h
 *
 * Define the interfaces used to associating Core and Plugins.
 */

namespace Foriou {

enum Info_t {
    nothing,

    on_init,
    on_end,

    on_request,
    on_service,

    on_notify,

    on_second_pass,
    on_minute_pass,

    on_timer
};

enum State_t {
    todo,
    done,
    failed,
    no_content
};

/*!
 * \brief   Used to communicate between Core and plugins.
 */
class Message {
    Info_t type_;
    State_t state_;
    std::wstring info_;

public:
    explicit Message(
        Info_t type = nothing,
        State_t state = todo,
        const std::wstring& info = L""
    ) : type_(type), state_(state), info_(info)
    {
    }

    Message(const Message& other)
        : type_(other.type_),
          state_(other.state_),
          info_(other.info_)
    {
    }

    Message(Message&& other)
        : type_(other.type_),
          state_(other.state_),
          info_(std::move(other.info_))
    {
    }

    ~Message() {}

public:
    Info_t type() const { return type_; }
    State_t state() const { return state_; }
    const std::wstring& info() const { return info_; }

private:
    Message& operator=(const Message&);
    Message& operator=(Message&&);
};

class IPlugin;

typedef unsigned int Plugin_id;

enum Log_level {
    trace,
    debug,
    info,
    warning,
    error,
    fatal
};

class IService {
public:
    virtual ~IService() {}

    virtual Message send_msg(const Message&) = 0;

    virtual void config(IPlugin*, const std::wstring& name, const std::wstring& val) = 0;
    virtual std::wstring config(IPlugin*, const std::wstring& name) = 0;

    virtual void log(IPlugin*, Log_level, const std::wstring&) = 0;

    virtual HINSTANCE app_handle() = 0;
};

class IPlugin {
public:
    virtual ~IPlugin() {}

    virtual bool load() = 0;
    virtual void unload() = 0;
    virtual Message request(const Message&) = 0;
    virtual const std::wstring& name() = 0;
};

typedef IPlugin* (*Plugin_factory)(IService*);

}  // of namespace Foriou

// note that the IPlugin* is managed by the DLL
FYAPI Foriou::IPlugin* create_plugin(Foriou::IService*);

#endif // !FORIOU_IPLUGIN_H_
