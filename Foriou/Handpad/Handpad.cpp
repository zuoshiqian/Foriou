#include "Handpad/Handpad.h"
#include <algorithm>
#include <iterator>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/log/trivial.hpp>

namespace Foriou {

namespace {

namespace Fs = boost::filesystem;

bool write_file(const std::wstring& file, boost::wstring_ref content)
{
    std::wofstream ofs(file);
    if (ofs) {
        std::copy(
            content.begin(),
            content.end(),
            std::ostreambuf_iterator<wchar_t>(ofs)
            );
    }
    return !!ofs;
}

}  // of namespace unnamed

Handpad::Handpad(const std::wstring& folder)
    : dir_(folder)
{
    if (!Fs::exists(dir_)) {
        boost::system::error_code ec;
        if (!Fs::create_directories(dir_, ec) && ec) {
            throw std::invalid_argument(ec.message());
        }
    }

    BOOST_LOG_TRIVIAL(trace) << "Handpad: init";
    std::for_each(Fs::directory_iterator(dir_),
                  Fs::directory_iterator(),
                  [this](const Fs::directory_entry& entry){
                        BOOST_LOG_TRIVIAL(trace) << "Add: " << entry.path().stem();
                        files_.push_back(entry.path().stem().wstring());
                  });
}

Handpad::~Handpad() throw()
{
}

std::wstring Handpad::load(const std::wstring& path)
{
    assert(std::find(files_.begin(), files_.end(), path) != files_.end());
    std::wifstream in(translate_name_(path));
    return std::wstring(
            std::istreambuf_iterator<wchar_t>(in),
            std::istreambuf_iterator<wchar_t>()
        );
}

void Handpad::save(const std::wstring& path, boost::wstring_ref content)
{
    BOOST_LOG_TRIVIAL(trace) << "Handpad: prepare save => " << content;
    if (write_file(translate_name_(path), content)) {
        if (std::find(files_.begin(), files_.end(), path) == files_.end()) {
            files_.push_back(path);
        }
    }
}

bool Handpad::del_note(const std::wstring& note)
{
    boost::system::error_code ec;
    Fs::remove(note, ec);
    auto iter = std::find(files_.begin(), files_.end(), note);
    if (iter != files_.end()) {
        files_.erase(iter);
        Fs::remove(translate_name_(note));
        return true;
    }
    return false;
}

std::wstring Handpad::translate_name_(const std::wstring& path) const
{
    return dir_ + L'/' + path;
}
//////////////////////////////////////////////////////////////////////////
}  // of namespace Foriou

//--------------------------------------------------------
// End of File
//--------------------------------------------------------
