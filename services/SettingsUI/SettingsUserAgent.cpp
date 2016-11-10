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

#include "SettingsUserAgent.h"

#include "Config.h"
#include "app_preference.h"

namespace tizen_browser{
namespace base_ui{

SettingsUserAgent::SettingsUserAgent(Evas_Object* parent)
{
    init(parent);
    updateButtonMap();
};

SettingsUserAgent::~SettingsUserAgent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsUserAgent::updateButtonMap()
{
    for (auto i = 0; i < UA_ITEMS_COUNT; ++i) {
        ItemData content;
        content.buttonText = m_uaList[i].name;
        content.subText = m_uaList[i].uaString;
        content.sui = this;
        content.id = i;
        m_buttonsMap[i] = content;
    }
}

bool SettingsUserAgent::populateList(Evas_Object* genlist)
{
    m_naviframe->setLeftButtonVisible(false);
    m_naviframe->setRightButtonVisible(false);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle("User Agent");

    for (auto i = 0; i < UA_ITEMS_COUNT; ++i) {
        auto radioData = new RadioData(this, i);
        elm_genlist_item_append(
            genlist,
            m_setting_check_radio_item_class,
            &m_buttonsMap[i],
            nullptr,
            ELM_GENLIST_ITEM_NONE,
            onGenlistClick,
            radioData);
    }
    return true;
}

Evas_Object* SettingsUserAgent::createRadioButton(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto radio_button = elm_radio_add(obj);
    if (radio_button) {
        elm_radio_state_value_set(radio_button, itd->id);
        elm_radio_group_add(radio_button, getRadioGroup());
        evas_object_propagate_events_set(radio_button, EINA_FALSE);
        evas_object_smart_callback_add(radio_button, "changed", onRadioClick, itd);
        elm_access_object_unregister(radio_button);
        m_radioMap[itd->id] = radio_button;
    }
    return radio_button;
}

void SettingsUserAgent::onGenlistClick(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGD("[%s:%d] data is null", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    auto radioData = static_cast<RadioData*>(data);
    int id = radioData->radioId;
    elm_radio_value_set(radioData->sua->m_radioMap[id], id);
    evas_object_smart_callback_call(radioData->sua->m_radioMap[id], "changed", nullptr);
}

void SettingsUserAgent::onRadioClick(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGD("[%s:%d] data is null", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    auto itemData = static_cast<ItemData*>(data);
    SPSC.userAgentItemClicked(itemData->subText);
}

}
}
