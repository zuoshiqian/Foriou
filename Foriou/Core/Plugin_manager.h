#ifndef FORIOU_PLUGIN_MANAGER_H_
#define FORIOU_PLUGIN_MANAGER_H_

#include "Util/Utility.h"
#include "Core/IPlugin.h"
#include "Core/Plugin_loader.h"
#include <vector>
#include <string>

namespace Foriou { namespace Detail {

class Plugin_manager : public Unique_object {
public:
    Plugin_manager(IService* app);
    ~Plugin_manager();

public:
    typedef std::vector<IPlugin*>::iterator iterator;

    //! \throw if one failed to load
    void load_all(const std::wstring& dir);    //载入dir目录中所有插件
    void unload_all();

    Plugin_id load_plugin(const std::wstring& plugin);
    void unload_plugin(Plugin_id pid);

    iterator begin() { return plugins_.begin(); }
    iterator end() { return plugins_.end(); }

    int size() const
    {
        assert(plugins_.size() == modules_.size());
        return plugins_.size();
    }

    IPlugin* get_plugin(Plugin_id pid);

private:
    IService* app_;

private:
    std::vector<IPlugin*> plugins_;
    std::vector<Plugin_module> modules_;
};


}} // of namespace Foriou::Detail


#endif // FORIOU_PLUGIN_MANAGER_H_
