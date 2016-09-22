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
#include <stdio.h>
#include <vector>
#include <AbstractMainWindow.h>
#include "app_i18n.h"
#include "SettingsUI_mob.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "AutoFillForm/AutoFillFormManager.h"
#include "UserAgentStrings.h"

namespace tizen_browser{
namespace base_ui{

enum SettingsOptions {
    BASIC,
    ADVANCED,
    DEL_WEB_BRO,
    RESET_MOST_VIS,
    RESET_BRO,
    AUTO_FILL,
    CONTENT,
    PRIVACY,
    DEVELOPER
};

typedef struct _genlistCallbackData {
    UserAgentItem uaItem;
    void *user_data;
    Elm_Object_Item *it;
} genlistCallbackData;

EXPORT_SERVICE(SettingsUI, "org.tizen.browser.settingsui")

SettingsUI::SettingsUI()
    : m_settings_layout(nullptr)
    , m_subpage_layout(nullptr)
    , m_items_layout(nullptr)
    , m_parent(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsMobileUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_setting_item_class = elm_genlist_item_class_new();
    m_setting_item_class->item_style = "settings_button";
    m_setting_item_class->func.text_get = _gengrid_item_text_get;
    m_setting_item_class->func.content_get = nullptr;
    m_setting_item_class->func.state_get = nullptr;
    m_setting_item_class->func.del = nullptr;

    m_setting_parent_item_class = elm_genlist_item_class_new();
    m_setting_parent_item_class->item_style = "settings_parent_button";
    m_setting_parent_item_class->func.text_get = _gengrid_item_text_get;
    m_setting_parent_item_class->func.content_get = nullptr;
    m_setting_parent_item_class->func.state_get = nullptr;
    m_setting_parent_item_class->func.del = nullptr;

    updateButtonMap();
}

SettingsUI::~SettingsUI()
{
    if(m_setting_item_class)
        elm_genlist_item_class_free(m_setting_item_class);
    if(m_setting_parent_item_class)
        elm_genlist_item_class_free(m_setting_parent_item_class);
}

void SettingsUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void SettingsUI::updateButtonMap() {
    ItemData deleteWebBrowsing;
    //TODO Add translation API
    deleteWebBrowsing.buttonText="Delete Web Browsing Data";

    ItemData resetMostVisited;
    resetMostVisited.buttonText="Reset Most Visited Site";

    ItemData resetBrowser;
    resetBrowser.buttonText="Reset Browser";

    ItemData autoFill;
    autoFill.buttonText=_("IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS");

    ItemData content;
    content.buttonText="Content Settings";

    ItemData privacy;
    privacy.buttonText="Privacy";

    ItemData developer;
    developer.buttonText="Developer Options";

    ItemData basic;
    basic.buttonText="<b>Basic</b>";

    ItemData advanced;
    advanced.buttonText="<b>Advanced</b>";

    m_buttonsMap[SettingsOptions::BASIC]=basic;
    m_buttonsMap[SettingsOptions::ADVANCED]=advanced;
    m_buttonsMap[SettingsOptions::DEL_WEB_BRO]=deleteWebBrowsing;
    m_buttonsMap[SettingsOptions::RESET_MOST_VIS]=resetMostVisited;
    m_buttonsMap[SettingsOptions::RESET_BRO]=resetBrowser;
    m_buttonsMap[SettingsOptions::AUTO_FILL]=autoFill;
    m_buttonsMap[SettingsOptions::CONTENT]=content;
    m_buttonsMap[SettingsOptions::PRIVACY]=privacy;
    m_buttonsMap[SettingsOptions::DEVELOPER]=developer;
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
    evas_object_show(m_settings_layout);
    evas_object_show(m_actionBar);
}

void SettingsUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_settings_layout);
    evas_object_hide(m_actionBar);
}

Evas_Object* SettingsUI::createSettingsUILayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    Evas_Object* settings_layout = elm_layout_add(parent);
    elm_layout_file_set(settings_layout, m_edjFilePath.c_str(), "settings-layout");
    evas_object_size_hint_weight_set(settings_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(settings_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(settings_layout);
    m_items_layout = createSettingsMobilePage(settings_layout);
    elm_object_tree_focus_allow_set(settings_layout, EINA_FALSE);

    orientationChanged();

    return settings_layout;
}

void SettingsUI::orientationChanged(){
    auto rot = isLandscape();
    if (rot && *rot) {
        if (m_actionBar)
            elm_object_signal_emit(m_actionBar,"rotation,landscape", "rot");
        if (m_items_layout) {
            elm_object_signal_emit(m_items_layout, "rotation,landscape,main", "rot");
        }
        if (m_subpage_layout)
            elm_object_signal_emit(m_subpage_layout,"rotation,landscape,sub", "rot");
    } else {
        if (m_actionBar)
            elm_object_signal_emit(m_actionBar,"rotation,portrait", "rot");
        if (m_items_layout) {
            elm_object_signal_emit(m_items_layout,"rotation,portrait,main", "rot");
        }
        if (m_subpage_layout)
            elm_object_signal_emit(m_subpage_layout, "rotation,portrait,sub", "rot");
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

Evas_Object* SettingsUI::createBackActionBar(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* actionBar = elm_layout_add(settings_layout);
    elm_object_part_content_set(settings_layout, "actionbar_swallow", actionBar);
    evas_object_size_hint_weight_set(actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(actionBar, m_edjFilePath.c_str(), "action_bar");
    Evas_Object *close_click_btn = elm_button_add(actionBar);
    elm_object_style_set(close_click_btn, "basic_button");
    evas_object_smart_callback_add(close_click_btn, "clicked", SettingsUI::back_clicked_cb, this);
    elm_object_part_content_set(actionBar, "close_click", close_click_btn);

    return actionBar;
}

char* SettingsUI::_gengrid_item_text_get(void* data, Evas_Object* /*obj*/, const char* part)
{
   M_ASSERT(data);
   if (!data)
       return nullptr;

   ItemData* it = static_cast<ItemData*>(data);

   if (strcmp(part, "button_text") == 0) {
       //TODO Implement translation API
       const char* item_name = it->buttonText.c_str();
       if (item_name)
          return strdup(item_name);
   }
   return nullptr;
}

void SettingsUI::_language_changed(void *data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->updateButtonMap();
        elm_genlist_realized_items_update(obj);
    }
}

Evas_Object* SettingsUI::createSettingsMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = createMainView(settings_layout);

    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object* scroller = elm_genlist_add(layout);
    elm_genlist_homogeneous_set(scroller, EINA_TRUE);
    elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);

    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, 0.0);
    elm_genlist_select_mode_set(scroller, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_multi_select_set(scroller, EINA_FALSE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
    evas_object_smart_callback_add(scroller, "language,changed", _language_changed, this);

    auto it = elm_genlist_item_append(scroller, m_setting_parent_item_class, &m_buttonsMap[SettingsOptions::BASIC], nullptr, ELM_GENLIST_ITEM_GROUP, nullptr, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::AUTO_FILL], it, ELM_GENLIST_ITEM_NONE,_auto_fill_data_menu_clicked_cb, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::RESET_MOST_VIS], it, ELM_GENLIST_ITEM_NONE, _reset_mv_menu_clicked_cb, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::DEL_WEB_BRO], it, ELM_GENLIST_ITEM_NONE, _del_selected_data_menu_clicked_cb, this);

    it = elm_genlist_item_append(scroller, m_setting_parent_item_class, &m_buttonsMap[SettingsOptions::ADVANCED], nullptr, ELM_GENLIST_ITEM_GROUP, nullptr, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::PRIVACY], it, ELM_GENLIST_ITEM_NONE,_privacy_menu_clicked_cb, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::CONTENT], it, ELM_GENLIST_ITEM_NONE,_content_settings_menu_clicked_cb, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::RESET_BRO], it, ELM_GENLIST_ITEM_NONE,_reset_browser_menu_clicked_cb, this);
    elm_genlist_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::DEVELOPER], it, ELM_GENLIST_ITEM_NONE,_developer_menu_clicked_cb, this);

    elm_object_part_content_set(layout, "options_swallow", scroller);
    evas_object_show(scroller);

    return layout;
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

Evas_Object* SettingsUI::createInfoField(const char* name, const char* text, Evas_Object* parent)
{
    auto info_field = elm_button_add(parent);
    elm_object_style_set(info_field, name);
    elm_layout_content_set(parent, "info_swallow", info_field);

    elm_object_translatable_part_text_set(info_field, "text", text); // TODO Add translation API
    return info_field;
}

Evas_Object* SettingsUI::createDelDataMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto main = createMainView(settings_layout);

    elm_object_signal_emit(m_actionBar,"back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Delete Web Browsing Data");

    createInfoField("info_field", "You can delete Web Browsing Data optionally.", main);

    auto check_boxes = createDelDataMobileCheckBoxes(main);

    auto scroller = elm_scroller_add(main);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_propagate_events_set(scroller, EINA_TRUE);
    elm_scroller_page_relative_set(scroller, 0, 1);
    elm_object_content_set(scroller, check_boxes);
    evas_object_show(scroller);

    elm_object_part_content_set(main, "options_swallow", scroller);

    return main;
}

Evas_Object* SettingsUI::createDelDataMobileCheckBoxes(Evas_Object* parent)
{
    auto box = elm_box_add(parent);
    elm_box_horizontal_set(box, EINA_FALSE);
    elm_object_content_set(parent, box);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    m_checkbox_layout = elm_layout_add(box);
    evas_object_size_hint_weight_set(m_checkbox_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_checkbox_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_checkbox_layout, m_edjFilePath.c_str(), "delete_browsing_data_mobile");

    auto cache_checkbox = createCheckBox(m_checkbox_layout, "cache", __checkbox_label_click_cb, this);
    auto cookies_checkbox = createCheckBox(m_checkbox_layout, "cookies", __checkbox_label_click_cb, this);
    auto history_checkbox = createCheckBox(m_checkbox_layout, "history", __checkbox_label_click_cb, this);
    auto password_checkbox = createCheckBox(m_checkbox_layout, "password", __checkbox_label_click_cb, this);
    auto formdata_checkbox = createCheckBox(m_checkbox_layout, "formdata", __checkbox_label_click_cb, this);

    elm_check_state_set(cache_checkbox, EINA_TRUE);
    elm_check_state_set(cookies_checkbox, EINA_TRUE);
    elm_check_state_set(history_checkbox, EINA_TRUE);
    elm_check_state_set(password_checkbox, EINA_TRUE);
    elm_check_state_set(formdata_checkbox, EINA_TRUE);

    evas_object_show(m_checkbox_layout);
    elm_box_pack_end(box, m_checkbox_layout);

    auto button_layout = elm_layout_add(box);
    elm_layout_file_set(button_layout, m_edjFilePath.c_str(), "delete_browsing_data_button");
    evas_object_size_hint_weight_set(button_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    auto del_selected_data_button = elm_button_add(button_layout);
    elm_object_style_set(del_selected_data_button, "custom_button");
    elm_layout_content_set(button_layout, "del_selected_data_click", del_selected_data_button);
    evas_object_show(del_selected_data_button);

    evas_object_smart_callback_add(del_selected_data_button, "clicked", _del_selected_data_clicked_cb, this);
    elm_object_translatable_part_text_set(del_selected_data_button, "button_text", "Delete Selected Data"); // TODO Add translation API
    evas_object_show(button_layout);
    elm_box_pack_end(box, button_layout);

    return box;
}

Evas_Object* SettingsUI::createRemoveMostVisitedMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "reset_most_visited");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Reset Most Visited Site");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *reset_mv_button = elm_button_add(layout);
    elm_object_style_set(reset_mv_button, "basic_button");
    evas_object_smart_callback_add(reset_mv_button, "clicked", _reset_mv_clicked_cb, this);
    elm_layout_content_set(layout, "reset_most_visited_click", reset_mv_button);

    elm_object_translatable_part_text_set(layout, "reset_most_visited_sub_text", "You can delete all items of Most Visited Site.");

    return layout;
}

Evas_Object* SettingsUI::createRemoveBrowserDataMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "reset_browser");
    elm_object_part_content_set(settings_layout, "settings_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Reset Browser");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *reset_browser_button = elm_button_add(layout);
    elm_object_style_set(reset_browser_button, "basic_button");
    evas_object_smart_callback_add(reset_browser_button, "clicked", _reset_browser_clicked_cb, this);
    elm_layout_content_set(layout, "reset_browser_click", reset_browser_button);

    elm_object_translatable_part_text_set(layout, "reset_browser_sub_text", "You can delete all data and return to initial setting.");

    return layout;
}

Evas_Object* SettingsUI::createDeveloperOptionsMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "developer_options");
    elm_object_part_content_set(settings_layout, "settings_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Developer Options");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *override_ua_button = elm_button_add(layout);
    elm_object_style_set(override_ua_button, "basic_button");
    evas_object_smart_callback_add(override_ua_button, "clicked", _override_useragent_clicked_cb, this);
    elm_layout_content_set(layout, "override_useragent_click", override_ua_button);

    elm_object_translatable_part_text_set(layout, "developer_options_sub_text", "You can override the Browser's UserAgent to desired string.");

    return layout;
}

Evas_Object* SettingsUI::createCheckBox(Evas_Object* layout, const std::string name, Edje_Signal_Cb func, void* data)
{
    Evas_Object* edje = elm_layout_edje_get(layout);
    Evas_Object* checkbox = elm_check_add(layout);
    elm_object_style_set(checkbox, "custom_check");
    elm_layout_content_set(layout, (name + "_cb").c_str(), checkbox);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", (name + "_cb_text_bg").c_str(), func, data);
    evas_object_show(checkbox);
    return checkbox;
}

void SettingsUI::__checkbox_label_click_cb(void *data, Evas_Object*, const char*, const char *source)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);

        if (strcmp(source, "cache_cb_text_bg") == 0 ){
            Evas_Object *cache_check = elm_layout_content_get(self->m_checkbox_layout, "cache_cb");
            elm_check_state_set(cache_check, !elm_check_state_get(cache_check));
        }
        else if (strcmp(source, "cookies_cb_text_bg") == 0 ){
            Evas_Object *cookies_check = elm_layout_content_get(self->m_checkbox_layout, "cookies_cb");
            elm_check_state_set(cookies_check, !elm_check_state_get(cookies_check));
        }
        else if (strcmp(source, "history_cb_text_bg") == 0 ){
            Evas_Object *history_check = elm_layout_content_get(self->m_checkbox_layout, "history_cb");
            elm_check_state_set(history_check, !elm_check_state_get(history_check));
        }
        else if (strcmp(source, "password_cb_text_bg") == 0 ){
            Evas_Object *password_check = elm_layout_content_get(self->m_checkbox_layout, "password_cb");
            elm_check_state_set(password_check, !elm_check_state_get(password_check));
        }
        else if (strcmp(source, "formdata_cb_text_bg") == 0 ){
            Evas_Object *formdata_check = elm_layout_content_get(self->m_checkbox_layout, "formdata_cb");
            elm_check_state_set(formdata_check, !elm_check_state_get(formdata_check));
        }
        else{
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}

Evas_Object* SettingsUI::createContentSettingsPage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto main = createMainView(settings_layout);

    elm_object_signal_emit(m_actionBar,"back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Content Settings");

    createInfoField("info_field", "Choose web page content", main);

    auto box = elm_box_add(main);
    elm_box_horizontal_set(box, EINA_FALSE);
    elm_object_content_set(main, box);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    m_checkbox_layout = elm_layout_add(box);
    evas_object_size_hint_weight_set(m_checkbox_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_checkbox_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_checkbox_layout, m_edjFilePath.c_str(), "content_settings_mobile");

    boost::optional<bool> sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::PAGE_OVERVIEW);
    Eina_Bool flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* overview_checkbox = createCheckBox(m_checkbox_layout, "overview", __checkbox_content_settings_label_click_cb, this);
    elm_check_state_set(overview_checkbox, flag);

    sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::LOAD_IMAGES);
    flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* images_checkbox = createCheckBox(m_checkbox_layout, "images", __checkbox_content_settings_label_click_cb, this);
    elm_check_state_set(images_checkbox, flag);

    sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT);
    flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* javascript_checkbox = createCheckBox(m_checkbox_layout, "javascript", __checkbox_content_settings_label_click_cb, this);
    elm_check_state_set(javascript_checkbox, flag);

    evas_object_show(m_checkbox_layout);
    elm_box_pack_end(box, m_checkbox_layout);

    auto scroller = elm_scroller_add(main);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_propagate_events_set(scroller, EINA_TRUE);
    elm_scroller_page_relative_set(scroller, 0, 1);
    elm_object_content_set(scroller, box);
    evas_object_show(scroller);

    elm_object_part_content_set(main, "options_swallow", scroller);

    return main;
}

Evas_Object* SettingsUI::createPrivacyPage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto main = createMainView(settings_layout);

    elm_object_signal_emit(m_actionBar,"back,icon,change", "del_but");
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Privacy");

    createInfoField("info_field", "Choose privacy settings", main);

    auto box = elm_box_add(main);
    elm_box_horizontal_set(box, EINA_FALSE);
    elm_object_content_set(main, box);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    m_checkbox_layout = elm_layout_add(box);
    elm_layout_file_set(m_checkbox_layout, m_edjFilePath.c_str(), "privacy_mobile");
    evas_object_size_hint_weight_set(m_checkbox_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_checkbox_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    boost::optional<bool> sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_FROM_DATA);
    Eina_Bool flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* form_data_checkbox = createCheckBox(m_checkbox_layout, "form_data", __checkbox_privacy_label_click_cb, this);
    elm_check_state_set(form_data_checkbox, flag);

    sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_PASSWORDS);
    flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* passwd_checkbox = createCheckBox(m_checkbox_layout, "passwords", __checkbox_privacy_label_click_cb, this);
    elm_check_state_set(passwd_checkbox, flag);

    evas_object_show(m_checkbox_layout);
    elm_box_pack_end(box, m_checkbox_layout);

    auto scroller = elm_scroller_add(main);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_propagate_events_set(scroller, EINA_TRUE);
    elm_scroller_page_relative_set(scroller, 0, 1);
    elm_object_content_set(scroller, box);
    evas_object_show(scroller);

    elm_object_part_content_set(main, "options_swallow", scroller);

    return main;
}

void SettingsUI::__checkbox_content_settings_label_click_cb(void* data, Evas_Object*, const char*, const char* source)
{
    if (data) {
        auto self = static_cast<SettingsUI*>(data);

        if (strcmp(source, "overview_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_checkbox_layout, "overview_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::PAGE_OVERVIEW, static_cast<bool>(value));
        } else if (strcmp(source, "images_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_checkbox_layout, "images_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::LOAD_IMAGES, static_cast<bool>(value));
        } else if (strcmp(source, "javascript_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_checkbox_layout, "javascript_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT, static_cast<bool>(value));
        } else {
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    } else {
        BROWSER_LOGD("[%s:%d] Warning no data specified!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SettingsUI::__checkbox_privacy_label_click_cb(void* data, Evas_Object*, const char*, const char* source)
{
    if (data) {
        auto self = static_cast<SettingsUI*>(data);

        if (strcmp(source, "form_data_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_checkbox_layout, "form_data_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_FROM_DATA, static_cast<bool>(value));
        } else if (strcmp(source, "passwords_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_checkbox_layout, "passwords_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_PASSWORDS, static_cast<bool>(value));
        } else {
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    } else {
        BROWSER_LOGD("[%s:%d] Warning no data specified!", __PRETTY_FUNCTION__, __LINE__);
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

void SettingsUI::back_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI * s_ui = static_cast<SettingsUI*>(data);
        s_ui->onBackKey();
    }
}

void SettingsUI::initializeAutoFillManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    resetItemsLayoutContent();

    m_actionBar = createBackActionBar(m_settings_layout);
    auto main = createMainView(m_settings_layout);
    m_autoFillManager = std::unique_ptr<AutoFillFormManager>(new AutoFillFormManager());
    m_autoFillManager->listViewBackClicked.connect(boost::bind(&SettingsUI::destroyAutoFillManager, this));
    m_autoFillManager->init(main, m_actionBar);

    m_subpage_layout = m_autoFillManager->showListView();
    elm_object_tree_focus_allow_set(m_settings_layout, EINA_TRUE);
}

void SettingsUI::destroyAutoFillManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_tree_focus_allow_set(m_settings_layout, EINA_FALSE);
}

void SettingsUI::_auto_fill_data_menu_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->initializeAutoFillManager();
    }
}

void SettingsUI::_del_selected_data_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        Evas_Object *cache_check = elm_layout_content_get(self->m_checkbox_layout, "cache_cb");
        Evas_Object *cookies_check = elm_layout_content_get(self->m_checkbox_layout, "cookies_cb");
        Evas_Object *history_check = elm_layout_content_get(self->m_checkbox_layout, "history_cb");
        Evas_Object *password_check = elm_layout_content_get(self->m_checkbox_layout, "password_cb");
        Evas_Object *formdata_check = elm_layout_content_get(self->m_checkbox_layout, "formdata_cb");
        std::string type;
        elm_check_state_get(cache_check) ? type += "_CACHE" : "";
        elm_check_state_get(cookies_check) ? type += "_COOKIES" : "";
        elm_check_state_get(history_check) ? type += "_HISTORY" : "";
        elm_check_state_get(password_check) ? type += "_PASSWORD" : "";
        elm_check_state_get(formdata_check) ? type += "_FORMDATA" : "";

        self->deleteSelectedDataClicked(type);
    }
}

void SettingsUI::_del_selected_data_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createDelDataMobilePage(self->m_settings_layout);
        self->orientationChanged();
    }
}

void SettingsUI::_reset_mv_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetMostVisitedClicked();
        self->orientationChanged();
    }
}

void SettingsUI::_reset_mv_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createRemoveMostVisitedMobilePage(self->m_settings_layout);
        self->orientationChanged();
    }
}

void SettingsUI::_reset_browser_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetBrowserClicked();
        self->orientationChanged();
    }
}

void SettingsUI::_reset_browser_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createRemoveBrowserDataMobilePage(self->m_settings_layout);
        self->orientationChanged();
    }
}

void SettingsUI::_content_settings_menu_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createContentSettingsPage(self->m_settings_layout);
        self->orientationChanged();
    }
}

void SettingsUI::_privacy_menu_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createPrivacyPage(self->m_settings_layout);
        self->orientationChanged();
    }
}

void SettingsUI::_developer_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createDeveloperOptionsMobilePage(self->m_settings_layout);
        self->orientationChanged();
    }
}

void SettingsUI::_override_useragent_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createUserAgentGenList(self->m_settings_layout);
        self->orientationChanged();
    }
}

Evas_Object* SettingsUI::createUserAgentGenList(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "useragent_list");
    elm_object_part_content_set(settings_layout, "settings_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Override UserAgent");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *genlist = elm_genlist_add(layout);
    if (!genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return NULL;
    }
    Elm_Genlist_Item_Class* itemClass = elm_genlist_item_class_new();
    itemClass->item_style = "useragent_item";
    itemClass->func.content_get = NULL;
    itemClass->func.text_get = _ua_text_get_cb;
    itemClass->func.state_get = NULL;
    itemClass->func.del = NULL;

    for (unsigned int i = 0; i < UA_ITEMS_COUNT; i++) {
       genlistCallbackData* item_callback_data = new genlistCallbackData;
       item_callback_data->uaItem = {uaList[i].name, uaList[i].uaString};
       item_callback_data->user_data = this;
       item_callback_data->it = elm_genlist_item_append(genlist, itemClass, item_callback_data, NULL,
                                    ELM_GENLIST_ITEM_NONE, _useragent_item_clicked_cb, item_callback_data);
    }
    evas_object_show(genlist);
    elm_object_part_content_set(layout, "ua_genlist_swallow", genlist);

    return layout;
}

void SettingsUI::_useragent_item_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
        SettingsUI *self = static_cast<SettingsUI*>(callback_data->user_data);
        self->userAgentItemClicked(std::string(callback_data->uaItem.uaString));
    }
}

char* SettingsUI::_ua_text_get_cb(void* data, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    if (!strcmp(part, "item_title"))
        return strdup(callback_data->uaItem.name);
    return NULL;
}

void SettingsUI::resetItemsLayoutContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_part_content_unset(this->m_settings_layout, "settings_swallow");
    evas_object_del(this->m_actionBar);
    evas_object_del(this->m_items_layout);
    evas_object_del(this->m_subpage_layout);
    evas_object_del(this->m_checkbox_layout);
    this->destroyAutoFillManager();
    this->m_subpage_layout = nullptr;
    this->m_checkbox_layout = nullptr;
    this->m_items_layout = nullptr;
    this->m_actionBar = nullptr;
}

bool SettingsUI::isSubpage()
{
    return (m_subpage_layout != nullptr);
}

void SettingsUI::onBackKey()
{
    resetItemsLayoutContent();
    m_actionBar = createActionBar(m_settings_layout);
    m_items_layout = createSettingsMobilePage(m_settings_layout);
    this->orientationChanged();
}
}
}
