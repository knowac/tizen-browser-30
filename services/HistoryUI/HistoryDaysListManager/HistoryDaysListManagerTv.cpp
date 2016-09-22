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

#include <services/HistoryUI/HistoryUI.h>
#include <services/HistoryService/HistoryItem.h>
#include "app_i18n.h"
#include "BrowserLogger.h"
#include "HistoryDaysListManagerTv.h"
#include "HistoryDayItemData.h"
#include "tv/HistoryDayItemTv.h"
#include "tv/WebsiteHistoryItem/WebsiteHistoryItemTv.h"
#include "tv/WebsiteHistoryItem/WebsiteHistoryItemTitleTv.h"
#include "tv/WebsiteHistoryItem/WebsiteHistoryItemVisitItemsTv.h"
#include <services/HistoryUI/HistoryDeleteManager.h>

#include <GeneralTools.h>
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

HistoryDaysListManagerTv::HistoryDaysListManagerTv(HistoryDeleteManagerPtrConst deleteManager)
    : m_edjeFiles(std::make_shared<HistoryDaysListManagerEdje>())
    , m_scrollerDaysColumns(nullptr)
    , m_layoutScrollerDaysColumns(nullptr)
    , m_boxDaysColumns(nullptr)
    , m_historyDeleteManager(deleteManager)
{
    connectSignals();
}

HistoryDaysListManagerTv::~HistoryDaysListManagerTv()
{
    for (auto& dayItem : m_dayItems)
        dayItem->setEflObjectsAsDeleted();
}

Evas_Object* HistoryDaysListManagerTv::createDaysList(Evas_Object* parent)
{
    m_scrollerDaysColumns = elm_scroller_add(parent);
    elm_scroller_bounce_set(m_scrollerDaysColumns, EINA_FALSE, EINA_FALSE);
    tools::EflTools::setExpandHints(m_scrollerDaysColumns);
    elm_scroller_policy_set(m_scrollerDaysColumns, ELM_SCROLLER_POLICY_OFF,
            ELM_SCROLLER_POLICY_OFF);
    elm_scroller_movement_block_set(m_scrollerDaysColumns,
            ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);

    m_layoutScrollerDaysColumns = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutScrollerDaysColumns);
    elm_layout_file_set(m_layoutScrollerDaysColumns,
            m_edjeFiles->historyDaysList.c_str(), "historyDaysList");

    elm_object_content_set(m_scrollerDaysColumns, m_layoutScrollerDaysColumns);

    m_boxDaysColumns = elm_box_add(parent);
    tools::EflTools::setExpandHints(m_boxDaysColumns);
    elm_box_horizontal_set(m_boxDaysColumns, EINA_TRUE);
    elm_object_part_content_set(m_layoutScrollerDaysColumns, "daysColumns",
            m_boxDaysColumns);
    elm_box_padding_set(m_boxDaysColumns, 65, 0.0);

    return m_scrollerDaysColumns;
}

void HistoryDaysListManagerTv::connectSignals()
{
    HistoryDayItemTv::signaButtonClicked.connect(
            boost::bind(&HistoryDaysListManagerTv::onHistoryDayItemClicked,
                    this, _1));
    WebsiteHistoryItemTitleTv::signalButtonClicked.connect(
            boost::bind(&HistoryDaysListManagerTv::onWebsiteHistoryItemClicked,
                    this, _1));
    WebsiteHistoryItemVisitItemsTv::signalButtonClicked.connect(
            boost::bind(
                    &HistoryDaysListManagerTv::onWebsiteHistoryItemVisitItemClicked,
                    this, _1));
}

void HistoryDaysListManagerTv::addHistoryItems(
        const std::map<std::string, services::HistoryItemVector>& items,
        HistoryPeriod period)
{
    std::vector<WebsiteHistoryItemDataPtr> historyItems;
    for (auto& itemPair : items) {
        std::vector<WebsiteVisitItemDataPtr> pageViewItems;
        std::shared_ptr<tools::BrowserImage> websiteFavicon = nullptr;
        for (auto& hi : itemPair.second) {
            pageViewItems.push_back(
                    std::make_shared<WebsiteVisitItemData>(hi));
            if (!websiteFavicon && hi->getFavIcon()->getSize() > 0)
                websiteFavicon = hi->getFavIcon();
        }
        historyItems.push_back(
                std::make_shared<WebsiteHistoryItemData>
        (_("IDS_BR_BODY_TITLE"), itemPair.first, websiteFavicon, pageViewItems));
    }
    HistoryDayItemDataPtr dayItem = std::make_shared < HistoryDayItemData
            > (toString(period), historyItems);
    appendDayItem(dayItem);
}

int HistoryDaysListManagerTv::getHistoryItemIndex(const HistoryDayItemTv* item)
{
    int index = 0;
    for (auto& dayItem : m_dayItems) {
        if (dayItem.get() == item)
            return index;
        index++;
    }
    return -1;
}

HistoryDayItemTvPtr HistoryDaysListManagerTv::getItem(
        HistoryDayItemDataPtrConst historyDayItemData)
{
    for (auto& historyDayItem : m_dayItems)
        if (historyDayItem->getData() == historyDayItemData)
            return historyDayItem;
    BROWSER_LOGE("%s no item", __PRETTY_FUNCTION__);
    return nullptr;
}

void HistoryDaysListManagerTv::clear()
{
    // clear days items main layouts
    elm_box_clear(m_boxDaysColumns);
    m_dayItems.clear();
}

void HistoryDaysListManagerTv::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    auto item = std::make_shared<HistoryDayItemTv>(dayItemData, m_historyDeleteManager);
    m_dayItems.push_back(item);
    elm_box_pack_end(m_boxDaysColumns, item->init(m_boxDaysColumns, m_edjeFiles));
}

void HistoryDaysListManagerTv::setFocusChain(Evas_Object* obj)
{
    for (auto& dayItem : m_dayItems)
        dayItem->setFocusChain(obj);
}

void HistoryDaysListManagerTv::onHistoryDayItemFocused(
        const HistoryDayItemTv* focusedItem)
{
    scrollToDayItem(focusedItem);
}

void HistoryDaysListManagerTv::onHistoryDayItemClicked(
        const HistoryDayItemDataPtrConst clickedItem)
{
    if (m_historyDeleteManager->getDeleteMode())
        removeItem(clickedItem);
}

void HistoryDaysListManagerTv::onWebsiteHistoryItemClicked(
        const WebsiteHistoryItemDataPtrConst clickedItem)
{
    if (m_historyDeleteManager->getDeleteMode())
        removeItem(clickedItem);
    else
        signalHistoryItemClicked(
                tools::PROTOCOL_DEFAULT + clickedItem->websiteDomain,
                clickedItem->websiteTitle);
}

void HistoryDaysListManagerTv::removeItem(
        HistoryDayItemDataPtrConst historyDayItemData)
{
    auto item = getItem(historyDayItemData);
    if (!item)
        return;
    signalDeleteHistoryItems(item->getVisitItemsIds());
    // remove day item from vector, destructor will clear efl objects
    remove(item);
    elm_box_unpack(m_boxDaysColumns, item->getLayoutMain());
}

void HistoryDaysListManagerTv::removeItem(
        WebsiteHistoryItemDataPtrConst websiteHistoryItemData)
{
    if (!websiteHistoryItemData) {
        BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
        return;
    }
    for (auto& dayItem : m_dayItems) {
        auto websiteHistoryItem = dayItem->getItem(websiteHistoryItemData);
        if (websiteHistoryItem) {
            signalDeleteHistoryItems(websiteHistoryItem->getVisitItemsIds());
            dayItem->removeItem(websiteHistoryItemData);
            return;
        }
    }
    BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
}

void HistoryDaysListManagerTv::removeItem(
        WebsiteVisitItemDataPtrConst websiteVisitItemData)
{
    if (!websiteVisitItemData) {
        BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
        return;
    }
    for (auto& dayItem : m_dayItems) {
        if (dayItem->getItem(websiteVisitItemData)) {
            dayItem->removeItem(websiteVisitItemData);
            return;
        }
    }
    BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
}

void HistoryDaysListManagerTv::onWebsiteHistoryItemVisitItemClicked(
        const WebsiteVisitItemDataPtrConst clickedItem)
{
    if (m_historyDeleteManager->getDeleteMode()) {
        removeItem(clickedItem);
        signalDeleteHistoryItems(
                std::make_shared<std::vector<int>>(std::initializer_list<int> {
                        clickedItem->historyItem->getId() }));
    } else {
        signalHistoryItemClicked(clickedItem->historyItem->getUrl(),
                clickedItem->historyItem->getTitle());
    }
}

void HistoryDaysListManagerTv::remove(HistoryDayItemTvPtr historyDayItem)
{
    for (auto it = m_dayItems.begin(); it != m_dayItems.end();) {
        if ((*it) == historyDayItem) {
            m_dayItems.erase(it);
            return;
        } else
            ++it;
    }
}

void HistoryDaysListManagerTv::scrollToDayItem(const HistoryDayItemTv* item)
{
    int itemX, itemY, itemW, itemH;
    itemX = itemY = itemW = itemH = 0;
    evas_object_geometry_get(item->getLayoutMain(), &itemX, &itemY, &itemW, &itemH);
    int index = getHistoryItemIndex(item);
    elm_scroller_region_show(m_scrollerDaysColumns, index*itemW, 1, 2*itemW, 1);
}

}
}
