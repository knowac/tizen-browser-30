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

#ifndef QUICKACCESS_H
#define QUICKACCESS_H

#include <Elementary.h>
#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include <cstdint>
#include <list>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "services/HistoryService/HistoryItemTypedef.h"
#include "QuickAccessItem.h"
#include "Tools/EflTools.h"

namespace tizen_browser{
namespace base_ui{

enum class QuickAccessState {
    Default,
    Edit,
    DeleteMostVisited,
};

class BROWSER_EXPORT QuickAccess
        : public core::AbstractService
        , public interfaces::AbstractRotatable
{
public:
    QuickAccess();
    ~QuickAccess();
    void init(Evas_Object *main_layout);
    Evas_Object* getContent();
    Evas_Object* getQuickAccessGengrid() {return m_quickAccessGengrid;}
    Evas_Object* getMostVisitedGengrid() {return m_mostVisitedGengrid;}
    void setQuickAccessState(QuickAccessState state) {m_state = state;}
    QuickAccessState getQuickAccessState() {return m_state;}
    void setMostVisitedItems(std::shared_ptr<services::HistoryItemVector> vec);
    void setQuickAccessItems(services::SharedQuickAccessItemVector vec);
    void hideUI();
    void showUI();
    virtual std::string getName();
    bool isDesktopMode() const;
    void setDesktopMode(bool mode);
    bool canBeBacked(int tabCount);
    void backButtonClicked();
    bool isMostVisitedActive();
    void orientationChanged();
    void showMostVisited();
    void showQuickAccess();
    void editQuickAccess();
    void deleteMostVisited();
    void deleteSelectedMostVisitedItems();
    void editingFinished();

    boost::signals2::signal<void (services::SharedQuickAccessItem, bool)> openURLquickaccess;
    boost::signals2::signal<void (std::shared_ptr<services::HistoryItem>, bool)> openURLhistory;
    boost::signals2::signal<void ()> getMostVisitedItems;
    boost::signals2::signal<void ()> getQuickAccessItems;
    boost::signals2::signal<void ()> addQuickAccessClicked;
    boost::signals2::signal<void ()> switchViewToWebPage;
    boost::signals2::signal<void (services::SharedQuickAccessItem)> deleteQuickAccessItem;
    boost::signals2::signal<void (std::shared_ptr<services::HistoryItem>, int)> removeMostVisitedItem;
    boost::signals2::signal<void (int)> sendSelectedMVItemsCount;

private:
    struct HistoryItemData
    {
        std::shared_ptr<services::HistoryItem> item;
        QuickAccess* quickAccess;
    };

    struct QuickAccessItemData
    {
        services::SharedQuickAccessItem item;
        QuickAccess* quickAccess;
    };

    void createItemClasses();
    void addMostVisitedItem(std::shared_ptr<services::HistoryItem>);
    void addQuickAccessItem(services::SharedQuickAccessItem);
    void clearMostVisitedGengrid();
    void clearQuickAccessGengrid();
    Evas_Object* createQuickAccessGengrid(Evas_Object *parent);
    Evas_Object* createMostVisitedGengrid(Evas_Object *parent);
    void showScrollerPage(int page);
    void setPageTitle();

    void addToQuickAccessTile();
    void setIndexPage(const void *page) const;
    bool isOrientationLandscape() const;
    static void _quickAccess_tile_realized(void * data, Evas_Object * obj, void * event_info);
    static void _layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

    void createBox(Evas_Object* parent);
    void createQuickAccessLayout(Evas_Object *parent);
    void createMostVisitedView(Evas_Object *parent);
    void createQuickAccessView(Evas_Object *parent);
    void deleteQuickAccessSelectedItem(Elm_Widget_Item *item);

    static Evas_Object * _grid_quickaccess_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_quickaccessADD_content_get(void *data, Evas_Object *obj, const char *part);
    static void _grid_quickaccess_del(void *data, Evas_Object *obj);
    static void __quckAccess_del_clicked(void *data, Evas_Object *, void *);
    static char* _grid_mostVisited_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_mostVisited_content_get(void *data, Evas_Object *obj, const char *part);
    static void _grid_mostVisited_del(void *data, Evas_Object *obj);
    static void _thumbQuickAccessClicked(void * data, Evas_Object * obj, void * event_info);
    static void _thumbMostVisitedClicked(void * data, Evas_Object * obj, void * event_info);
    static void _check_state_changed(void *data, Evas_Object *obj, void *);
    static void setButtonColor(Evas_Object* button, int r, int g, int b, int a);

    static void _addToQuickAccess_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _horizontalScroller_scroll(void* data, Evas_Object* scroller, void* event_info);

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object* m_horizontalScroller;
    Evas_Object *m_box;
    Evas_Object *m_quickAccessView;
    Evas_Object *m_mostVisitedView;
    Evas_Object *m_quickAccessGengrid;
    Evas_Object *m_mostVisitedGengrid;
    Evas_Object* m_index;
    std::vector<Evas_Object *> m_tiles;

    int m_currPage;
    Elm_Gengrid_Item_Class * m_quickAccess_item_class;
    Elm_Gengrid_Item_Class * m_mostVisited_item_class;
    Elm_Gengrid_Item_Class * m_quickAccess_tile_class;
    std::string edjFilePath;
    bool m_desktopMode;
    QuickAccessState m_state;

    Evas_Object* m_verticalScroller;
    std::list<std::shared_ptr<services::HistoryItem>> m_mv_delete_list;
    static const int MOST_VISITED_PAGE;
    static const int QUICKACCESS_PAGE;
    static const int QUICKACCESS_ITEM_WIDTH = 150;
    static const int QUICKACCESS_ITEM_HEIGHT = 204;
    static const int QUICKACCESS_ITEM_WIDTH_LANDSCAPE = 150;
    static const int QUICKACCESS_ITEM_HEIGHT_LANDSCAPE = 204;
    static const int MOSTVISITED_ITEM_WIDTH = 200;
    static const int MOSTVISITED_ITEM_HEIGHT = 208;
    static const int MOSTVISITED_ITEM_WIDTH_LANDSCAPE = 200;
    static const int MOSTVISITED_ITEM_HEIGHT_LANDSCAPE = 208;
    static const int HEADER_HEIGHT = 116+38;
    static const int DEFAULT_BUTTON_COLOR = 190;
};

}
}

#endif // QUICKACCESS_H
