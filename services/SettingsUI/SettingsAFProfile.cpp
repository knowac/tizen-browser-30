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


#include "SettingsAFProfile.h"

namespace tizen_browser{
namespace base_ui{

SettingsAFProfile::SettingsAFProfile(Evas_Object* parent)
    : m_profile(nullptr)
    , m_itemData(nullptr)
    , m_profileName(std::string())
{
    init(parent);
    SPSC.setProfileName.connect([this](std::string name){m_profileName = name;});
    SPSC.autoFillCleared.connect([this](){
        m_profileName.clear();
        m_profile = nullptr;
    });
};

SettingsAFProfile::~SettingsAFProfile()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAFProfile::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData profileName;
    void* item_data(nullptr);
    Eina_List* list(nullptr);
    Eina_List* entire_item_list(
        ewk_context_form_autofill_profile_get_all(ewk_context_default_get()));

    // ID for the item is not always 1 so we need to return the first existing one
    EINA_LIST_FOREACH(entire_item_list, list, item_data) {
        if (item_data) {
            m_profile = static_cast<Ewk_Autofill_Profile*>(item_data);
            break;
        }
    }
    profileName.buttonText = m_profile ?
        profileName.buttonText = ewk_autofill_profile_data_get(m_profile, EWK_PROFILE_NAME) :
        profileName.buttonText = _(Translations::SettingsAutoFillProfileSetMyProfile.c_str());

    if (!m_profileName.empty())
        profileName.buttonText = m_profileName;
    m_itemData = std::make_shared<ItemData>(profileName);
}

bool SettingsAFProfile::populateList(Evas_Object* genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_naviframe->setLeftButtonVisible(false);
    m_naviframe->setRightButtonVisible(false);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle(_(Translations::SettingsAutoFillProfileTitle.c_str()));

    appendGenlist(genlist, m_setting_item_class, m_itemData.get(), _select_profile_cb);
    return true;
}

void SettingsAFProfile::_select_profile_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsAFProfile*>(data);
    if (self->m_profile)
        SPSC.settingsAutofillProfileClicked(true);
    else
        SPSC.settingsAutofillProfileClicked(false);
}
}
}
