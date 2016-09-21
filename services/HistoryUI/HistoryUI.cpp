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
#include <boost/format.hpp>
#include <vector>
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>

#include "app_i18n.h"
#include "HistoryUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "Tools/GeneralTools.h"
#include "HistoryDaysListManager/HistoryDaysListManagerMob.h"
#include "services/HistoryService/HistoryItem.h"
#include "HistoryDeleteManager.h"

namespace tizen_browser{
namespace base_ui{

// TODO History needs solid refactoring. A lot of features are not used in any place.

using namespace services;
EXPORT_SERVICE(HistoryUI, "org.tizen.browser.historyui")

HistoryUI::HistoryUI()
    : m_parent(nullptr)
    , m_main_layout(nullptr)
    , m_buttonClose(nullptr)
    , m_buttonClear(nullptr)
    , m_daysList(nullptr)
    , m_historyDaysListManager(nullptr)
    , m_naviframe(nullptr)
    , m_modulesToolbar(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("HistoryUI/History.edj");

    if (!m_historyDaysListManager)
        m_historyDaysListManager = std::make_shared<HistoryDaysListManagerMob>();

    m_historyDaysListManager->signalHistoryItemClicked.connect(signalHistoryItemClicked);
    m_historyDaysListManager->signalDeleteHistoryItems.connect(signalDeleteHistoryItems);
    m_historyDaysListManager->setRightButtonEnabledForHistory.connect(
        boost::bind(&HistoryUI::setRightButtonEnabled, this, _1));
    m_historyDaysListManager->setSelectedItemsCount.connect([this](auto count){
        m_naviframe->setTitle((boost::format(_("IDS_BR_HEADER_PD_SELECTED_ABB")) % count).str());
    });
}

HistoryUI::~HistoryUI()
{
}

void HistoryUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_main_layout);
    m_naviframe->show();
    evas_object_show(m_main_layout);
}

void HistoryUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_main_layout);
    evas_object_hide(m_main_layout);
    clearItems();
    m_naviframe->hide();
}

void HistoryUI::setRightButtonEnabled(bool enable)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, enable);
    m_naviframe->setRightButtonEnabled(enable);
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
    createHistoryUILayout();
    return m_naviframe->getLayout();
}

void HistoryUI::createHistoryUILayout()
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    if (!m_naviframe)
        m_naviframe = std::make_shared<NaviframeWrapper>(m_parent);

    m_main_layout = elm_layout_add(m_naviframe->getLayout());
    m_naviframe->setContent(m_main_layout);

    elm_layout_file_set(m_main_layout, m_edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_daysList = createDaysList(m_main_layout);
    clearItems();
    createTopContent();

    elm_object_signal_emit(m_naviframe->getLayout(), "show_toolbars", "ui");
}

void HistoryUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());

    m_naviframe->addPrevButton(_close_clicked_cb, this);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle(_("IDS_BR_TAB2_HISTORY"));
}

Evas_Object* HistoryUI::createDaysList(Evas_Object* parent, bool isRemoveMode)
{
    M_ASSERT(history_layout);

    auto list = m_historyDaysListManager->createDaysList(parent, isRemoveMode);

    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return list;
}

void HistoryUI::removeSelectedHistoryItems()
{
    if (!m_historyDaysListManager) {
        BROWSER_LOGD("[%s:%d] No selected elements to delete");
        return;
    }
    if (m_historyDaysListManager->isSelectAllChecked())
        clearHistoryClicked();
    m_historyDaysListManager->removeSelectedItems();
}

void HistoryUI::_close_clicked_cb(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
        return;
    }
    auto self = static_cast<HistoryUI*>(data);
    self->closeHistoryUIClicked();
}

void HistoryUI::addHistoryItems(
    std::shared_ptr<HistoryItemVector> items,
    HistoryPeriod period)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (items->size() == 0)
        return;
    m_historyDaysListManager->addHistoryItems(items, period);
}

void HistoryUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyDaysListManager->clear();
}

}
}
