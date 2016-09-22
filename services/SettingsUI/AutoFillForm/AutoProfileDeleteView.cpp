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

#include "AutoFillFormManager.h"
#include "AutoProfileDeleteView.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

AutoProfileDeleteView::AutoProfileDeleteView(AutoFillFormManager* manager)
    : m_manager(manager)
    , m_parent(nullptr)
    , m_back_button(nullptr)
    , m_del_button(nullptr)
    , m_checkbox(nullptr)
    , m_mainLayout(nullptr)
    , m_genlist(nullptr)
    , m_action_bar(nullptr)
    , m_itemClass(nullptr)
    , m_checked_count(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

AutoProfileDeleteView::~AutoProfileDeleteView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_smart_callback_del(m_back_button, "clicked", __back_button_cb);
    evas_object_smart_callback_del(m_del_button, "clicked", __delete_button_cb);
    evas_object_smart_callback_del(m_checkbox, "clicked", __select_all_checkbox_changed_cb);

    elm_object_signal_emit(m_action_bar, "hide,autofill,close,icon", "del_but");
    elm_object_signal_emit(m_action_bar, "show,close,icon", "del_but");

    if (m_genlist) {
        elm_genlist_clear(m_genlist);
        evas_object_del(m_genlist);
    }
    if (m_mainLayout) {
        evas_object_hide(m_mainLayout);
        evas_object_del(m_mainLayout);
    }
    m_mainLayout = nullptr;
    m_genlist = nullptr;
}

Evas_Object* AutoProfileDeleteView::show(Evas_Object* parent, Evas_Object* action_bar)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_action_bar = action_bar;
    elm_object_translatable_part_text_set(m_action_bar, "settings_title", "Auto Fill Delete");

    m_mainLayout = createMainLayout(parent);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return nullptr;
    }

    m_back_button = elm_button_add(m_mainLayout);
    elm_object_style_set(m_back_button, "basic_button");
    evas_object_smart_callback_add(m_back_button, "clicked", __back_button_cb, this);
    elm_object_part_content_set(m_action_bar, "close_autofill_del_click", m_back_button);
    elm_object_signal_emit(m_action_bar, "hide,close,icon", "del_but");
    elm_object_signal_emit(m_action_bar, "show,autofill,close,icon", "del_but");

    m_del_button = elm_button_add(m_mainLayout);
    elm_object_style_set(m_del_button, "basic_button");
    evas_object_smart_callback_add(m_del_button, "clicked", __delete_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "del_button", m_del_button);
    elm_object_signal_emit(m_mainLayout, "dim,del,button,signal", "");
    elm_object_disabled_set(m_del_button, true);

    evas_object_show(m_mainLayout);
    elm_layout_content_set(parent, "autofill_del_swallow", m_mainLayout);
    m_parent = parent;
    return m_mainLayout;
}

Evas_Object *AutoProfileDeleteView::createMainLayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *layout = elm_layout_add(parent);
    if (!layout) {
        BROWSER_LOGD("elm_layout_add failed");
        return nullptr;
    }
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "affdv-layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(layout, "select_all_text", "IDS_BR_OPT_SELECT_ALL");
    elm_object_translatable_part_text_set(layout, "del_text", "IDS_BR_SK_DELETE");

    Evas_Object* checkbox = elm_check_add(layout);
    if (!checkbox) {
        BROWSER_LOGE("Failed to add check");
        return nullptr;
    }
    elm_object_style_set(checkbox, "custom_check");
    Eina_Bool checked = false;
    elm_check_state_pointer_set(checkbox, &checked);
    evas_object_propagate_events_set(checkbox, EINA_FALSE);
    elm_object_part_content_set(layout, "select_all_checkbox", checkbox);

    m_checkbox = elm_button_add(layout);
    if (!m_checkbox) {
        BROWSER_LOGE("Failed to add button");
        return nullptr;
    }
    elm_object_style_set(m_checkbox, "basic_button");
    evas_object_smart_callback_add(m_checkbox, "clicked", __select_all_checkbox_changed_cb, this);
    elm_object_part_content_set(layout, "select_all_checkbox_button", m_checkbox);

    m_genlist = createGenlist(layout);
    if (!m_genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return nullptr;
    }
    evas_object_show(m_genlist);
    elm_object_part_content_set(layout, "affdv_genlist", m_genlist);

    return layout;
}

Evas_Object *AutoProfileDeleteView::createGenlist(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *genlist = elm_genlist_add(parent);
    if (!genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return nullptr;
    }

    elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    elm_genlist_multi_select_set(genlist, EINA_FALSE);
    elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_itemClass = elm_genlist_item_class_new();
    if (!m_itemClass) {
        BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
        return nullptr;
    }
    m_itemClass->item_style = "affdv_item";
    m_itemClass->func.content_get = __content_get_cb;

    m_itemClass->func.text_get = __text_get_cb;
    m_itemClass->func.state_get = NULL;
    m_itemClass->func.del = NULL;

    appendGenlist(genlist);

    return genlist;
}

Eina_Bool AutoProfileDeleteView::appendGenlist(Evas_Object *genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_checkbox_delete_state_map.clear();
    unsigned int item_count = m_manager->getAutoFillFormItemCount();
    if (item_count > 0) {
        for (unsigned int i = 0; i < item_count; i++) {
           genlistCallbackData* item_callback_data = new genlistCallbackData;
           item_callback_data->menu_index = i;
           item_callback_data->user_data = this;
           item_callback_data->it = elm_genlist_item_append(genlist, m_itemClass,
                                        item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
                                        __genlist_item_selected_cb, item_callback_data);
           m_checkbox_delete_state_map.insert(std::pair<Elm_Object_Item*, bool>(item_callback_data->it, false));
        }
    }

    return EINA_TRUE;
}

void AutoProfileDeleteView::__select_all_checkbox_changed_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoProfileDeleteView *apdv = static_cast<AutoProfileDeleteView*>(data);
    Evas_Object* sel_all_checkbox = elm_object_part_content_get(apdv->m_mainLayout, "select_all_checkbox");
    bool sel_all_state = elm_check_state_get(sel_all_checkbox) == EINA_TRUE;
    elm_check_state_set(sel_all_checkbox, !sel_all_state);

    for (auto it = apdv->m_checkbox_delete_state_map.begin(); it != apdv->m_checkbox_delete_state_map.end(); ++it)
        if (it->second == sel_all_state) {
            it->second = !sel_all_state;
            elm_genlist_item_update(it->first);
        }

    elm_genlist_realized_items_update(apdv->m_genlist);

    if (sel_all_state)
        apdv->m_checked_count = 0;
    else
        apdv->m_checked_count = elm_genlist_items_count(apdv->m_genlist);

    if (apdv->m_checked_count >= 1) {
        elm_object_signal_emit(apdv->m_mainLayout, "show,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), false);
    }
    else {
        elm_object_signal_emit(apdv->m_mainLayout, "dim,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), true);
    }
}

void AutoProfileDeleteView::__back_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoProfileDeleteView *view = static_cast<AutoProfileDeleteView*>(data);
    elm_object_translatable_part_text_set(view->m_action_bar, "settings_title", "IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS");
    view->hide();
}

void AutoProfileDeleteView::__genlist_item_selected_cb(void* data, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);

    Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
    elm_genlist_item_selected_set(item, EINA_FALSE);
    Evas_Object *checkbox = elm_object_item_part_content_get(item, "checkbox");
    Eina_Bool state = elm_check_state_get(checkbox);
    elm_check_state_set(checkbox, !state);
    callback_data->user_data->m_checkbox_delete_state_map[item] = !callback_data->user_data->m_checkbox_delete_state_map[item];
    elm_genlist_item_update(item);

    if (state)
        callback_data->user_data->m_checked_count--;
    else
        callback_data->user_data->m_checked_count++;

    BROWSER_LOGD("----Checked count : [%d]---- ", callback_data->user_data->m_checked_count);
    Evas_Object* sel_all_checkbox = elm_object_part_content_get(callback_data->user_data->m_mainLayout, "select_all_checkbox");
    if (callback_data->user_data->m_checked_count == elm_genlist_items_count(callback_data->user_data->m_genlist))
        elm_check_state_set(sel_all_checkbox, true);
    else
        elm_check_state_set(sel_all_checkbox, false);

    if (callback_data->user_data->m_checked_count == 0) {
        elm_object_signal_emit(callback_data->user_data->m_mainLayout, "dim,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(callback_data->user_data->m_mainLayout, "del_button"), true);
    } else {
        elm_object_signal_emit(callback_data->user_data->m_mainLayout, "show,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(callback_data->user_data->m_mainLayout, "del_button"), false);
    }
}

void AutoProfileDeleteView::refreshView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_clear(m_genlist);
    appendGenlist(m_genlist);
}

void AutoProfileDeleteView::__delete_button_cb(void* data, Evas_Object* /*obj*/,void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoProfileDeleteView *apdv = static_cast<AutoProfileDeleteView*>(data);
    if (apdv->m_checked_count == elm_genlist_items_count(apdv->m_genlist))
        apdv->deleteAllItems();
    else
        apdv->deleteSelectedItems();
    elm_genlist_realized_items_update(apdv->m_genlist);
    auto sel_all_checkbox = elm_object_part_content_get(apdv->m_mainLayout, "select_all_checkbox");
    if (elm_check_state_get(sel_all_checkbox) == EINA_TRUE) {
        elm_check_state_set(sel_all_checkbox, EINA_FALSE);
        elm_object_signal_emit(apdv->m_mainLayout, "dim,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), true);
    }
}

void AutoProfileDeleteView::deleteAllItems(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_checkbox_delete_state_map.clear();
    m_manager->deleteAllAutoFillFormItems();
    m_manager->refreshListView();
}

void AutoProfileDeleteView::deleteSelectedItems(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *cb_data = nullptr;
    Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
    Evas_Object *checkbox;
    int del_count = 0;

    while (it) {
        checkbox = elm_object_item_part_content_get(it, "checkbox");
        cb_data = static_cast<genlistCallbackData*>(elm_object_item_data_get(it));
        if (elm_check_state_get(checkbox)) {
            AutoFillFormItem *item = m_manager->getItem(cb_data->menu_index - del_count);
            m_manager->deleteAutoFillFormItem(item);
            del_count++;
            m_checkbox_delete_state_map.erase(it);
        }
        it = elm_genlist_item_next_get(it);
    }
    BROWSER_LOGD("Total items deleted %d",del_count);
    m_manager->refreshListView();
}

char *AutoProfileDeleteView::__text_get_cb(void* data, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);

    if (!strcmp(part, "item_title")) {
        const char *item_full_name = callback_data->user_data->getEachItemFullName(
                (unsigned int)callback_data->menu_index);
        if (item_full_name)
           return strdup(item_full_name);
    }
    return NULL;
}

const char *AutoProfileDeleteView::getEachItemFullName(unsigned int index)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_manager->getAutoFillFormItemCount() == 0)
        return NULL;
    return (m_manager->getItem(index))->getName();
}

Evas_Object *AutoProfileDeleteView::__content_get_cb(void* data, Evas_Object* obj, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    if (!strcmp(part, "checkbox")) {
        Evas_Object *checkbox = elm_check_add(obj);
        if (!checkbox) {
            BROWSER_LOGE("Failed to add check");
            return NULL;
        }

        genlistCallbackData *itemData = static_cast<genlistCallbackData*>(data);
        elm_object_style_set(checkbox, "custom_check");
        elm_check_state_set(checkbox, itemData->user_data->m_checkbox_delete_state_map[itemData->it]
                                        ? EINA_TRUE : EINA_FALSE);
        evas_object_propagate_events_set(checkbox, EINA_FALSE);
        return checkbox;
    }
    return NULL;
}

void AutoProfileDeleteView::hide(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_manager)
        m_manager->deleteDeleteView();
}

}
}
