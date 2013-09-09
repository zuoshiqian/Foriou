#ifndef FORIOU_PLUGIN_HANDPAD_H_
#define FORIOU_PLUGIN_HANDPAD_H_

#include "Util/Utility.h"
#include <string>
#include <vector>
#include <boost/utility/string_ref.hpp>

namespace Foriou {

/*!
 * \brief   Handpad manager class.
 */
class Handpad : private Unique_object {
public:
    /*!
     * \brief   Store all notes in \a folder.
     * \throw   if \a folder do not exist, std::invalid_argument is throwed.
     */
    Handpad(const std::wstring& folder);

    ~Handpad() throw();

public:
    typedef std::vector<std::wstring> Container_t;
    typedef Container_t::size_type size_type;
    typedef Container_t::iterator  iterator;
    typedef Container_t::const_iterator const_iterator;
    typedef Container_t::reference reference;
    typedef Container_t::const_reference const_reference;

public:
    bool empty() const      { return files_.empty(); }
    size_type size() const  { return files_.size(); }

    iterator begin() { return files_.begin(); }
    iterator end()   { return files_.end(); }
    const_iterator begin() const { return files_.cbegin(); }
    const_iterator end() const   { return files_.cend(); }

    /*!
     * \pre \a 0 <= i < size();
     */
    reference operator[](size_type i) { return files_[i]; }

    /*!
     * \pre \a 0 <= i < size();
     */
    const_reference operator[](size_type i) const { return files_[i]; }

    const std::wstring& folder() const { return dir_; }

public:
    /*!
     * \brief load file content from \a path.
     * \throw std::invalid_argument if \a path do not exist or invalid.
     */
    std::wstring load(const std::wstring& path);

    /*!
     * \brief    saver.
     * \note    if \a path is nonexistent, a new file will be created.
     */
    void save(const std::wstring& path, boost::wstring_ref content);

    /*!
     * \pre     \a path is a valid path.
     * \return  true is succeeded, fail is \a path do not exist.
     * \post    \a path no longer exists.
     */
    bool del_note(const std::wstring& path);

private:
    std::wstring translate_name_(const std::wstring& path) const;

private:
    std::wstring dir_;
    Container_t files_;
};

template <class T>
inline boost::wstring_ref make_string_ref(T&& ref)
{
    return boost::wstring_ref(std::forward<T>(ref));
}

}  // of namespace Foriou

#endif  //!FORIOU_PLUGIN_HANDPAD_H_
