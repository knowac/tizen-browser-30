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

/*
 * BookmarkDetailsUI.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#include <Elementary.h>
#include <boost/format.hpp>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>
#include "app_i18n.h"
#include "BookmarkDetailsUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

namespace tizen_browser {
namespace base_ui {

EXPORT_SERVICE(BookmarkDetailsUI, "org.tizen.browser.bookmarkdetailsui")

struct ItemData
{
    tizen_browser::base_ui::BookmarkDetailsUI * m_bookmarkDetails;
    tizen_browser::services::BookmarkItem * h_item;
    Elm_Object_Item * e_item;
};

typedef struct
{
    std::shared_ptr<tizen_browser::services::BookmarkItem> item;
    BookmarkDetailsUI* bookmarkDetailsUI;
} BookmarkItemData;

BookmarkDetailsUI::BookmarkDetailsUI()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_top_content(nullptr)
    , m_gengrid(nullptr)
#if !PROFILE_MOBILE
    , m_bottom_content(nullptr)
#else
    , m_more_button(nullptr)
    , m_menu_bg_button(nullptr)
    , m_menu(nullptr)
    , m_edit_button(nullptr)
    , m_delete_button(nullptr)
    , m_remove_button(nullptr)
    , m_cancel_top_button(nullptr)
    , m_remove_top_button(nullptr)
    , m_delete_count(0)
    , m_remove_bookmark_mode(false)
#endif
    , m_close_button(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_rotation_state(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkDetailsUI/BookmarkDetailsUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    createGengridItemClasses();
}

BookmarkDetailsUI::~BookmarkDetailsUI()
{
    elm_gengrid_clear(m_gengrid);

    evas_object_smart_callback_del(m_close_button, "clicked", _close_button_clicked);
#if PROFILE_MOBILE
    evas_object_smart_callback_del(m_more_button, "clicked", _more_button_clicked);
    evas_object_smart_callback_del(m_menu_bg_button, "clicked", _menu_bg_button_clicked);
    evas_object_smart_callback_del(m_edit_button, "clicked", _edit_button_clicked);
    evas_object_smart_callback_del(m_delete_button, "clicked", _delete_button_clicked);
    evas_object_smart_callback_del(m_remove_button, "clicked", _remove_button_clicked);
    evas_object_smart_callback_del(m_cancel_top_button, "clicked", _cancel_top_button_clicked);
    evas_object_smart_callback_del(m_remove_top_button, "clicked", _remove_top_button_clicked);
#endif

    evas_object_del(m_top_content);
    evas_object_del(m_close_button);
    evas_object_del(m_layout);
    evas_object_del(m_gengrid);
#if PROFILE_MOBILE
    evas_object_del(m_more_button);
    evas_object_del(m_menu_bg_button);
    evas_object_del(m_menu);
    evas_object_del(m_edit_button);
    evas_object_del(m_delete_button);
    evas_object_del(m_remove_button);
    evas_object_del(m_cancel_top_button);
    evas_object_del(m_remove_top_button);
#endif
    if (m_bookmark_item_class)
        elm_gengrid_item_class_free(m_bookmark_item_class);
}

void BookmarkDetailsUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_bookmark_item_class = elm_gengrid_item_class_new();
    m_bookmark_item_class->item_style = "grid_bookmark_item";
    m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
    m_bookmark_item_class->func.content_get = _grid_bookmark_content_get;
    m_bookmark_item_class->func.state_get = nullptr;
    m_bookmark_item_class->func.del = _grid_content_delete;
}

void BookmarkDetailsUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkDetailsUI::showUI()
{
#if !PROFILE_MOBILE
    m_focusManager.startFocusManager(m_gengrid);
#endif
    elm_object_signal_emit(m_layout, "hide_menu", "ui");
#if PROFILE_MOBILE
    evas_object_hide(m_menu);
    evas_object_hide(elm_object_part_content_get(m_layout, "more_swallow"));
    evas_object_hide(m_menu_bg_button);
    evas_object_hide(elm_object_part_content_get(m_layout, "more_bg"));
#endif
    evas_object_show(m_layout);
}

void BookmarkDetailsUI::hideUI()
{
    evas_object_hide(m_layout);
    evas_object_hide(m_gengrid);
    elm_gengrid_clear(m_gengrid);
#if PROFILE_MOBILE
    elm_object_signal_emit(m_top_content, "icon_less", "ui");
    m_map_bookmark.clear();
#else
    m_focusManager.stopFocusManager();
#endif

}

Evas_Object* BookmarkDetailsUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_layout)
        m_layout = createLayout(m_parent);
    setEmpty(true);
    return m_layout;
}

void BookmarkDetailsUI::onBackPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    if (m_remove_bookmark_mode) {
        updateGengridItems();
        resetRemovalMode();
    }
#endif
    closeBookmarkDetailsClicked();
}

#if PROFILE_MOBILE
void BookmarkDetailsUI::setLandscape(bool state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_rotation_state = state ? 0 : 1;
    resetContent();
}

void BookmarkDetailsUI::resetContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_rotation_state) {
        elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(290+18), ELM_SCALE_SIZE(308+18));
        elm_object_signal_emit(m_layout, "switch_landscape", "ui");
    } else {
        elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(319+18), ELM_SCALE_SIZE(361+18));
        elm_object_signal_emit(m_layout, "switch_vertical", "ui");
    }
}
#endif

char* BookmarkDetailsUI::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "page_title";
        const char *part_name2 = "page_url";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name1, part, part_name1_len)) {
            return strdup(itemData->item->getTitle().c_str());
        } else if (!strncmp(part_name2, part, part_name2_len)) {
            return strdup(itemData->item->getAddress().c_str());
        }
    }
    return strdup("");
}

void BookmarkDetailsUI::_grid_content_delete(void *data, Evas_Object */*obj*/)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    auto itemData = static_cast<BookmarkItemData*>(data);
    if (itemData)
        delete itemData;
}

Evas_Object * BookmarkDetailsUI::_grid_bookmark_content_get(void *data,
        Evas_Object *obj, const char *part)
{
    if ((data != nullptr) && (obj != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "elm.thumbnail";
        static const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len)) {
            std::shared_ptr<tizen_browser::tools::BrowserImage> image = itemData->item->getThumbnail();
            if (image)
                return image->getEvasImage(itemData->bookmarkDetailsUI->m_parent);
        }
#if PROFILE_MOBILE
        const char *part_name2 = "remove_checkbox_swallow";
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name2, part, part_name2_len)) {
            if (itemData->bookmarkDetailsUI->m_remove_bookmark_mode) {
                Evas_Object* box = elm_check_add(obj);
                elm_object_style_set(box, "custom_check");
                evas_object_propagate_events_set(box, EINA_FALSE);
                elm_check_state_set(box, itemData->bookmarkDetailsUI->m_map_delete[itemData->item->getId()]
                        ? EINA_TRUE : EINA_FALSE);
                evas_object_show(box);
                return box;
            }
        }
#endif
    }
    return nullptr;
}

void BookmarkDetailsUI::_bookmark_item_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
#if PROFILE_MOBILE
        if (itemData->bookmarkDetailsUI->m_remove_bookmark_mode) {
            itemData->bookmarkDetailsUI->m_delete_count -= itemData->bookmarkDetailsUI->
                    m_map_delete[itemData->item->getId()] ? 1 : -1;
            itemData->bookmarkDetailsUI->m_map_delete[itemData->item->getId()] =
                    !itemData->bookmarkDetailsUI->m_map_delete[itemData->item->getId()];
            elm_object_signal_emit(itemData->bookmarkDetailsUI->m_top_content, itemData->
                    bookmarkDetailsUI->m_delete_count ? "removal_mode" : "removal_mode_dissabled", "ui");
            elm_object_part_text_set(itemData->bookmarkDetailsUI->m_top_content,
                    "title_text", (boost::format("%d %s") % itemData->bookmarkDetailsUI->
                            m_delete_count % _("IDS_BR_OPT_SELECTED")).str().c_str());
            elm_gengrid_item_update(itemData->bookmarkDetailsUI->
                    m_map_bookmark[itemData->item->getId()]);
            elm_gengrid_realized_items_update(itemData->bookmarkDetailsUI->m_gengrid);
        }
        else
            itemData->bookmarkDetailsUI->bookmarkItemClicked(itemData->item);
#else
        itemData->bookmarkDetailsUI->bookmarkItemClicked(itemData->item);
#endif
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

#if !PROFILE_MOBILE
void BookmarkDetailsUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(m_close_button);
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}
#endif

void BookmarkDetailsUI::_close_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->onBackPressed();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}
#if PROFILE_MOBILE
void BookmarkDetailsUI::_more_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        if (evas_object_visible_get(bookmarkDetailsUI->m_menu) == EINA_FALSE) {
            elm_object_signal_emit(bookmarkDetailsUI->m_layout, "show_menu", "ui");
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "icon_more", "ui");
            evas_object_show(bookmarkDetailsUI->m_menu);
            evas_object_show(elm_object_part_content_get(bookmarkDetailsUI->m_layout, "more_swallow"));
        } else {
            elm_object_signal_emit(bookmarkDetailsUI->m_layout, "hide_menu", "ui");
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "icon_less", "ui");
            evas_object_hide(bookmarkDetailsUI->m_menu);
            evas_object_hide(elm_object_part_content_get(bookmarkDetailsUI->m_layout, "more_swallow"));
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_menu_bg_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "icon_less", "ui");
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_edit_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->editFolderButtonClicked(bookmarkDetailsUI->getFolderName());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_delete_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->deleteFolderButtonClicked(bookmarkDetailsUI->getFolderName());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_remove_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->m_map_delete.clear();
        bookmarkDetailsUI->m_remove_bookmark_mode = true;
        for (auto it = bookmarkDetailsUI->m_map_bookmark.begin(); it != bookmarkDetailsUI->m_map_bookmark.end(); ++it)
        bookmarkDetailsUI->m_map_delete.insert(std::pair<unsigned int, bool>(it->first, false));
        elm_gengrid_realized_items_update(bookmarkDetailsUI->m_gengrid);
        elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "icon_less", "ui");
        elm_object_signal_emit(bookmarkDetailsUI->m_layout, "hide_menu", "ui");
        elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "removal_mode_dissabled", "ui");
        elm_object_part_text_set(bookmarkDetailsUI->m_top_content, "title_text", (boost::format("%d %s")
                        % bookmarkDetailsUI->m_delete_count % _("IDS_BR_OPT_SELECTED")).str().c_str());
        evas_object_hide(bookmarkDetailsUI->m_menu);
        evas_object_hide(elm_object_part_content_get(bookmarkDetailsUI->m_layout, "more_swallow"));
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_cancel_top_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->updateGengridItems();
        bookmarkDetailsUI->resetRemovalMode();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_remove_top_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);

        std::vector<std::shared_ptr<services::BookmarkItem>> bookmarks;
        bookmarks.clear();

        for (auto it = bookmarkDetailsUI->m_map_delete.begin(); it != bookmarkDetailsUI->m_map_delete.end(); ++it)
            if (it->second) {
                BookmarkItemData * itemData = static_cast<BookmarkItemData*>(elm_object_item_data_get(
                                bookmarkDetailsUI->m_map_bookmark[it->first]));
                bookmarks.push_back(itemData->item);
                elm_object_item_del(bookmarkDetailsUI->m_map_bookmark[it->first]);
                bookmarkDetailsUI->m_map_bookmark.erase(it->first);
            }
        bookmarkDetailsUI->resetRemovalMode();
        bookmarkDetailsUI->removeFoldersButtonClicked(bookmarks);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}
#endif

std::string BookmarkDetailsUI::getFolderName()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string title = elm_object_part_text_get(m_top_content, "title_text");
    auto i = 0;
    auto pos = title.find_last_of("(");
    return title.substr(i, pos-i);
}

void BookmarkDetailsUI::setEmpty(bool isEmpty)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (isEmpty)
        elm_object_signal_emit(m_layout, "show_no_favorites", "ui");
    else
        elm_object_signal_emit(m_layout, "hide_no_favorites", "ui");
}

void BookmarkDetailsUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_top_content = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "top_content", m_top_content);
    evas_object_size_hint_weight_set(m_top_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_top_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_top_content);

    elm_layout_file_set(m_top_content, m_edjFilePath.c_str(), "top-content");

    m_close_button = elm_button_add(m_top_content);
    elm_object_style_set(m_close_button, "invisible_button");
    evas_object_smart_callback_add(m_close_button, "clicked", _close_button_clicked, this);
    evas_object_show(m_close_button);
    elm_object_part_content_set(m_top_content, "close_click", m_close_button);

    elm_object_focus_custom_chain_append(m_top_content, m_close_button, nullptr);
    elm_object_focus_set(m_close_button, EINA_TRUE);
    elm_object_tree_focus_allow_set(m_layout, EINA_TRUE);
    elm_object_focus_allow_set(m_close_button, EINA_TRUE);
#if PROFILE_MOBILE
    m_more_button = elm_button_add(m_top_content);
    elm_object_style_set(m_more_button, "invisible_button");
    evas_object_smart_callback_add(m_more_button, "clicked", _more_button_clicked, this);
    elm_object_part_content_set(m_top_content, "more_click", m_more_button);
    evas_object_show(m_more_button);

    elm_object_focus_custom_chain_append(m_top_content, m_more_button, nullptr);
    elm_object_focus_set(m_more_button, EINA_TRUE);
    elm_object_tree_focus_allow_set(m_layout, EINA_TRUE);
    elm_object_focus_allow_set(m_more_button, EINA_TRUE);

    m_cancel_top_button = elm_button_add(m_top_content);
    elm_object_style_set(m_cancel_top_button, "invisible_button");
    evas_object_smart_callback_add(m_cancel_top_button, "clicked", _cancel_top_button_clicked, this);
    elm_object_part_content_set(m_top_content, "cancel_click_2", m_cancel_top_button);

    m_remove_top_button = elm_button_add(m_top_content);
    elm_object_style_set(m_remove_top_button, "invisible_button");
    evas_object_smart_callback_add(m_remove_top_button, "clicked", _remove_top_button_clicked, this);
    elm_object_part_content_set(m_top_content, "remove_click_2", m_remove_top_button);
#endif
}

void BookmarkDetailsUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_gengrid = elm_gengrid_add(m_layout);
    edje_object_update_hints_set(m_gengrid, EINA_TRUE);
    elm_object_part_content_set(m_layout, "elm.swallow.grid", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_gengrid, EINA_TRUE);
#else
    elm_object_style_set(m_gengrid, "back_ground");
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(404), ELM_SCALE_SIZE(320));
#endif
}

#if PROFILE_MOBILE
void BookmarkDetailsUI::createMenuDetails()
{
    m_menu_bg_button = elm_button_add(m_layout);
    elm_object_style_set(m_menu_bg_button, "invisible_button");
    evas_object_smart_callback_add(m_menu_bg_button, "clicked", _menu_bg_button_clicked, this);
    elm_object_part_content_set(m_layout, "more_bg_click", m_menu_bg_button);

    m_menu = elm_box_add(m_layout);
    evas_object_size_hint_weight_set(m_menu, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_menu, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_edit_button = elm_button_add(m_menu);
    elm_object_style_set(m_edit_button, "more-button");
    evas_object_size_hint_weight_set(m_edit_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_edit_button, 0.0, 0.0);
    elm_object_part_text_set(m_edit_button, "elm.text", "Edit folder name");
    elm_box_pack_end(m_menu, m_edit_button);
    evas_object_smart_callback_add(m_edit_button, "clicked", _edit_button_clicked, this);

    evas_object_show(m_edit_button);
    elm_object_tree_focus_allow_set(m_edit_button, EINA_FALSE);

    m_delete_button = elm_button_add(m_menu);
    elm_object_style_set(m_delete_button, "more-button");
    evas_object_size_hint_weight_set(m_delete_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_delete_button, 0.0, 0.0);
    elm_object_part_text_set(m_delete_button, "elm.text", "Delete folder");
    elm_box_pack_end(m_menu, m_delete_button);
    evas_object_smart_callback_add(m_delete_button, "clicked", _delete_button_clicked, this);

    evas_object_show(m_delete_button);
    elm_object_tree_focus_allow_set(m_delete_button, EINA_FALSE);

    m_remove_button = elm_button_add(m_menu);
    elm_object_style_set(m_remove_button, "more-shadow-button");
    evas_object_size_hint_weight_set(m_remove_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_remove_button, 0.0, 0.0);
    elm_object_part_text_set(m_remove_button, "elm.text", "Remove Bookmarks");
    elm_box_pack_end(m_menu, m_remove_button);
    evas_object_smart_callback_add(m_remove_button, "clicked", _remove_button_clicked, this);

    evas_object_show(m_remove_button);
    elm_object_tree_focus_allow_set(m_remove_button, EINA_FALSE);

    elm_object_part_content_set(m_layout, "more_swallow", m_menu);
    evas_object_show(m_menu);
}

void BookmarkDetailsUI::updateGengridItems()
{
    for (auto it = m_map_delete.begin(); it != m_map_delete.end(); ++it)
        if (it->second) {
            it->second = false;
            elm_gengrid_item_update(m_map_bookmark[it->first]);
        }
}

void BookmarkDetailsUI::resetRemovalMode()
{
    m_map_delete.clear();
    m_delete_count = 0;
    m_remove_bookmark_mode = false;
    elm_object_signal_emit(m_top_content, "default_mode", "ui");
    elm_object_part_text_set(m_top_content, "title_text", (boost::format("%s(%d)") % m_folder_name.c_str()
                    % elm_gengrid_items_count(m_gengrid)).str().c_str());
    elm_gengrid_realized_items_update(m_gengrid);
}
#else
void BookmarkDetailsUI::createBottomContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_bottom_content = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "bottom_content", m_bottom_content);
    evas_object_size_hint_weight_set(m_bottom_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_bottom_content);

    elm_layout_file_set(m_bottom_content, m_edjFilePath.c_str(),
            "bottom-content");
}
#endif

Evas_Object* BookmarkDetailsUI::createLayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(),
            "bookmark-details-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND,
            EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    createTopContent();
    createGengrid();
#if PROFILE_MOBILE
    resetContent();
    createMenuDetails();
#else
    createBottomContent();
    createFocusVector();
#endif
    return m_layout;
}

void BookmarkDetailsUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkDetailsUI = this;
    Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid,
            m_bookmark_item_class, itemData, _bookmark_item_clicked, itemData);
#if PROFILE_MOBILE
    m_map_bookmark.insert(std::pair<unsigned int,Elm_Object_Item*>(hi->getId(), bookmarkView));
#endif
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
}

void BookmarkDetailsUI::addBookmarks(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items, std::string folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_gengrid_clear(m_gengrid);
    m_folder_name = folder_name;
    for (auto it = items.begin(); it != items.end(); ++it)
        addBookmarkItem(*it);
    elm_object_part_text_set(m_top_content, "title_text",
#if PROFILE_MOBILE
                            (boost::format("%s(%d)") % m_folder_name.c_str() % items.size()).str().c_str());
#else
                            (boost::format("Bookmark Manager > %s") % m_folder_name.c_str()).str().c_str());
#endif
    elm_object_part_content_set(m_layout, "elm.swallow.grid", m_gengrid);
#if PROFILE_MOBILE
    elm_box_unpack_all(m_menu);
    if (m_folder_name != _("IDS_BR_BODY_ALL") && m_folder_name != "Mobile") {
        evas_object_show(m_edit_button);
        evas_object_show(m_delete_button);
        elm_box_pack_end(m_menu, m_edit_button);
        elm_box_pack_end(m_menu, m_delete_button);
    } else {
        evas_object_hide(m_edit_button);
        evas_object_hide(m_delete_button);
    }
    elm_box_pack_end(m_menu, m_remove_button);
#else
    elm_object_part_text_set(m_bottom_content, "text", (boost::format("%d %s") % elm_gengrid_items_count(m_gengrid) %
                                                    (elm_gengrid_items_count(m_gengrid) == 1 ? "bookmark" : "bookmarks")).str().c_str());
#endif
    if (items.size() != 0)
        setEmpty(false);
    evas_object_show(m_gengrid);
    items.clear();
}

}
}

