/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CapiWebErrorCodes.h"

#include <web/web_bookmark.h>
#include <web/web_history.h>
#include <web/web_tab.h>

namespace tizen_browser {
namespace tools {
namespace capiWebError {

std::string bookmarkErrorToString(int errorCode)
{
    bp_bookmark_error_defs code = static_cast<bp_bookmark_error_defs>(errorCode);
    switch (code) {
    case (BP_BOOKMARK_ERROR_NONE):
        return "BP_BOOKMARK_ERROR_NONE";
    case (BP_BOOKMARK_ERROR_INVALID_PARAMETER):
        return "BP_BOOKMARK_ERROR_INVALID_PARAMETER";
    case (BP_BOOKMARK_ERROR_OUT_OF_MEMORY):
        return "BP_BOOKMARK_ERROR_OUT_OF_MEMORY";
    case (BP_BOOKMARK_ERROR_IO_ERROR):
        return "BP_BOOKMARK_ERROR_IO_ERROR";
    case (BP_BOOKMARK_ERROR_NO_DATA):
        return "BP_BOOKMARK_ERROR_NO_DATA";
    case (BP_BOOKMARK_ERROR_ID_NOT_FOUND):
        return "BP_BOOKMARK_ERROR_ID_NOT_FOUND";
    case (BP_BOOKMARK_ERROR_DUPLICATED_ID):
        return "BP_BOOKMARK_ERROR_DUPLICATED_ID";
    case (BP_BOOKMARK_ERROR_PERMISSION_DENY):
        return "BP_BOOKMARK_ERROR_PERMISSION_DENY";
    case (BP_BOOKMARK_ERROR_DISK_BUSY):
        return "BP_BOOKMARK_ERROR_DISK_BUSY";
    case (BP_BOOKMARK_ERROR_DISK_FULL):
        return "BP_BOOKMARK_ERROR_DISK_FULL";
    case (BP_BOOKMARK_ERROR_TOO_BIG_DATA):
        return "BP_BOOKMARK_ERROR_TOO_BIG_DATA";
    case (BP_BOOKMARK_ERROR_UNKNOWN):
        return "BP_BOOKMARK_ERROR_UNKNOWN";
    default:
        return "";
    }
}

std::string historyErrorToString(int errorCode)
{
    bp_history_error_defs code = static_cast<bp_history_error_defs>(errorCode);
    switch (code) {
    case (BP_HISTORY_ERROR_NONE):
        return "BP_HISTORY_ERROR_NONE";
    case (BP_HISTORY_ERROR_INVALID_PARAMETER):
        return "BP_HISTORY_ERROR_INVALID_PARAMETER";
    case (BP_HISTORY_ERROR_OUT_OF_MEMORY):
        return "BP_HISTORY_ERROR_OUT_OF_MEMORY";
    case (BP_HISTORY_ERROR_IO_ERROR):
        return "BP_HISTORY_ERROR_IO_ERROR";
    case (BP_HISTORY_ERROR_NO_DATA):
        return "BP_HISTORY_ERROR_NO_DATA";
    case (BP_HISTORY_ERROR_ID_NOT_FOUND):
        return "BP_HISTORY_ERROR_ID_NOT_FOUND";
    case (BP_HISTORY_ERROR_DUPLICATED_ID):
        return "BP_HISTORY_ERROR_DUPLICATED_ID";
    case (BP_HISTORY_ERROR_PERMISSION_DENY):
        return "BP_HISTORY_ERROR_PERMISSION_DENY";
    case (BP_HISTORY_ERROR_DISK_BUSY):
        return "BP_HISTORY_ERROR_DISK_BUSY";
    case (BP_HISTORY_ERROR_DISK_FULL):
        return "BP_HISTORY_ERROR_DISK_FULL";
    case (BP_HISTORY_ERROR_TOO_BIG_DATA):
        return "BP_HISTORY_ERROR_TOO_BIG_DATA";
    case (BP_HISTORY_ERROR_UNKNOWN):
        return "BP_HISTORY_ERROR_UNKNOWN";
    default:
        return "";
    }
}

std::string tabErrorToString(int errorCode)
{
    bp_tab_error_defs code = static_cast<bp_tab_error_defs>(errorCode);
    switch (code) {
    case (BP_TAB_ERROR_NONE):
        return "BP_TAB_ERROR_NONE";
    case (BP_TAB_ERROR_INVALID_PARAMETER):
        return "BP_TAB_ERROR_INVALID_PARAMETER";
    case (BP_TAB_ERROR_OUT_OF_MEMORY):
        return "BP_TAB_ERROR_OUT_OF_MEMORY";
    case (BP_TAB_ERROR_IO_ERROR):
        return "BP_TAB_ERROR_IO_ERROR";
    case (BP_TAB_ERROR_NO_DATA):
        return "BP_TAB_ERROR_NO_DATA";
    case (BP_TAB_ERROR_ID_NOT_FOUND):
        return "BP_TAB_ERROR_ID_NOT_FOUND";
    case (BP_TAB_ERROR_DUPLICATED_ID):
        return "BP_TAB_ERROR_DUPLICATED_ID";
    case (BP_TAB_ERROR_PERMISSION_DENY):
        return "BP_TAB_ERROR_PERMISSION_DENY";
    case (BP_TAB_ERROR_DISK_BUSY):
        return "BP_TAB_ERROR_DISK_BUSY";
    case (BP_TAB_ERROR_DISK_FULL):
        return "BP_TAB_ERROR_DISK_FULL";
    case (BP_TAB_ERROR_TOO_BIG_DATA):
        return "BP_TAB_ERROR_TOO_BIG_DATA";
    case (BP_TAB_ERROR_UNKNOWN):
        return "BP_TAB_ERROR_UNKNOWN";
    default:
        return "";
    }
}

}
}
}
