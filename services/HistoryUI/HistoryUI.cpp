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
#include <boost/concept_check.hpp>
#include <vector>
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>

#include "HistoryUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "Tools/GeneralTools.h"
#include "HistoryDaysListManager/HistoryDaysListManagerMob.h"
#include "HistoryDaysListManager/HistoryDaysListManagerTv.h"
#include "services/HistoryService/HistoryItem.h"
#if !PROFILE_MOBILE
#include "HistoryUIFocusManager.h"
#endif
#include "HistoryDeleteManager.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(HistoryUI, "org.tizen.browser.historyui")

typedef struct _HistoryItemData
{
    std::shared_ptr<tizen_browser::services::HistoryItem> item;
    std::shared_ptr<tizen_browser::base_ui::HistoryUI> historyUI;
} HistoryItemData;

struct ItemData{
    tizen_browser::base_ui::HistoryUI* historyUI;
    Elm_Object_Item * e_item;
};

static std::vector<HistoryItemData*> _history_item_data;

HistoryUI::HistoryUI()
    : m_parent(nullptr)
    , m_main_layout(nullptr)
    , m_actionBar(nullptr)
    , m_buttonClose(nullptr)
    , m_buttonClear(nullptr)
    , m_daysList(nullptr)
    , m_historyDeleteManager(std::make_shared<HistoryDeleteManager>())
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("HistoryUI/History.edj");

#if PROFILE_MOBILE
    m_historyDaysListManager = std::make_shared<HistoryDaysListManagerMob>();
#else
    m_historyDaysListManager = std::make_shared<HistoryDaysListManagerTv>(m_historyDeleteManager);
#endif

    m_historyDaysListManager->signalHistoryItemClicked.connect(signalHistoryItemClicked);
    m_historyDaysListManager->signalDeleteHistoryItems.connect(signalDeleteHistoryItems);
#if !PROFILE_MOBILE
    m_focusManager = std::unique_ptr<HistoryUIFocusManager>(
            new HistoryUIFocusManager(m_historyDaysListManager));
#endif
}

HistoryUI::~HistoryUI()
{
}

void HistoryUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_main_layout);
    evas_object_show(m_actionBar);
    evas_object_show(m_main_layout);
#if !PROFILE_MOBILE
    m_focusManager->refreshFocusChain();
#endif
}

void HistoryUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_main_layout);
    evas_object_hide(m_actionBar);
    evas_object_hide(m_main_layout);
    clearItems();
#if !PROFILE_MOBILE
    m_focusManager->unsetFocusChain();
#endif
    m_historyDeleteManager->setDeleteMode(false);
}


void HistoryUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* HistoryUI::getContent()
{
    M_ASSERT(m_parent);
    if (!m_main_layout)
        createHistoryUILayout(m_parent);
    return m_main_layout;
}

void HistoryUI::createHistoryUILayout(Evas_Object* parent)
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_main_layout = elm_layout_add(parent);

    elm_layout_file_set(m_main_layout, m_edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(m_main_layout);
    m_daysList = createDaysList(m_main_layout);
    clearItems();
#if !PROFILE_MOBILE
    m_focusManager->setFocusObj(m_main_layout);
#endif
}

std::map<std::string, services::HistoryItemVector>
HistoryUI::groupItemsByDomain(const services::HistoryItemVector& historyItems) {
    std::map<std::string, services::HistoryItemVector> groupedMap;
    for(auto& item : historyItems) {
        std::string domain = tools::extractDomain(item->getUrl());
        if(groupedMap.find(domain) == groupedMap.end()) {
            groupedMap.insert(std::pair<std::string, services::HistoryItemVector>(domain, {}));
        }
        groupedMap.find(domain)->second.push_back(item);
    }
    return groupedMap;
}

Evas_Object *HistoryUI::createDaysList(Evas_Object *history_layout)
{
    M_ASSERT(history_layout);

    Evas_Object* list = m_historyDaysListManager->createDaysList(
            history_layout);

    elm_object_part_content_set(history_layout, "history_list", list);

    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return list;
}

Evas_Object* HistoryUI::createActionBar(Evas_Object* history_layout)
{
    Evas_Object* actionBar = elm_layout_add(history_layout);
    elm_object_part_content_set(history_layout, "action_bar_history", actionBar);
    evas_object_size_hint_weight_set(actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(actionBar, m_edjFilePath.c_str(), "action_bar");

    m_buttonClear = elm_button_add(actionBar);
    elm_object_style_set(m_buttonClear, "history_button");
    evas_object_smart_callback_add(m_buttonClear, "clicked", HistoryUI::_clearHistory_clicked, this);
    elm_object_part_content_set(actionBar, "clearhistory_click", m_buttonClear);

    m_buttonClose = elm_button_add(actionBar);
    elm_object_style_set(m_buttonClose, "close_history_button");
    evas_object_smart_callback_add(m_buttonClose, "clicked", HistoryUI::_close_clicked_cb, this);
    elm_object_part_content_set(actionBar, "close_click", m_buttonClose);
#if !PROFILE_MOBILE
    m_focusManager->setHistoryUIButtons(m_buttonClose, m_buttonClear);
#endif
    return actionBar;
}

void HistoryUI::_close_clicked_cb(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        HistoryUI *historyUI = static_cast<HistoryUI*>(data);
        historyUI->closeHistoryUIClicked();
    }
}

void HistoryUI::_clearHistory_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) return;
    HistoryUI *historyUI = static_cast<HistoryUI*>(data);
#if PROFILE_MOBILE
    historyUI->clearItems();
    historyUI->clearHistoryClicked();
#else
    historyUI->getHistoryDeleteManager()->toggleDeleteMode();
#endif

}

void HistoryUI::addHistoryItems(std::shared_ptr<services::HistoryItemVector> items,
        HistoryPeriod period)
{
    if(items->size() == 0) return;
    auto grouped = groupItemsByDomain(*items);
    m_historyDaysListManager->addHistoryItems(grouped, period);
}

void HistoryUI::removeHistoryItem(const std::string& uri)
{
    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
}

void HistoryUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyDaysListManager->clear();
}

}
}
