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

#ifndef HISTORYPERIOD_H_
#define HISTORYPERIOD_H_

#include <string>

namespace tizen_browser{
namespace base_ui{

enum class HistoryPeriod
{
    HISTORY_TODAY,
    HISTORY_YESTERDAY,
    HISTORY_LASTWEEK,
    HISTORY_LASTMONTH,
    HISTORY_OLDER
};

std::string toString(HistoryPeriod period);

}
}

#endif /* HISTORYPERIOD_H_ */
