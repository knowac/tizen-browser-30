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

#ifndef URIENTRY_H
#define URIENTRY_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include "memory.h"

#include "BasicUI/Action.h"
#include "BrowserImage.h"
#include "EflTools.h"


namespace tizen_browser {
namespace base_ui {

class URIEntry {
public:
    enum IconType {
        IconTypeSearch
        , IconTypeDoc
        , IconTypeFav
    };
    // Enumerate whether we should keep selected txt
    // TODO It is temporary solution. Remove when input events will be fixed
    enum class SelectionState {
        SELECTION_KEEP
        , SELECTION_NONE
    };
    URIEntry();
    ~URIEntry();
    void init(Evas_Object* parent);
    Evas_Object* getContent();
    Evas_Object* getEntryWidget();

    void changeUri(const std::string&);
    boost::signals2::signal<void (const std::string&)> uriChanged;

    // uri edition change (by a user)
    boost::signals2::signal<void (const std::shared_ptr<std::string>)> uriEntryEditingChangedByUser;

#if PROFILE_MOBILE
    boost::signals2::signal<void ()> mobileEntryFocused;
    boost::signals2::signal<void ()> mobileEntryUnfocused;
    boost::signals2::signal<void ()> secureIconClicked;
    boost::signals2::signal<bool (const std::string&)> isValidCert;
    void updateSecureIcon();
    void showSecureIcon(bool show, bool secure);
#endif

    void setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);
    void setCurrentFavIcon();
    void setSearchIcon();
    void setDocIcon();
    IconType getCurrentIconTyep();
    /**
     * \brief Adds Action to URI bar.
     *
     * All Actions will be displayed before URI entry.
     */
    void AddAction(sharedAction action);

    /**
     * \brief returns list of stored actions
     */
    std::list<sharedAction> actions() const;

    /**
     * \brief Sets Focus to URI entry.
     */
    void setFocus();

    /**
     * @brief Remove focus form URI
     */
    void clearFocus();

    /**
     * @brief check if URI is focused
     */
    bool hasFocus() const;

    void setDisabled(bool disabled);

private:
    static void activated(void* data, Evas_Object* obj, void* event_info);
    static void preeditChange(void* data, Evas_Object* obj, void* event_info);
    static void focused(void* data, Evas_Object* obj, void* event_info);
    static void unfocused(void* data, Evas_Object* obj, void* event_info);

    void editingCompleted();
    // TODO This method should be removed when input events will be fixed
    void selectionTool();
    void setUrlGuideText(const char* txt) const;

    /**
     * \brief Rewrites URI to support search and prefixing http:// if needed
     */
    std::string rewriteURI(const std::string& url);

    static void _fixed_entry_key_down_handler(void* data, Evas* e, Evas_Object* obj, void* event_info);
    static void _uri_entry_clicked(void* data, Evas_Object* obj, void* event_info);
    static void _uri_entry_editing_changed_user(void* data, Evas_Object* obj, void* event_info);
    static void _uri_entry_double_clicked(void* data, Evas_Object* obj, void* event_info);
    static void _uri_entry_selection_changed(void* data, Evas_Object* obj, void* event_info);
    static void _uri_entry_longpressed(void* data, Evas_Object* obj, void* event_info);
#if PROFILE_MOBILE
    enum class RightIconType {
        NONE,
        CANCEL,
        SECURE
    };

    static void _uri_right_icon_clicked(void* data, Evas_Object*, const char*, const char*);
    void showCancelIcon();
#endif

private:
    Evas_Object* m_parent;
    IconType m_currentIconType;
    std::list<sharedAction> m_actions;
    Evas_Object* m_entry;
    Evas_Object* m_favicon;
    Evas_Object* m_entry_layout;
    SelectionState m_entrySelectionState;
    std::string m_pageTitle;
    std::string m_URI;
    bool m_entryContextMenuOpen;
    bool m_searchTextEntered;
    bool m_first_click;
#if PROFILE_MOBILE
    RightIconType m_rightIconType;
    bool m_securePageIcon;
    bool m_showSecureIcon;
#endif
};


}
}

#endif // URIENTRY_H
