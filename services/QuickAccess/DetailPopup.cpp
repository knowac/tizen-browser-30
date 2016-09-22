/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <vector>
#include <algorithm>
#include "app_i18n.h"
#include "services/HistoryService/HistoryItem.h"
#include "BrowserAssert.h"
#include "DetailPopup.h"
#include "BrowserLogger.h"
#include "Tools/GeneralTools.h"
#include "Tools/EflTools.h"
#include "QuickAccess.h"

namespace tizen_browser{
namespace base_ui{

const char * DetailPopup::URL_SEPARATOR = " - ";
const int DetailPopup::HISTORY_ITEMS_NO = 5;

DetailPopup::DetailPopup(QuickAccess *quickAccess)
    : m_main_view(nullptr)
    , m_parent(nullptr)
    , m_layout(nullptr)
    , m_historyList(nullptr)
    , m_urlButton(nullptr)
    , m_history_item_class(nullptr)
    , m_quickAccess(quickAccess)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("QuickAccess/DetailPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());

    m_history_item_class = elm_genlist_item_class_new();
    m_history_item_class->item_style = "history_grid_item";
    m_history_item_class->func.text_get = _get_history_link_text;
    m_history_item_class->func.content_get =  nullptr;
    m_history_item_class->func.state_get = nullptr;
    m_history_item_class->func.del = nullptr;
}

DetailPopup::~DetailPopup()
{
    elm_genlist_item_class_free(m_history_item_class);
}

void DetailPopup::createLayout()
{
    m_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "popup");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    // TODO: add following (or simillar) callback for hardware back button when it will be available
    //evas_object_event_callback_add(m_layout, EVAS_CALLBACK_KEY_DOWN, _back, this);
    edje_object_signal_callback_add(elm_layout_edje_get(m_layout), "mouse,clicked,1", "bg", _bg_click, this);
    m_urlButton = elm_button_add(m_layout);     // add button to receive focus
    elm_object_style_set(m_urlButton, "invisible_button");
    evas_object_smart_callback_add(m_urlButton, "clicked", _url_click_button, this);
    evas_object_show(m_urlButton);
    elm_layout_content_set(m_layout, "url_over", m_urlButton);
    elm_object_focus_custom_chain_unset(m_main_view);
    elm_object_focus_custom_chain_append(m_main_view, m_urlButton, NULL);
    elm_object_focus_set(m_urlButton, EINA_TRUE);

    edje_object_signal_callback_add(elm_layout_edje_get(m_layout), "mouse,clicked,1", "thumbnail", _url_click, this);
    elm_object_translatable_part_text_set(m_layout, "history_title", "IDS_BR_MBODY_HISTORY");
    elm_layout_text_set(m_layout, "url", tools::clearURL(m_item->getUrl()).c_str());

    m_historyList = elm_genlist_add(m_layout);
    evas_object_size_hint_weight_set(m_historyList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_historyList, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_genlist_select_mode_set(m_historyList, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_object_focus_custom_chain_append(m_main_view, m_historyList, NULL);
    for (auto it = m_prevItems->begin(); it != m_prevItems->end(); ++it) {
        elm_genlist_item_append(m_historyList, m_history_item_class, it->get(), nullptr, ELM_GENLIST_ITEM_NONE, _history_url_click, this);
    }
    evas_object_show(m_historyList);
    elm_object_part_content_set(m_layout, "history_list", m_historyList);

    if (m_item->getThumbnail()) {
        Evas_Object * thumb = m_item->getThumbnail()->getEvasImage(m_layout);
        elm_object_part_content_set(m_layout, "thumbnail", thumb);
    }

    evas_object_show(m_layout);
}

void DetailPopup::show(Evas_Object *parent, Evas_Object *main_view, std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_main_view = main_view;
    m_parent = parent;
    m_item = currItem;
    m_prevItems = prevItems;
    createLayout();
}

void DetailPopup::hide()
{
    edje_object_signal_callback_del(elm_layout_edje_get(m_layout), "mouse,clicked,1", "bg", _bg_click);
    elm_object_focus_custom_chain_unset(m_main_view);
    evas_object_smart_callback_del(m_urlButton, "clicked", _url_click_button);
    edje_object_signal_callback_del(elm_layout_edje_get(m_layout), "mouse,clicked,1", "thumbnail", _url_click);
    elm_genlist_clear(m_historyList);
    evas_object_hide(m_layout);
    evas_object_del(m_layout);
    m_layout = nullptr;
#if !PROFILE_MOBILE
    m_quickAccess->refreshFocusChain();
#endif
}

void DetailPopup::_bg_click(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    dp->hide();
}

void DetailPopup::_url_click(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    dp->openURL(dp->m_item, dp->m_quickAccess->isDesktopMode());
    dp->hide();
}

void DetailPopup::_url_click_button(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    dp->openURL(dp->m_item, dp->m_quickAccess->isDesktopMode());
    dp->hide();
}

void DetailPopup::_history_url_click(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *glit = reinterpret_cast<Elm_Object_Item*>(event_info);
    void *itemData = elm_object_item_data_get(glit);
    services::HistoryItem *item = reinterpret_cast<services::HistoryItem*>(itemData);

    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    // find shared pointer pointed to HistoryItem object
    auto it = std::find_if(dp->m_prevItems->begin(),
                           dp->m_prevItems->end(),
                           [item] (const std::shared_ptr<services::HistoryItem> i)
                           { return i.get() == item; }
                          );
    std::shared_ptr<services::HistoryItem> itemPtr= *it;
    dp->openURL(itemPtr, dp->m_quickAccess->isDesktopMode());
    dp->hide();
}

char* DetailPopup::_get_history_link_text(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    services::HistoryItem *item = reinterpret_cast<services::HistoryItem*>(data);
    if (!strcmp(part, "page_dsc")) {
        if (item != nullptr) {
            std::string text = item->getTitle() + URL_SEPARATOR + tools::clearURL(item->getUrl());
            return strdup(text.c_str());
        }
    }
    return strdup("");
}

}
}
