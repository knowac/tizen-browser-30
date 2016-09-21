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

#include "SettingsMain.h"

#include "Config.h"

namespace tizen_browser{
namespace base_ui{

SettingsMain::SettingsMain(Evas_Object* parent)
{
    init(parent);
};

SettingsMain::~SettingsMain()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsMain::updateButtonMap()
{
    ItemData homePage;
    homePage.buttonText = _(Translations::SettingsMainHomePage.c_str());
    homePage.subText = _(Translations::SettingsMainHomePageDefault.c_str());
    homePage.sui = this;
    homePage.id = HOME;

    ItemData search;
    search.buttonText = _(Translations::SettingsMainDefaultSearchEngine.c_str());
    search.subText =  []() -> std::string {
        auto sig =
            SPSC.getWebEngineSettingsParamString(
                basic_webengine::WebEngineSettings::DEFAULT_SEARCH_ENGINE);
        return (sig && !sig->empty()) ?
            *sig :
            Translations::Google;
    }();
    search.sui = this;
    search.id = SEARCH;

    ItemData autofill;
    autofill.buttonText = _(Translations::SettingsMainAutoFillProfiles.c_str());
    autofill.subText = _(Translations::SettingsMainAutoFillProfilesSub.c_str());
    autofill.sui = this;
    autofill.id = AUTO_FILL_PROFILE;

    ItemData zoom;
    zoom.buttonText = _(Translations::SettingsMainManualZoom.c_str());
    zoom.subText = _(Translations::SettingsMainManualZoomSub.c_str());
    zoom.sui = this;
    zoom.id = ZOOM;

    ItemData privacy;
    privacy.buttonText = _(Translations::SettingsMainPrivacy.c_str());
    privacy.sui = this;
    privacy.id = PRIVACY;

    ItemData advanced;
    advanced.buttonText = _(Translations::SettingsMainAdvanced.c_str());
    advanced.sui = this;
    advanced.id = ADVANCED;

    m_buttonsMap[SettingsMainOptions::HOME] = homePage;
    m_buttonsMap[SettingsMainOptions::SEARCH] = search;
    m_buttonsMap[SettingsMainOptions::AUTO_FILL_PROFILE] = autofill;
    m_buttonsMap[SettingsMainOptions::ZOOM] = zoom;
    m_buttonsMap[SettingsMainOptions::PRIVACY] = privacy;
    m_buttonsMap[SettingsMainOptions::ADVANCED] = advanced;

    SPSC.setSearchEngineSubText.connect(
        boost::bind(&SettingsMain::setSearchEngineSubText, this, _1));
    setHomePageSubText();
}

bool SettingsMain::populateList(Evas_Object* genlist)
{
    m_naviframe->setLeftButtonVisible(false);
    m_naviframe->setRightButtonVisible(false);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle(_(Translations::SettingsMainTitle.c_str()));

    updateButtonMap();
    m_genlistItems[SettingsMainOptions::HOME] =
        appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::HOME], _home_page_cb);
    m_genlistItems[SettingsMainOptions::SEARCH] =
        appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::SEARCH], _search_engine_cb);
    m_genlistItems[SettingsMainOptions::AUTO_FILL_PROFILE] =
        appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::AUTO_FILL_PROFILE], _auto_fill_cb);
    m_genlistItems[SettingsMainOptions::ZOOM] =
        appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsMainOptions::ZOOM], _zoom_cb);
    m_genlistItems[SettingsMainOptions::PRIVACY] =
        appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsMainOptions::PRIVACY], _privacy_cb);
    m_genlistItems[SettingsMainOptions::ADVANCED] =
        appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsMainOptions::ADVANCED], _advanced_cb);
    return true;
}

Evas_Object* SettingsMain::createOnOffCheckBox(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "on&off");
    elm_check_state_set(check, getOriginalZoomState());
    evas_object_smart_callback_add(check, "changed", grid_item_check_changed, itd);
    return check;
}

Eina_Bool SettingsMain::getOriginalZoomState()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> sig =
        SPSC.getWebEngineSettingsParam(
            basic_webengine::WebEngineSettings::PAGE_OVERVIEW);

    return (sig && *sig) ? EINA_TRUE : EINA_FALSE;
}

std::string SettingsMain::getHomePage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<std::string> sig =
        SPSC.getWebEngineSettingsParamString(
            basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE);

    return (sig && !sig->empty()) ? (*sig) : Translations::SamsungPage;
}

void SettingsMain::_home_page_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsHomePageClicked();
}

void SettingsMain::_search_engine_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsBaseShowRadioPopup();
}

void SettingsMain::_zoom_cb(void *, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto el = elm_genlist_selected_item_get(obj);
    auto check = elm_object_item_part_content_get(el, "elm.swallow.end");
    auto value = !elm_check_state_get(check);

    elm_check_state_set(check, value);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::PAGE_OVERVIEW,
        static_cast<bool>(value));
}

void SettingsMain::_advanced_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsAdvancedClicked();
}

void SettingsMain::_auto_fill_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsAutofillClicked();
}

void SettingsMain::_privacy_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsPrivacyClicked();
}

void SettingsMain::grid_item_check_changed(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto value = !elm_check_state_get(obj);

    elm_check_state_set(obj, value);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::PAGE_OVERVIEW,
        static_cast<bool>(value));
}

void SettingsMain::setSearchEngineSubText(int button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (static_cast<RadioButtons>(button)) {
        case RadioButtons::GOOGLE:
            m_buttonsMap[SettingsMainOptions::SEARCH].subText = Translations::Google;
            break;
        case RadioButtons::YAHOO:
            m_buttonsMap[SettingsMainOptions::SEARCH].subText = Translations::Yahoo;
            break;
        case RadioButtons::BING:
            m_buttonsMap[SettingsMainOptions::SEARCH].subText = Translations::Bing;
            break;
        default:
            return;
    }
    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::DEFAULT_SEARCH_ENGINE,
        m_buttonsMap[SettingsMainOptions::SEARCH].subText);
    elm_genlist_realized_items_update(m_genlist);
}

void SettingsMain::setHomePageSubText()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto homePage(getHomePage());
    auto it(homePage.find(Translations::CurrentPage));

    boost::optional<std::string> currentOpt(SPSC.requestCurrentPage());
    std::string currentURL = std::string();

    if (currentOpt && !currentOpt->empty())
        currentURL = *currentOpt;

    if (!homePage.compare(Translations::SamsungPage)) {
        m_buttonsMap[SettingsMainOptions::HOME].subText =
            Translations::SamsungPage;
    } else if (!homePage.compare(Translations::QuickPage)) {
        m_buttonsMap[SettingsMainOptions::HOME].subText =
            Translations::SettingsHomePageQuickAccess;
    } else if (!homePage.compare(Translations::MostVisitedPage)) {
        m_buttonsMap[SettingsMainOptions::HOME].subText =
            Translations::SettingsHomePageMostVisited;
    } else if (!homePage.compare(currentURL)) {
        m_buttonsMap[SettingsMainOptions::HOME].subText =
            Translations::SettingsHomePageCurrentPage;
    } else if (it != std::string::npos) {
        homePage.erase(it, Translations::CurrentPage.length());
        m_buttonsMap[SettingsMainOptions::HOME].subText = homePage;
        SPSC.setWebEngineSettingsParamString(
            basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE,
            homePage);
    } else {
        m_buttonsMap[SettingsMainOptions::HOME].subText = homePage;
    }
    elm_genlist_realized_items_update(m_genlist);
}
}
}
