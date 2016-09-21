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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "app_i18n.h"
#include "QuickAccess.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"
#include "Tools/GeneralTools.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(QuickAccess, "org.tizen.browser.quickaccess")

typedef struct _HistoryItemData
{
        std::shared_ptr<tizen_browser::services::HistoryItem> item;
        QuickAccess* quickAccess;
} HistoryItemData;

typedef struct _BookmarkItemData
{
        std::shared_ptr<tizen_browser::services::BookmarkItem> item;
        QuickAccess* quickAccess;
} BookmarkItemData;

QuickAccess::QuickAccess()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_horizontalScroller(nullptr)
    , m_box(nullptr)
    , m_quickAccessView(nullptr)
    , m_mostVisitedView(nullptr)
    , m_mostVisitedButton(nullptr)
    , m_quickAccessGengrid(nullptr)
    , m_after_history_thumb(false)
    , m_parentFocusChain(nullptr)
    , m_currPage(QuickAccess::QUICKACCESS_PAGE)
    , m_quickAccess_item_class(nullptr)
    , m_mostVisited_item_class(nullptr)
    , m_index(nullptr)
    , m_verticalScroller(nullptr)
    , m_quickAccess_tile_class(nullptr)
    , m_landscapeView(isOrientationLandscape())
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("QuickAccess/QuickAccess.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    createItemClasses();
}

QuickAccess::~QuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    clearQuickAccessGengrid();
    clearMostVisitedGengrid();
    elm_gengrid_item_class_free(m_quickAccess_item_class);
    elm_gengrid_item_class_free(m_mostVisited_item_class);
    elm_gengrid_item_class_free(m_quickAccess_tile_class);
    eina_list_free(m_parentFocusChain);
}

void QuickAccess::init(Evas_Object* parent)
{
    M_ASSERT(parent);
    m_parent = parent;
}


Evas_Object* QuickAccess::getContent()
{
    M_ASSERT(m_parent);
    if (!m_layout) {
        createQuickAccessLayout(m_parent);
    }
    return m_layout;
}

void QuickAccess::createItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_quickAccess_item_class) {
        m_quickAccess_item_class = elm_gengrid_item_class_new();
        m_quickAccess_item_class->item_style = "quickAccess";
        m_quickAccess_item_class->func.text_get = _grid_bookmark_text_get;
        m_quickAccess_item_class->func.content_get =  _grid_bookmark_content_get;
        m_quickAccess_item_class->func.state_get = nullptr;
        m_quickAccess_item_class->func.del = _grid_bookmark_del;
    }
    if (!m_mostVisited_item_class) {
        m_mostVisited_item_class = elm_gengrid_item_class_new();
        m_mostVisited_item_class->item_style = "mostVisited";
        m_mostVisited_item_class->func.text_get = _grid_mostVisited_text_get;
        m_mostVisited_item_class->func.content_get = _grid_mostVisited_content_get;
        m_mostVisited_item_class->func.state_get = nullptr;
        m_mostVisited_item_class->func.del = _grid_mostVisited_del;
    }
    if (!m_quickAccess_tile_class) {
        m_quickAccess_tile_class = elm_gengrid_item_class_new();
        m_quickAccess_tile_class->item_style = "quickAccessAdd";
        m_quickAccess_tile_class->func.text_get = nullptr;
        m_quickAccess_tile_class->func.content_get = nullptr;
        m_quickAccess_tile_class->func.state_get = nullptr;
        m_quickAccess_tile_class->func.del = nullptr;
    }
}


void QuickAccess::createQuickAccessLayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_desktopMode = false;  //TODO: delete this

    if (m_layout)
        evas_object_del(m_layout);

    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "main_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);

    evas_object_event_callback_add(m_layout, EVAS_CALLBACK_RESIZE, _layout_resize_cb, this);

    m_index = elm_index_add(m_layout);
    evas_object_size_hint_weight_set(m_index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_index, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_style_set(m_index, "pagecontrol");
    elm_index_horizontal_set(m_index, EINA_TRUE);
    elm_index_autohide_disabled_set(m_index, EINA_TRUE);
    elm_object_part_content_set(m_layout, "index", m_index);

    elm_index_item_append(m_index, "1", NULL, (void *) QuickAccess::QUICKACCESS_PAGE);
    elm_index_item_append(m_index, "2", NULL, (void *) QuickAccess::MOST_VISITED_PAGE);
    elm_index_level_go(m_index, 0);

    m_horizontalScroller = elm_scroller_add(m_layout);
    elm_scroller_page_scroll_limit_set(m_horizontalScroller, 1, 0);
    elm_scroller_movement_block_set(m_horizontalScroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
    elm_scroller_loop_set(m_horizontalScroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_page_relative_set(m_horizontalScroller, 1.0, 0.0);
    elm_scroller_policy_set(m_horizontalScroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(m_horizontalScroller, EINA_FALSE, EINA_FALSE);
    elm_object_part_content_set(m_layout, "view", m_horizontalScroller);
    evas_object_smart_callback_add(m_horizontalScroller, "scroll", _horizontalScroller_scroll, this);

    createBox(m_horizontalScroller);
}

void QuickAccess::createBox(Evas_Object* parent)
{
    if (m_box)
        elm_box_clear(m_box);
    m_box = elm_box_add(parent);
    elm_box_horizontal_set(m_box, EINA_TRUE);
    elm_object_content_set(parent, m_box);
    evas_object_show(m_box);

    createQuickAccessView(m_box);
    elm_box_pack_end(m_box, m_quickAccessView);
    createMostVisitedView(m_box);
    elm_box_pack_end(m_box, m_mostVisitedView);
}

void QuickAccess::createMostVisitedView(Evas_Object * parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_mostVisitedView)
        evas_object_del(m_mostVisitedView);

    m_mostVisitedView = elm_layout_add(parent);
    elm_layout_theme_set(m_mostVisitedView, "layout", "application", "default");
    evas_object_size_hint_weight_set(m_mostVisitedView, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_mostVisitedView, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_mostVisitedView);

    evas_object_event_callback_add(m_mostVisitedView, EVAS_CALLBACK_RESIZE, _layout_resize_cb, this);

    m_mostVisitedGengrid = createMostVisitedGengrid(m_mostVisitedView);
    evas_object_smart_callback_add(m_mostVisitedGengrid, "realized", _quickAccess_tile_realized, this);  //TODO
    elm_object_part_content_set(m_mostVisitedView, "elm.swallow.content", m_mostVisitedGengrid);

    evas_object_show(m_mostVisitedGengrid);
}

void QuickAccess::createQuickAccessView (Evas_Object * parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_quickAccessView)
        evas_object_del(m_quickAccessView);

    m_quickAccessView = elm_layout_add(parent);
    elm_layout_theme_set(m_quickAccessView, "layout", "application", "default");
    evas_object_size_hint_weight_set(m_quickAccessView, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_quickAccessView, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_quickAccessView);

    m_quickAccessGengrid = createQuickAccessGengrid(m_quickAccessView);
    evas_object_smart_callback_add(m_quickAccessGengrid, "realized", _quickAccess_tile_realized, this);
    elm_object_part_content_set(m_quickAccessView, "elm.swallow.content", m_quickAccessGengrid);

    evas_object_show(m_quickAccessGengrid);
}

Evas_Object* QuickAccess::createQuickAccessGengrid(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *quickAccessGengrid = elm_gengrid_add(parent);

    elm_gengrid_select_mode_set(quickAccessGengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(quickAccessGengrid, EINA_TRUE);
    elm_scroller_policy_set(quickAccessGengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    evas_object_size_hint_weight_set(quickAccessGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(quickAccessGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_gengrid_align_set(quickAccessGengrid, 0.5, 0.1);
    if (isOrientationLandscape()) {
        elm_gengrid_item_size_set(quickAccessGengrid, Z3_SCALE_SIZE(BOOKMARK_ITEM_WIDTH_LANDSCAPE), Z3_SCALE_SIZE(BOOKAMRK_ITEM_HEIGHT_LANDSCAPE));
    } else {
        elm_gengrid_item_size_set(quickAccessGengrid, Z3_SCALE_SIZE(BOOKMARK_ITEM_WIDTH), Z3_SCALE_SIZE(BOOKAMRK_ITEM_HEIGHT));
    }
    elm_scroller_bounce_set(quickAccessGengrid, EINA_FALSE, EINA_FALSE);

    return quickAccessGengrid;
}

Evas_Object *QuickAccess::createMostVisitedGengrid(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *mostVisitedGengrid = elm_gengrid_add(parent);
    evas_object_size_hint_weight_set(mostVisitedGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mostVisitedGengrid, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
    elm_gengrid_align_set(mostVisitedGengrid, 0.5, 0.1);
    if (isOrientationLandscape()) {
        elm_gengrid_item_size_set(mostVisitedGengrid, Z3_SCALE_SIZE(MOSTVISITED_ITEM_WIDTH_LANDSCAPE), Z3_SCALE_SIZE(MOSTVISITED_ITEM_HEIGHT_LANDSCAPE));
    } else {
        elm_gengrid_item_size_set(mostVisitedGengrid, Z3_SCALE_SIZE(MOSTVISITED_ITEM_WIDTH), Z3_SCALE_SIZE(MOSTVISITED_ITEM_HEIGHT));
    }
    elm_scroller_bounce_set(mostVisitedGengrid, EINA_FALSE, EINA_FALSE);

    return mostVisitedGengrid;
}

void QuickAccess::_mostVisited_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = reinterpret_cast<QuickAccess *>(data);
        self->showMostVisited();
        elm_scroller_page_show(self->m_horizontalScroller, MOST_VISITED_PAGE, 0);
    }
}

void QuickAccess::_quickAccess_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = reinterpret_cast<QuickAccess *>(data);
        self->showQuickAccess();
        elm_scroller_page_show(self->m_horizontalScroller, QUICKACCESS_PAGE, 0);
    }
}

void QuickAccess::_addToQuickAccess_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        QuickAccess*  quickAccess = static_cast<QuickAccess *>(data);
        quickAccess->addQuickAccessClicked();
    }
}

void QuickAccess::_horizontalScroller_scroll(void* data, Evas_Object* /*scroller*/, void* /*event_info*/)
{
    if (data) {
        auto self = static_cast<QuickAccess*>(data);
        int page_no;

        elm_scroller_current_page_get(self->m_horizontalScroller, &page_no, NULL);
        if (self->m_currPage != page_no) {
            self->showScrollerPage(page_no);
        }
    }
}

void QuickAccess::addMostVisitedItem(std::shared_ptr<services::HistoryItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->quickAccess = this;

    elm_gengrid_item_append(m_mostVisitedGengrid, m_mostVisited_item_class, itemData, _thumbMostVisitedClicked, itemData);
}

void QuickAccess::setMostVisitedItems(std::shared_ptr<services::HistoryItemVector> items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    clearMostVisitedGengrid();
    m_mostVisitedItems = items;


    for (auto it = items->begin(); it != items->end(); ++it)
         addMostVisitedItem(*it);
    items->clear();
}

void QuickAccess::addQuickAccessItem(std::shared_ptr<tizen_browser::services::BookmarkItem> bi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (bi->is_folder())
        return;
    BookmarkItemData *itemData = new BookmarkItemData();        // deleted in _grid_bookmark_del
    itemData->item = bi;
    itemData->quickAccess = this;
    elm_gengrid_item_append(m_quickAccessGengrid, m_quickAccess_item_class, itemData, _thumbQuickAccessClicked, itemData);
}

void QuickAccess::clearMostVisitedGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_clear(m_mostVisitedGengrid);
}

void QuickAccess::setQuickAccessItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    clearQuickAccessGengrid();
    m_QuickAccessItems = items;

    for (auto it = items.begin(); it != items.end(); ++it)
         addQuickAccessItem(*it);

    items.clear();
    addToQuickAccessTile();
}

void QuickAccess::addToQuickAccessTile()
{
    elm_gengrid_item_append(m_quickAccessGengrid, m_quickAccess_tile_class, this, _addToQuickAccess_clicked, this);
}

void QuickAccess::setIndexPage(const uintptr_t page) const
{
    Elm_Object_Item* it = elm_index_item_find(m_index, (void *)page);
    if (it != NULL) {
        elm_index_item_selected_set(it, EINA_TRUE);
    }
}

bool QuickAccess::isOrientationLandscape() const
{
    auto landscape = isLandscape();
    if (landscape) {
        return *landscape;
    } else {
        BROWSER_LOGD("[%s:%d] Warning: orientation check signal failed!", __PRETTY_FUNCTION__, __LINE__);
        return false;
    }
}

void QuickAccess::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_landscapeView = isOrientationLandscape();
    createBox(m_horizontalScroller);
    getQuickAccessItems();
    getMostVisitedItems();
}

void QuickAccess::_quickAccess_tile_realized(void* data, Evas_Object*, void* event_info)
{
    if (data) {
        auto self = static_cast<QuickAccess*>(data);
        auto tile = static_cast<Elm_Object_Item*>(event_info);
        if (self->isOrientationLandscape())
            elm_object_item_signal_emit(tile, "set,landscape", "ui");

        int pageHorizontal = 0;
        int pageVertical = 0;
        elm_scroller_current_page_get(self->m_horizontalScroller, &pageHorizontal, &pageVertical);
        if (pageHorizontal != self->m_currPage) {
            if (self->isMostVisitedActive())
                _mostVisited_clicked(data, nullptr, nullptr);
            else
                _quickAccess_clicked(data, nullptr, nullptr);
        }
    }
}

void QuickAccess::_layout_resize_cb(void* data, Evas* /*e*/, Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (data) {
        auto self = static_cast<QuickAccess*>(data);
        int w, h;
        evas_object_geometry_get(self->m_layout, NULL, NULL, &w, &h);
        evas_object_size_hint_min_set(self->m_mostVisitedView, w, h-Z3_SCALE_SIZE(HEADER_HEIGHT));
        evas_object_size_hint_min_set(self->m_quickAccessGengrid, w, h-Z3_SCALE_SIZE(HEADER_HEIGHT));
    }
}


char* QuickAccess::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
    if (data) {
        BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);
        if (!strcmp(part, "elm.text"))
                return strdup(itemData->item->getTitle().c_str());
    }
    return strdup("");
}

Evas_Object * QuickAccess::_grid_bookmark_content_get(void *data, Evas_Object* obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
    if (data) {
        BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);

        if (!strcmp(part, "elm.swallow.icon")) {
            if (itemData->item->getThumbnail()) {
                // Favicon
                Evas_Object * thumb = itemData->item->getFavicon()->getEvasImage(obj);
                elm_image_resizable_set(thumb, EINA_TRUE, EINA_TRUE);
                evas_object_size_hint_min_set(thumb, ELM_SCALE_SIZE(114), ELM_SCALE_SIZE(114));
                evas_object_size_hint_max_set(thumb, ELM_SCALE_SIZE(114), ELM_SCALE_SIZE(114));
                return thumb;
            } else {
                // Default color
                Evas_Object *textblock = evas_object_textblock_add(obj);
                Evas_Textblock_Style *st = evas_textblock_style_new();
                evas_textblock_style_set(st, "DEFAULT='font=Sans font_size=45 color=#555 align=center valign=center'");
                evas_object_textblock_style_set(textblock, st);
                evas_object_textblock_valign_set(textblock, 0.5);
                evas_textblock_style_free(st);
                const char *fName = itemData->item->getTitle().substr(0, 1).c_str();
                evas_object_textblock_text_markup_set(textblock, fName);


                Evas_Object *button = elm_button_add(obj);
                evas_object_color_set(button, 190, 190, 190, 255);
                elm_object_content_set(button, textblock);

                return button;

            }
        }
        if (!strcmp(part, "elm.button")) {
            auto button = elm_button_add(obj);
            elm_object_style_set(button, "delete_button");
            evas_object_smart_callback_add(button, "clicked", __quckAccess_del_clicked, data);
            return button;
        }
    }
    return nullptr;
}

void QuickAccess::_grid_bookmark_del(void* data, Evas_Object*)
{
    auto itemData = static_cast<BookmarkItemData*>(data);
    if (itemData)
        delete itemData;
}

void QuickAccess::__quckAccess_del_clicked(void *data, Evas_Object */*obj*/, void *)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__);
    auto itemData = static_cast<BookmarkItemData*>(data);
    itemData->quickAccess->deleteQuickAccessItem(itemData->item);
}

char *QuickAccess::_grid_mostVisited_text_get(void *data, Evas_Object *, const char *part)
{
    HistoryItemData *itemData = reinterpret_cast<HistoryItemData*>(data);
    if (!strcmp(part, "elm.text")) {
            return strdup(itemData->item->getTitle().c_str());
    }
    return strdup("");
}

Evas_Object *QuickAccess::_grid_mostVisited_content_get(void *data, Evas_Object *, const char *part)
{
    HistoryItemData *itemData = reinterpret_cast<HistoryItemData*>(data);
    if (!strcmp(part, "elm.swallow.icon")) {
        if (itemData->item->getThumbnail()) {
                Evas_Object * thumb = itemData->item->getThumbnail()->getEvasImage(itemData->quickAccess->m_parent);
                return thumb;
        }
        else {
                return nullptr;
        }
    }
    return nullptr;
}

void QuickAccess::_grid_mostVisited_del(void *data, Evas_Object *)
{
    if (data) {
        auto itemData = static_cast<HistoryItemData*>(data);
        if (itemData)
            delete itemData;
    }
}

void QuickAccess::_thumbQuickAccessClicked(void * data, Evas_Object * , void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->quickAccess->openURL(itemData->item, itemData->quickAccess->isDesktopMode());
    itemData->quickAccess->m_after_history_thumb = false;
}

void QuickAccess::_thumbMostVisitedClicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->quickAccess->openURL(itemData->item, false);

}

void QuickAccess::showMostVisited()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_currPage = QuickAccess::MOST_VISITED_PAGE;

    elm_object_translatable_part_text_set(m_layout, "screen_title", "Most visited websites");  //TODO: translate
    setIndexPage(QuickAccess::MOST_VISITED_PAGE);

}

void QuickAccess::clearQuickAccessGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_clear(m_quickAccessGengrid);
}

void QuickAccess::showQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currPage = QuickAccess::QUICKACCESS_PAGE;

    elm_object_translatable_part_text_set(m_layout, "screen_title", "Quick access");  //TODO: translate
    setIndexPage(QuickAccess::QUICKACCESS_PAGE);

}

void QuickAccess::editQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_gengrid_reorder_mode_set(m_quickAccessGengrid, EINA_TRUE);

    //TODO: show delete buttons in corners

}

void QuickAccess::showScrollerPage(int page)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (page) {
        case MOST_VISITED_PAGE:
            showMostVisited();
            break;
        case QUICKACCESS_PAGE:
            showQuickAccess();
            break;
        default:
            BROWSER_LOGD("[%s:%d] Warning: unknown page value!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void QuickAccess::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bool currentViewLandscape = isOrientationLandscape();
    if (currentViewLandscape != m_landscapeView) {
        m_landscapeView = currentViewLandscape;
        createBox(m_horizontalScroller);
    }

    evas_object_show(m_layout);
    getMostVisitedItems();
    getQuickAccessItems();
    showScrollerPage(m_currPage);
}

void QuickAccess::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_layout);
    clearMostVisitedGengrid();
    clearQuickAccessGengrid();
}

void QuickAccess::showNoMostVisitedLabel()
{
    elm_object_translatable_part_text_set(m_mostVisitedView, "elm.text.empty", "IDS_BR_BODY_NO_VISITED_SITES");
    elm_layout_signal_emit(m_mostVisitedView, "empty,view", "quickaccess");
}

void QuickAccess::setEmptyView(bool empty)
{
    BROWSER_LOGD("[%s:%d], empty: %d", __PRETTY_FUNCTION__, __LINE__, empty);
    if(empty) {
        showNoMostVisitedLabel();
    } else {
        if (isOrientationLandscape())
            elm_layout_signal_emit(m_mostVisitedView, "set,landscape", "quickaccess");
        else
            elm_layout_signal_emit(m_mostVisitedView, "not,empty,view", "quickaccess");
    }
}

bool QuickAccess::isDesktopMode() const
{
    return m_desktopMode;
}

void QuickAccess::setDesktopMode(bool mode)
{
    m_desktopMode = mode;
}

bool QuickAccess::canBeBacked(int tabCount)
{
    return (tabCount != 0);
}

void QuickAccess::backButtonClicked()
{
    hideUI();
    switchViewToWebPage();
}

bool QuickAccess::isMostVisitedActive() const
{
    return m_currPage == MOST_VISITED_PAGE;
}

}
}
