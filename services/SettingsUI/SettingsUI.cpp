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

#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <AbstractMainWindow.h>
#include "app_i18n.h"
#include "SettingsUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

namespace tizen_browser{
namespace base_ui{

SettingsUI::SettingsUI()
    : m_naviframe(nullptr)
    , m_genlist(nullptr)
    , m_settings_layout(nullptr)
    , m_items_layout(nullptr)
    , m_parent(nullptr)
    , m_radio(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsMobileUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_setting_item_class = createItemClass("type1",_gengrid_item_text_get);
    m_setting_double_item_class = createItemClass("type1",_gengrid_item_text_get);
    m_setting_check_on_of_item_class = createItemClass("type1",_gengrid_item_text_get, _gengrid_item_content_onoff_get);
    m_setting_check_normal_item_class = createItemClass("type1",_gengrid_item_text_get, _gengrid_item_content_normal_get);
    m_setting_check_radio_item_class = createItemClass("type1",_gengrid_item_text_get, _gengrid_item_content_radio_get);
}

Elm_Gengrid_Item_Class* SettingsUI::createItemClass(
    const char* style,
    Elm_Gen_Item_Text_Get_Cb text_cb,
    Elm_Gen_Item_Content_Get_Cb con_cb)
{
    auto ic = elm_genlist_item_class_new();
    ic->item_style = style;
    ic->func.text_get = text_cb;
    ic->func.content_get = con_cb;
    ic->func.state_get = nullptr;
    ic->func.del = nullptr;
    ic->decorate_all_item_style = "edit_default";
    return ic;
}

SettingsUI::~SettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_setting_item_class)
        elm_genlist_item_class_free(m_setting_item_class);
    if (m_setting_double_item_class)
        elm_genlist_item_class_free(m_setting_double_item_class);
    if (m_setting_check_normal_item_class)
        elm_genlist_item_class_free(m_setting_check_normal_item_class);
    if (m_setting_check_on_of_item_class)
        elm_genlist_item_class_free(m_setting_check_on_of_item_class);
    if (m_setting_check_radio_item_class)
        elm_genlist_item_class_free(m_setting_check_radio_item_class);

    for (auto check : m_checkboxes)
        evas_object_del(check.second);
    for (auto radio : m_radios)
        evas_object_del(radio);

    if (m_genlist) {
        evas_object_smart_callback_del(m_genlist, "language,changed", _language_changed);
        evas_object_del(m_genlist);
    }
    if (m_settings_layout)
        evas_object_del(m_settings_layout);
    if (m_items_layout)
        evas_object_del(m_items_layout);
    if (m_layout)
        evas_object_del(m_layout);
    if (m_radio)
        evas_object_del(m_radio);
}

void SettingsUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* SettingsUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_settings_layout)
        m_settings_layout = createSettingsUILayout(m_parent);
    return m_naviframe->getLayout();
}

void SettingsUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_naviframe->show();
    updateButtonMap();
    if (m_genlist || elm_genlist_items_count(m_genlist)) {
        elm_genlist_clear(m_genlist);
        populateList(m_genlist);
    }
}

void SettingsUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_naviframe->hide();
}

Evas_Object* SettingsUI::createSettingsUILayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);

    m_naviframe = std::make_shared<NaviframeWrapper>(parent);
    m_naviframe->addPrevButton(close_clicked_cb, this);

    Evas_Object* settings_layout = elm_layout_add(m_naviframe->getLayout());
    elm_layout_file_set(settings_layout, m_edjFilePath.c_str(), "settings-layout");
    tools::EflTools::setExpandHints(settings_layout);

    m_items_layout = createSettingsMobilePage(settings_layout);
    m_naviframe->setContent(settings_layout);

    orientationChanged();

    return settings_layout;
}

void SettingsUI::orientationChanged()
{
    auto rot = isLandscape();
    if (rot && *rot) {
        if (m_items_layout) {
            elm_object_signal_emit(m_items_layout, "rotation,landscape,main", "rot");
        }
    } else {
        if (m_items_layout) {
            elm_object_signal_emit(m_items_layout,"rotation,portrait,main", "rot");
        }
    }
}

Evas_Object* SettingsUI::createActionBar(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* actionBar = elm_layout_add(settings_layout);
    elm_object_part_content_set(settings_layout, "actionbar_swallow", actionBar);
    evas_object_size_hint_weight_set(actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(actionBar, m_edjFilePath.c_str(), "action_bar");
    Evas_Object *close_click_btn = elm_button_add(actionBar);
    elm_object_style_set(close_click_btn, "basic_button");
    evas_object_smart_callback_add(close_click_btn, "clicked", SettingsUI::close_clicked_cb, this);
    elm_object_part_content_set(actionBar, "close_click", close_click_btn);
    elm_object_translatable_part_text_set(actionBar, "settings_title", "IDS_BR_BODY_SETTINGS");

    return actionBar;
}

Evas_Object* SettingsUI::_gengrid_item_content_radio_get(void* data, Evas_Object* obj, const char* part)
{
    if (!data) {
        BROWSER_LOGE("data is null");
        return nullptr;
    }
    if (strcmp(part, "elm.swallow.end") == 0) {
        auto itd = static_cast<ItemData*>(data);
        auto radio = itd->sui->createRadioButton(obj, itd);
        itd->sui->m_radios.push_back(radio);
        return radio;
    }
    return nullptr;
}

char* SettingsUI::_gengrid_item_text_get(void* data, Evas_Object*, const char* part)
{
   M_ASSERT(data);
   if (!data) {
       BROWSER_LOGE("data is null");
       return nullptr;
   }

   ItemData* it = static_cast<ItemData*>(data);

   if (strcmp(part, "elm.text") == 0) {
       const char* item_name = it->buttonText.c_str();
       if (item_name)
           return strdup(item_name);
   } else if (strcmp(part, "elm.text.sub") == 0 && !(it->subText.empty())) {
       const char* item_name = it->subText.c_str();
       if (item_name)
           return strdup(item_name);
   }
   return nullptr;
}

Evas_Object* SettingsUI::createOnOffCheckBox(Evas_Object* obj, ItemData*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "on&off");
    elm_check_state_set(check, EINA_TRUE);
    evas_object_propagate_events_set(check, EINA_FALSE);
    return check;
}

Evas_Object* SettingsUI::createNormalCheckBox(Evas_Object* obj, ItemData*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "default");
    elm_check_state_set(check, EINA_TRUE);
    evas_object_propagate_events_set(check, EINA_FALSE);
    return check;
}

Evas_Object* SettingsUI::_gengrid_item_content_onoff_get(void* data, Evas_Object* obj, const char* part)
{
    if (!data) {
        BROWSER_LOGE("data is null");
        return nullptr;
    }
    auto itd = static_cast<ItemData*>(data);
    if (strcmp(part, "elm.swallow.end") == 0) {
        auto check = itd->sui->createOnOffCheckBox(obj, itd);
        itd->sui->setCheckboxes(itd->id, check);
        return check;
    }
    return nullptr;
}

Evas_Object* SettingsUI::_gengrid_item_content_normal_get(void* data, Evas_Object* obj, const char* part)
{
    auto itd = static_cast<ItemData*>(data);
    if (strcmp(part, "elm.swallow.end") == 0) {
        auto check = itd->sui->createNormalCheckBox(obj, itd);
        itd->sui->setCheckboxes(itd->id, check);
        return check;
    }
    return nullptr;
}

void SettingsUI::_language_changed(void *data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    SettingsUI* self = static_cast<SettingsUI*>(data);
    self->updateButtonMap();
    elm_genlist_realized_items_update(obj);
}

Evas_Object* SettingsUI::createSettingsMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = createMainView(settings_layout);

    m_genlist = elm_genlist_add(layout);
    m_radio = elm_radio_add(m_genlist);
    elm_radio_state_value_set(m_radio, -1);

    if (populateList(m_genlist)) {
        elm_genlist_homogeneous_set(m_genlist, EINA_FALSE);
        elm_scroller_movement_block_set(m_genlist, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
        evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, 0.0);
        elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
        elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
        elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        elm_scroller_bounce_set(m_genlist, EINA_FALSE, EINA_FALSE);
        evas_object_smart_callback_add(m_genlist, "language,changed", _language_changed, this);
        elm_object_part_content_set(layout, "options_swallow", m_genlist);
        evas_object_show(m_genlist);
    } else {
        evas_object_del(m_genlist);
        m_genlist = nullptr;
        m_layout = elm_layout_add(layout);
        evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
        populateLayout(layout);
        elm_object_part_content_set(layout, "options_swallow", m_layout);
        evas_object_show(m_layout);
    }
    return layout;
}

Elm_Object_Item* SettingsUI::appendGenlist(
    Evas_Object* genlist,
    Elm_Gengrid_Item_Class* it_class,
    const void *data,
    Evas_Smart_Cb func)
{
    return elm_genlist_item_append(genlist, it_class, data, nullptr, ELM_GENLIST_ITEM_NONE, func, this);
}

Evas_Object* SettingsUI::createMainView(Evas_Object* settings_layout)
{
    auto main = elm_layout_add(settings_layout);
    elm_layout_file_set(main, m_edjFilePath.c_str(), "settings_items");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", main);
    evas_object_size_hint_weight_set(main, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(main, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return main;
}

void SettingsUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    SPSC.closeSettingsUIClicked();
}

}
}
