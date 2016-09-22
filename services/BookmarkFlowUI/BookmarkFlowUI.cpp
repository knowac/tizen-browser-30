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

/*
 * BookmarkFlowUI.cpp
 *
 *  Created on: Nov 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <boost/format.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "BookmarkFlowUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkFlowUI, BOOKMARK_FLOW_SERVICE)

BookmarkFlowUI::BookmarkFlowUI()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_title_area(nullptr)
#if PROFILE_MOBILE
    , m_contents_area(nullptr)
    , m_remove_button(nullptr)
    , m_entry(nullptr)
    , m_save_button(nullptr)
    , m_cancel_button(nullptr)
    , m_input_cancel_button(nullptr)
    , m_folder_button(nullptr)
#else
    , m_gengrid(nullptr)
    , m_bg(nullptr)
#endif
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkFlowUI/BookmarkFlowUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
#if PROFILE_MOBILE
    createGenlistItemClasses();
#else
    createGengridItemClasses();
#endif
}

BookmarkFlowUI::~BookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    evas_object_smart_callback_del(m_save_button, "clicked", _save_clicked);
    evas_object_smart_callback_del(m_cancel_button, "clicked", _cancel_clicked);
    evas_object_smart_callback_del(m_entry, "focused", _entry_focused);
    evas_object_smart_callback_del(m_entry, "unfocused", _entry_unfocused);
    evas_object_smart_callback_del(m_entry, "changed,user",_entry_changed);
    evas_object_smart_callback_del(m_input_cancel_button, "clicked", _inputCancel_clicked);
    evas_object_smart_callback_del(m_folder_button, "clicked", _folder_clicked);
    evas_object_smart_callback_del(m_remove_button, "clicked", _remove_clicked);

    evas_object_del(m_remove_button);
    evas_object_del(m_entry);
    evas_object_del(m_save_button);
    evas_object_del(m_save);
    evas_object_del(m_save_box);
    evas_object_del(m_cancel_button);
    evas_object_del(m_cancel);
    evas_object_del(m_cancel_box);
    evas_object_del(m_input_cancel_button);
    evas_object_del(m_folder_button);
    evas_object_del(m_contents_area);

    if(m_folder_custom_item_class)
        elm_gengrid_item_class_free(m_folder_custom_item_class);
    if(m_folder_selected_item_class)
        elm_gengrid_item_class_free(m_folder_selected_item_class);
#else
    if(m_folder_new_item_class)
        elm_gengrid_item_class_free(m_folder_new_item_class);
    if(m_folder_custom_item_class)
        elm_gengrid_item_class_free(m_folder_custom_item_class);

    elm_gengrid_clear(m_gengrid);
    evas_object_del(m_gengrid);

    popupDismissed.disconnect_all_slots();
    popupShown.disconnect_all_slots();
#endif
    evas_object_del(m_title_area);
    evas_object_del(m_layout);

    closeBookmarkFlowClicked.disconnect_all_slots();
    saveBookmark.disconnect_all_slots();
    editBookmark.disconnect_all_slots();
    removeBookmark.disconnect_all_slots();
    addFolder.disconnect_all_slots();
}

void BookmarkFlowUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* BookmarkFlowUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_layout)
        m_layout = createBookmarkFlowLayout();
    return m_layout;
}

void BookmarkFlowUI::addCustomFolders(services::SharedBookmarkFolderList folders)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = folders.begin(); it != folders.end(); ++it) {
#if PROFILE_MOBILE
        if ((*it)->getId() == m_all_folder_id)
            continue;
#endif
        FolderData *folderData = new FolderData();
        folderData->name = (*it)->getName();
        folderData->folder_id = (*it)->getId();
        folderData->bookmarkFlowUI.reset(this);
#if PROFILE_MOBILE
        listAddCustomFolder(folderData);
#else
        gridAddCustomFolder(folderData);
#endif
    }

#if PROFILE_MOBILE
    elm_object_part_content_set(m_contents_area, "dropdown_swallow", m_genlist);
    evas_object_show(m_genlist);
    elm_object_item_signal_emit(elm_genlist_last_item_get(m_genlist), "invisible", "ui");
#else
    if (elm_gengrid_items_count(m_gengrid) < 10)
        elm_object_signal_emit(m_layout, "upto9", "ui");
    if (elm_gengrid_items_count(m_gengrid) < 7)
        elm_object_signal_emit(m_layout, "upto6", "ui");

    elm_object_part_content_set(m_layout, "folder_grid_swallow", m_gengrid);
    evas_object_show(m_gengrid);
#endif
}

#if PROFILE_MOBILE
void BookmarkFlowUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "show_popup", "ui");
    evas_object_show(m_layout);
    elm_object_signal_emit(m_contents_area, "close_icon_show", "ui");
    resetContent();
}

void BookmarkFlowUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "hide_popup", "ui");
    evas_object_hide(m_layout);
    evas_object_hide(m_genlist);
    elm_object_signal_emit(m_contents_area, "dropdown_swallow_hide", "ui");
    evas_object_hide(elm_object_part_content_get(m_contents_area, "dropdown_swallow"));
    elm_genlist_clear(m_genlist);
    m_map_folders.clear();
}

void BookmarkFlowUI::setState(bool state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_state = state;
    if (m_state) {
        elm_object_signal_emit(m_contents_area, "show_remove", "ui");
        elm_object_translatable_part_text_set(m_title_area, "title_area_text", _("IDS_BR_HEADER_EDIT_BOOKMARK"));
        evas_object_show(m_remove_button);
    }
    else {
        elm_object_signal_emit(m_contents_area, "hide_remove", "ui");
        elm_object_translatable_part_text_set(m_title_area, "title_area_text", _("IDS_BR_OPT_ADD_BOOKMARK"));
        evas_object_hide(m_remove_button);
    }
}

void BookmarkFlowUI::setTitle(const std::string& title)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, title.c_str());
    elm_object_part_text_set(m_entry, "elm.text", elm_entry_utf8_to_markup(title.c_str()));

    if (title.empty()) {
        elm_object_disabled_set(m_save_button, EINA_TRUE);
        elm_object_signal_emit(m_save, "text_dissabled", "ui");
    }
    else {
        elm_object_disabled_set(m_save_button, EINA_FALSE);
        elm_object_signal_emit(m_save, "text_enabled", "ui");
    }
}

void BookmarkFlowUI::setURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    elm_object_part_text_set(m_contents_area, "site_url_text", url.c_str());
}

void BookmarkFlowUI::setFolder(unsigned int folder_id, const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_item_item_class_update(m_map_folders[m_folder_id], m_folder_custom_item_class);
    m_folder_id = folder_id;
    elm_object_signal_emit(m_contents_area, (m_folder_id == m_special_folder_id)
                           ? "folder_icon_special" : "folder_icon_normal", "ui");
    elm_object_part_text_set(m_contents_area, "dropdown_text", elm_entry_utf8_to_markup(folder_name.c_str()));
    elm_genlist_item_item_class_update(m_map_folders[m_folder_id], m_folder_selected_item_class);
}

void BookmarkFlowUI::setFoldersId(unsigned int all, unsigned int special)
{
    m_all_folder_id = all;
    m_special_folder_id = special;
}

void BookmarkFlowUI::resetContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> rotated = isRotated();
    if (rotated) {
        if (*rotated) {
            elm_scroller_page_size_set(m_genlist, 0, ELM_SCALE_SIZE(GENLIST_HEIGHT_LANDSCAPE));
            m_max_items = MAX_ITEMS_LANDSCAPE;
            elm_object_signal_emit(m_contents_area, "dropdown_text_landscape", "ui");
        } else {
            elm_scroller_page_size_set(m_genlist, 0, ELM_SCALE_SIZE(GENLIST_HEIGHT));
            m_max_items = MAX_ITEMS;
            elm_object_signal_emit(m_contents_area, "dropdown_text_portrait", "ui");
        }
        if (evas_object_visible_get(m_genlist) == EINA_TRUE) {
            unsigned int count = elm_genlist_items_count(m_genlist);
            if (count > m_max_items)
                count = m_max_items;
            elm_object_signal_emit(m_contents_area, (boost::format("%s_%u") % "dropdown_swallow_show" % count).str().c_str(), "ui");
            evas_object_show(m_genlist);
            evas_object_show(elm_object_part_content_get(m_contents_area, "dropdown_swallow"));
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarkFlowUI::_save_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_UNUSED(data);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        BookmarkUpdate update;
        update.folder_id = bookmarkFlowUI->m_folder_id;
        update.title = elm_entry_markup_to_utf8(elm_object_part_text_get(bookmarkFlowUI->m_entry, "elm.text"));
        if (!bookmarkFlowUI->m_state)
            bookmarkFlowUI->saveBookmark(update);
        else
            bookmarkFlowUI->editBookmark(update);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}

void BookmarkFlowUI::_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}

void BookmarkFlowUI::createContentsArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_contents_area = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "contents_area_swallow", m_contents_area);
    evas_object_size_hint_weight_set(m_contents_area, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_contents_area, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_contents_area);

    elm_layout_file_set(m_contents_area, m_edjFilePath.c_str(), "contents-area-layout");

    m_entry = elm_entry_add(m_contents_area);
    elm_object_style_set(m_entry, "title_input_entry");

    elm_entry_single_line_set(m_entry, EINA_TRUE);
    elm_entry_scrollable_set(m_entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);
    elm_object_part_content_set(m_contents_area, "title_input_swallow", m_entry);

    evas_object_smart_callback_add(m_entry, "focused", _entry_focused, this);
    evas_object_smart_callback_add(m_entry, "unfocused", _entry_unfocused, this);
    evas_object_smart_callback_add(m_entry, "changed,user",_entry_changed, this);

    m_input_cancel_button = elm_button_add(m_contents_area);
    elm_object_style_set(m_input_cancel_button, "invisible_button");
    evas_object_smart_callback_add(m_input_cancel_button, "clicked", _inputCancel_clicked, this);
    evas_object_show(m_input_cancel_button);

    elm_object_part_content_set(m_contents_area, "input_cancel_click", m_input_cancel_button);

    m_folder_button = elm_button_add(m_contents_area);
    elm_object_style_set(m_folder_button, "invisible_button");
    evas_object_smart_callback_add(m_folder_button, "clicked", _folder_clicked, this);
    evas_object_show(m_input_cancel_button);

    elm_object_part_content_set(m_contents_area, "folder_button_click", m_folder_button);

    m_folder_dropdown_button = elm_button_add(m_contents_area);
    elm_object_style_set(m_folder_dropdown_button, "invisible_button");
    evas_object_smart_callback_add(m_folder_dropdown_button, "clicked", _folder_dropdown_clicked, this);
    evas_object_show(m_folder_dropdown_button);

    elm_object_part_content_set(m_contents_area, "folder_dropdown_click", m_folder_dropdown_button);

    m_remove_button = elm_button_add(m_contents_area);
    elm_object_style_set(m_remove_button, "invisible_button");
    evas_object_smart_callback_add(m_remove_button, "clicked", _remove_clicked, this);
    evas_object_show(m_remove_button);

    elm_object_part_content_set(m_contents_area, "remove_click", m_remove_button);

    elm_object_translatable_part_text_set(m_contents_area, "folder_text", _("IDS_BR_HEADER_FOLDER"));
}

void BookmarkFlowUI::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_folder_custom_item_class = elm_genlist_item_class_new();
    m_folder_custom_item_class->item_style = "folder-custom-item";
    m_folder_custom_item_class->func.text_get = _folder_title_text_get;
    m_folder_custom_item_class->func.content_get = nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = nullptr;

    m_folder_selected_item_class = elm_genlist_item_class_new();
    m_folder_selected_item_class->item_style = "folder-selected-item";
    m_folder_selected_item_class->func.text_get = _folder_title_text_get;
    m_folder_selected_item_class->func.content_get = nullptr;
    m_folder_selected_item_class->func.state_get = nullptr;
    m_folder_selected_item_class->func.del = nullptr;
}

void BookmarkFlowUI::createGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_genlist = elm_genlist_add(m_contents_area);
    elm_object_part_content_set(m_contents_area, "dropdown_swallow", m_genlist);
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void BookmarkFlowUI::listAddCustomFolder(FolderData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* custom_folder = elm_genlist_item_append(m_genlist, item->folder_id == m_folder_id ?
                                                             m_folder_selected_item_class : m_folder_custom_item_class, item,
                                                             nullptr, ELM_GENLIST_ITEM_NONE, _listCustomFolderClicked, item);
    m_map_folders.insert(std::pair<unsigned int, Elm_Object_Item*>(item->folder_id, custom_folder));
    elm_genlist_item_selected_set(custom_folder, EINA_FALSE);
}

void BookmarkFlowUI::_listCustomFolderClicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData* folderData = static_cast<FolderData*>(data);
        elm_genlist_item_item_class_update(folderData->bookmarkFlowUI->m_map_folders[folderData->bookmarkFlowUI->m_folder_id],
                folderData->bookmarkFlowUI->m_folder_custom_item_class);
        folderData->bookmarkFlowUI->m_folder_id = folderData->folder_id;
        elm_object_part_text_set(folderData->bookmarkFlowUI->m_contents_area,
                                 "dropdown_text", elm_entry_utf8_to_markup(folderData->name.c_str()));
        elm_object_signal_emit(folderData->bookmarkFlowUI->m_contents_area,
                               (folderData->bookmarkFlowUI->m_folder_id == folderData->bookmarkFlowUI->m_special_folder_id)
                               ? "folder_icon_special" : "folder_icon_normal", "ui");
        elm_object_signal_emit(folderData->bookmarkFlowUI->m_contents_area, "dropdown_swallow_hide", "ui");
        evas_object_hide(folderData->bookmarkFlowUI->m_genlist);
        evas_object_hide(elm_object_part_content_get(folderData->bookmarkFlowUI->m_contents_area, "dropdown_swallow"));
        elm_genlist_item_item_class_update(folderData->bookmarkFlowUI->m_map_folders[folderData->bookmarkFlowUI->m_folder_id],
                folderData->bookmarkFlowUI->m_folder_selected_item_class);
    }
}

void BookmarkFlowUI::_entry_focused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_focus_allow_set(bookmarkFlowUI->m_input_cancel_button, EINA_TRUE);
        elm_object_signal_emit(bookmarkFlowUI->m_entry, "unfocused", "ui");
    }
}

void BookmarkFlowUI::_entry_unfocused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_focus_allow_set(bookmarkFlowUI->m_input_cancel_button, EINA_FALSE);
        elm_object_signal_emit(bookmarkFlowUI->m_entry, "unfocused", "ui");
    }
}

void BookmarkFlowUI::_entry_changed(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        std::string text = elm_entry_markup_to_utf8(elm_object_part_text_get(bookmarkFlowUI->m_entry, "elm.text"));

        if (text.empty()) {
            elm_object_signal_emit(bookmarkFlowUI->m_contents_area, "close_icon_hide", "ui");
            elm_object_disabled_set(bookmarkFlowUI->m_save_button, EINA_TRUE);
            elm_object_signal_emit(bookmarkFlowUI->m_save, "text_dissabled", "ui");
        } else {
            elm_object_signal_emit(bookmarkFlowUI->m_contents_area, "close_icon_show", "ui");
            elm_object_disabled_set(bookmarkFlowUI->m_save_button, EINA_FALSE);
            elm_object_signal_emit(bookmarkFlowUI->m_save, "text_enabled", "ui");
        }
    }
}

void BookmarkFlowUI::_inputCancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_part_text_set(bookmarkFlowUI->m_entry, "elm.text", "");
        elm_object_disabled_set(bookmarkFlowUI->m_save_button, EINA_TRUE);
        elm_object_signal_emit(bookmarkFlowUI->m_save, "text_dissabled", "ui");
        elm_object_signal_emit(bookmarkFlowUI->m_contents_area, "close_icon_hide", "ui");
    }
}

void BookmarkFlowUI::_folder_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->addFolder();
    }
}

void BookmarkFlowUI::_folder_dropdown_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (evas_object_visible_get(bookmarkFlowUI->m_genlist) == EINA_FALSE) {
            unsigned int count = elm_genlist_items_count(bookmarkFlowUI->m_genlist);
            if (count > bookmarkFlowUI->m_max_items)
                count = bookmarkFlowUI->m_max_items;
            elm_object_signal_emit(bookmarkFlowUI->m_contents_area, (boost::format("%s_%u") % "dropdown_swallow_show" % count).str().c_str(), "ui");
            evas_object_show(bookmarkFlowUI->m_genlist);
            evas_object_show(elm_object_part_content_get(bookmarkFlowUI->m_contents_area,"dropdown_swallow"));
        } else {
            elm_object_signal_emit(bookmarkFlowUI->m_contents_area, "dropdown_swallow_hide", "ui");
            evas_object_hide(bookmarkFlowUI->m_genlist);
            evas_object_hide(elm_object_part_content_get(bookmarkFlowUI->m_contents_area, "dropdown_swallow"));
        }
    }
}

void BookmarkFlowUI::_remove_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->removeBookmark();
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}
#else
BookmarkFlowUI* BookmarkFlowUI::createPopup(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkFlowUI *bookmarkFlow = new BookmarkFlowUI();
    bookmarkFlow->m_parent = parent;
    return bookmarkFlow;
}

void BookmarkFlowUI::show()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    createBookmarkFlowLayout();
    evas_object_show(m_layout);
    evas_object_show(m_gengrid);
#if !PROFILE_MOBILE
    m_focusManager.startFocusManager(m_gengrid);
#endif
    popupShown(this);
}

void BookmarkFlowUI::dismiss()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if !PROFILE_MOBILE
    m_focusManager.stopFocusManager();
#endif
    popupDismissed(this);
}

void BookmarkFlowUI::onBackPressed()
{
    dismiss();
}

void BookmarkFlowUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_folder_new_item_class = elm_gengrid_item_class_new();
    m_folder_new_item_class->item_style = "folder-new-item";
    m_folder_new_item_class->func.text_get = nullptr;
    m_folder_new_item_class->func.content_get =  nullptr;
    m_folder_new_item_class->func.state_get = nullptr;
    m_folder_new_item_class->func.del = nullptr;

    m_folder_custom_item_class = elm_gengrid_item_class_new();
    m_folder_custom_item_class->item_style = "folder-custom-item";
    m_folder_custom_item_class->func.text_get = _folder_title_text_get;
    m_folder_custom_item_class->func.content_get =  nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = nullptr;
}

void BookmarkFlowUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_gengrid = elm_gengrid_add(m_layout);
    elm_object_part_content_set(m_layout, "folder_grid_swallow", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_page_size_set(m_gengrid, 0, 750);
    elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(208+18), ELM_SCALE_SIZE(208+18));
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(m_gengrid);
}

void BookmarkFlowUI::addNewFolder()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* new_folder = elm_gengrid_item_append(m_gengrid, m_folder_new_item_class,
                                                            NULL, _gridNewFolderClicked, this);
    elm_gengrid_item_selected_set(new_folder, EINA_FALSE);
}

void BookmarkFlowUI::_gridNewFolderClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->addFolder();
        bookmarkFlowUI->dismiss();
    }
}

void BookmarkFlowUI::gridAddCustomFolder(FolderData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* custom_folder = elm_gengrid_item_append(m_gengrid, m_folder_custom_item_class,
                                                            item, _gridCustomFolderClicked, item);
    elm_gengrid_item_selected_set(custom_folder, EINA_FALSE);
}

void BookmarkFlowUI::_gridCustomFolderClicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData* folderData = static_cast<FolderData*>(data);
        BookmarkUpdate update;
        update.folder_id = folderData->folder_id;
        folderData->bookmarkFlowUI->saveBookmark(update);
        folderData->bookmarkFlowUI->dismiss();
    }
}

void BookmarkFlowUI::_bg_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->dismiss();
    }
}

void BookmarkFlowUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}
#endif


char* BookmarkFlowUI::_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        FolderData *folderData = static_cast<FolderData*>(data);
        const char *folder_text = "folder_text";
        if (!strncmp(folder_text, part, strlen(folder_text)))
            return strdup(elm_entry_utf8_to_markup(folderData->name.c_str()));
    }
    return strdup("");
}

Evas_Object* BookmarkFlowUI::createBookmarkFlowLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "bookmarkflow-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    createTitleArea();
#if PROFILE_MOBILE
    createContentsArea();
    createGenlist();
#else
    m_bg = elm_button_add(m_layout);
    elm_object_style_set(m_bg, "invisible_button");
    evas_object_smart_callback_add(m_bg, "clicked", _bg_clicked, this);
    elm_object_part_content_set(m_layout, "bg_click", m_bg);
    createGengrid();
    createFocusVector();
#endif

    evas_object_show(m_layout);
    return m_layout;
}

void BookmarkFlowUI::createTitleArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_title_area = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "title_area_swallow", m_title_area);
    evas_object_size_hint_weight_set(m_title_area, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_title_area, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_title_area);

    elm_layout_file_set(m_title_area, m_edjFilePath.c_str(), "title-area-layout");
#if PROFILE_MOBILE

    m_save_box = elm_box_add(m_title_area);
    m_save = elm_layout_add(m_save_box);
    elm_layout_file_set(m_save, m_edjFilePath.c_str(), "button_with_background");
    evas_object_size_hint_weight_set(m_save, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_save, 1, 0);
    elm_object_translatable_text_set(m_save, _("IDS_BR_SK_SAVE")); //TODO: ADD UPPERCASE SAVE
    elm_box_pack_end(m_save_box, m_save);
    evas_object_show(m_save);

    m_save_button = elm_button_add(m_save);
    elm_object_style_set(m_save_button, "invisible_button");
    evas_object_smart_callback_add(m_save_button, "clicked", _save_clicked, this);
    elm_object_part_content_set(m_save, "button_click", m_save_button);
    evas_object_show(m_save_button);

    elm_object_part_content_set(m_title_area, "save_swallow", m_save_box);
    evas_object_show(m_save_box);


    m_cancel_box = elm_box_add(m_title_area);
    m_cancel = elm_layout_add(m_cancel_box);
    elm_layout_file_set(m_cancel, m_edjFilePath.c_str(), "button_with_background");
    evas_object_size_hint_weight_set(m_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_cancel, 0, 0);
    elm_object_translatable_text_set(m_cancel, _("IDS_BR_SK_CANCEL"));
    elm_box_pack_end(m_cancel_box, m_cancel);
    evas_object_show(m_cancel);

    m_cancel_button = elm_button_add(m_cancel);
    elm_object_style_set(m_cancel_button, "invisible_button");
    evas_object_smart_callback_add(m_cancel_button, "clicked", _cancel_clicked, this);
    elm_object_part_content_set(m_cancel, "button_click", m_cancel_button);
    evas_object_show(m_cancel_button);

    elm_object_part_content_set(m_title_area, "cancel_swallow", m_cancel_box);
    evas_object_show(m_cancel_box);
#endif
}

}
}
