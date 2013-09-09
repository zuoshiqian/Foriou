#ifndef FORIOU_SHELL_H_
#define FORIOU_SHELL_H_

/**
 *
 * \file    Shell.h
 * \author	QQ Wu 373490201@qq.com
 *
 */

#include <memory>
#include <string>
#include <boost/optional.hpp>
#include "Util/Utility.h"
#include "Core/IPlugin.h"

namespace Foriou {

namespace Detail {

class Shell_imp;

}

/*!
 * \brief   Callback of shell.
 */
class IShell_action {
public:
    virtual ~IShell_action() {};

    virtual void on_click() = 0;
    virtual void on_dbclick() = 0;
    virtual void on_right_click() = 0;
    virtual void on_drag() = 0;
    virtual void on_drag_end() = 0;
    virtual void on_select(Plugin_id id) = 0;
};

class Timeout_second {
public:
    explicit Timeout_second(int s) : _Second(s)
    {
        assert(s > 0);
    }

    int second() const { return _Second; }

private:
    int _Second;
};

/*!
 * This class represents the shell concept of the app.
 * It is the Spirit object by visually. The tray and menu
 * is managed by the Core object.
 */
class Shell : private Unique_object {
//! \name   Structors
//! @{
public:
    /*!
     * \brief   Init the object using file specified by \a config.
     */
    Shell(const std::string& config);
    ~Shell();
//! @}

//! \name   Actions
//! Interface used by the core.
//! @{
public:
    void register_action(IShell_action* action);

    /*!
     * \brief   Show the shell.
     * \post    The shell is visible.
     */
    void show();

    /*!
     * \brief   Hide the shell.
     * \post    The shell is no longer visible.
     */
    void hide();

    /*!
     * \brief   Set the shell to topmost.
     * \note    Do not change visibility.
     */
    void set_top();

    /*!
     * \brief   Cancel topmost of the Shell.
     * \note    Do not change visibility.
     */
    void cancel_top();

    /*!
     * \brief   Query whether is shell is topmost.
     */
    bool is_top();

    /*!
     * \brief   Minimize the Shell.
     */
    void minimize();

    /*!
     * \brief   Restore the Shell.
     */
    void restore();

    bool is_minimal();

//! @}

//! \name   dialog support
//! @{
    boost::optional<std::wstring>
        require_input(
            const std::wstring& prompt, Timeout_second t = Timeout_second(1)
            );

    void notify_user(const std::wstring& info, Timeout_second t = Timeout_second(1));

    boost::optional<bool>
        query_user(const std::wstring& question, Timeout_second t = Timeout_second(1));
//! @}

//! \name   Impl
//! @{
private:
    std::unique_ptr<Detail::Shell_imp> imp_;
//! @}
};

}  // of namespace Foriou

#endif // FORIOU_SHELL_H_
