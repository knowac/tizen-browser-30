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

#ifndef WEBSITEHISTORYITEMMOB_H_
#define WEBSITEHISTORYITEMMOB_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include <memory>
#include "../../HistoryDayItemDataTypedef.h"
#include "../../HistoryDaysListManagerEdje.h"

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemTitleMob;
class WebsiteHistoryItemVisitItemsMob;

using WebsiteHistoryItemTitleMobPtr = std::shared_ptr<WebsiteHistoryItemTitleMob>;
using WebsiteHistoryItemVisitItemsMobPtr = std::shared_ptr<WebsiteHistoryItemVisitItemsMob>;

class WebsiteHistoryItemMob
{
public:
    WebsiteHistoryItemMob(WebsiteHistoryItemDataPtr websiteHistoryItemData);
    virtual ~WebsiteHistoryItemMob();
    Evas_Object* init(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);

    Evas_Object* getLayoutMain() {return m_layoutMain;}
    WebsiteHistoryItemDataPtrConst getData() const {return m_websiteHistoryItemData;}
    void removeItem(WebsiteVisitItemDataPtrConst historyVisitItemData);
    bool contains(WebsiteVisitItemDataPtrConst websiteVisitItemData);

    int getVisitItemsId();

    /**
     * @brief invoked when main layout is already removed.
     * prevents from second evas_object_del() on main layout in destructor
     */
    void setEflObjectsAsDeleted();
    int sizeHistoryVisitItems();

private:
    Evas_Object* createBoxMainVertical(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted = false;

    WebsiteHistoryItemDataPtr m_websiteHistoryItemData;
    WebsiteHistoryItemTitleMobPtr m_websiteHistoryItemTitle;
    WebsiteHistoryItemVisitItemsMobPtr m_websiteHistoryItemVisitItems;

    Evas_Object* m_layoutMain;
    Evas_Object* m_boxMainVertical;
};

}
}

#endif /* WEBSITEHISTORYITEMMOB_H_ */
