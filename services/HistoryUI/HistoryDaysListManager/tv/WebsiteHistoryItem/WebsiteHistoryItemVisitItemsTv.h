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

#ifndef WEBSITEHISTORYITEMVISITITEMSTV_H_
#define WEBSITEHISTORYITEMVISITITEMSTV_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include "../../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser {
namespace base_ui {

class HistoryDeleteManager;
typedef std::shared_ptr<const HistoryDeleteManager> HistoryDeleteManagerPtrConst;

class WebsiteHistoryItemVisitItemsTv
{
    struct LayoutVisitItemObjects
    {
        Evas_Object* layout = nullptr;
        Evas_Object* buttonSelect = nullptr;
        Evas_Object* imageClear = nullptr;
    };
    struct VisitItemObjects
    {
        WebsiteVisitItemDataPtr websiteVisitItemData;
        LayoutVisitItemObjects layoutVisitItemObjects;
        HistoryDeleteManagerPtrConst deleteManager;
    };
public:
    WebsiteHistoryItemVisitItemsTv(
            const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems,
            HistoryDeleteManagerPtrConst historyDeleteManager);
    virtual ~WebsiteHistoryItemVisitItemsTv();
    Evas_Object* init(Evas_Object* parent, const std::string& edjeFilePath);
    void setFocusChain(Evas_Object* obj);

    // static signals to allow easy connection in HistoryDaysListManager
    static boost::signals2::signal<void(const WebsiteVisitItemDataPtr)>
    signalButtonClicked;

    bool contains(WebsiteVisitItemDataPtrConst websiteVisitItemData);
    void removeItem(WebsiteVisitItemDataPtrConst websiteVisitItemData);
    std::shared_ptr<std::vector<int>> getVisitItemsIds();

    void setEflObjectsAsDeleted();
    int size() {return eina_list_count(elm_box_children_get(m_boxMainVertical));}

private:
    LayoutVisitItemObjects createLayoutVisitItem(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    Evas_Object* createImageClear(Evas_Object* parent,
            const std::string& edjeFilePath);
    Evas_Object* createLayoutVisitItemDate(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    Evas_Object* createLayoutVisitItemUrl(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);

    void remove(VisitItemObjects);
    void initCallbacks();
    static void _buttonSelectClicked(void* data, Evas_Object* obj, void* event_info);
    static void _buttonSelectFocused(void* data, Evas_Object* obj, void* event_info);
    static void _buttonSelectUnfocused(void* data, Evas_Object* obj, void* event_info);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;
    std::vector<VisitItemObjects> m_websiteVisitItems;
    Evas_Object* m_layoutMain;
    Evas_Object* m_boxMainVertical;

    HistoryDeleteManagerPtrConst m_historyDeleteManager;
};

}
}

#endif /* WEBSITEHISTORYITEMVISITITEMSTV_H_ */
