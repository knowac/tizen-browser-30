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
 *
 *
 */

#ifndef WEB_ENGINE_SETTINGS_H
#define WEB_ENGINE_SETTINGS_H

#include <map>
#include <string>

namespace tizen_browser {
namespace basic_webengine {

enum class WebEngineSettings {
    PAGE_OVERVIEW,
    LOAD_IMAGES,
    ENABLE_JAVASCRIPT,
    REMEMBER_FROM_DATA,
    REMEMBER_PASSWORDS,
    AUTOFILL_PROFILE_DATA,
    SCRIPTS_CAN_OPEN_PAGES,
    SAVE_CONTENT_LOCATION,
    DEFAULT_SEARCH_ENGINE,
    CURRENT_HOME_PAGE
};

// string parameters mapping
const std::map<WebEngineSettings, std::string> PARAMS_NAMES = {
    {WebEngineSettings::PAGE_OVERVIEW, "page_overview"},
    {WebEngineSettings::LOAD_IMAGES, "load_images"},
    {WebEngineSettings::ENABLE_JAVASCRIPT, "enable_javascript"},
    {WebEngineSettings::REMEMBER_FROM_DATA, "remember_form_data"},
    {WebEngineSettings::REMEMBER_PASSWORDS, "remember_passwords"},
    {WebEngineSettings::AUTOFILL_PROFILE_DATA, "autofill_profile_data"},
    {WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES, "scripts_can_open_pages"},
    {WebEngineSettings::SAVE_CONTENT_LOCATION, "save_content_location"},
    {WebEngineSettings::DEFAULT_SEARCH_ENGINE, "default_search_engine"},
    {WebEngineSettings::CURRENT_HOME_PAGE, "current_home_page"}
};

}
}

#endif
