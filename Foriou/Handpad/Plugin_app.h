#ifndef FORIOU_PLUGIN_HANDPAD_APP_H_
#define FORIOU_PLUGIN_HANDPAD_APP_H_

#include "Core/IPlugin.h"
#include "Util/Utility.h"
#include <memory>

namespace Foriou {

class Plugin_view;

class Plugin_app : private Unique_object, public IPlugin {
public:
    Plugin_app(IService* app);
    virtual ~Plugin_app() throw();

    virtual bool load() override;
    virtual void unload() override;
    virtual Message request(const Message&) override;
    virtual const std::wstring& name() override;

public:
    void log(Log_level level, const std::wstring& content);

private:
    static const std::wstring name_;

private:
    IService* app_;
    std::unique_ptr<Plugin_view> view_;
};

}  // of namespace Foriou

#endif // !FORIOU_PLUGIN_HANDPAD_APP_H_
