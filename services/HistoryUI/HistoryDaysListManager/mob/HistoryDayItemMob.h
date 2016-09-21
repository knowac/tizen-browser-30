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

#ifndef HistoryDayItemMob_H_
#define HistoryDayItemMob_H_

#include <memory>
#include <Elementary.h>
#include <vector>
#include "../HistoryDayItemDataTypedef.h"
#include "../HistoryDaysListManagerEdje.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemMob;
using WebsiteHistoryItemMobPtr = std::shared_ptr<WebsiteHistoryItemMob>;

class HistoryDayItemMob
{
public:
    HistoryDayItemMob(HistoryDayItemDataPtr dayItemData);
    virtual ~HistoryDayItemMob();
    Evas_Object* init(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);
    Evas_Object* getLayoutMain() {return m_layoutMain;}

    WebsiteHistoryItemMobPtr getItem(
            WebsiteHistoryItemDataPtrConst historyDayItemData);
    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteHistoryItemDataPtrConst websiteHistoryItemData);
    WebsiteHistoryItemMobPtr getItem(
            WebsiteVisitItemDataPtrConst historyVisitItemData);

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteVisitItemDataPtrConst historyVisitItemData);

    /**
     * @brief invoked when main layout is already removed.
     * prevents from second evas_object_del() on main layout in destructor
     */
    void setEflObjectsAsDeleted();
    HistoryDayItemDataPtrConst getData() const {return m_dayItemData;}

    // static signals to allow easy connection in HistoryDaysListManager
    static boost::signals2::signal<void(const HistoryDayItemDataPtr, bool)> signaButtonClicked;

private:
    Evas_Object* createBoxWebsites(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);

    /**
     * @brief remove website history item from vector
     */
    void remove(WebsiteHistoryItemMobPtr websiteHistoryItem);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;

    HistoryDayItemDataPtr m_dayItemData;
    std::vector<WebsiteHistoryItemMobPtr> m_websiteHistoryItems;

    Evas_Object* m_layoutMain;
    // vertical box: day label + websites history scroller
    Evas_Object* m_boxMainVertical;

    Evas_Object* m_layoutHeader;
    Evas_Object* m_boxHeader;

    Evas_Object* m_layoutBoxWebsites;
    Evas_Object* m_boxWebsites;
};

}
}

#endif /* BROWSER_MERGING_SERVICES_HISTORYUI_HISTORYDAYSLISTMANAGER_MOB_HISTORYDAYITEMMOB_H_ */
