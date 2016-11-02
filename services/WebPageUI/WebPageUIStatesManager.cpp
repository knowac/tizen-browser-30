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

#include "WebPageUIStatesManager.h"
#include "BrowserLogger.h"

namespace tizen_browser {
namespace base_ui {

WebPageUIStatesManager::WebPageUIStatesManager(WPUState initialState) :
        m_state(initialState)
{
    namesMap = {
        { WPUState::QUICK_ACCESS, "QUICK_ACCESS" },
        { WPUState::EDIT_MODE, "EDIT_MODE" },
        { WPUState::MAIN_WEB_PAGE, "MAIN_WEB_PAGE" },
        { WPUState::MAIN_ERROR_PAGE, "MAIN_ERROR_PAGE" },
        { WPUState::MAIN_INCOGNITO_PAGE, "MAIN_INCOGNITO_PAGE" }
    };
}

void WebPageUIStatesManager::set(WPUState state) {
    m_state = state;
}

bool WebPageUIStatesManager::equals(WPUState state) const
{
    return (m_state == state);
}

bool WebPageUIStatesManager::equals(std::initializer_list<WPUState> states) const
{
    for (auto s : states) {
        if (equals(s))
            return true;
    }
    return false;
}

std::string WebPageUIStatesManager::toString(WPUState state) const
{
    return namesMap.find(state)->second;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
