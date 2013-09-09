/**
 *
 *  @file   Win_toper.h:    Interface of the class Win_toper
 *  @ingroup    plugin::helper
 *  @since  12/22/2012
 *  @par    modified at
 *              2013-08-20 15:17
 *
 */

#ifndef WIN_TOPER_H_
#define WIN_TOPER_H_

#include <string>
#include <vector>
#include <Windows.h>

namespace Foriou {
namespace Plugin {

class Win_toper {
public:
    typedef std::wstring string_t;

    /**
     * @brief   default constructor
     */
    Win_toper() {}

    /**
     * @brief   untopmost windows contained
     */
    ~Win_toper();

    /**
     * @brief   set the window to topmost
     */
    void top(HWND win);

    /**
     * @brief   recover the topmost window to normal state
     */
    void untop(HWND win);

    /**
     * @brief   untop all the windows which have been set
     */
    void untop_all();

    /**
     * @brief   judge if the window is topmost
     * @param   win: the window to judge
     * @return  true if the window is topmost; otherwise false
     */
    bool is_top(HWND win) const;

    /**
     * @brief   get the description of the window's top-level window
     * @param   window
     * @return  description
     */
    string_t description(HWND hwnd) const;

private:
    /**
     * @brief   get the top-level window of hwnd
     */
    HWND top_level(HWND hwnd) const;

    /**
     * @brief   filter specific window
     */
    bool filter(HWND hwnd) const;

private:
    std::vector<HWND> top_windows_;
};

}  // of namespace Plugin
}  // of namespace Foriou

#endif  //!WIN_TOPER_H_
