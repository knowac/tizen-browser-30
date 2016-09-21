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

#include "HistoryDayItemMob.h"
#include "WebsiteHistoryItem/WebsiteHistoryItemMob.h"
#include "../HistoryDayItemData.h"

#include "EflTools.h"

namespace tizen_browser {
namespace base_ui {

boost::signals2::signal<void(const HistoryDayItemDataPtr, bool)> HistoryDayItemMob::signaButtonClicked;

HistoryDayItemMob::HistoryDayItemMob(HistoryDayItemDataPtr dayItemData)
    : m_eflObjectsDeleted(false)
    , m_dayItemData(dayItemData)
    , m_layoutMain(nullptr)
    , m_boxMainVertical(nullptr)
    , m_layoutHeader(nullptr)
    , m_boxHeader(nullptr)
{
    for (auto& websiteHistoryItemData : dayItemData->websiteHistoryItems) {
        auto websiteHistoryItem =
                std::make_shared<WebsiteHistoryItemMob>(websiteHistoryItemData);
        m_websiteHistoryItems.push_back(websiteHistoryItem);
    }
}

HistoryDayItemMob::~HistoryDayItemMob()
{
    // clear all widgets aded by this class
    if (!m_eflObjectsDeleted)
        evas_object_del(m_layoutMain);
}

Evas_Object* HistoryDayItemMob::init(Evas_Object* parent,
        HistoryDaysListManagerEdjePtr edjeFiles)
{
    m_layoutMain = elm_layout_add(parent);
    evas_object_size_hint_align_set(m_layoutMain, EVAS_HINT_FILL, 0.0);

    elm_layout_file_set(m_layoutMain, edjeFiles->historyDaysList.c_str(), "layoutDayItem");

    // add all children widgets with m_layoutMain as parent
    m_boxMainVertical = elm_box_add(m_layoutMain);
    elm_box_horizontal_set(m_boxMainVertical, EINA_FALSE);
    elm_object_part_content_set(m_layoutMain, "boxMainVertical", m_boxMainVertical);

    m_layoutHeader = elm_layout_add(m_layoutMain);
    evas_object_size_hint_align_set(m_layoutHeader, 0.0, 0.0);
    elm_layout_file_set(m_layoutHeader, edjeFiles->historyDaysList.c_str(), "layoutHeader");
    elm_object_text_set(m_layoutHeader, m_dayItemData->day.c_str());

    m_layoutBoxWebsites = elm_layout_add(m_layoutMain);
    tools::EflTools::setExpandHints(m_layoutBoxWebsites);
    elm_layout_file_set(m_layoutBoxWebsites, edjeFiles->historyDaysList.c_str(), "layoutBoxWebsites");
    m_boxWebsites = createBoxWebsites(m_layoutMain, edjeFiles);
    elm_box_padding_set(m_boxWebsites, 0.0, 18.0);
    elm_object_part_content_set(m_layoutBoxWebsites, "boxWebsites", m_boxWebsites);

    elm_box_pack_end(m_boxMainVertical, m_layoutHeader);
    elm_box_pack_end(m_boxMainVertical, m_layoutBoxWebsites);

    evas_object_show(m_layoutHeader);
    evas_object_show(m_layoutBoxWebsites);
    evas_object_show(m_boxWebsites);

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutMain);

    return m_layoutMain;
}

void HistoryDayItemMob::setEflObjectsAsDeleted()
{
    m_eflObjectsDeleted = true;
    for (auto& websiteHistoryItem : m_websiteHistoryItems)
        websiteHistoryItem->setEflObjectsAsDeleted();
}

Evas_Object* HistoryDayItemMob::createBoxWebsites(Evas_Object* parent,
        HistoryDaysListManagerEdjePtr edjeFiles)
{
    Evas_Object* boxWebsites = elm_box_add(parent);
    tools::EflTools::setExpandHints(boxWebsites);
    elm_box_horizontal_set(boxWebsites, EINA_FALSE);

    for (auto& websiteHistoryItem : m_websiteHistoryItems) {
        Evas_Object* boxSingleWebsite = websiteHistoryItem->init(m_layoutMain,
                edjeFiles);
        elm_box_pack_end(boxWebsites, boxSingleWebsite);
    }
    return boxWebsites;
}

WebsiteHistoryItemMobPtr HistoryDayItemMob::getItem(
        WebsiteHistoryItemDataPtrConst websiteHistoryItemData)
{
    for (auto& websiteHistoryItem : m_websiteHistoryItems) {
        if (websiteHistoryItem->getData() == websiteHistoryItemData)
            return websiteHistoryItem;
    }
    return nullptr;
}

void HistoryDayItemMob::removeItem(
        WebsiteHistoryItemDataPtrConst websiteHistoryItemData)
{
    auto item = getItem(websiteHistoryItemData);
    if (!item)
        return;
    elm_box_unpack(m_boxWebsites, item->getLayoutMain());
    remove(item);
    if (m_websiteHistoryItems.size() == 0)
        signaButtonClicked(m_dayItemData, true);
}

WebsiteHistoryItemMobPtr HistoryDayItemMob::getItem(
        WebsiteVisitItemDataPtrConst websiteVisitItemData)
{
    for (auto& websiteHistoryItem : m_websiteHistoryItems) {
        if (websiteHistoryItem->contains(websiteVisitItemData))
            return websiteHistoryItem;
    }
    return nullptr;
}

void HistoryDayItemMob::removeItem(
        WebsiteVisitItemDataPtrConst historyVisitItemData)
{
    auto item = getItem(historyVisitItemData);
    item->removeItem(historyVisitItemData);
    if (item->sizeHistoryVisitItems() == 0) {
        removeItem(item->getData());
    }
}

void HistoryDayItemMob::remove(WebsiteHistoryItemMobPtr websiteHistoryItem)
{
    for (auto it = m_websiteHistoryItems.begin(); it != m_websiteHistoryItems.end();)
        if ((*it) == websiteHistoryItem) {
            m_websiteHistoryItems.erase(it);
            return;
        } else
            ++it;
}

}
}
