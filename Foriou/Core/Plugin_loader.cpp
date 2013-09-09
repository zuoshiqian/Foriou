#include "Core/Plugin_loader.h"
#include <stdexcept>

namespace {

inline std::string to_string(const std::wstring& wstr)
{
    return std::string(wstr.begin(), wstr.end());
}

const std::string factory("create_plugin");

}  // helper

Foriou::Plugin_module::Plugin_module(const std::wstring& module, IService* service)
    : plugin_()
{
    module_ = ::LoadLibrary(module.c_str());
    if (module_) {
        auto creater = reinterpret_cast<Plugin_factory>(
                ::GetProcAddress(module_, factory.c_str())
            );
        if (creater) {
            plugin_ = creater(service);
            return;
        }
        else {
            ::FreeLibrary(module_);
            module_ = NULL;
        }
    }
    throw std::runtime_error("cannot load module " + to_string(module));
}

Foriou::Plugin_module::~Plugin_module()
{
    plugin_ = nullptr;
    if (module_) {
        ::FreeLibrary(module_);
        module_ = NULL;
    }
}

//-------------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------------
