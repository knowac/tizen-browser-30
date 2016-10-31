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

#include "SettingsPrivacy.h"

#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace tizen_browser{
namespace base_ui{


SettingsPrivacy::SettingsPrivacy(Evas_Object* parent)
{
    init(parent);
    updateButtonMap();
};

SettingsPrivacy::~SettingsPrivacy()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsPrivacy::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData cookies;
    cookies.buttonText = _(Translations::SettingsPrivacyAcceptCookies.c_str());
    cookies.subText = _(Translations::SettingsPrivacyAcceptCookiesSub.c_str());
    cookies.sui = this;
    cookies.id = SettingsPrivacyOptions::COOKIES;

    ItemData signIn;
    signIn.buttonText = _(Translations::SettingsPrivacySaveSigninInfo.c_str());
    signIn.subText = _(Translations::SettingsPrivacySaveSigninInfo.c_str());
    signIn.sui = this;
    signIn.id = SettingsPrivacyOptions::SIGNIN_INFO;

    ItemData delPerData;
    delPerData.buttonText = _(Translations::SettingsPrivacyDeletePersonalData.c_str());
    delPerData.sui = this;
    delPerData.id = SettingsPrivacyOptions::DEL_PER_DATA;

    m_buttonsMap[SettingsPrivacyOptions::COOKIES] = cookies;
    m_buttonsMap[SettingsPrivacyOptions::SIGNIN_INFO] = signIn;
    m_buttonsMap[SettingsPrivacyOptions::DEL_PER_DATA] = delPerData;
}

bool SettingsPrivacy::populateList(Evas_Object* genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_naviframe->setLeftButtonVisible(false);
    m_naviframe->setRightButtonVisible(false);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle(_(Translations::SettingsPrivacyTitle.c_str()));

    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsPrivacyOptions::COOKIES], _check_cb);
    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsPrivacyOptions::SIGNIN_INFO], _check_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsPrivacyOptions::DEL_PER_DATA], _del_per_data_cb);
    evas_object_smart_callback_add(genlist, "realized", _realized_cb, this);
    return true;
}

void SettingsPrivacy::_realized_cb(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto ev = static_cast<Elm_Object_Item*>(event_info);
    auto item = static_cast<ItemData*>(elm_object_item_data_get(ev));
    auto sp = static_cast<SettingsPrivacy*>(data);
    auto check = sp->m_checkboxes[item->id];
    switch (item->id) {
        case SettingsPrivacyOptions::COOKIES:
            evas_object_smart_callback_del(check, "changed", _cookies_cb);
            evas_object_smart_callback_add(check, "changed", _cookies_cb, nullptr);
            break;
        case SettingsPrivacyOptions::SIGNIN_INFO:
            evas_object_smart_callback_del(check, "changed", _signin_cb);
            evas_object_smart_callback_add(check, "changed", _signin_cb, nullptr);
            break;
        default:
            break;
    }
}

void SettingsPrivacy::_check_cb(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto el = elm_genlist_selected_item_get(obj);
    auto check = elm_object_item_part_content_get(el, "elm.swallow.end");
    auto value = !elm_check_state_get(check);

    elm_check_state_set(check, value);
    evas_object_smart_callback_call(check, "changed", nullptr);
}

void SettingsPrivacy::_cookies_cb(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto value = elm_check_state_get(obj);
    if (value == EINA_TRUE)
        ewk_cookie_manager_accept_policy_set(
            ewk_context_cookie_manager_get(ewk_context_default_get()), EWK_COOKIE_ACCEPT_POLICY_ALWAYS);
    else
        ewk_cookie_manager_accept_policy_set(
            ewk_context_cookie_manager_get(ewk_context_default_get()), EWK_COOKIE_ACCEPT_POLICY_NEVER);
}

void SettingsPrivacy::_signin_cb(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto value = elm_check_state_get(obj);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::REMEMBER_PASSWORDS,
        static_cast<bool>(value));
}

void SettingsPrivacy::_del_per_data_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsDelPersDataClicked();
}

}
}
