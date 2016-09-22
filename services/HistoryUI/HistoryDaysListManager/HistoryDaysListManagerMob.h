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

#ifndef HISTORYDAYSLISTMANAGERMOB_H_
#define HISTORYDAYSLISTMANAGERMOB_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include "HistoryDayItemDataTypedef.h"
#include "HistoryDaysListManager.h"
#include "HistoryDaysListManagerEdje.h"

namespace tizen_browser {
namespace base_ui {

class HistoryDayItemMob;
typedef std::shared_ptr<HistoryDayItemMob> HistoryDayItemMobPtr;

class HistoryDaysListManagerMob : public HistoryDaysListManager
{
public:
    HistoryDaysListManagerMob();
    virtual ~HistoryDaysListManagerMob();

    Evas_Object* createDaysList(Evas_Object* parent) override;
    void addHistoryItems(const std::map<std::string, services::HistoryItemVector>&,
            HistoryPeriod period) override;
    void sortDayItems(std::vector<WebsiteHistoryItemDataPtr>& historyItems);
    void clear() override;
    void setFocusChain(Evas_Object* /*obj*/) override {}

    void onHistoryDayItemButtonClicked(
            const HistoryDayItemDataPtrConst clickedItem, bool remove);
    void onWebsiteHistoryItemClicked(
            const WebsiteHistoryItemDataPtrConst websiteHistoryItemData,
            bool remove);
    void onWebsiteHistoryItemVisitItemClicked(
            const WebsiteVisitItemDataPtrConst websiteVisitItemData,
            bool remove);

private:
    void connectSignals();
    void appendDayItem(HistoryDayItemDataPtr dayItemData);
    void showNoHistoryMessage(bool show);
    bool isHistoryDayListEmpty() {return m_dayItems.empty();}

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(HistoryDayItemDataPtrConst historyDayItemData);

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteHistoryItemDataPtrConst websiteHistoryItemData);

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteVisitItemDataPtrConst websiteVisitItemData);

    /**
     * @brief remove history day item from vector
     */
    void remove(HistoryDayItemMobPtr historyDayItem);

    HistoryDayItemMobPtr getItem(HistoryDayItemDataPtrConst historyDayItemData);

    HistoryDaysListManagerEdjePtr m_edjeFiles;
    std::vector<HistoryDayItemMobPtr> m_dayItems;

    Evas_Object* m_parent;
    Evas_Object* m_scrollerDays;
    Evas_Object* m_layoutScrollerDays;
    Evas_Object* m_boxDays;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* HISTORYDAYSLISTMANAGERMOB_H_ */
