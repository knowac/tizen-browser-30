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
    , m_done_button(nullptr)
    , m_cancel_button(nullptr)
    , m_genlist(nullptr)
    , m_qa_checkbox(nullptr)
    , m_title_entry(nullptr)
    , m_url_entry(nullptr)
    , m_entry_item_class(nullptr)
    , m_group_item_class(nullptr)
    , m_folder_item_class(nullptr)
    , m_add_to_qa_item_class(nullptr)
    , m_edjFilePath("")
    , m_title("")
    , m_url("")
    , m_add_to_qa(false)
    , m_state(false)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkFlowUI/BookmarkFlowUI.edj");
    createGenlistItemClasses();
}

BookmarkFlowUI::~BookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_smart_callback_del(m_done_button, "clicked", _done_clicked);
    evas_object_smart_callback_del(m_cancel_button, "clicked", _cancel_clicked);

    if (m_entry_item_class)
        elm_genlist_item_class_free(m_entry_item_class);
    if (m_group_item_class)
        elm_genlist_item_class_free(m_group_item_class);
    if (m_folder_item_class)
        elm_genlist_item_class_free(m_folder_item_class);
    if (m_add_to_qa_item_class)
        elm_genlist_item_class_free(m_add_to_qa_item_class);

    evas_object_del(m_qa_checkbox);
    evas_object_del(m_genlist);
    evas_object_del(m_cancel_button);
    evas_object_del(m_done_button);
    evas_object_del(m_layout);

    closeBookmarkFlowClicked.disconnect_all_slots();
    saveBookmark.disconnect_all_slots();
    editBookmark.disconnect_all_slots();
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

void BookmarkFlowUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "show_popup", "ui");
    evas_object_show(m_layout);
    fillGenlist();
    updateTopContent();
}

void BookmarkFlowUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "hide_popup", "ui");
    evas_object_hide(m_layout);
}

void BookmarkFlowUI::setState(bool state)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, state ? "edit" : "add");
    m_state = state;
}

void BookmarkFlowUI::setTitle(const std::string& title)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, title.c_str());
    m_title = title;
    if (m_title.empty())
        elm_object_signal_emit(m_done_button, "elm,state,disabled", "elm");
    else
        elm_object_signal_emit(m_done_button, "elm,state,enabled", "elm");
}

void BookmarkFlowUI::setURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    m_url = url;
}

void BookmarkFlowUI::setFolder(services::SharedBookmarkItem bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] %d %s", __PRETTY_FUNCTION__, __LINE__, bookmarkItem->getId(),
        bookmarkItem->getTitle().c_str());
    m_bookmarkItem = bookmarkItem;
}

Evas_Object* BookmarkFlowUI::createBookmarkFlowLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_layout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
    elm_layout_theme_set(m_layout, "naviframe", "item/basic", "default");

    createTopContent();
    createGenlist();

    return m_layout;
}

void BookmarkFlowUI::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_entry_item_class = createGenlistItemClass("entry_custom_layout", _genlist_entry_text_get,
        _genlist_entry_content_get);
    m_group_item_class = createGenlistItemClass("group_index", _genlist_text_get);
    m_folder_item_class = createGenlistItemClass("type1", _genlist_folder_text_get,
        _genlist_folder_content_get);
    m_add_to_qa_item_class = createGenlistItemClass("type1", _genlist_add_to_qa_text_get,
        _genlist_add_to_qa_content_get);
}

Elm_Genlist_Item_Class* BookmarkFlowUI::createGenlistItemClass(
    const char* style, Elm_Gen_Item_Text_Get_Cb text_cb, Elm_Gen_Item_Content_Get_Cb content_cb)
{
    auto ic = elm_genlist_item_class_new();
    ic->item_style = style;
    ic->func.text_get = text_cb;
    ic->func.content_get = content_cb;
    ic->func.state_get = nullptr;
    ic->func.del = nullptr;
    ic->decorate_all_item_style = "edit_default";
    return ic;
}

void BookmarkFlowUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_cancel_button = elm_button_add(m_layout);
    elm_object_part_content_set(m_layout, "title_left_btn", m_cancel_button);
    elm_object_style_set(m_cancel_button, "naviframe/title_left");
    elm_object_text_set(m_cancel_button, _("IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"));
    evas_object_smart_callback_add(m_cancel_button, "clicked", _cancel_clicked, this);
    evas_object_size_hint_weight_set(m_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_layout, "elm,state,title_left_btn,show", "elm");

    m_done_button = elm_button_add(m_layout);
    elm_object_part_content_set(m_layout, "title_right_btn", m_done_button);
    elm_object_style_set(m_done_button, "naviframe/title_right");
    elm_object_text_set(m_done_button, _("IDS_TPLATFORM_ACBUTTON_DONE_ABB"));
    evas_object_smart_callback_add(m_done_button, "clicked", _done_clicked, this);
    evas_object_size_hint_weight_set(m_done_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_done_button, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_layout, "elm,state,title_right_btn,show", "elm");
}

void BookmarkFlowUI::createGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_genlist = elm_genlist_add(m_layout);
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_part_content_set(m_layout, "elm.swallow.content", m_genlist);
    evas_object_show(m_genlist);
}

void BookmarkFlowUI::fillGenlist()
{
    elm_genlist_clear(m_genlist);

    //Add title entry
    EntryData *titleEntryData = new EntryData();
    titleEntryData->category = _("IDS_BR_BODY_TITLE");
    titleEntryData->entry = m_title;
    titleEntryData->bookmarkFlowUI.reset(this);

    m_title_entry = elm_genlist_item_append(m_genlist, m_entry_item_class, titleEntryData, nullptr,
        ELM_GENLIST_ITEM_NONE, nullptr, titleEntryData);

    //Add URL entry
    if (m_state) {
        EntryData *urlEntryData = new EntryData();
        urlEntryData->category = _("IDS_BR_BODY_ADDBOOKMARKURL");
        urlEntryData->entry = m_url;
        urlEntryData->bookmarkFlowUI.reset(this);
        m_url_entry = elm_genlist_item_append(m_genlist, m_entry_item_class, urlEntryData, nullptr,
            ELM_GENLIST_ITEM_NONE, nullptr, urlEntryData);
    }

    //Add group header
    elm_genlist_item_append(m_genlist, m_group_item_class, _("IDS_BR_BODY_LOCATION_M_INFORMATION"),
        nullptr, ELM_GENLIST_ITEM_NONE, nullptr, _("IDS_BR_BODY_LOCATION_M_INFORMATION"));

    FolderData *folderData = new FolderData();
    folderData->name = m_bookmarkItem->getTitle();
    folderData->folder_id = m_bookmarkItem->getId();
    folderData->bookmarkFlowUI.reset(this);

    //Add folder picker
    elm_genlist_item_append(m_genlist, m_folder_item_class, folderData, nullptr, ELM_GENLIST_ITEM_NONE,
        _folder_selector_clicked, this);

    //Add QuickAccess checkbox
    elm_genlist_item_append(m_genlist, m_add_to_qa_item_class, this, nullptr, ELM_GENLIST_ITEM_NONE,
        _qa_clicked, this);
}

void BookmarkFlowUI::updateTopContent()
{
    if (m_state)
        elm_object_translatable_part_text_set(m_layout, "elm.text.title", _("IDS_BR_HEADER_EDIT_BOOKMARK"));
    else
        elm_object_translatable_part_text_set(m_layout, "elm.text.title", _("IDS_BR_OPT_ADD_BOOKMARK"));
}

void BookmarkFlowUI::updateDoneButton()
{
    Evas_Object* layout = elm_object_item_part_content_get(m_title_entry, "elm.swallow.content");
    Evas_Object* entry = elm_object_part_content_get(layout, "elm.swallow.content");
    bool first = elm_entry_is_empty(entry);
    bool second = false;
    if (m_state) {
        layout = elm_object_item_part_content_get(m_url_entry, "elm.swallow.content");
        entry = elm_object_part_content_get(layout, "elm.swallow.content");
        second = elm_entry_is_empty(entry);
    }
    if (first || second)
        elm_object_signal_emit(m_done_button, "elm,state,disabled", "elm");
    else
        elm_object_signal_emit(m_done_button, "elm,state,enabled", "elm");
}

Evas_Object *BookmarkFlowUI::_genlist_entry_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        EntryData *entryData = static_cast<EntryData*>(data);
        if (!strcmp(part, "elm.swallow.content")) {
            Evas_Object* entry_layout = elm_layout_add(obj);
            elm_layout_theme_set(entry_layout, "layout", "editfield", "multiline");
            evas_object_size_hint_weight_set(entry_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(entry_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

            Evas_Object* entry = elm_entry_add(entry_layout);
            elm_entry_single_line_set(entry, EINA_TRUE);
            elm_entry_scrollable_set(entry, EINA_TRUE);
            evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
            elm_entry_entry_set(entry, elm_entry_utf8_to_markup(entryData->entry.c_str()));
            if (entryData->category == _("IDS_BR_BODY_ADDBOOKMARKURL"))
                elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);

            ObjectData* objectData = new ObjectData();
            objectData->object = entry_layout;
            objectData->bookmarkFlowUI = entryData->bookmarkFlowUI;

            evas_object_smart_callback_add(entry, "changed", _entry_changed, objectData);
            evas_object_smart_callback_add(entry, "focused", _entry_focused, entry_layout);
            evas_object_smart_callback_add(entry, "unfocused", _entry_unfocused, entry_layout);

            elm_object_part_content_set(entry_layout, "elm.swallow.content", entry);

            Evas_Object* input_cancel_button = elm_button_add(entry_layout);
            elm_object_style_set(input_cancel_button, "editfield_clear");
            elm_object_focus_allow_set(input_cancel_button, EINA_FALSE);

            evas_object_smart_callback_add(input_cancel_button, "clicked", _input_cancel_clicked, entry);

            elm_object_part_content_set(entry_layout, "elm.swallow.button", input_cancel_button);

            return entry_layout;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkFlowUI::_genlist_folder_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        FolderData *folderData = static_cast<FolderData*>(data);
        if (!strcmp(part, "elm.swallow.icon")) {
            Evas_Object* layout = elm_layout_add(obj);
            elm_layout_file_set(layout, folderData->bookmarkFlowUI->m_edjFilePath.c_str(), "folder_image");
            return layout;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkFlowUI::_genlist_add_to_qa_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        BookmarkFlowUI *bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (!strcmp(part, "elm.swallow.end")) {
            bookmarkFlowUI->m_qa_checkbox = elm_check_add(obj);
            evas_object_propagate_events_set(bookmarkFlowUI->m_qa_checkbox, EINA_FALSE);
            elm_check_state_set(bookmarkFlowUI->m_qa_checkbox, bookmarkFlowUI->m_add_to_qa ? EINA_TRUE : EINA_FALSE);
            evas_object_smart_callback_add(bookmarkFlowUI->m_qa_checkbox, "changed", _add_to_qa_state_changed, bookmarkFlowUI);
            evas_object_show(bookmarkFlowUI->m_qa_checkbox);
            return bookmarkFlowUI->m_qa_checkbox;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_entry_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        if (!strcmp(part, "elm.text")) {
            EntryData *entryData = static_cast<EntryData*>(data);
            return strdup(entryData->category.c_str());
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        if (!strcmp(part, "elm.text"))
            return strdup(elm_entry_utf8_to_markup(static_cast<char*>(data)));
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_folder_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        FolderData *folderData = static_cast<FolderData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup(elm_entry_utf8_to_markup(folderData->name.c_str()));
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_add_to_qa_text_get(void *, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (part) {
        if (!strcmp(part, "elm.text"))
            return strdup(elm_entry_utf8_to_markup(_("IDS_BR_OPT_ADD_TO_QUICK_ACCESS")));
    } else
        BROWSER_LOGE("[%s:%d] Part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

void BookmarkFlowUI::_done_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        BookmarkUpdate update;
        update.folder_id = bookmarkFlowUI->m_bookmarkItem->getId();

        Evas_Object* layout = elm_object_item_part_content_get(bookmarkFlowUI->m_title_entry,
            "elm.swallow.content");
        Evas_Object* entry = elm_object_part_content_get(layout, "elm.swallow.content");
        update.title = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

        if (!bookmarkFlowUI->m_state)
            bookmarkFlowUI->saveBookmark(update);
        else {
            update.old_url = bookmarkFlowUI->m_url;
            layout = elm_object_item_part_content_get(bookmarkFlowUI->m_url_entry,
                "elm.swallow.content");
            entry = elm_object_part_content_get(layout, "elm.swallow.content");
            update.url = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
            bookmarkFlowUI->editBookmark(update);
        }
        bookmarkFlowUI->closeBookmarkFlowClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_entry_focused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data)
        elm_object_signal_emit((Evas_Object*)data, "elm,state,focused", "");
    else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_entry_unfocused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data)
        elm_object_signal_emit((Evas_Object*)data, "elm,state,unfocused", "");
    else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_entry_changed(void * data, Evas_Object * obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        ObjectData* objectData = static_cast<ObjectData*>(data);
        if (elm_entry_is_empty(obj))
            elm_object_signal_emit(objectData->object, "elm,action,hide,button", "");
        else
            elm_object_signal_emit(objectData->object, "elm,action,show,button", "");
        objectData->bookmarkFlowUI->updateDoneButton();
    } else
        BROWSER_LOGW("[%s] data or obj = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_input_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        elm_entry_entry_set((Evas_Object*)data, "");
        elm_object_focus_set((Evas_Object*)data, EINA_TRUE);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_folder_selector_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->showSelectFolderUI(bookmarkFlowUI->m_bookmarkItem);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_add_to_qa_state_changed(void *data, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkFlowUI *bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->m_add_to_qa = elm_check_state_get(obj) == EINA_TRUE;
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_qa_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->m_add_to_qa = !(elm_check_state_get(bookmarkFlowUI->m_qa_checkbox) == EINA_TRUE);
        elm_check_state_set(bookmarkFlowUI->m_qa_checkbox, bookmarkFlowUI->m_add_to_qa ? EINA_TRUE : EINA_FALSE);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

}
}
