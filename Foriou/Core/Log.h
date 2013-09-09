#ifndef FORIOU_CORE_LOG_H_
#define FORIOU_CORE_LOG_H_

#include <cassert>
#include <utility>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions.hpp>

namespace Foriou {

namespace Detail {


}  // of namespace Detail

inline void init_log(const std::string& file)
{
    namespace Log = boost::log;
    namespace expr = Log::expressions;

    Log::add_common_attributes();

    auto format = expr::stream
        << expr::attr<unsigned>("LineID")
        << ": <" << Log::trivial::severity
        << "> " << expr::smessage;

    auto sink = Log::add_file_log(file);
    sink->set_formatter(format);
    sink->locked_backend()->auto_flush(true);
}

inline void flush_log()
{
    ::boost::log::core::get()->flush();
}

template <class String_type>
inline void do_log(
    const std::wstring& category,
    int level,
    String_type&& content
    )
{
    assert(trace <= level && level <= fatal);
    BOOST_LOG_STREAM_WITH_PARAMS(
        ::boost::log::trivial::logger::get(),
        (::boost::log::keywords::severity = ::boost::log::trivial::severity_level(level))
        )
        << "[" << category << "]"
        << std::forward<String_type>(content);
}

}  // of namespace Foriou

#endif // !FORIOU_CORE_LOG_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-04
//////////////////////////////////////////
