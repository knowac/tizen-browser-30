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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <vector>
#include <AbstractMainWindow.h>

#include "SettingsUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SettingsUI, "org.tizen.browser.settingsui")

struct ItemData {
    tizen_browser::base_ui::SettingsUI* settingsUI;
    Elm_Object_Item * e_item;
};

SettingsUI::SettingsUI()
    : m_settings_layout(nullptr)
    , m_actionBar(nullptr)
    , m_scroller(nullptr)
    , m_items_layout(nullptr)
    , m_parent(nullptr)
    , m_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsUI.edj");
}

SettingsUI::~SettingsUI()
{

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
    return m_settings_layout;
}

void SettingsUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(m_scroller);
    evas_object_show(m_items_layout);
    evas_object_show(m_settings_layout);
    evas_object_show(m_actionBar);
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);
}

void SettingsUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_scroller);
    evas_object_hide(m_items_layout);
    evas_object_hide(m_settings_layout);
    evas_object_hide(m_actionBar);
}

Evas_Object* SettingsUI::createSettingsUILayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    Evas_Object* settings_layout = elm_layout_add(parent);
    elm_layout_file_set(settings_layout, m_edjFilePath.c_str(), "settings-layout");
    evas_object_size_hint_weight_set(settings_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(settings_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(settings_layout);
    m_scroller = createSettingsPage(settings_layout);
    return settings_layout;
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

Evas_Object* SettingsUI::createSettingsPage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData *id = new ItemData;
    id->settingsUI = this;

    Evas_Object* scroller = elm_scroller_add(settings_layout);
    m_items_layout = elm_layout_add(scroller);
    elm_object_content_set(scroller, m_items_layout);
    elm_layout_file_set(m_items_layout, m_edjFilePath.c_str(), "settings_items");
    elm_object_part_content_set(settings_layout, "settings_scroller_swallow", scroller);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_policy_set(m_items_layout, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_propagate_events_set(scroller, EINA_TRUE);

    Evas_Object *del_selected_data_button = elm_button_add(m_items_layout);
    elm_object_style_set(del_selected_data_button, "basic_button");
    evas_object_smart_callback_add(del_selected_data_button, "clicked", _del_selected_data_clicked_cb, (void*)id);
    elm_layout_content_set(m_items_layout, "del_selected_data_click", del_selected_data_button);

    Evas_Object *reset_mv_button = elm_button_add(m_items_layout);
    elm_object_style_set(reset_mv_button, "basic_button");
    evas_object_smart_callback_add(reset_mv_button, "clicked", _reset_mv_clicked_cb, (void*)id);
    elm_layout_content_set(m_items_layout, "reset_mv_click", reset_mv_button);

    Evas_Object *reset_browser_button = elm_button_add(m_items_layout);
    elm_object_style_set(reset_browser_button, "basic_button");
    evas_object_smart_callback_add(reset_browser_button, "clicked", _reset_browser_clicked_cb, (void*)id);
    elm_layout_content_set(m_items_layout, "reset_browser_click", reset_browser_button);


    Evas_Object *cache_checkbox = elm_check_add(m_items_layout);
    elm_layout_content_set(m_items_layout, "cache_cb", cache_checkbox);
    elm_check_state_set(cache_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(m_items_layout), "mouse,clicked,1", "cache_cb_text", __checkbox_label_click_cb, (void*)id);
    elm_object_translatable_part_text_set(m_items_layout, "cache_cb_text", "IDS_BR_OPT_CACHE");

    Evas_Object *cookies_checkbox = elm_check_add(m_items_layout);
    elm_layout_content_set(m_items_layout, "cookies_cb", cookies_checkbox);
    elm_check_state_set(cookies_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(m_items_layout), "mouse,clicked,1", "cookies_cb_text", __checkbox_label_click_cb, (void*)id);
    elm_object_translatable_part_text_set(m_items_layout, "cookies_cb_text", "IDS_BR_BODY_COOKIES");

    Evas_Object *history_checkbox = elm_check_add(m_items_layout);
    elm_layout_content_set(m_items_layout, "history_cb", history_checkbox);
    elm_check_state_set(history_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(m_items_layout), "mouse,clicked,1", "history_cb_text", __checkbox_label_click_cb, (void*)id);
    elm_object_translatable_part_text_set(m_items_layout, "history_cb_text", "IDS_BR_MBODY_HISTORY");

    Evas_Object *accept_all_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(accept_all_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "accept_all_rb", accept_all_rb);
    Evas_Object *sharingRequestGroup = accept_all_rb;
    elm_radio_state_value_set(accept_all_rb, SR_ACCEPT_ALL);

    Evas_Object *ask_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(ask_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "ask_rb", ask_rb);
    elm_radio_group_add(ask_rb, sharingRequestGroup);
    elm_radio_state_value_set(ask_rb, SR_ASK);

    Evas_Object *sr_disable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(sr_disable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "sr_disable_rb", sr_disable_rb);
    elm_radio_group_add(sr_disable_rb, sharingRequestGroup);
    elm_radio_state_value_set(sr_disable_rb, SR_DISABLE);

    Evas_Object *bs_enable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(bs_enable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "bs_enable_rb", bs_enable_rb);
    Evas_Object *bookmarkSyncGroup = bs_enable_rb;
    elm_radio_state_value_set(bs_enable_rb, BS_ENABLE);
    elm_object_translatable_part_text_set(m_items_layout, "bs_enable_rb_text", "IDS_BR_BUTTON_ENABLE_ABB");

    Evas_Object *bs_disable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(bs_disable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "bs_disable_rb", bs_disable_rb);
    elm_radio_group_add(bs_disable_rb, bookmarkSyncGroup);
    elm_radio_state_value_set(bs_disable_rb, BS_DISABLE);

    Evas_Object *ts_enable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(ts_enable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "ts_enable_rb", ts_enable_rb);
    Evas_Object *tabSyncGroup = ts_enable_rb;
    elm_radio_state_value_set(ts_enable_rb, TS_ENABLE);
    elm_object_translatable_part_text_set(m_items_layout, "ts_enable_rb_text", "IDS_BR_BUTTON_ENABLE_ABB");

    Evas_Object *ts_disable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(ts_disable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "ts_disable_rb", ts_disable_rb);
    elm_radio_group_add(ts_disable_rb, tabSyncGroup);
    elm_radio_state_value_set(ts_disable_rb, TS_DISABLE);

    return scroller;
}

Evas_Object* SettingsUI::listActionBarContentGet(void* data, Evas_Object* obj , const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (obj && part) {
        const char *part_name = "close_click";
        static const int part_name_len = strlen(part_name);
        if (!strncmp(part_name, part, part_name_len)) {
            Evas_Object *close_click = elm_button_add(obj);
            elm_object_style_set(close_click, "basic_button");
            evas_object_smart_callback_add(close_click, "clicked", SettingsUI::close_clicked_cb, data);
            return close_click;
        }
    }
    return nullptr;
}

void SettingsUI::__checkbox_label_click_cb(void *data, Evas_Object*, const char*, const char *source)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);

        if(strcmp(source, "cache_cb_text") == 0 ){
            Evas_Object *cache_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cache_cb");
            elm_check_state_set(cache_check, !elm_check_state_get(cache_check));
        }
        else if (strcmp(source, "cookies_cb_text") == 0 ){
            Evas_Object *cookies_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cookies_cb");
            elm_check_state_set(cookies_check, !elm_check_state_get(cookies_check));
        }
        else if (strcmp(source, "history_cb_text") == 0 ){
            Evas_Object *history_check = elm_layout_content_get(id->settingsUI->m_items_layout, "history_cb");
            elm_check_state_set(history_check, !elm_check_state_get(history_check));
        }
        else{
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}

void SettingsUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI * s_ui = static_cast<SettingsUI*>(data);
        s_ui->closeSettingsUIClicked();
    }
}

void SettingsUI::_del_selected_data_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);
        Evas_Object *cache_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cache_cb");
        Evas_Object *cookies_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cookies_cb");
        Evas_Object *history_check = elm_layout_content_get(id->settingsUI->m_items_layout, "history_cb");
        std::string type;
        elm_check_state_get(cache_check) ? type += "_CACHE" : "";
        elm_check_state_get(cookies_check) ? type += "_COOKIES" : "";
        elm_check_state_get(history_check) ? type += "_HISTORY" : "";
        id->settingsUI->deleteSelectedDataClicked(type);
    }
}

void SettingsUI::_reset_mv_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->settingsUI->resetMostVisitedClicked();
    }
}

void SettingsUI::_reset_browser_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->settingsUI->resetBrowserClicked();
    }
}

}
}
