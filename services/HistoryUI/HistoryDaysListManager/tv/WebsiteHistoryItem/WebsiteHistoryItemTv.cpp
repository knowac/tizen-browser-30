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

#include "WebsiteHistoryItemTv.h"
#include "WebsiteHistoryItemTitleTv.h"
#include "WebsiteHistoryItemVisitItemsTv.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

WebsiteHistoryItemTv::WebsiteHistoryItemTv(
        WebsiteHistoryItemDataPtr websiteHistoryItemData,
        HistoryDeleteManagerPtrConst historyDeleteManager)
    : m_eflObjectsDeleted(false)
    , m_websiteHistoryItemData(websiteHistoryItemData)
    , m_websiteHistoryItemTitle(
            std::make_shared<WebsiteHistoryItemTitleTv>(websiteHistoryItemData,
                    historyDeleteManager))
    , m_websiteHistoryItemVisitItems(
            std::make_shared<WebsiteHistoryItemVisitItemsTv>(
                    websiteHistoryItemData->websiteVisitItems,
                    historyDeleteManager))
    , m_layoutMain(nullptr)
    , m_boxMainHorizontal(nullptr)
    , m_historyDeleteManager(historyDeleteManager)
{
}

WebsiteHistoryItemTv::~WebsiteHistoryItemTv()
{
    // clear all widgets aded by this class
    if (!m_eflObjectsDeleted)
        evas_object_del(m_layoutMain);
}

Evas_Object* WebsiteHistoryItemTv::init(Evas_Object* parent,
        HistoryDaysListManagerEdjePtr edjeFiles)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain,
            edjeFiles->websiteHistoryItem.c_str(), "layoutWebsiteHistoryItem");

    m_boxMainHorizontal = createBoxMainHorizontal(m_layoutMain, edjeFiles);
    elm_object_part_content_set(m_layoutMain, "boxMainHorizontal",
            m_boxMainHorizontal);

    evas_object_show(m_boxMainHorizontal);
    evas_object_show(m_layoutMain);

    return m_layoutMain;
}

void WebsiteHistoryItemTv::setFocusChain(Evas_Object* obj)
{
    m_websiteHistoryItemTitle->setFocusChain(obj);
}

Evas_Object* WebsiteHistoryItemTv::createBoxMainHorizontal(Evas_Object* parent,
        HistoryDaysListManagerEdjePtr edjeFiles)
{
    Evas_Object* box = elm_box_add(parent);
    elm_box_horizontal_set(box, EINA_TRUE);

    elm_box_pack_end(box, m_websiteHistoryItemTitle->init(parent,
            edjeFiles->websiteHistoryItemTitle.c_str()));
    elm_box_pack_end(box, m_websiteHistoryItemVisitItems->init(parent,
            edjeFiles->websiteHistoryItemVisitItems.c_str()));

    elm_box_align_set(box, 0.0, 0.0);

    return box;
}

bool WebsiteHistoryItemTv::contains(
        WebsiteVisitItemDataPtrConst historyVisitItemData)
{
    return m_websiteHistoryItemVisitItems->contains(historyVisitItemData);
}

std::shared_ptr<std::vector<int>> WebsiteHistoryItemTv::getVisitItemsIds()
{
    return m_websiteHistoryItemVisitItems->getVisitItemsIds();
}

void WebsiteHistoryItemTv::setEflObjectsAsDeleted()
{
    m_eflObjectsDeleted = true;
    m_websiteHistoryItemVisitItems->setEflObjectsAsDeleted();
}

int WebsiteHistoryItemTv::sizeHistoryVisitItems() {
    return m_websiteHistoryItemVisitItems->size();
}

void WebsiteHistoryItemTv::removeItem(
        WebsiteVisitItemDataPtrConst historyVisitItemData)
{
    m_websiteHistoryItemVisitItems->removeItem(historyVisitItemData);
}

}
}
