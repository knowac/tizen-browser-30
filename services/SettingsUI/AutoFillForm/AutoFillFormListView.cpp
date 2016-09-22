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
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "AutoProfileDeleteView.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

AutoFillFormListView::AutoFillFormListView(AutoFillFormManager *affm)
    : m_manager(affm)
    , m_parent(nullptr)
    , m_mainLayout(nullptr)
    , m_add_btn(nullptr)
    , m_del_btn(nullptr)
    , m_genlist(nullptr)
    , m_action_bar(nullptr)
    , m_itemClass(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsMobileUI.edj");
}

AutoFillFormListView::~AutoFillFormListView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_smart_callback_del(m_add_btn, "clicked", __add_profile_button_cb);
    evas_object_smart_callback_del(m_del_btn, "clicked", __delete_profile_button_cb);

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

Evas_Object* AutoFillFormListView::show(Evas_Object* parent, Evas_Object* action_bar)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_action_bar = action_bar;
    elm_object_translatable_part_text_set(m_action_bar, "settings_title", "IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS");

    m_mainLayout = createMainLayout(parent);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return EINA_FALSE;
    }

    m_add_btn = elm_button_add(m_mainLayout);
    elm_object_style_set(m_add_btn, "basic_button");
    evas_object_smart_callback_add(m_add_btn, "clicked", __add_profile_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "add_profile_button", m_add_btn);

    m_del_btn = elm_button_add(m_mainLayout);
    elm_object_style_set(m_del_btn, "basic_button");
    evas_object_smart_callback_add(m_del_btn, "clicked", __delete_profile_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "delete_profile_button", m_del_btn);

    evas_object_show(m_mainLayout);
    elm_layout_content_set(parent, "autofill_sub_swallow", m_mainLayout);
    m_parent = parent;

    return m_mainLayout;
}

void AutoFillFormListView::refreshView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_clear(m_genlist);
    appendGenlist(m_genlist);
}

Evas_Object *AutoFillFormListView::createMainLayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *layout = elm_layout_add(parent);
    if (!layout) {
        BROWSER_LOGD("elm_layout_add failed");
        return nullptr;
    }
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "afflv-layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(layout, "title_text", "IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS");
    elm_object_translatable_part_text_set(layout, "profile_text", "IDS_BR_HEADER_PROFILES");
    elm_object_translatable_part_text_set(layout, "add_profile_text", "IDS_BR_OPT_ADD");
    elm_object_translatable_part_text_set(layout, "delete_profile_text", "IDS_BR_SK_DELETE_ABB");

    m_genlist = createGenlist(layout);
    if (!m_genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return nullptr;
    }
    evas_object_show(m_genlist);
    elm_object_part_content_set(layout, "afflv_genlist", m_genlist);
    return layout;
}

Evas_Object *AutoFillFormListView::createGenlist(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *genlist = elm_genlist_add(parent);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    if (!genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return nullptr;
    }

    m_itemClass = elm_genlist_item_class_new();
    if (!m_itemClass) {
        BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
        return EINA_FALSE;
    }
    m_itemClass->item_style = "afflv_item";
    m_itemClass->func.content_get = nullptr;

    m_itemClass->func.text_get = __text_get_cb;
    m_itemClass->func.state_get = nullptr;
    m_itemClass->func.del = nullptr;

    m_manager->refreshListView();

    return genlist;
}

const char *AutoFillFormListView::getEachItemFullName(unsigned int index)
{
    if (m_manager->getAutoFillFormItemCount() == 0)
        return nullptr;
    return (m_manager->getItemList())[index]->getName();
}

Eina_Bool AutoFillFormListView::appendGenlist(Evas_Object *genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    unsigned int item_count = m_manager->getAutoFillFormItemCount();
    BROWSER_LOGD("Item count : [%d]", item_count);
    if (item_count > 0) {
        for (unsigned int i = 0; i < item_count; i++) {
            genlistCallbackData* item_callback_data = new genlistCallbackData;
            item_callback_data->menu_index = i;
            item_callback_data->user_data = this;
            item_callback_data->it = elm_genlist_item_append(genlist, m_itemClass,
            item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, item_callback_data);
        }
        if (m_mainLayout) {
            elm_object_signal_emit(m_mainLayout, "show,del,button,signal", "");
            elm_object_disabled_set(m_del_btn, false);
        }
    }
    else {
        if (m_mainLayout) {
            elm_object_signal_emit(m_mainLayout, "dim,del,button,signal", "");
            elm_object_disabled_set(m_del_btn, true);
        }
    }

    return EINA_TRUE;
}

char *AutoFillFormListView::__text_get_cb(void* data, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormListView *view = static_cast<AutoFillFormListView*>(callback_data->user_data);

    if (!strcmp(part, "item_title")) {
        const char *item_full_name = view->getEachItemFullName((unsigned int)callback_data->menu_index);
        if (item_full_name)
           return strdup(item_full_name);
    }
    return nullptr;

}

void AutoFillFormListView::__add_profile_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    /* create new profile */
    AutoFillFormListView *list_view = static_cast<AutoFillFormListView*>(data);
    list_view->m_manager->showComposeView();
}

void AutoFillFormListView::__delete_profile_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoFillFormListView *afflv = static_cast<AutoFillFormListView*>(data);
    afflv->m_manager->showDeleteView();
}

void AutoFillFormListView::__back_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoFillFormListView *list_view = static_cast<AutoFillFormListView*>(data);
    list_view->hide();
}

void AutoFillFormListView::__genlist_item_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormListView *view = static_cast<AutoFillFormListView*>(callback_data->user_data);

    elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
    view->m_manager->showComposeView((view->m_manager->getItemList())[callback_data->menu_index]);
}

void AutoFillFormListView::hide()
{
    evas_object_hide(m_mainLayout);
    m_manager->listViewBackClicked();
}

}
}
