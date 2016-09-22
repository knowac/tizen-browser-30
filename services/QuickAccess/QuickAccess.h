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
#if PROFILE_MOBILE
#include <cstdint>
#endif

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "DetailPopup.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT QuickAccess
        : public tizen_browser::core::AbstractService
#if PROFILE_MOBILE
        , public interfaces::AbstractRotatable
#endif
{
public:
    QuickAccess();
    ~QuickAccess();
    void init(Evas_Object *main_layout);
    Evas_Object* getContent();
    void setMostVisitedItems(std::shared_ptr<services::HistoryItemVector> vec);
    void setBookmarksItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > vec);
    void hideUI();
    void showUI();
    virtual std::string getName();
    void openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems);
    bool isDesktopMode() const;
    void setDesktopMode(bool mode);
    DetailPopup & getDetailPopup();
    bool canBeBacked(int tabCount);
    void backButtonClicked();
    inline bool isMostVisitedActive() const;
#if PROFILE_MOBILE
    void orientationChanged();
#else
    void refreshFocusChain();
#endif

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, int)> mostVisitedTileClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, bool)> openURL;
    boost::signals2::signal<void ()> getMostVisitedItems;
    boost::signals2::signal<void ()> getBookmarksItems;
    boost::signals2::signal<void ()> bookmarkManagerClicked;
    boost::signals2::signal<void ()> switchViewToWebPage;

    static const int MAX_THUMBNAIL_WIDTH;
    static const int MAX_THUMBNAIL_HEIGHT;

private:
    void createItemClasses();
    void addMostVisitedItem(std::shared_ptr<services::HistoryItem>);
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void clearMostVisitedGenlist();
    void clearBookmarkGengrid();
    Evas_Object* createBookmarkGengrid(Evas_Object *parent);
    void showMostVisited();
    void showBookmarks();
    void showScrollerPage(int page);

#if PROFILE_MOBILE
    void addBookmarkManagerTile();
    void setIndexPage(const uintptr_t page) const;
    bool isOrientationLandscape() const;
    static void _bookmark_tile_realized(void * data, Evas_Object * obj, void * event_info);
    static void _layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
#else
    Evas_Object* createTopButtons(Evas_Object *parent);
    Evas_Object* createBottomButton(Evas_Object *parent);
#endif

    void createBox(Evas_Object* parent);
    void createQuickAccessLayout(Evas_Object *parent);
    void createMostVisitedView(Evas_Object *parent);
    void createBookmarksView(Evas_Object *parent);

    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static void _grid_bookmark_del(void *data, Evas_Object *obj);
    static void _thumbBookmarkClicked(void * data, Evas_Object * obj, void * event_info);
    static void _thumbMostVisitedClicked(void * data, Evas_Object * obj, void * event_info);
    void setEmptyView(bool empty);
    void showNoMostVisitedLabel();

    static void _mostVisited_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_manager_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _horizontalScroller_scroll(void* data, Evas_Object* scroller, void* event_info);
#if !PROFILE_MOBILE
    static void _category_btn_mouse_in(void* data, Evas_Object* obj, void* event_info);
    static void _category_btn_mouse_out(void* data, Evas_Object* obj, void* event_info);
    static void _bookmark_btn_show(void* data, Evas* e, Evas_Object* obj, void* event_info);
#endif

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object* m_horizontalScroller;
    Evas_Object *m_box;
    Evas_Object *m_bookmarksView;
    Evas_Object *m_mostVisitedView;
    Evas_Object *m_bookmarksButton;
    Evas_Object *m_mostVisitedButton;
    Evas_Object *m_bookmarkGengrid;
    Evas_Object *m_bookmarkManagerButton;
    bool m_after_history_thumb;
    std::vector<Evas_Object *> m_tiles;
    Eina_List* m_parentFocusChain;

    int m_currPage;
    Elm_Gengrid_Item_Class * m_bookmark_item_class;
    DetailPopup m_detailPopup;
    std::shared_ptr<services::HistoryItemVector> m_mostVisitedItems;
    bool m_gengridSetup;
    std::string edjFilePath;
    bool m_desktopMode;

    static const int MAX_TILES_NUMBER;
    static const int BIG_TILE_INDEX;
    static const int TOP_RIGHT_TILE_INDEX;
    static const int BOTTOM_RIGHT_TILE_INDEX;
    static const std::vector<std::string> TILES_NAMES;
    static const int MOST_VISITED_PAGE = 0;
    static const int BOOKMARK_PAGE = 1;

#if PROFILE_MOBILE
    Evas_Object* m_index;
    Evas_Object* m_verticalScroller;
    Evas_Object* m_centerLayout;
    Elm_Gengrid_Item_Class * m_bookmarkManagerTileclass;
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > m_bookmarkItems;
    bool m_landscapeView;
    static const int FIXED_SIZE_TILES_NUMBER = 3;
    static const int BOOKMARK_ITEM_WIDTH = 337;
    static const int BOOKAMRK_ITEM_HEIGHT = 379;
    static const int BOOKMARK_ITEM_WIDTH_LANDSCAPE = 308;
    static const int BOOKAMRK_ITEM_HEIGHT_LANDSCAPE = 326;
    static const int HEADER_HEIGHT = 116+38;
#endif
};

}
}

#endif // QUICKACCESS_H
