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

#include <app.h>
#include <app_common.h>
#include <app_control.h>
#include <Elementary.h>
#include <boost/format.hpp>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "BookmarkManagerUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "app_i18n.h"
#include "Tools/BookmarkItem.h"
#include "Tools/BrowserImage.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkManagerUI, "org.tizen.browser.bookmarkmanagerui")

BookmarkManagerUI::BookmarkManagerUI()
    : m_parent(nullptr)
    , m_content(nullptr)
    , m_modulesToolbar(nullptr)
    , m_navigatorToolbar(nullptr)
    , m_genlist(nullptr)
    , m_empty_layout(nullptr)
    , m_select_all(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_state(BookmarkManagerState::Default)
    , m_reordered(false)
    , m_selected_count(0)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkManagerUI/BookmarkManagerUI.edj");
    createGenlistItemClasses();
}

BookmarkManagerUI::~BookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_bookmark_item_class)
        elm_genlist_item_class_free(m_bookmark_item_class);
}

void BookmarkManagerUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkManagerUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->show();
}

void BookmarkManagerUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->hide();
}

Evas_Object* BookmarkManagerUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_naviframe)
        createBookmarksLayout();
    changeState(m_state);

    return m_naviframe->getLayout();
}

void BookmarkManagerUI::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_bookmark_item_class = elm_genlist_item_class_new();
    m_bookmark_item_class->item_style = "type1";
    m_bookmark_item_class->func.text_get = _genlist_bookmark_text_get;
    m_bookmark_item_class->func.content_get =  _genlist_bookmark_content_get;
    m_bookmark_item_class->func.state_get = nullptr;
    m_bookmark_item_class->func.del = _genlist_del<BookmarkData>;
    m_bookmark_item_class->decorate_all_item_style = "edit_default";
}

char* BookmarkManagerUI::_genlist_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
    if (data && part) {
        BookmarkData *bookmarkData = static_cast<BookmarkData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup(bookmarkData->bookmarkItem->getTitle().c_str());
        if (!bookmarkData->bookmarkItem->is_folder() && !strcmp(part, "elm.text.sub"))
            return strdup(elm_entry_utf8_to_markup(bookmarkData->bookmarkItem->getAddress().c_str()));
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkManagerUI::_genlist_bookmark_content_get(void *data, Evas_Object *obj, const char *part)
{
    if (data && part) {
        BookmarkData *bookmarkData = static_cast<BookmarkData*>(data);
        if (!strcmp(part, "elm.swallow.icon")) {
            Evas_Object *icon = elm_icon_add(obj);
            if (bookmarkData->bookmarkItem->is_folder())
                elm_image_file_set(icon, bookmarkData->bookmarkManagerUI->m_edjFilePath.c_str(), "folder_image");
            else if (bookmarkData->bookmarkItem->has_favicon()) {
                std::shared_ptr<tools::BrowserImage> image = bookmarkData->bookmarkItem->getFavicon();
                icon = image->getEvasImage(obj);
            } else
                elm_image_file_set(icon, bookmarkData->bookmarkManagerUI->m_edjFilePath.c_str(), "favicon_image");
            elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
            evas_object_size_hint_min_set(icon,
                ELM_SCALE_SIZE(bookmarkData->bookmarkManagerUI->ICON_SIZE),
                ELM_SCALE_SIZE(bookmarkData->bookmarkManagerUI->ICON_SIZE));
            evas_object_size_hint_max_set(icon,
                ELM_SCALE_SIZE(bookmarkData->bookmarkManagerUI->ICON_SIZE),
                ELM_SCALE_SIZE(bookmarkData->bookmarkManagerUI->ICON_SIZE));
            return icon;
        } else if (!strcmp(part, "elm.swallow.end")) {
            switch (bookmarkData->bookmarkManagerUI->m_state) {
            case BookmarkManagerState::Share:
            case BookmarkManagerState::Delete:
                {
                Evas_Object* checkbox = elm_check_add(obj);
                evas_object_propagate_events_set(checkbox, EINA_FALSE);
                elm_check_state_set(checkbox, bookmarkData->bookmarkManagerUI->
                    m_map_selected[bookmarkData->bookmarkItem->getId()] ? EINA_TRUE : EINA_FALSE);
                evas_object_smart_callback_add(checkbox, "changed", _check_state_changed, bookmarkData);
                evas_object_show(checkbox);
                return checkbox;
                }
                break;
            case BookmarkManagerState::Reorder:
                {
                Evas_Object *reorder_button = elm_button_add(obj);
                elm_object_style_set(reorder_button, "icon_reorder");
                return reorder_button;
                }
                break;
            default:
                break;
            }
        } else if (!strcmp(part, "elm.swallow.icon.2") && bookmarkData->bookmarkItem->getPrivate()) {
            Evas_Object* layout = elm_layout_add(obj);
            elm_layout_file_set(layout, bookmarkData->bookmarkManagerUI->m_edjFilePath.c_str(), "private_image");
            return layout;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

void BookmarkManagerUI::createBookmarksLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_naviframe = std::make_shared<NaviframeWrapper>(m_parent);

    m_content = elm_layout_add(m_naviframe->getLayout());
    evas_object_size_hint_weight_set(m_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_content);
    elm_layout_file_set(m_content, m_edjFilePath.c_str(), "naviframe_content");
    m_naviframe->setContent(m_content);

    createTopContent();
    createModulesToolbar();
    createNavigatorToolbar();
    elm_object_signal_emit(m_content, "show_toolbars", "ui");

    createGenlist();
    createEmptyLayout();
}

void BookmarkManagerUI::createModulesToolbar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_modulesToolbar = elm_toolbar_add(m_content);

    elm_object_style_set(m_modulesToolbar, "tabbar/notitle");
    elm_toolbar_shrink_mode_set(m_modulesToolbar, ELM_TOOLBAR_SHRINK_EXPAND);
    elm_toolbar_select_mode_set(m_modulesToolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_toolbar_transverse_expanded_set(m_modulesToolbar, EINA_TRUE);
    elm_object_part_content_set(m_content, "modules_toolbar", m_modulesToolbar);
    evas_object_show(m_modulesToolbar);

    elm_toolbar_item_append(m_modulesToolbar, nullptr, _("IDS_BR_BODY_BOOKMARKS"), _modules_bookmarks_clicked, this);
    elm_toolbar_item_append(m_modulesToolbar, nullptr, _("IDS_BR_MBODY_HISTORY"), _modules_history_clicked, this);
}

void BookmarkManagerUI::createNavigatorToolbar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_navigatorToolbar = elm_toolbar_add(m_content);

    elm_object_style_set(m_navigatorToolbar, "navigationbar");
    elm_toolbar_shrink_mode_set(m_navigatorToolbar, ELM_TOOLBAR_SHRINK_SCROLL);
    elm_toolbar_transverse_expanded_set(m_navigatorToolbar, EINA_TRUE);
    elm_toolbar_align_set(m_navigatorToolbar, 0.0);
    elm_toolbar_homogeneous_set(m_navigatorToolbar, EINA_FALSE);
    elm_toolbar_select_mode_set(m_navigatorToolbar, ELM_OBJECT_SELECT_MODE_DEFAULT);
    elm_object_part_content_set(m_content, "navigator_toolbar", m_navigatorToolbar);
    evas_object_show(m_navigatorToolbar);
}

void BookmarkManagerUI::createGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_box = elm_box_add(m_content);
    elm_object_focus_set(m_box, EINA_FALSE);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_clear(m_box);

    elm_object_part_content_set(m_content, "elm.swallow.content", m_box);
    evas_object_show(m_box);

    m_genlist = elm_genlist_add(m_box);

    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(m_genlist, "moved", _genlist_bookmark_moved, this);
    evas_object_smart_callback_add(m_genlist, "realized", _genlist_bookmark_realized, this);

    elm_box_pack_end(m_box, m_genlist);
    evas_object_show(m_genlist);

    m_select_all = elm_layout_add(m_genlist);
    elm_object_focus_allow_set(m_select_all, EINA_TRUE);
    elm_layout_theme_set(m_select_all, "genlist", "item", "type1/default");
    evas_object_size_hint_weight_set(m_select_all, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
    evas_object_size_hint_align_set(m_select_all, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object *checkbox = elm_check_add(m_select_all);
    elm_object_part_content_set(m_select_all, "elm.swallow.end", checkbox);
    evas_object_propagate_events_set(checkbox, EINA_FALSE);

    Evas_Object *bg = elm_bg_add(m_select_all);
    elm_bg_color_set(bg, 255, 255, 255);
    elm_object_part_content_set(m_select_all, "elm.swallow.bg", bg);

    elm_object_part_text_set(m_select_all, "elm.text", _("IDS_BR_OPT_SELECT_ALL"));

    evas_object_event_callback_add(m_select_all, EVAS_CALLBACK_MOUSE_DOWN, _select_all_down, this);
    evas_object_smart_callback_add(checkbox, "changed", _select_all_state_changed, this);

    evas_object_show(m_select_all);
}

void BookmarkManagerUI::createEmptyLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_empty_layout = elm_layout_add(m_content);
    elm_layout_theme_set(m_empty_layout, "layout", "nocontents", "default");

    evas_object_size_hint_weight_set(m_empty_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_empty_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_empty_layout, "elm.text", _("IDS_BR_BODY_NO_BOOKMARKS"));
    elm_object_translatable_part_text_set(m_empty_layout, "elm.help.text", "After you add bookmarks, they will be shown here.");

    elm_layout_signal_emit(m_empty_layout, "text,disabled", "");
    elm_layout_signal_emit(m_empty_layout, "align.center", "elm");

    elm_object_part_content_set(m_content, "elm.swallow.content_overlay", m_empty_layout);
}

void BookmarkManagerUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());

    m_naviframe->addLeftButton(_cancel_clicked, this);
    //TODO: Missing translation. In guidelines this should be uppercase
    m_naviframe->setLeftButtonText(_("IDS_BR_SK_CANCEL"));
    m_naviframe->addRightButton(_accept_clicked, this);
    m_naviframe->addPrevButton(_prev_clicked, this);
}

void BookmarkManagerUI::_cancel_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<BookmarkManagerUI*>(data);
        self->onBackPressed();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_accept_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        switch (bookmarkManagerUI->m_state) {
        case BookmarkManagerState::Delete:
            for (const auto& it : bookmarkManagerUI->m_map_selected)
                if (it.second) {
                    BookmarkData *bookmarkData = static_cast<BookmarkData*>(elm_object_item_data_get(
                        bookmarkManagerUI->m_map_bookmark[it.first]));
                    bookmarkManagerUI->bookmarkItemDeleted(bookmarkData->bookmarkItem);
                    elm_object_item_del(bookmarkManagerUI->m_map_bookmark[it.first]);
                    bookmarkManagerUI->m_map_bookmark.erase(it.first);
                }
            break;
        case BookmarkManagerState::Share:
            bookmarkManagerUI->bookmarkItemsShare();
            break;
        case BookmarkManagerState::SelectFolder:
            bookmarkManagerUI->folderSelected(bookmarkManagerUI->m_folder_path.back());
            break;
        case BookmarkManagerState::HistoryDeleteView:
            bookmarkManagerUI->removeHistoryItems();
            break;
        default:
            break;
        }
        bookmarkManagerUI->onBackPressed();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::removeHistoryItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto historyListSize = elm_genlist_items_count(m_historyGenlist);

    for (int i = historyListSize; i > 1; --i) {
        auto it = elm_genlist_nth_item_get(m_historyGenlist, i);
        auto check = elm_object_item_part_content_get(it, "elm.swallow.end");
        if (!check)
            continue;
        auto state = elm_check_state_get(check);
        if (state == EINA_TRUE) {
            elm_object_item_del(it);
        }
    }
    elm_genlist_realized_items_update(m_historyGenlist);
    removeSelectedItemsFromHistory();
    m_naviframe->setRightButtonEnabled(false);

}
void BookmarkManagerUI::_prev_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bookmarkManagerUI->onBackPressed();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_check_state_changed(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkData* bookmarkData = static_cast<BookmarkData*>(data);
        bookmarkData->bookmarkManagerUI->updateDeleteClick(bookmarkData->bookmarkItem->getId());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_genlist_bookmark_moved(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bookmarkManagerUI->m_reordered = true;
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_genlist_bookmark_realized(void *, Evas_Object *, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        auto bookmarkItem = static_cast<Elm_Object_Item*>(event_info);
        auto bookmarkData = static_cast<BookmarkData*>(elm_object_item_data_get(bookmarkItem));

        if (!bookmarkData->bookmarkItem->is_folder())
            elm_object_item_signal_emit(bookmarkItem, "elm,state,elm.text.sub,visible", "elm");
        if (bookmarkData->bookmarkItem->getPrivate())
            elm_object_item_signal_emit(bookmarkItem, "elm,state,elm.swallow.end,visible", "elm");
    } else {
        BROWSER_LOGW("[%s] event_info = nullptr", __PRETTY_FUNCTION__);
    }
}

void BookmarkManagerUI::_select_all_down(void *data, Evas *, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        Evas_Object *checkbox = elm_object_part_content_get(bookmarkManagerUI->m_select_all, "elm.swallow.end");
        elm_check_state_set(checkbox, !elm_check_state_get(checkbox));
        _select_all_state_changed(bookmarkManagerUI, checkbox, nullptr);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_select_all_state_changed(void *data, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bool state = elm_check_state_get(obj) == EINA_TRUE;
        Elm_Object_Item *it = elm_genlist_first_item_get(bookmarkManagerUI->m_genlist);
        while (it) {
            services::SharedBookmarkItem bookmarkItem = (static_cast<BookmarkData*>(elm_object_item_data_get(it)))->bookmarkItem;
            if (state != bookmarkManagerUI->m_map_selected[bookmarkItem->getId()]) {
                bookmarkManagerUI->m_selected_count -= bookmarkManagerUI-> m_map_selected[bookmarkItem->getId()] ? 1 : -1;
                bookmarkManagerUI->m_map_selected[bookmarkItem->getId()] = state;
                elm_genlist_item_update(bookmarkManagerUI-> m_map_bookmark[bookmarkItem->getId()]);
            }
            it = elm_genlist_item_next_get(it);
        }

        bookmarkManagerUI->updateDeleteTopContent();
        elm_genlist_realized_items_update(bookmarkManagerUI->m_genlist);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::addBookmarkItems(std::shared_ptr<services::BookmarkItem> parent,
                                         std::vector<std::shared_ptr<services::BookmarkItem> > items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_added_bookmarks = items;
    if (parent && (m_folder_path.empty() || m_folder_path.back()->getId() != parent->getId())) {
        if (parent->getParent() == -1) {
            int count = elm_toolbar_items_count(m_navigatorToolbar);
            for (int i = 0; i < count; ++i)
                elm_object_item_del(elm_toolbar_last_item_get(m_navigatorToolbar));
            m_folder_path.clear();
        }
        m_folder_path.push_back(parent);
        elm_toolbar_item_append(m_navigatorToolbar, NULL, parent->getTitle().c_str(), _navigatorFolderClicked, this);
    }
    addFilteredBookmarkItems(BookmarkManagerGenlistFilter::All);
}

void BookmarkManagerUI::addFilteredBookmarkItems(BookmarkManagerGenlistFilter filter) {
    elm_genlist_clear(m_genlist);
    m_map_bookmark.clear();
    for (const auto& it : m_added_bookmarks) {
        if (filter != BookmarkManagerGenlistFilter::All
            && static_cast<int>(filter) != (int)(*it).is_folder())
            continue;
        BookmarkData* data = new BookmarkData();
        data->bookmarkManagerUI = this;
        data->bookmarkItem = it;
        addBookmarkItem(data);
    }
    updateNoBookmarkText();
}

void BookmarkManagerUI::addBookmarkItemCurrentFolder(services::SharedBookmarkItem item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_added_bookmarks.push_back(item);
    BookmarkData* data = new BookmarkData();
    data->bookmarkManagerUI = this;
    data->bookmarkItem = item;
    addBookmarkItem(data);
    updateNoBookmarkText();
}

void BookmarkManagerUI::addBookmarkItem(BookmarkData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* bookmarkView = elm_genlist_item_append(
        m_genlist,
        m_bookmark_item_class,
        item,
        nullptr,
        ELM_GENLIST_ITEM_NONE,
        _bookmarkItemClicked,
        item);
    m_map_bookmark.insert(
        std::pair<unsigned int, Elm_Object_Item*>(item->bookmarkItem->getId(), bookmarkView));
    elm_genlist_item_selected_set(bookmarkView, EINA_FALSE);
}

void BookmarkManagerUI::onBackPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (m_state) {
    case BookmarkManagerState::SelectFolder:
        closeBookmarkManagerClicked();
        changeState(BookmarkManagerState::Default);
        break;
    case BookmarkManagerState::Default:
        //TODO: We should go to the previous navigatorToolbar element if it exists.
        closeBookmarkManagerClicked();
        break;
    case BookmarkManagerState::Share:
        addFilteredBookmarkItems(BookmarkManagerGenlistFilter::All);
        changeState(BookmarkManagerState::Default);
        break;
    case BookmarkManagerState::HistoryDeleteView:
        changeState(BookmarkManagerState::HistoryView);
        prepareHistoryContent();
        break;
    case BookmarkManagerState::HistoryView:
        elm_toolbar_item_selected_set(
            elm_toolbar_first_item_get(m_modulesToolbar),
            EINA_TRUE);
        prepareBookmarksContent();
        closeBookmarkManagerClicked();
        changeState(BookmarkManagerState::Default);
        break;
    default:
        changeState(BookmarkManagerState::Default);
        break;
    }
}

void BookmarkManagerUI::showContextMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<Evas_Object*> window = getWindow();
    if (window) {
        if (m_state == BookmarkManagerState::Default ||
            m_state == BookmarkManagerState::SelectFolder) {
            createContextMenu(*window);
            if (m_state == BookmarkManagerState::Default && elm_genlist_items_count(m_genlist)) {
                elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_SK_DELETE"), nullptr,
                                         _cm_delete_clicked, this);
                elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_SHARE"), nullptr,
                                         _cm_share_clicked, this);
                elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_REORDER_ABB"), nullptr,
                                         _cm_reorder_clicked, this);
                elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_BUTTON_EDIT"), nullptr,
                                         _cm_edit_clicked, this);
            }
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_SK3_CREATE_FOLDER"), nullptr,
                                     _cm_create_folder_clicked, this);
            alignContextMenu(*window);
        } else if (m_state == BookmarkManagerState::HistoryView) {
            createContextMenu(*window);
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_REMOVE"), nullptr, _cm_history_remove_clicked, this);
            alignContextMenu(*window);
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarkManagerUI::_cm_history_remove_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::HistoryDeleteView);
        bookmarkManagerUI->prepareHistoryContent();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_cm_delete_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Delete);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_cm_share_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Share);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_cm_reorder_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Reorder);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_cm_edit_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Edit);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_cm_create_folder_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->newFolderItemClicked(bookmarkManagerUI->m_folder_path.back()->getId());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_navigatorFolderClicked(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkManagerUI *bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        Elm_Object_Item *clicked = static_cast<Elm_Object_Item*>(event_info);
        Elm_Object_Item *it = elm_toolbar_last_item_get(bookmarkManagerUI->m_navigatorToolbar);
        if (clicked == it)
            return;
        while (clicked != it) {
            Elm_Object_Item *it_prev = elm_toolbar_item_prev_get(it);
            elm_object_item_del(it);
            bookmarkManagerUI->m_folder_path.pop_back();
            it = it_prev;
        }
        bookmarkManagerUI->bookmarkItemClicked(bookmarkManagerUI->m_folder_path.back());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::prepareBookmarksContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_box_unpack_all(m_box);
    elm_box_pack_end(m_box, m_genlist);
    evas_object_hide(m_historyGenlist);
    evas_object_show(m_genlist);
}

void BookmarkManagerUI::prepareHistoryContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bool removeMode = (m_state == BookmarkManagerState::HistoryDeleteView);
    auto historyGenlist = getHistoryGenlistContent(m_box, m_naviframe, removeMode);

    elm_box_unpack_all(m_box);

    if (historyGenlist && *historyGenlist) {
        m_historyGenlist = *historyGenlist;
        elm_box_pack_end(m_box, m_historyGenlist);
        evas_object_hide(m_genlist);
        evas_object_show(m_historyGenlist);
    }
}

void BookmarkManagerUI::_modules_bookmarks_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<BookmarkManagerUI*>(data);
        self->changeState(BookmarkManagerState::Default);
        self->prepareBookmarksContent();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_modules_history_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<BookmarkManagerUI*>(data);
        if (self->m_state != BookmarkManagerState::HistoryView) {
            self->changeState(BookmarkManagerState::HistoryView);
            self->prepareHistoryContent();
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_bookmarkItemClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkData *bookmarkData = static_cast<BookmarkData*>(data);
        switch (bookmarkData->bookmarkManagerUI->m_state) {
        case BookmarkManagerState::Default:
            bookmarkData->bookmarkManagerUI->bookmarkItemClicked(bookmarkData->bookmarkItem);
            break;
        case BookmarkManagerState::SelectFolder:
            bookmarkData->bookmarkManagerUI->bookmarkItemClicked(bookmarkData->bookmarkItem);
            break;
        case BookmarkManagerState::Edit:
            bookmarkData->bookmarkManagerUI->bookmarkItemEdit(bookmarkData->bookmarkItem);
            bookmarkData->bookmarkManagerUI->changeState(BookmarkManagerState::Default);
            break;
        case BookmarkManagerState::Delete:
        case BookmarkManagerState::Share:
            bookmarkData->bookmarkManagerUI->updateDeleteClick(bookmarkData->bookmarkItem->getId());
            elm_genlist_item_update(bookmarkData->bookmarkManagerUI->
                    m_map_bookmark[bookmarkData->bookmarkItem->getId()]);
            elm_genlist_realized_items_update(bookmarkData->bookmarkManagerUI->m_genlist);
            break;
        default:
            break;
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::changeState(BookmarkManagerState state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_state = state;
    switch (state) {
    case BookmarkManagerState::SelectFolder:
        elm_genlist_realized_items_update(m_genlist);
        showNaviframePrevButton(false);
        //TODO: Missing translation. In guidelines this should be uppercase
        m_naviframe->setRightButtonText(_("IDS_BR_SK_DONE"));
        showToolbars(true);
        elm_object_signal_emit(m_content, "hide_modules_toolbar", "ui");
        evas_object_hide(m_modulesToolbar);
        break;
    case BookmarkManagerState::Edit:
        m_reordered = false;
        m_naviframe->setTitle(_("IDS_BR_HEADER_SELECT_BOOKMARK"));
        showToolbars(false);
        evas_object_hide(m_navigatorToolbar);
        break;
    case BookmarkManagerState::Delete:
        clearSelection();
        elm_genlist_realized_items_update(m_genlist);
        showNaviframePrevButton(false);
        //TODO: Missing translation. In guidelines this should be uppercase
        m_naviframe->setRightButtonText(_("IDS_BR_SK_DELETE"));
        updateDeleteTopContent();
        showToolbars(false);
        evas_object_hide(m_navigatorToolbar);
        insertSelectAll();
        break;
    case BookmarkManagerState::Share:
        clearSelection();
        addFilteredBookmarkItems(BookmarkManagerGenlistFilter::Websites);
        elm_genlist_realized_items_update(m_genlist);
        showNaviframePrevButton(false);
        //TODO: Missing translation. In guidelines this should be uppercase
        m_naviframe->setRightButtonText(_("IDS_BR_SK_DONE"));
        updateDeleteTopContent();
        showToolbars(false);
        evas_object_hide(m_navigatorToolbar);
        insertSelectAll();
        break;
    case BookmarkManagerState::Reorder:
        showNaviframePrevButton(true);
        m_naviframe->setTitle(_("IDS_BR_OPT_REORDER_ABB"));
        showToolbars(false);
        evas_object_hide(m_navigatorToolbar);
        elm_genlist_reorder_mode_set(m_genlist, EINA_TRUE);
        break;
    case BookmarkManagerState::HistoryView:
        updateNoBookmarkText();
        showNaviframePrevButton(true);
        m_naviframe->setTitle(_("IDS_BR_MBODY_HISTORY"));
        showToolbars(true);
        evas_object_hide(m_navigatorToolbar);
        elm_object_part_content_unset(m_content, "navigator_toolbar");
        elm_object_signal_emit(m_content, "hide_navigator_toolbar", "ui");
        elm_box_unpack(m_box, m_select_all);
        evas_object_hide(m_select_all);
        break;
    case BookmarkManagerState::HistoryDeleteView:
        updateNoBookmarkText();
        m_naviframe->setTitle(_("IDS_BR_MBODY_HISTORY"));
        showNaviframePrevButton(false);
        m_naviframe->setRightButtonText(_("IDS_BR_SK_DONE"));
        updateDeleteTopContent();
        showToolbars(false);
        evas_object_hide(m_navigatorToolbar);
        break;
    case BookmarkManagerState::Default:
    default:
        updateNoBookmarkText();
        reoderBookmarkItems();
        showNaviframePrevButton(true);
        m_naviframe->setTitle(_("IDS_BR_BODY_BOOKMARKS"));
        showToolbars(true);
        evas_object_show(m_navigatorToolbar);
        elm_object_part_content_set(m_content, "navigator_toolbar", m_navigatorToolbar);
        elm_genlist_reorder_mode_set(m_genlist, EINA_FALSE);
        elm_box_unpack(m_box, m_select_all);
        evas_object_hide(m_select_all);
        break;
    }
    elm_genlist_realized_items_update(m_genlist);
}

void BookmarkManagerUI::clearSelection()
{
    m_selected_count = 0;
    m_map_selected.clear();
    for (const auto& it : m_map_bookmark)
        m_map_selected.insert(std::pair<unsigned int, bool>(it.first, false));
}

void BookmarkManagerUI::showNaviframePrevButton(bool show)
{
    m_naviframe->setLeftButtonVisible(!show);
    m_naviframe->setRightButtonVisible(!show);
    m_naviframe->setPrevButtonVisible(show);
}

void BookmarkManagerUI::showToolbars(bool show)
{
    if (show) {
        elm_object_signal_emit(m_content, "show_toolbars", "ui");
        evas_object_show(m_navigatorToolbar);
        auto secret = isEngineSecretMode();
        if (secret) {
            if (*secret) {
                elm_object_signal_emit(m_content, "hide_modules_toolbar", "ui");
                evas_object_hide(m_modulesToolbar);
            }
        } else {
             BROWSER_LOGE("[%s:%d] unknow isEngineSecretMode value!", __PRETTY_FUNCTION__, __LINE__);
        }
    } else {
        elm_object_signal_emit(m_content, "hide_toolbars", "ui");
        evas_object_hide(m_modulesToolbar);
    }
}

void BookmarkManagerUI::insertSelectAll()
{
    elm_check_state_set(elm_object_part_content_get(m_select_all, "elm.swallow.end"), EINA_FALSE);
    elm_box_pack_start(m_box, m_select_all);
    evas_object_show(m_select_all);
}

void BookmarkManagerUI::reoderBookmarkItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_reordered) {
        m_reordered = false;
        Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
        BookmarkData *bookmarkData;
        int order = 1;
        while (it) {
            bookmarkData = static_cast<BookmarkData*>(elm_object_item_data_get(it));
            if (order !=  bookmarkData->bookmarkItem->getOrder()) {
                bookmarkData->bookmarkItem->setOrder(order);
                bookmarkItemOrderEdited(bookmarkData->bookmarkItem);
            }
            it = elm_genlist_item_next_get(it);
            order++;
        }
    }
}

void BookmarkManagerUI::updateNoBookmarkText()
{
    if (m_map_bookmark.size() ||
        (m_state == BookmarkManagerState::HistoryView || m_state == BookmarkManagerState::HistoryDeleteView)) {
        evas_object_hide(m_empty_layout);
        elm_object_signal_emit(m_content, "hide_overlay", "ui");
    } else {
        evas_object_show(m_empty_layout);
        elm_object_signal_emit(m_content, "show_overlay", "ui");
    }
}

void BookmarkManagerUI::updateDeleteClick(int id)
{
    m_selected_count -= m_map_selected[id] ? 1 : -1;
    m_map_selected[id] = !m_map_selected[id];
    elm_check_state_set(
        elm_object_part_content_get(m_select_all, "elm.swallow.end"),
        m_selected_count == elm_genlist_items_count(m_genlist));
    updateDeleteTopContent();
}

void BookmarkManagerUI::updateDeleteTopContent()
{
    if (m_selected_count)
        m_naviframe->setTitle((boost::format(_("IDS_BR_HEADER_PD_SELECTED_ABB")) % m_selected_count).str());
    else
        m_naviframe->setTitle(_("IDS_BR_HEADER_SELECT_ITEMS_ABB2"));
    m_naviframe->setRightButtonEnabled(m_selected_count);
}

void BookmarkManagerUI::bookmarkItemsShare()
{
    std::string subject = "";
    std::string message = "";
    std::string url = "";
    std::string title = "";

    for (const auto& it : m_map_selected)
        if (it.second) {
            BookmarkData *bookmarkData = static_cast<BookmarkData*>(
                elm_object_item_data_get(m_map_bookmark[it.first]));

            url = bookmarkData->bookmarkItem->getAddress();
            title = bookmarkData->bookmarkItem->getTitle();
            if (!title.size())
                title = url;
            if (m_selected_count == 1)
                subject = title;

            message.append(title);
            message.append("\n");
            message.append(url);
            message.append("\n");
        }

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGE("[%s:%d] Fail to app_control_create", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_SHARE_TEXT) < 0) {
        BROWSER_LOGE("[%s:%d] Fail to app_control_set_operation", __PRETTY_FUNCTION__, __LINE__);
        app_control_destroy(app_control);
        return;
    }
    if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TEXT, message.c_str()) < 0) {
        BROWSER_LOGE("[%s:%d] Fail to app_control_add_extra_data", __PRETTY_FUNCTION__, __LINE__);
        app_control_destroy(app_control);
        return;
    }
    if (m_selected_count == 1)
        if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_SUBJECT, subject.c_str()) < 0) {
            BROWSER_LOGE("[%s:%d] Fail to app_control_add_extra_data", __PRETTY_FUNCTION__, __LINE__);
            app_control_destroy(app_control);
            return;
        }
    if (app_control_set_app_id(app_control, "org.tizen.share-panel") < 0) {
        BROWSER_LOGE("[%s:%d] Fail to app_control_set_app_id", __PRETTY_FUNCTION__, __LINE__);
        app_control_destroy(app_control);
        return;
    }
    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGE("[%s:%d] Fail to app_control_send_launch_request", __PRETTY_FUNCTION__, __LINE__);
        app_control_destroy(app_control);
        return;
    }
    app_control_destroy(app_control);
}

}
}
