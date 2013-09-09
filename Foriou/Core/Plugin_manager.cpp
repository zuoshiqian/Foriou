#include <algorithm>
#include <string>
#include <Windows.h>
#include "Plugin_manager.h"

namespace {

//get all file names in the directory
std::vector<std::wstring> get_files(const std::wstring& dir)
{
    std::vector<std::wstring> files;

    WIN32_FIND_DATA dir_data;
    HANDLE hDir = FindFirstFile(dir.c_str(), &dir_data);
    if (hDir == INVALID_HANDLE_VALUE) {
        return files;
    }

    while (FindNextFile(hDir, &dir_data)) {
        files.push_back(dir_data.cFileName);
    }

    FindClose(hDir);
    return files;
}

}  // helper functions

namespace Foriou { namespace Detail {

Plugin_manager::Plugin_manager(IService* app)
    : app_(app)
{
    assert(app != nullptr);
}

Plugin_manager::~Plugin_manager()
{
    if (!plugins_.empty()) {
        unload_all();
    }

    app_ = nullptr;
}

void Plugin_manager::load_all(const std::wstring& dir)
{
    for (auto& file: get_files(dir)) {
        auto full_name = dir + L"\\" + file;
        this->load_plugin(full_name);
    }
    assert(plugins_.size() == modules_.size());
}

void Plugin_manager::unload_all()
{
    for (auto& plugin: plugins_) {
        plugin->unload();
    }

    plugins_.clear();
    modules_.clear();

    assert(plugins_.size() == modules_.size());
}

Plugin_id Plugin_manager::load_plugin(const std::wstring& plugin)
try {
    Plugin_module loader(plugin, app_);
    IPlugin* plugin = loader.module_obj();
    if (plugin->load()) {
        plugins_.push_back(plugin);
        modules_.push_back(std::move(loader));

        assert(plugins_.size() == modules_.size());
        return plugins_.size();
    }
    else {
        return 0;
    }
}
catch (std::exception&) {
    return 0;
}

void Plugin_manager::unload_plugin(Plugin_id pid)
{
    assert(0 < pid && pid <= plugins_.size());
    auto index = pid - 1;
    auto iter = plugins_.begin() + index;
    (*iter)->unload();
    plugins_.erase(iter);
    modules_.erase(modules_.begin() + index);

    assert(plugins_.size() == modules_.size());
}

IPlugin* Plugin_manager::get_plugin(Plugin_id pid)
{
    assert(0 < pid && pid <= plugins_.size());
    assert(plugins_.size() == modules_.size());
    return plugins_[pid - 1];
}

}}  // of namespace Foriou::Detail
