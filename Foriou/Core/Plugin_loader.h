/**
 *  @brief  Load plugin module
 *  @author Wu QQ
 *  @date   1/10/2013
 */

#ifndef FY_PLUGIN_LOADER_H_
#define FY_PLUGIN_LOADER_H_

#include "Core/IPlugin.h"

namespace Foriou {

/**
 * @class   Plugin_module
 * @brief   resource class; maintaining the module represented as a IPlugin
 */
class Plugin_module {
public:
    Plugin_module(const std::wstring& module, IService* service);
    ~Plugin_module();

    Plugin_module(Plugin_module&& other)
        : module_(other.module_), plugin_(other.plugin_)
    {
        other.module_ = NULL;
        other.plugin_ = nullptr;
    }

    Plugin_module& operator=(Plugin_module&& other)
    {
        module_ = other.module_;
        plugin_ = other.plugin_;

        other.module_ = NULL;
        other.plugin_ = nullptr;

        return *this;
    }

    /**
     * @brief   return IPlugin which represents the module (Singleton)
     */
    IPlugin* module_obj() { return plugin_; }

private:
    HMODULE  module_;
    IPlugin* plugin_;

private:
    Plugin_module(const Plugin_module&);
    Plugin_module& operator=(const Plugin_module&);
};

}  // of namespace Foriou

#endif  //!FY_PLUGIN_LOADER_H_
