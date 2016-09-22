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

#ifndef HistoryDayItemTv_H_
#define HistoryDayItemTv_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include "../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

#include "../HistoryDaysListManagerEdje.h"

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemTv;
typedef std::shared_ptr<WebsiteHistoryItemTv> WebsiteHistoryItemTvPtr;
class HistoryDeleteManager;
typedef std::shared_ptr<const HistoryDeleteManager> HistoryDeleteManagerPtrConst;

class HistoryDayItemTv {
public:
    HistoryDayItemTv(HistoryDayItemDataPtr dayItemData,
            HistoryDeleteManagerPtrConst historyDeleteManager);
    virtual ~HistoryDayItemTv();
    Evas_Object* init(Evas_Object* parent, HistoryDaysListManagerEdjePtr edjeFiles);
    void setFocusChain(Evas_Object* obj);
    Evas_Object* getLayoutMain() const {return m_layoutMain;}
    Evas_Object* getLayoutHeader() const {return m_layoutHeader;}

    std::shared_ptr<std::vector<int>> getVisitItemsIds();

    WebsiteHistoryItemTvPtr getItem(
            WebsiteHistoryItemDataPtrConst historyDayItemData);
    WebsiteHistoryItemTvPtr getItem(
            WebsiteVisitItemDataPtrConst historyVisitItemData);

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteHistoryItemDataPtrConst websiteHistoryItemData);

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

    HistoryDeleteManagerPtrConst getDeleteManager() const {return m_historyDeleteManager;}

    // static signals to allow easy connection in HistoryDaysListManager
    static boost::signals2::signal<void(const HistoryDayItemDataPtr)> signaButtonClicked;

private:
    void initBoxWebsites(HistoryDaysListManagerEdjePtr edjeFiles);
    Evas_Object* createScrollerWebsites(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);
    Evas_Object* createImageClear(Evas_Object* parent,
            const std::string& edjeFilePath);
    void initCallbacks();
    void deleteCallbacks();
    static void _buttonSelectClicked(void *data, Evas_Object *obj, void *event_info);
    static void _buttonSelectFocused(void *data, Evas_Object *obj, void *event_info);
    static void _buttonSelectUnfocused(void *data, Evas_Object *obj, void *event_info);

    /**
     * @brief remove website history item from vector
     */
    void remove(WebsiteHistoryItemTvPtr websiteHistoryItem);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;

    HistoryDayItemDataPtr m_dayItemData;
    std::vector<WebsiteHistoryItemTvPtr> m_websiteHistoryItems;

    // main layout: all children widgets will have this as their parent
    Evas_Object* m_layoutMain;
    Evas_Object* m_buttonSelect;
    Evas_Object* m_imageClear;

    // vertical box: day label + websites history scroller
    Evas_Object* m_boxMainVertical;

    Evas_Object* m_layoutHeader;
    Evas_Object* m_boxHeader;

    // box containing scroller
    Evas_Object* m_layoutBoxScrollerWebsites;
    Evas_Object* m_boxScrollerWebsites;
    Evas_Object* m_scrollerWebsites;
    Evas_Object* m_layoutScrollerWebsites;
    Evas_Object* m_boxWebsites;

    HistoryDeleteManagerPtrConst m_historyDeleteManager;
};

}
}

#endif /* HistoryDayItemTv_H_ */
