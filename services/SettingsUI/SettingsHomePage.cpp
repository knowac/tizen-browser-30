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

#include "SettingsHomePage.h"

namespace tizen_browser{
namespace base_ui{

const std::string SettingsHomePage::DEF_HOME_PAGE = Translations::SamsungPage;
const std::string SettingsHomePage::QUICK_PAGE = _(Translations::QuickPage.c_str());
const std::string SettingsHomePage::MOST_VISITED_PAGE = _(Translations::MostVisitedPage.c_str());
const std::string SettingsHomePage::OTHER_PAGE = _(Translations::OtherPage.c_str());
const std::string SettingsHomePage::CURRENT_PAGE = _(Translations::CurrentPage.c_str());

SettingsHomePage::SettingsHomePage(Evas_Object* parent)
{
    init(parent);
};

SettingsHomePage::~SettingsHomePage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

std::string SettingsHomePage::getCurrentPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_current = SPSC.requestCurrentPage();
    BROWSER_LOGD("[%s:%s] ", __PRETTY_FUNCTION__, (*m_current).c_str());
    if (m_current && !m_current->empty())
        return *m_current;
    return SettingsHomePage::DEF_HOME_PAGE;
}

void SettingsHomePage::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData defaultPage;
    defaultPage.buttonText = _(Translations::SettingsHomePageDefault.c_str());
    defaultPage.subText = DEF_HOME_PAGE;
    defaultPage.sui = this;
    defaultPage.id = DEFAULT;

    ItemData current;
    current.buttonText = _(Translations::SettingsHomePageCurrentPage.c_str());
    current.subText = getCurrentPage();
    current.sui = this;
    current.id = CURRENT;

    ItemData quick;
    quick.buttonText = _(Translations::SettingsHomePageQuickAccess.c_str());
    quick.sui = this;
    quick.id = QUICK_ACCESS;

    ItemData most;
    most.buttonText = _(Translations::SettingsHomePageMostVisited.c_str());
    most.sui = this;
    most.id = MOST_VIS;

    ItemData other;
    other.buttonText = _(Translations::SettingsHomePageOther.c_str());
    other.subText = [this]() -> std::string {
        auto sig = SPSC.getWebEngineSettingsParamString(
            basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE);
        auto otherPage = (sig && !sig->empty()) ? *sig : _(Translations::SamsungPage.c_str());

        if (!otherPage.compare(QUICK_PAGE) ||
            !otherPage.compare(MOST_VISITED_PAGE) ||
            !otherPage.compare(DEF_HOME_PAGE) ||
            otherPage.find(CURRENT_PAGE) != std::string::npos) {
            otherPage = m_buttonsMap[SettingsHomePageOptions::OTHER].subText;
        }
        return otherPage;
    }();
    other.sui = this;
    other.id = OTHER;

    m_buttonsMap[SettingsHomePageOptions::DEFAULT] = defaultPage;
    m_buttonsMap[SettingsHomePageOptions::CURRENT] = current;
    m_buttonsMap[SettingsHomePageOptions::QUICK_ACCESS] = quick;
    m_buttonsMap[SettingsHomePageOptions::MOST_VIS] = most;
    m_buttonsMap[SettingsHomePageOptions::OTHER] = other;

    setRadioOnChange();
}

void SettingsHomePage::setRadioOnChange()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto stateString = []() -> std::string {
        auto sig = SPSC.getWebEngineSettingsParamString(
            basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE);
        return (sig && !sig->empty()) ? *sig : DEF_HOME_PAGE;
    }();
    if (!stateString.compare(QUICK_PAGE)) {
        elm_radio_value_set(m_radio, SettingsHomePageOptions::QUICK_ACCESS);
    } else if (!stateString.compare(MOST_VISITED_PAGE)) {
        elm_radio_value_set(m_radio, SettingsHomePageOptions::MOST_VIS);
    } else if (!stateString.compare(DEF_HOME_PAGE)) {
        elm_radio_value_set(m_radio, SettingsHomePageOptions::DEFAULT);
    } else if (stateString.find(CURRENT_PAGE) != std::string::npos ||
        !stateString.compare(getCurrentPage())) {
        elm_radio_value_set(m_radio, SettingsHomePageOptions::CURRENT);
    } else {
        elm_radio_value_set(m_radio, SettingsHomePageOptions::OTHER);
    }
    elm_genlist_realized_items_update(m_genlist);
}

bool SettingsHomePage::populateList(Evas_Object* genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_naviframe->setLeftButtonVisible(false);
    m_naviframe->setRightButtonVisible(false);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle(_(Translations::SettingsHomePageTitle.c_str()));

    updateButtonMap();
    m_itemsMap[SettingsHomePageOptions::DEFAULT] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::DEFAULT], _default_cb);
    m_itemsMap[SettingsHomePageOptions::CURRENT] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::CURRENT], _current_cb);
    m_itemsMap[SettingsHomePageOptions::QUICK_ACCESS] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::QUICK_ACCESS], _quick_cb);
    m_itemsMap[SettingsHomePageOptions::MOST_VIS] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::MOST_VIS], _most_visited_cb);
    m_itemsMap[SettingsHomePageOptions::OTHER] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::OTHER], _other_cb);

    return true;
}

Evas_Object* SettingsHomePage::createRadioButton(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto radio_button = elm_radio_add(obj);
    if (radio_button) {
        elm_radio_state_value_set(radio_button, itd->id);
        elm_radio_group_add(radio_button, getRadioGroup());
        evas_object_propagate_events_set(radio_button, EINA_FALSE);
        switch (itd->id) {
            case SettingsHomePageOptions::DEFAULT:
                evas_object_smart_callback_add(radio_button, "changed", _default_cb, this);
                break;
            case SettingsHomePageOptions::CURRENT:
                evas_object_smart_callback_add(radio_button, "changed", _current_cb, this);
                break;
            case SettingsHomePageOptions::QUICK_ACCESS:
                evas_object_smart_callback_add(radio_button, "changed", _quick_cb, this);
                break;
            case SettingsHomePageOptions::MOST_VIS:
                evas_object_smart_callback_add(radio_button, "changed", _most_visited_cb, this);
                break;
            case SettingsHomePageOptions::OTHER:
                evas_object_smart_callback_add(radio_button, "changed", _other_cb, this);
                break;
        }
        elm_access_object_unregister(radio_button);
    }
    return radio_button;
}

void SettingsHomePage::_current_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsHomePage*>(data);
    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE,
        SettingsHomePage::CURRENT_PAGE + self->getCurrentPage());
    self->updateButtonMap();
}

void SettingsHomePage::_default_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsHomePage*>(data);
    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE,
        SettingsHomePage::DEF_HOME_PAGE);
    self->updateButtonMap();
}

void SettingsHomePage::_quick_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsHomePage*>(data);
    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE,
        SettingsHomePage::QUICK_PAGE);
    self->updateButtonMap();
}

void SettingsHomePage::_most_visited_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsHomePage*>(data);
    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE,
        SettingsHomePage::MOST_VISITED_PAGE);
    self->updateButtonMap();
}

void SettingsHomePage::_other_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.showTextPopup();
}

}
}
