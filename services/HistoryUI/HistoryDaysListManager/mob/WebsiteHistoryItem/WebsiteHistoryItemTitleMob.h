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

#ifndef WEBSITEHISTORYITEMTITLEMOB_H_
#define WEBSITEHISTORYITEMTITLEMOB_H_

#include <Elementary.h>
#include <string>
#include "../../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemTitleMob
{
public:
    WebsiteHistoryItemTitleMob(
            WebsiteHistoryItemDataPtr websiteHistoryItemData);
    virtual ~WebsiteHistoryItemTitleMob();
    Evas_Object* init(Evas_Object* parent, const std::string& edjeFilePath);

    WebsiteHistoryItemDataPtr getWebsiteHistoryItemDataPtr()
    {
        return m_websiteHistoryItemData;
    }

    // static signals to allow easy connection in HistoryDaysListManager
    static boost::signals2::signal<void(const WebsiteHistoryItemDataPtr, bool)> signalButtonClicked;

    void showButtonDelete(bool show);
    void setClickBlock(bool blocked) {clickBlocked = blocked;}
    bool getClickBlock() {return clickBlocked;}
    // prevents click event, when gesture occured
    bool clickBlocked = false;

private:
    Evas_Object* createLayoutContent(Evas_Object* parent,
            const std::string& edjeFilePath);
    Evas_Object* createLayoutButtonDelete(Evas_Object* parent,
            const std::string& edjeFilePath);
    Evas_Object* createLayoutIcon(Evas_Object* parent,
            const std::string& edjeFilePath);
    Evas_Object* createLayoutSummary(Evas_Object* parent,
            const std::string& edjeFilePath);
    void initCallbacks();

    static void _title_mouse_down(void* data, Evas*, Evas_Object* obj, void* event_info);
    static void _title_mouse_up(void* data, Evas*, Evas_Object* obj, void* event_info);
    static void _buttonSelectClicked(void* data, Evas_Object* obj,
            void* event_info);
    static void _buttonDeleteClicked(void* data, Evas_Object* obj,
            void* event_info);
    static Evas_Event_Flags _gestureOccured(void *data, void *event_info);

    WebsiteHistoryItemDataPtr m_websiteHistoryItemData;

    Evas_Object* m_buttonSelect;
    Evas_Object* m_buttonDelete;
    Evas_Object* m_imageFavIcon;
    Evas_Object* m_imageFavIconMask;

    Evas_Object* m_layoutMain;
    Evas_Object* m_layerGesture;
    Evas_Object* m_boxMain;
    Evas_Object* m_layoutContent;
    Evas_Object* m_layoutButtonDelete;
    Evas_Object* m_boxContentHorizontal;

    // minimum value for which gesture will be considered
    static const int GESTURE_MOMENTUM_MIN;
};

}
}

#endif /* WEBSITEHISTORYITEMTITLEMOB_H_ */
