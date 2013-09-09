/*!
 *
 *  A mail checker class implemented using CPop3 from Internet.
 *
 */

#ifndef FORIOU_PLUGIN_APP_MAIL_CHECKER_H_
#define FORIOU_PLUGIN_APP_MAIL_CHECKER_H_

#include "Mail_checker/Push_option.h"
#include "Mail_checker/Pop3.h"
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Foriou {

using namespace boost::posix_time;

class Mail_checker {
private:
    Mail_checker(const Mail_checker&);
    Mail_checker(Mail_checker&&);
    Mail_checker& operator=(const Mail_checker&);
    Mail_checker& operator=(Mail_checker&&);

public:
    Mail_checker() {};
    ~Mail_checker() throw();

    /*!
     * \brief   connect to server.
     * \throw   std::runtime_error on error.
     */
    void connect_server(const std::string& server);

    /*!
     * \brief   set user name as well as password.
     * \throw   std::runtime_error on error.
     */
    void login(const std::string& username, const std::string& passwd);


    /*!
     * \brief   get number of unread emails.
     * \pre     this->is_login().
     * \throw   std::runtime_error on error.
     */
    //int number_of_unread();

    /*!
     * \brief   query whether there is a new email.
     * \pre     this->is_login().
     * \throw   std::runtime_error on error.
     */
    bool has_new_email();

    /*!
     * \brief   query whether you've login.
     */
    bool is_login();

    /*!
     * \brief   clear all status.
     */
    void clear();

private:
    ptime get_recent_email_();
    void mail_error_();

    void reconnect_();

private:
    CPop3 pop3_;
    ptime last_email_time_;

    std::string server_;
    std::string username_;
    std::string password_;
};

}  // of namespace Foriou


#include "Mail_checker/Pop_option.h"
#endif // !FORIOU_PLUGIN_APP_MAIL_CHECKER_H_

///////////////////////////////////////////////////////////////////////
// End of File
//      Dedicated for myself
///////////////////////////////////////////////////////////////////////
