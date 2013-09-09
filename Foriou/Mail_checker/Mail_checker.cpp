#include "Mail_checker/Mail_checker.h"
#include <sstream>
#include <boost/log/trivial.hpp>

namespace {

}  // of namespace unnamed

namespace Foriou {

void Mail_checker::mail_error_()
{
    CString error;
    pop3_.GetLastError(&error);
    throw std::runtime_error(error);
}


Mail_checker::~Mail_checker() throw()
{
    pop3_.Close();
}

void Mail_checker::connect_server(const std::string& server)
{
    if (!pop3_.Create(server.c_str())) {
        mail_error_();
    }

    server_ = server;
}

void Mail_checker::login(const std::string& username, const std::string& passwd)
{
    if (!pop3_.Connect(username.c_str(), passwd.c_str())) {
        mail_error_();
    }

    username_ = username;
    password_ = passwd;

    last_email_time_ = get_recent_email_();
    if (last_email_time_.is_special()) {
        throw std::runtime_error("Network error");
    }
}

bool Mail_checker::is_login()
{
    return pop3_.IsConnected();
}

void Mail_checker::reconnect_()
    // refresh, or we will always read the same thing.
{
    assert(this->is_login());

    pop3_.Close();

    pop3_.Create(server_.c_str());
    pop3_.Connect(username_.c_str(), password_.c_str());

    assert(this->is_login());
}

bool Mail_checker::has_new_email()
{
    assert(this->is_login());

    reconnect_();
    auto time = get_recent_email_();

    if (time > last_email_time_) {
        last_email_time_ = time;
        return true;
    }

    return false;
}

void Mail_checker::clear()
{
    pop3_.Close();
}

ptime Mail_checker::get_recent_email_()
{
    assert(this->is_login());

    std::vector<CString> emails;
    if (!pop3_.GetMailList(emails)) {
        mail_error_();
    }

    CString date;
    if (!pop3_.GetMailDate(emails.size(), &date)) {
        mail_error_();
    }

    std::istringstream iss((std::string(date)));

    using namespace boost::posix_time;

    auto time_facet = new time_input_facet("%a, %d %b %Y %H:%M:%S");
    iss.imbue(std::locale(iss.getloc(), time_facet));

    ptime time;
    iss >> time;

    return time;
}

}  // of namespace Foriou

///////////////////////////////////////////////////////////////////////
// End of File
//      Dedicated for myself
///////////////////////////////////////////////////////////////////////
