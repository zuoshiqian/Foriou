#ifndef FORIOU_CORE_APP_CONFIG_H_
#define FORIOU_CORE_APP_CONFIG_H_

/*!
 * \brief   Define global app configuration such as the folder structure.
 */

namespace Foriou { namespace App_config { namespace Path {

//! \name   Folders
//! @{
const auto plugin = L"Plugin/";
const auto ui_resource = L"UIRes/";
const auto shell_resource = L"Resource/";
const auto data_resource = L"Data/";
//! @}

//! \name crucial files
//! @{
const auto shell_config = L"Resource/shell.config";
const auto log_file = L"Foriou.log";
//! @}

//! \name   app related
//! @{
const auto app_name = L"Foriou";
const auto app_window_class = L"FORIOU_CORE_APP";
//! @}

}}}  // of namespace Foriou::App_config::Path

#endif // !FORIOU_CORE_APP_CONFIG_H_
//////////////////////////////////////////
//    End of File
//        Dedicated for myself
//        2013-09-09
//////////////////////////////////////////
