#include <exception>
#include <string>
#include <cstdlib>
#include <Windows.h>
#include "Core/Error_handle.h"
#include "Core/Log.h"
#include "Core/IPlugin.h"  // for log level

namespace Foriou { namespace Error {

void terminate_handler()
{
    static std::wstring _category_(L"Terminate");

    // write to log
    do_log(_category_, fatal, L"Program has abnormally terminated");

    if (std::uncaught_exception()) try {
        std::rethrow_exception(std::current_exception());
    }
    catch (std::exception& e) {
        do_log(_category_, fatal, e.what());
    }

    // flush the log in that system may be not close the log file.
    boost::log::core::get()->flush();

    // notify the user
    ::MessageBox(
        NULL,
        L"Program has abnormally terminated, please send me the Foriou.log to "
        L"give me more clue about the crash.",
        _category_.c_str(),
        MB_ICONERROR | MB_OK
        );

    // exit without any cleanup
    std::abort();
}

}}  // of namespace Foriou::Error
