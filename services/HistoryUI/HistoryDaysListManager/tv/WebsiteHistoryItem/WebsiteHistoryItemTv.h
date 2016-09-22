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

#ifndef WEBSITEHISTORYITEMTV_H_
#define WEBSITEHISTORYITEMTV_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include <memory>
#include "../../HistoryDayItemDataTypedef.h"
#include "../../HistoryDaysListManagerEdje.h"

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemTitleTv;
typedef std::shared_ptr<WebsiteHistoryItemTitleTv> WebsiteHistoryItemTitleTvPtr;
class WebsiteHistoryItemVisitItemsTv;
typedef std::shared_ptr<WebsiteHistoryItemVisitItemsTv> WebsiteHistoryItemVisitItemsTvPtr;
class HistoryDeleteManager;
typedef std::shared_ptr<const HistoryDeleteManager> HistoryDeleteManagerPtrConst;

class WebsiteHistoryItemTv
{
public:
    WebsiteHistoryItemTv(WebsiteHistoryItemDataPtr websiteHistoryItemData,
            HistoryDeleteManagerPtrConst historyDeleteManager);
    virtual ~WebsiteHistoryItemTv();
    Evas_Object* init(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);
    void setFocusChain(Evas_Object* obj);
    Evas_Object* getLayoutMain() {return m_layoutMain;}
    WebsiteHistoryItemDataPtrConst getData() const {return m_websiteHistoryItemData;}
    WebsiteVisitItemDataPtrConst getItem(WebsiteVisitItemDataPtrConst historyVisitItemData);
    void removeItem(WebsiteVisitItemDataPtrConst historyVisitItemData);
    bool contains(WebsiteVisitItemDataPtrConst websiteVisitItemData);

    std::shared_ptr<std::vector<int>> getVisitItemsIds();

    void setEflObjectsAsDeleted();
    int sizeHistoryVisitItems();

private:
    Evas_Object* createBoxMainHorizontal(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;

    WebsiteHistoryItemDataPtr m_websiteHistoryItemData;
    WebsiteHistoryItemTitleTvPtr m_websiteHistoryItemTitle;
    WebsiteHistoryItemVisitItemsTvPtr m_websiteHistoryItemVisitItems;

    Evas_Object* m_layoutMain;
    Evas_Object* m_boxMainHorizontal;

    HistoryDeleteManagerPtrConst m_historyDeleteManager;
};

}
}

#endif /* WEBSITEHISTORYITEMTV_H_ */
