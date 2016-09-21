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

#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include <cstdint>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "services/HistoryService/HistoryItemTypedef.h"
#include "BookmarkItem.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT QuickAccess
        : public tizen_browser::core::AbstractService
        , public interfaces::AbstractRotatable
{
public:
    QuickAccess();
    ~QuickAccess();
    void init(Evas_Object *main_layout);
    Evas_Object* getContent();
    void setMostVisitedItems(std::shared_ptr<services::HistoryItemVector> vec);
    void setQuickAccessItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > vec);
    void hideUI();
    void showUI();
    virtual std::string getName();
    bool isDesktopMode() const;
    void setDesktopMode(bool mode);
    bool canBeBacked(int tabCount);
    void backButtonClicked();
    inline bool isMostVisitedActive() const;
    void orientationChanged();
    void showMostVisited();
    void showQuickAccess();
    void editQuickAccess();

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, bool)> openURL;
    boost::signals2::signal<void ()> getMostVisitedItems;
    boost::signals2::signal<void ()> getQuickAccessItems;
    boost::signals2::signal<void ()> addQuickAccessClicked;
    boost::signals2::signal<void ()> switchViewToWebPage;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> deleteQuickAccessItem;

private:
    void createItemClasses();
    void addMostVisitedItem(std::shared_ptr<services::HistoryItem>);
    void addQuickAccessItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void clearMostVisitedGengrid();
    void clearQuickAccessGengrid();
    Evas_Object* createQuickAccessGengrid(Evas_Object *parent);
    Evas_Object* createMostVisitedGengrid(Evas_Object *parent);
    void showScrollerPage(int page);

    void addToQuickAccessTile();
    void setIndexPage(const uintptr_t page) const;
    bool isOrientationLandscape() const;
    static void _quickAccess_tile_realized(void * data, Evas_Object * obj, void * event_info);
    static void _layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

    void createBox(Evas_Object* parent);
    void createQuickAccessLayout(Evas_Object *parent);
    void createMostVisitedView(Evas_Object *parent);
    void createQuickAccessView(Evas_Object *parent);

    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static void _grid_bookmark_del(void *data, Evas_Object *obj);
    static void __quckAccess_del_clicked(void *data, Evas_Object *, void *);
    static char* _grid_mostVisited_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_mostVisited_content_get(void *data, Evas_Object *obj, const char *part);
    static void _grid_mostVisited_del(void *data, Evas_Object *obj);
    static void _thumbQuickAccessClicked(void * data, Evas_Object * obj, void * event_info);
    static void _thumbMostVisitedClicked(void * data, Evas_Object * obj, void * event_info);
    void setEmptyView(bool empty);
    void showNoMostVisitedLabel();

    static void _mostVisited_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _quickAccess_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _addToQuickAccess_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _horizontalScroller_scroll(void* data, Evas_Object* scroller, void* event_info);

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object* m_horizontalScroller;
    Evas_Object *m_box;
    Evas_Object *m_quickAccessView;
    Evas_Object *m_mostVisitedView;
    Evas_Object *m_mostVisitedButton;
    Evas_Object *m_quickAccessGengrid;
    Evas_Object *m_mostVisitedGengrid;
    bool m_after_history_thumb;
    std::vector<Evas_Object *> m_tiles;
    Eina_List* m_parentFocusChain;

    int m_currPage;
    Elm_Gengrid_Item_Class * m_quickAccess_item_class;
    Elm_Gengrid_Item_Class * m_mostVisited_item_class;
    std::shared_ptr<services::HistoryItemVector> m_mostVisitedItems;
    bool m_gengridSetup;
    std::string edjFilePath;
    bool m_desktopMode;

    Evas_Object* m_index;
    Evas_Object* m_verticalScroller;
    Elm_Gengrid_Item_Class * m_quickAccess_tile_class;
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > m_QuickAccessItems;
    bool m_landscapeView;
    static const int MOST_VISITED_PAGE = 1;
    static const int QUICKACCESS_PAGE = 0;
    static const int BOOKMARK_ITEM_WIDTH = 150;
    static const int BOOKAMRK_ITEM_HEIGHT = 196;
    static const int BOOKMARK_ITEM_WIDTH_LANDSCAPE = 150;
    static const int BOOKAMRK_ITEM_HEIGHT_LANDSCAPE = 196;
    static const int MOSTVISITED_ITEM_WIDTH = 200;
    static const int MOSTVISITED_ITEM_HEIGHT = 208;
    static const int MOSTVISITED_ITEM_WIDTH_LANDSCAPE = 200;
    static const int MOSTVISITED_ITEM_HEIGHT_LANDSCAPE = 208;
    static const int HEADER_HEIGHT = 116+38;
};

}
}

#endif // QUICKACCESS_H
