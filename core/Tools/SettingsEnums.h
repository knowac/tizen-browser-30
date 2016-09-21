/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef __SETTINGS_ENUMS_H__
#define __SETTINGS_ENUMS_H__

#include <string>

#include "app_i18n.h"

#define ADD_TRAN(x,y) static std::string x = _(y);

namespace tizen_browser
{
namespace base_ui
{
namespace Translations {
    ADD_TRAN(Google,"Google")
    ADD_TRAN(Yahoo,"Yahoo!")
    ADD_TRAN(Bing,"Bing")
    ADD_TRAN(SamsungPage,"http://www.samsung.com")
    ADD_TRAN(QuickPage,"QUICK_ACCESS")
    ADD_TRAN(MostVisitedPage,"MOST_VISITED")
    ADD_TRAN(OtherPage,"OTHER")
    ADD_TRAN(CurrentPage,"CURRENT")
    ADD_TRAN(Device,"IDS_BR_OPT_DEVICE")
    ADD_TRAN(SDCard,"IDS_BR_OPT_SD_CARD")
    ADD_TRAN(SettingsMainTitle,"IDS_BR_OPT_SETTINGS")
    ADD_TRAN(SettingsMainHomePage, "IDS_BR_BUTTON_HOMEPAGE_ABB")
    ADD_TRAN(SettingsMainHomePageDefault,"IDS_BR_BODY_DEFAULT")
    ADD_TRAN(SettingsMainDefaultSearchEngine,"IDS_BR_TMBODY_DEFAULT_SEARCH_ENGINE")
    ADD_TRAN(SettingsMainAutoFillProfiles,"IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS")
    ADD_TRAN(SettingsMainAutoFillProfilesSub,"IDS_BR_SBODY_MANAGE_DATA_USED_TO_FILL_IN_FORMS_ONLINE")
    ADD_TRAN(SettingsMainManualZoom,"IDS_BR_MBODY_MANUAL_ZOOM")
    ADD_TRAN(SettingsMainManualZoomSub,"IDS_BR_BODY_OVERRIDE_WEBSITES_REQUEST_TO_CONTROL_ZOOM")
    ADD_TRAN(SettingsMainPrivacy,"IDS_BR_BODY_PRIVACY")
    ADD_TRAN(SettingsMainAdvanced,"IDS_BR_BODY_ADVANCED")
    ADD_TRAN(SettingsDefaultSearchEngineTitle,"IDS_BR_TMBODY_DEFAULT_SEARCH_ENGINE")
    ADD_TRAN(SettingsAdvancedTitle,"IDS_BR_BODY_ADVANCED")
    ADD_TRAN(SettingsAdvancedEnableJavaScript,"IDS_BR_OPT_ENABLE_JAVASCRIPT")
    ADD_TRAN(SettingsAdvancedEnableJavaScriptSub,"IDS_BR_BODY_ALLOW_SITES_TO_RUN_JAVASCRIPT")
    ADD_TRAN(SettingsAdvancedBlockPopups,"IDS_BR_BODY_BLOCK_POP_UPS")
    ADD_TRAN(SettingsAdvancedBlockPopupsSub,"IDS_BR_BODY_BLOCK_POP_UPS_ON_WEBPAGES")
    ADD_TRAN(SettingsAdvancedSaveContent,"IDS_BR_HEADER_SAVE_CONTENT_TO_ABB")
    ADD_TRAN(SettingsAdvancedSaveContentTitle,"IDS_BR_HEADER_SAVE_CONTENT_TO_ABB")
    ADD_TRAN(SettingsAdvancedManageWebsiteData,"IDS_BR_TMBODY_MANAGE_WEBSITE_DATA")
    ADD_TRAN(SettingsAdvancedManageWebsiteDataSub,"IDS_BR_SBODY_SET_ADVANCED_SETTINGS_FOR_INDIVIDUAL_WEBSITES")
    ADD_TRAN(SettingsPrivacyTitle,"IDS_BR_BODY_PRIVACY");
    ADD_TRAN(SettingsPrivacyAcceptCookies,"IDS_BR_BODY_ACCEPT_COOKIES")
    ADD_TRAN(SettingsPrivacyAcceptCookiesSub,"IDS_BR_SBODY_ALLOW_SITES_TO_SAVE_AND_READ_COOKIES")
    ADD_TRAN(SettingsPrivacySuggestSearches,"IDS_BR_MBODY_SUGGEST_SEARCHES")
    ADD_TRAN(SettingsPrivacySuggestSearchesSub,"IDS_BR_SBODY_SET_THE_DEVICE_TO_SUGGEST_QUERIES_AND_SITES_IN_THE_WEB_ADDRESS_BAR_AS_YOU_TYPE")
    ADD_TRAN(SettingsPrivacySaveSigninInfo,"IDS_BR_MBODY_SAVE_SIGN_IN_INFO")
    ADD_TRAN(SettingsPrivacySaveSigninInfoSub,"IDS_BR_SBODY_SET_YOUR_DEVICE_TO_SHOW_A_POP_UP_WITH_THE_OPTION_MSG")
    ADD_TRAN(SettingsPrivacyDeletePersonalData,"IDS_BR_BODY_DELETE_PERSONAL_DATA")
    ADD_TRAN(SettingsDelPersDataTitle,"IDS_BR_BODY_DELETE_PERSONAL_DATA")
    ADD_TRAN(SettingsDelPersDataCancel,"IDS_TPLATFORM_ACBUTTON_DONE_ABB")
    ADD_TRAN(SettingsDelPersDataDelete,"IDS_TPLATFORM_ACBUTTON_DELETE_ABB")
    ADD_TRAN(SettingsDelPersDataSelectAll,"IDS_BR_OPT_SELECT_ALL_ABB")
    ADD_TRAN(SettingsDelPersDataBrowsingHistory,"IDS_BR_BODY_BROWSING_HISTORY")
    ADD_TRAN(SettingsDelPersDataCache,"IDS_BR_OPT_CACHE")
    ADD_TRAN(SettingsDelPersDataCookies,"IDS_BR_BODY_COOKIES_AND_SITE_DATA_ABB")
    ADD_TRAN(SettingsDelPersDataPasswords,"IDS_BR_OPT_PASSWORDS")
    ADD_TRAN(SettingsDelPersDataAutoFillData,"IDS_BR_OPT_AUTO_FILL_DATA")
    ADD_TRAN(SettingsDelPersDataLocationData,"IDS_BR_OPT_LOCATION_ACCESS_DATA")
    ADD_TRAN(SettingsAutoFillProfileTitle,"IDS_BR_MBODY_MY_AUTO_FILL_PROFILE")
    ADD_TRAN(SettingsAutoFillProfileSetMyProfile,"IDS_BR_MBODY_SET_MY_PROFILE")
    ADD_TRAN(SettingsHomePageTitle,"IDS_BR_BUTTON_HOMEPAGE_ABB")
    ADD_TRAN(SettingsHomePageCurrentPage,"IDS_BR_BODY_CURRENT_PAGE")
    ADD_TRAN(SettingsHomePageDefault,"IDS_BR_BODY_DEFAULT")
    ADD_TRAN(SettingsHomePageOther,"IDS_BR_BODY_OTHER_ABB")
    ADD_TRAN(SettingsHomePageMostVisited,"IDS_BR_MBODY_MOST_VISITED_WEBSITES")
    ADD_TRAN(SettingsHomePageQuickAccess,"IDS_BR_HEADER_QUICK_ACCESS_ABB")
};

struct SearchEngineTranslation
{
    static SearchEngineTranslation& instance() {
       static SearchEngineTranslation INSTANCE;
       return INSTANCE;
    }
    SearchEngineTranslation(SearchEngineTranslation const&) = delete;
    void operator=(SearchEngineTranslation const&) = delete;
    std::string get(const std::string& key) { return searchEngine[key]; }
private:
    SearchEngineTranslation() {
        searchEngine[Translations::Google] = "http://www.google.com/search?q=";
        searchEngine[Translations::Yahoo] = "http://www.yahoo.com/search?q=";
        searchEngine[Translations::Bing] = "http://www.bing.com/search?q=";
    }
    std::map<std::string, std::string> searchEngine;
};

enum struct RadioButtons : int
{
    NONE    = 0,
    // WebSearch Engines
    GOOGLE  = 1,
    YAHOO,
    BING,
    // Save Content
    DEVICE  = 10,
    SD_CARD
};

}
}

#endif
