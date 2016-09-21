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

#include "SettingsManager.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SettingsManager, "org.tizen.browser.settingsui")

SettingsManager::SettingsManager()
    : m_parent(nullptr)
{
};

SettingsManager::~SettingsManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
};

void SettingsManager::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
}

std::shared_ptr<SettingsUI> SettingsManager::getView(const SettingsMainOptions& s)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return addView(s);
}

void SettingsManager::connectOpenSignals()
{
    SPSC.settingsHomePageClicked.connect(
        boost::bind(&SettingsManager::showSettingsHomePageUI, this));
    SPSC.settingsAutofillClicked.connect(
        boost::bind(&SettingsManager::showSettingsAutofillUI, this));
    SPSC.settingsPrivacyClicked.connect(
        boost::bind(&SettingsManager::showSettingsPrivacyUI, this));
    SPSC.settingsAdvancedClicked.connect(
        boost::bind(&SettingsManager::showSettingsAdvancedUI, this));
    SPSC.settingsDelPersDataClicked.connect(
        boost::bind(&SettingsManager::showSettingsDelPrivDataUI, this));
    SPSC.settingsAutofillProfileClicked.connect(
        boost::bind(&SettingsManager::showSettingsAutofillCreatorUI, this, _1));
}

std::shared_ptr<SettingsUI> SettingsManager::addView(const SettingsMainOptions& s)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_settingsViews.find(s) == m_settingsViews.end()) {
        switch (s) {
            case BASE:
                m_settingsViews[s] = std::make_shared<SettingsMain>(m_parent);
                break;
            case HOME:
                m_settingsViews[s] = std::make_shared<SettingsHomePage>(m_parent);
                break;
            case AUTO_FILL_PROFILE:
                m_settingsViews[s] = std::make_shared<SettingsAFProfile>(m_parent);
                break;
            case AUTO_FILL_CREATOR_WITH_PROFILE:
                m_settingsViews[s] = std::make_shared<SettingsAFCreator>(m_parent, true);
                break;
            case AUTO_FILL_CREATOR_WITHOUT_PROFILE:
                m_settingsViews[s] = std::make_shared<SettingsAFCreator>(m_parent, false);
                break;
            case PRIVACY:
                m_settingsViews[s] = std::make_shared<SettingsPrivacy>(m_parent);
                break;
            case ADVANCED:
                m_settingsViews[s] = std::make_shared<SettingsAdvanced>(m_parent);
                break;
            case DEL_PERSONAL_DATA:
                m_settingsViews[s] = std::make_shared<SettingsDelPersData>(m_parent);
                break;
            default:
                break;
        }
    } else {
        m_settingsViews[s]->updateButtonMap();
        if (m_settingsViews[s]->getGenlist())
           elm_genlist_realized_items_update(m_settingsViews[s]->getGenlist());
    }
    return m_settingsViews[s];
}

void SettingsManager::showSettingsBaseUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showSettings(SettingsMainOptions::BASE);
}

void SettingsManager::showSettingsHomePageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showSettings(SettingsMainOptions::HOME);
}

void SettingsManager::showSettingsAutofillUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showSettings(SettingsMainOptions::AUTO_FILL_PROFILE);
}

void SettingsManager::showSettingsAutofillCreatorUI(bool profile)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (profile)
        SPSC.showSettings(
            SettingsMainOptions::AUTO_FILL_CREATOR_WITH_PROFILE);
    else
        SPSC.showSettings(
            SettingsMainOptions::AUTO_FILL_CREATOR_WITHOUT_PROFILE);
}

void SettingsManager::showSettingsPrivacyUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showSettings(SettingsMainOptions::PRIVACY);
}

void SettingsManager::showSettingsAdvancedUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showSettings(SettingsMainOptions::ADVANCED);
}

void SettingsManager::showSettingsDelPrivDataUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showSettings(SettingsMainOptions::DEL_PERSONAL_DATA);
}
}
}
