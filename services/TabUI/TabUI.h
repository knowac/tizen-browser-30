/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef TABUI_H
#define TABUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractContextMenu.h"
#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "AbstractWebEngine/State.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "TabIdTypedef.h"
#include "AbstractRotatable.h"
#include "NaviframeWrapper.h"
#include "PasswordUI.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT TabUI
        : public tizen_browser::interfaces::AbstractContextMenu
        , public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
        , public tizen_browser::interfaces::AbstractRotatable
{
public:
    TabUI();
    ~TabUI();
    //AbstractUIComponent interface implementation
    void showUI();
    void hideUI();
    void init(Evas_Object *parent);
    Evas_Object* getContent();

    virtual std::string getName();

    void addTabItems(std::vector<basic_webengine::TabContentPtr>& items, bool secret);
    virtual void orientationChanged() override;

    //AbstractContextMenu interface implementation
    virtual void showContextMenu() override;
    std::shared_ptr<PasswordUI> getPasswordUI() { return m_passwordUI; };
    void switchToSecretFirstTime();

    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> tabClicked;
    boost::signals2::signal<void ()> newTabClicked;
    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> closeTabsClicked;
    boost::signals2::signal<void ()> closeTabUIClicked;
    boost::signals2::signal<void ()> changeEngineState;
    boost::signals2::signal<void ()> refetchTabUIData;
    boost::signals2::signal<bool (std::string)> checkIfParamExistsInDB;
    boost::signals2::signal<void (std::string, bool)> setDBBoolParamValue;
    boost::signals2::signal<void (std::string, std::string)> setDBStringParamValue;
    boost::signals2::signal<bool (std::string)> getDBBoolParamValue;
    boost::signals2::signal<std::string (std::string)> getDBStringParamValue;
    boost::signals2::signal<void ()> showPasswordUI;
    boost::signals2::signal<void ()> showNoPasswordWarning;

private:
    struct TabData
    {
        basic_webengine::TabContentPtr item;
        TabUI* tabUI;
    };

    enum class State {
        NORMAL,
        SECRET,
        PASSWORD_DECISION
    };

    static char* _gengrid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _gengrid_content_get(void *data, Evas_Object *obj, const char *part);

    static void _gengrid_tab_pressed(void * data, Evas_Object * obj, void * event_info);
    static void _gengrid_tab_released(void * data, Evas_Object * obj, void * event_info);
    static void _gengrid_tab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _gengrid_tab_realized(void * data, Evas_Object * obj, void * event_info);
    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    void updateNoTabsText(bool forceShow=false);

    static void _openedtabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _right_button_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _left_button_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _close_tab_clicked(void *data, Evas_Object*, void*);
    static void _cm_secret_clicked(void*, Evas_Object*, void*);
    static void _cm_close_clicked(void*, Evas_Object*, void*);
    static Evas_Event_Flags _gesture_occured(void * data, void * event_info);

    SharedNaviframeWrapper m_naviframe;
    void createTabUILayout();
    void createTopContent();
    void createBottomContent();
    void createEmptyLayout();
    void createGengrid();
    void createTabItemClass();
    void addTabItem(basic_webengine::TabContentPtr);
    void setStateButtons();

    Evas_Object *m_parent;
    Evas_Object *m_content;
    Evas_Object *m_gengrid;
    Evas_Object *m_empty_layout;

    Elm_Object_Item* m_itemToShow;
    Elm_Object_Item* m_last_pressed_gengrid_item;

    Elm_Gengrid_Item_Class * m_item_class;
    std::string m_edjFilePath;

    State m_state;
    std::shared_ptr<PasswordUI> m_passwordUI;

    const unsigned int GENGRID_ITEM_WIDTH = 700;
    const unsigned int GENGRID_ITEM_HEIGHT = 312;
    const unsigned int GENGRID_ITEM_WIDTH_LANDSCAPE = 636;
    const unsigned int GENGRID_ITEM_HEIGHT_LANDSCAPE = 274;
    const unsigned int GESTURE_MOMENTUM_MIN = 4000;
    static const int SWIPE_MOMENTUM_TRESHOLD = 400;
    static const int SWIPE_MOVE_TRESHOLD = 140;
    static constexpr double SWIPE_LEFT_ANGLE1 = 260.0;
    static constexpr double SWIPE_LEFT_ANGLE2 = 280.0;
    static constexpr double SWIPE_RIGHT_ANGLE1 = 80.0;
    static constexpr double SWIPE_RIGHT_ANGLE2 = 100.0;
};
}
}

#endif // BOOKMARKSUI_H
