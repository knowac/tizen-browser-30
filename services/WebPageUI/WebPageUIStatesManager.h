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

#ifndef WEBPAGEUISTATESMANAGER_H_
#define WEBPAGEUISTATESMANAGER_H_

#include <map>
#include <string>
#include <initializer_list>
#include <memory>

namespace tizen_browser {
namespace base_ui {

enum class WPUState {
    QUICK_ACCESS,
    EDIT_MODE,
    // displaying web page content
    MAIN_WEB_PAGE,
    MAIN_ERROR_PAGE,
    // displaying incognito message page
    MAIN_INCOGNITO_PAGE
};

class WebPageUIStatesManager
{
public:
    WebPageUIStatesManager(WPUState initialState);
    virtual ~WebPageUIStatesManager() {}

    void set(WPUState state);

    /**
     * @param state The state to compare.
     * @return True if current state equals the passed state
     */
    bool equals(WPUState state) const;

    /**
     * @param states The states to compare.
     * @return True if current state is included in states to compare
     */
    bool equals(std::initializer_list<WPUState> states) const;

    std::string toString(WPUState state) const;

private:
    WPUState m_state;
    std::map<WPUState, std::string> namesMap;

};

typedef std::shared_ptr<WebPageUIStatesManager> WPUStatesManagerPtr;
typedef std::shared_ptr<const WebPageUIStatesManager> WPUStatesManagerPtrConst;

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* WEBPAGEUISTATESMANAGER_H_ */
