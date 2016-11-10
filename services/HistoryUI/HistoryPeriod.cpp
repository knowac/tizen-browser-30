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

#include "HistoryPeriod.h"
#include "BrowserLogger.h"
#include "app_i18n.h"

namespace tizen_browser{
namespace base_ui{

std::string toString(HistoryPeriod period)
{
    switch (period) {
    case HistoryPeriod::HISTORY_TODAY:
        return _("IDS_BR_BODY_TODAY");
    case HistoryPeriod::HISTORY_YESTERDAY:
        return _("IDS_BR_BODY_YESTERDAY");
    case HistoryPeriod::HISTORY_LASTWEEK:
        return _("IDS_BR_BODY_LAST_WEEK");
    case HistoryPeriod::HISTORY_LASTMONTH:
        return _("IDS_BR_BODY_LAST_MONTH");
    case HistoryPeriod::HISTORY_OLDER:
        return _("IDS_BR_BODY_OLDER");
    default:
        BROWSER_LOGE("[%s:%d]not handled period ", __PRETTY_FUNCTION__,
                __LINE__);
        return "";
    }
}

}
}
