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

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "TabIdTypedef.h"
#if PROFILE_MOBILE
#include "AbstractRotatable.h"
#endif

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT TabUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
#if PROFILE_MOBILE
        , public tizen_browser::interfaces::AbstractRotatable
#endif
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

    void addTabItems(std::vector<basic_webengine::TabContentPtr>& items);
    bool isEditMode();
    void onBackKey();
#if PROFILE_MOBILE
    virtual void orientationChanged() override;
#endif

    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> tabClicked;
    boost::signals2::signal<void ()> newTabClicked;
    boost::signals2::signal<void ()> newIncognitoTabClicked;
    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> closeTabsClicked;
    boost::signals2::signal<void (const std::string & )> openedTabsClicked;
#if ON_OTHER_DEVICES
    boost::signals2::signal<void (const std::string & )> onOtherDevicesClicked;
#endif
    boost::signals2::signal<void ()> closeTabUIClicked;
    boost::signals2::signal<int () > tabsCount;
    boost::signals2::signal<bool (const tizen_browser::basic_webengine::TabId& )> isIncognito;

private:

    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _tab_grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _del_item(void* /*data*/, Evas_Object* obj);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _item_deleted(void *data, Evas_Object *obj);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _deleteBookmark(void *data, Evas_Object *obj, void *event_info);
    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    void setEmptyGengrid(bool setEmpty);

    static void _openedtabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newtab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newincognitotab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _closetabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _close_tab_clicked(void *data, Evas_Object*, void*);
#if ON_OTHER_DEVICES
    static void _onotherdevices_clicked(void * data, Evas_Object * obj, void * event_info);
#endif
    static void _focus_in(void * data, Evas*, Evas_Object * obj, void * event_info);
    static Eina_Bool _ready(void *data);

    void createTabUILayout();
    Evas_Object* createActionBar(Evas_Object* parent);
    Evas_Object* createGengrid(Evas_Object* parent);
    Evas_Object* createTopButtons(Evas_Object* parent);
    Evas_Object* createBottomBar(Evas_Object* parent);
    Evas_Object* createNoHistoryLabel();
    void createTabItemClass();
    void closeAllTabs();
    void addTabItem(basic_webengine::TabContentPtr);

    Evas_Object* m_button_left;
    Evas_Object* m_button_right;
    Evas_Object *m_tab_layout;
    Evas_Object* m_gengrid_layout;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Elm_Object_Item* m_itemToShow;
    Ecore_Timer* m_timer;
    bool editMode;
    bool onOtherDevicesSwitch;

    Elm_Gengrid_Item_Class * m_item_class;
    std::string m_edjFilePath;

#if PROFILE_MOBILE
    const unsigned int GENGRID_ITEM_WIDTH = 674;
    const unsigned int GENGRID_ITEM_HEIGHT = 468;
    const unsigned int GENGRID_ITEM_WIDTH_LANDSCAPE = 308;
    const unsigned int GENGRID_ITEM_HEIGHT_LANDSCAPE = 326;
#else
    const unsigned int GENGRID_ITEM_WIDTH = 364;
    const unsigned int GENGRID_ITEM_HEIGHT = 320;
#endif
};
}
}

#endif // BOOKMARKSUI_H
