#include "Shell/Detail/Shell_imp.h"

namespace Foriou {

Shell::Shell(const std::string& config)
    : imp_(new Detail::Shell_imp(config))
{
}

Shell::~Shell()
{
}

void Shell::show()
{
    assert(imp_);
    imp_->show();
}

void Shell::hide()
{
    assert(imp_);
    imp_->hide();
}

void Shell::set_top()
{
    assert(imp_);
    imp_->set_top();
}

void Shell::cancel_top()
{
    assert(imp_);
    imp_->cancel_top();
}

bool Shell::is_top()
{
    assert(imp_);
    return imp_->is_top();
}

void Shell::minimize()
{
    assert(imp_);
    imp_->minimize();
}

void Shell::restore()
{
    assert(imp_);
    imp_->restore();
}

bool Shell::is_minimal()
{
    assert(imp_);
    return imp_->is_minimal();
}

void Shell::register_action(IShell_action* action)
{
    assert(imp_);
    return imp_->register_action(action);
}

boost::optional<std::wstring>
    Shell::require_input(const std::wstring& prompt, Timeout_second timeout)
{
    assert(imp_);
    return imp_->require_input(prompt, timeout);
}

void Shell::notify_user(const std::wstring& info, Timeout_second timeout)
{
    assert(imp_);
    return imp_->notify_user(info, timeout);
}

boost::optional<bool>
    Shell::query_user(const std::wstring& question, Timeout_second timeout)
{
    assert(imp_);
    return imp_->query_user(question, timeout);
}

}  // of namespace Foriou
//--------------------------------------------------------
// End of File
//--------------------------------------------------------
