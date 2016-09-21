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

#ifndef SETTINGSPRETTYSIGNALCONNECTOR_H_
#define SETTINGSPRETTYSIGNALCONNECTOR_H_

#include <boost/signals2/signal.hpp>
#include <map>
#include <memory>
#include "Tools/GeneralTools.h"
#include "../../core/AbstractWebEngine/WebEngineSettings.h"
#include "../SimpleUI/RadioPopup.h"

#define SPSC SettingsPrettySignalConnector::Instance()

namespace tizen_browser{
namespace base_ui{

enum SettingsDelPersDataOptions {
    SELECT_ALL,
    BROWSING_HISTORY,
    CACHE,
    COOKIES_AND_SITE,
    PASSWORDS,
    DEL_PERS_AUTO_FILL,
    LOCATION
};

class SettingsPrettySignalConnector
{
public:
    static SettingsPrettySignalConnector& Instance()
    {
        static SettingsPrettySignalConnector instance;
        return instance;
    }
    SettingsPrettySignalConnector(SettingsPrettySignalConnector const&) = delete;
    void operator=(SettingsPrettySignalConnector const&) = delete;

    B_SIG<void ()> settingsPrivacyClicked;
    B_SIG<void ()> settingsDelPersDataClicked;
    B_SIG<void (const std::map<SettingsDelPersDataOptions, bool>&)> deleteSelectedDataClicked;
    B_SIG<void ()> closeSettingsUIClicked;
    B_SIG<bool (const basic_webengine::WebEngineSettings&)>getWebEngineSettingsParam;
    B_SIG<std::string (const basic_webengine::WebEngineSettings&)> getWebEngineSettingsParamString;
    B_SIG<void (const basic_webengine::WebEngineSettings&, bool)> setWebEngineSettingsParam;
    B_SIG<void (const basic_webengine::WebEngineSettings&, std::string)> setWebEngineSettingsParamString;
    B_SIG<void ()> settingsBaseClicked;
    B_SIG<void ()> settingsHomePageClicked;
    B_SIG<void ()> settingsAutofillClicked;
    B_SIG<void ()> settingsAdvancedClicked;
    B_SIG<void (bool)> settingsAutofillProfileClicked;
    B_SIG<void (const std::string&)> homePageChanged;
    B_SIG<std::string ()> requestCurrentPage;
    B_SIG<bool ()> isLandscape;
    B_SIG<void (unsigned)> showSettings;
    B_SIG<void ()> settingsBaseShowRadioPopup;
    B_SIG<void ()> settingsSaveContentToRadioPopup;
    B_SIG<void (RadioPopup*)> settingsSaveContentRadioPopupPtr;
    B_SIG<void (int)> setSearchEngineSubText;
    B_SIG<void (int)> setContentDestination;
    B_SIG<void ()> showTextPopup;
    B_SIG<void (std::string)> setProfileName;

private:
    SettingsPrettySignalConnector(){};
};

}
}

#endif /* SETTINGSPRETTYSIGNALCONNECTOR_H_ */
