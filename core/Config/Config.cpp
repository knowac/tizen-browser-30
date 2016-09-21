/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include "browser_config.h"
#include "Config.h"
#include "BrowserLogger.h"
#include "Ecore_Wayland.h"
#include "Tools/SettingsEnums.h"
#include <Elementary.h>
#include <app_common.h>

namespace tizen_browser
{
namespace config
{

Config::Config()
{
    m_data["main_service_name"] = std::string("org.tizen.browser.base_UI");
    m_data["favorite_service_name"] = std::string("org.tizen.browser.favoriteservice");
    m_data["DB_FOLDERS"] = std::string(".browser.bookmark.db");
    m_data["DB_SETTINGS"] = std::string(".browser.settings.db");
    m_data["DB_HISTORY"] = std::string(".browser.history.db");
    m_data["DB_SESSION"] = std::string(".browser.session.db");
    m_data["DB_CERTIFICATE"] = std::string(".browser.certificate.db");

    m_data["TOOLTIP_DELAY"] = 0.05;       // time from mouse in to tooltip show
    m_data["TOOLTIP_HIDE_TIMEOUT"] = 2.0; // time from tooltip show to tooltip hide
    m_data["TAB_LIMIT"] = 10;             // max number of open tabs
    m_data["FAVORITES_LIMIT"] = 40;       // max number of added favorites

#   include "ConfigValues.h"

    m_data["resourcedb/dir"] = std::string(app_get_data_path());

    m_keysValues[CONFIG_KEY::HISTORY_TAB_SERVICE_THUMB_HEIGHT] = 315;
    m_keysValues[CONFIG_KEY::HISTORY_TAB_SERVICE_THUMB_WIDTH] = 590;
    m_keysValues[CONFIG_KEY::FAVORITESERVICE_THUMB_HEIGHT] = 261;
    m_keysValues[CONFIG_KEY::FAVORITESERVICE_THUMB_WIDTH] = 319;

    m_keysValues[CONFIG_KEY::URLHISTORYLIST_ITEMS_NUMBER_MAX] = 12;
    m_keysValues[CONFIG_KEY::URLHISTORYLIST_ITEMS_VISIBLE_NUMBER_MAX] = 5;
    m_keysValues[CONFIG_KEY::URLHISTORYLIST_KEYWORD_LENGTH_MIN] = 3;
#if PROFILE_MOBILE
    m_keysValues[CONFIG_KEY::URLHISTORYLIST_ITEM_HEIGHT] = 74;
#else
    m_keysValues[CONFIG_KEY::URLHISTORYLIST_ITEM_HEIGHT] = 82;
#endif

    m_keysValues[CONFIG_KEY::WEB_ENGINE_PAGE_OVERVIEW] = true;
    m_keysValues[CONFIG_KEY::WEB_ENGINE_LOAD_IMAGES] = true;
    m_keysValues[CONFIG_KEY::WEB_ENGINE_ENABLE_JAVASCRIPT] = true;
    m_keysValues[CONFIG_KEY::WEB_ENGINE_REMEMBER_FROM_DATA] = true;
    m_keysValues[CONFIG_KEY::WEB_ENGINE_REMEMBER_PASSWORDS] = true;
    m_keysValues[CONFIG_KEY::WEB_ENGINE_AUTOFILL_PROFILE_DATA] = true;
    m_keysValues[CONFIG_KEY::WEB_ENGINE_SCRIPTS_CAN_OPEN_PAGES] = true;

    m_keysValues[CONFIG_KEY::CACHE_ENABLE_VALUE] = EINA_TRUE;
    m_keysValues[CONFIG_KEY::CACHE_FONT_VALUE] = 0;
    m_keysValues[CONFIG_KEY::CACHE_IMAGE_VALUE] = 2048;
    m_keysValues[CONFIG_KEY::CACHE_INTERVAL_VALUE] = 32;

    m_keysValues[CONFIG_KEY::SAVE_CONTENT_LOCATION] = base_ui::Translations::Device;
    m_keysValues[CONFIG_KEY::DEFAULT_SEARCH_ENGINE] = base_ui::Translations::Google;
    m_keysValues[CONFIG_KEY::CURRENT_HOME_PAGE] = "http://www.samsung.com";
}

boost::any Config::get(const std::string& key)
{
    return m_data[key];
}

const boost::any& Config::get(const CONFIG_KEY& key) const
{
    return m_keysValues.find(key)->second;
}

void Config::set(const std::string & key, const boost::any & value)
{
    m_data[key] = value;
}

bool Config::isMobileProfile() const
{
    return boost::any_cast<std::string>(m_data.at("profile")) == MOBILE;
}

} /* end of namespace config */
} /* end of namespace tizen_browser */
