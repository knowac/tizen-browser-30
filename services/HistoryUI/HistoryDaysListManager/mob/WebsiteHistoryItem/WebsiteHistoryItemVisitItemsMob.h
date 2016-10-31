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

#ifndef WEBSITEHISTORYITEMVISITITEMSMOB_H_
#define WEBSITEHISTORYITEMVISITITEMSMOB_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include "../../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemVisitItemsMob
{
    struct LayoutVisitItemObjects
    {
        Evas_Object* layout = nullptr;
        Evas_Object* buttonSelect = nullptr;
        Evas_Object* layerGesture = nullptr;
        Evas_Object* boxMain = nullptr;
        Evas_Object* layoutButtonDelete = nullptr;
        Evas_Object* buttonDelete = nullptr;
        // prevents click event, when gesture occured
        bool clickBlocked = false;
    };
    struct VisitItemObjects
    {
        WebsiteVisitItemDataPtr websiteVisitItemData;
        struct LayoutVisitItemObjects layoutVisitItemObjects;
    };
public:
    WebsiteHistoryItemVisitItemsMob(WebsiteVisitItemDataPtr websiteVisitItems);
    virtual ~WebsiteHistoryItemVisitItemsMob();
    Evas_Object* init(Evas_Object* parent, const std::string& edjeFilePath);
    /**
     * @brief invoked when main layout is already removed.
     * prevents from second evas_object_del() on main layout in destructor
     */
    void setEflObjectsAsDeleted();

    // static signals to allow easy connection in HistoryDaysListManager
    static boost::signals2::signal<void(const WebsiteVisitItemDataPtr, bool)> signalButtonClicked;

    bool contains(WebsiteVisitItemDataPtrConst websiteVisitItemData);
    void removeItem(WebsiteVisitItemDataPtrConst websiteVisitItemData);
    int getVisitItemsId();
    int size() {return eina_list_count(elm_box_children_get(m_boxMainVertical));}

private:
    LayoutVisitItemObjects createLayoutVisitItem(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    Evas_Object* createLayoutContent(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    Evas_Object* createLayoutButtonDelete(Evas_Object* parent,
            const std::string& edjeFilePath);

    void initCallbacks();
    static void _buttonSelectClicked(void* data, Evas_Object* obj,
            void* event_info);
    static void _buttonDeleteClicked(void* data, Evas_Object* obj,
            void* event_info);
    static Evas_Event_Flags _gestureOccured(void *data, void *event_info);
    static void showButtonDelete(Evas_Object* layoutButtonDelete, Evas_Object* box, bool show);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;

    VisitItemObjects m_websiteVisitItem;
    Evas_Object* m_layoutMain;
    Evas_Object* m_boxMainVertical;

    // minimum value for which gesture will be considered
    static const int GESTURE_MOMENTUM_MIN;
};

}
}

#endif /* WEBSITEHISTORYITEMVISITITEMSMOB_H_ */
