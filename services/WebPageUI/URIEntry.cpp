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
#include <Evas.h>
#include "URIEntry.h"
#include "BrowserLogger.h"
#include "MenuButton.h"
#include <algorithm>
#include <boost/regex.hpp>
#include "BrowserAssert.h"
#include "Tools/URIschemes.h"

namespace tizen_browser {
namespace base_ui {

#define GUIDE_TEXT_FOCUSED "Search or URL"
#if PROFILE_MOBILE
#define GUIDE_TEXT_UNFOCUSED "Search or URL"
#else
#define GUIDE_TEXT_UNFOCUSED "Search or URL - Press [A] to enter"
#endif

const std::string keynameSelect = "Select";
const std::string keynameClear = "Clear";
const std::string keynameKP_Enter = "KP_Enter";
const std::string keynameReturn = "Return";
const std::string keynameEsc = "XF86Back";

URIEntry::URIEntry()
    : m_parent(nullptr)
    , m_currentIconType(IconTypeSearch)
    , m_entry(NULL)
    , m_favicon(0)
    , m_entry_layout(NULL)
    , m_entrySelectionState(SelectionState::SELECTION_NONE)
    , m_entryContextMenuOpen(false)
    , m_searchTextEntered(false)
    , m_first_click(true)
#if PROFILE_MOBILE
    , m_rightIconType(RightIconType::NONE)
#endif
{
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("WebPageUI/URIEntry.edj");
    elm_theme_extension_add(NULL, edjFilePath.c_str());
}

URIEntry::~URIEntry()
{}

void URIEntry::init(Evas_Object* parent)
{
    m_parent = parent;
}

Evas_Object* URIEntry::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    if (!m_entry_layout) {
        m_entry_layout = elm_layout_add(m_parent);
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("WebPageUI/URIEntry.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(m_entry_layout, edjFilePath.c_str(), "uri_entry_layout");
        if (!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

        m_entry = elm_entry_add(m_entry_layout);
        elm_object_style_set(m_entry, "uri_entry");

        elm_entry_single_line_set(m_entry, EINA_TRUE);
        elm_entry_scrollable_set(m_entry, EINA_TRUE);
        elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);
#if PROFILE_MOBILE
        elm_object_signal_callback_add(m_entry_layout,  "right_icon_clicked", "ui", _uri_right_icon_clicked, this);
#endif

        setUrlGuideText(GUIDE_TEXT_UNFOCUSED);

        evas_object_smart_callback_add(m_entry, "activated", URIEntry::activated, this);
        evas_object_smart_callback_add(m_entry, "preedit,changed", URIEntry::preeditChange, this);
        evas_object_smart_callback_add(m_entry, "changed,user", URIEntry::_uri_entry_editing_changed_user, this);
        evas_object_smart_callback_add(m_entry, "focused", URIEntry::focused, this);
        evas_object_smart_callback_add(m_entry, "unfocused", URIEntry::unfocused, this);
        evas_object_smart_callback_add(m_entry, "clicked", _uri_entry_clicked, this);
        evas_object_smart_callback_add(m_entry, "clicked,double", _uri_entry_double_clicked, this);
        evas_object_smart_callback_add(m_entry, "selection,changed", _uri_entry_selection_changed, this);
        evas_object_smart_callback_add(m_entry, "longpressed", _uri_entry_longpressed, this);

        evas_object_event_callback_priority_add(m_entry, EVAS_CALLBACK_KEY_DOWN, 2 * EVAS_CALLBACK_PRIORITY_BEFORE, URIEntry::_fixed_entry_key_down_handler, this);

        elm_object_part_content_set(m_entry_layout, "uri_entry_swallow", m_entry);
    }
    return m_entry_layout;
}

Evas_Object* URIEntry::getEntryWidget()
{
    return m_entry;
}

void URIEntry::changeUri(const std::string& newUri)
{
    BROWSER_LOGD("%s: newUri=%s", __func__, newUri.c_str());
    m_URI = newUri;
    if (elm_object_focus_get(m_entry) == EINA_FALSE) {
        if (!m_URI.empty()) {
            elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(m_URI.c_str()));
    #if PROFILE_MOBILE
            m_rightIconType = RightIconType::NONE;
    #endif
        } else
            elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(""));
    }
#if PROFILE_MOBILE
    updateSecureIcon();
#endif
}

void URIEntry::setFavIcon(std::shared_ptr< tizen_browser::tools::BrowserImage > favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (favicon->getSize() > 0) {
        m_favicon = favicon->getEvasImage(m_entry_layout);
        evas_object_image_fill_set(m_favicon, 0, 0, 36, 36);
        evas_object_resize(m_favicon, 36, 36);
        elm_object_part_content_set(m_entry_layout, "fav_icon", m_favicon);
        setCurrentFavIcon();
    } else {
        setDocIcon();
    }
}

void URIEntry::setCurrentFavIcon()
{
    m_currentIconType = IconTypeFav;
    elm_object_signal_emit(m_entry_layout, "show_favicon", "model");
}

void URIEntry::setSearchIcon()
{
    m_currentIconType = IconTypeSearch;
    elm_object_signal_emit(m_entry_layout, "set_search_icon", "model");
}

void URIEntry::setDocIcon()
{
    m_currentIconType = IconTypeDoc;
    elm_object_signal_emit(m_entry_layout, "set_doc_icon", "model");
}

URIEntry::IconType URIEntry::getCurrentIconTyep()
{
    return m_currentIconType;
}

void URIEntry::selectionTool()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_entrySelectionState == SelectionState::SELECTION_KEEP) {
        m_entrySelectionState = SelectionState::SELECTION_NONE;
    } else {
        elm_entry_select_none(m_entry);
    }
}

void URIEntry::_uri_entry_clicked(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);
#if PROFILE_MOBILE
    self->showCancelIcon();
#endif
    // TODO This line should be uncommented when input events will be fixed
//    elm_entry_select_none(self->m_entry);
    self->selectionTool();
}

void URIEntry::activated(void* /* data */, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void URIEntry::preeditChange(void* /* data */, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void URIEntry::_uri_entry_editing_changed_user(void* data, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = reinterpret_cast<URIEntry*>(data);
    std::string entry(elm_entry_markup_to_utf8(elm_entry_entry_get(self->m_entry)));
    if ((entry.find("http://") == 0)
            || (entry.find("https://") == 0)
            || (entry.find(".") != std::string::npos)) {
        self->setDocIcon();
    } else {
        self->setSearchIcon();
    }
#if PROFILE_MOBILE
    self->showCancelIcon();
#endif
    if(entry.find(" ") != std::string::npos)
        return;
    self->uriEntryEditingChangedByUser(std::make_shared<std::string>(entry));
}

void URIEntry::setUrlGuideText(const char* txt) const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_translatable_part_text_set(m_entry, "elm.guide", txt);
}

void URIEntry::unfocused(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);

    if (!self->m_entryContextMenuOpen) {
        self->m_entrySelectionState = SelectionState::SELECTION_NONE;
#if PROFILE_MOBILE
        self->mobileEntryUnfocused();
        self->showCancelIcon();
#endif
    }
    self->m_first_click = true;
    elm_entry_select_none(self->m_entry);

#if PROFILE_MOBILE
    self->showSecureIcon(self->m_showSecureIcon, self->m_securePageIcon);
#endif

    elm_entry_entry_set(self->m_entry, elm_entry_utf8_to_markup(self->m_URI.c_str()));
}

void URIEntry::focused(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);
    if (!self->m_entryContextMenuOpen) {
#if PROFILE_MOBILE
        self->mobileEntryFocused();
#endif
    } else {
        self->m_entryContextMenuOpen = false;
    }
    if(self->m_first_click) {
        elm_entry_select_all(self->m_entry);
        self->m_first_click = false;
        self->m_entrySelectionState = SelectionState::SELECTION_NONE;
    }
}

void URIEntry::_fixed_entry_key_down_handler(void* data, Evas* /*e*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("%s", __func__);
    Evas_Event_Key_Down* ev = static_cast<Evas_Event_Key_Down*>(event_info);
    if (!data || !ev || !ev->keyname)
        return;
    URIEntry* self = static_cast<URIEntry*>(data);

    if (keynameClear == ev->keyname) {
        elm_entry_entry_set(self->m_entry, "");
        return;
    }
    if (keynameSelect == ev->keyname
            || keynameReturn == ev->keyname
            || keynameKP_Enter == ev->keyname) {
        self->editingCompleted();
        return;
    }
    if (keynameEsc == ev->keyname) {
#if !PROFILE_MOBILE
        elm_object_focus_set(self->m_entry, EINA_TRUE);
#endif
        return;
    }
}

void URIEntry::editingCompleted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char* text = elm_entry_markup_to_utf8(elm_entry_entry_get(m_entry));
    std::string userString(text);
    free(text);

    elm_entry_input_panel_hide(m_entry);
    m_URI = rewriteURI(userString);
    uriChanged(m_URI);
#if !PROFILE_MOBILE
    elm_object_focus_set(m_entry, EINA_TRUE);
#else
    showSecureIcon(false, false);
#endif
}

std::string URIEntry::rewriteURI(const std::string& url)
{
    BROWSER_LOGD("%s: %s", __PRETTY_FUNCTION__, url.c_str());
    boost::regex urlRegex(R"(^(https?|ftp)://[^\s/$.?#].[^\s]*$)");
    boost::regex fileRegex(R"(^file:///[^\s]*$)");

    if (!url.empty() && url != "about:blank" && url != "about:home") {
        if (boost::regex_match(url, urlRegex) || boost::regex_match(url, fileRegex))
            return url;
        else if (boost::regex_match(std::string("http://") + url, urlRegex) &&  url.find(".") != std::string::npos)
            return std::string("http://") + url;
        else {
            std::string searchString("http://www.google.com/search?q=");
            searchString += url;
            std::replace(searchString.begin(), searchString.end(), ' ', '+');
            BROWSER_LOGD("[%s:%d] Search string: %s", __PRETTY_FUNCTION__, __LINE__, searchString.c_str());
            return searchString;
        }
    }

    return url;
}

void URIEntry::AddAction(sharedAction action)
{
    m_actions.push_back(action);
}

std::list<sharedAction> URIEntry::actions() const
{
    return m_actions;
}

void URIEntry::clearFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_focus_set(m_entry, EINA_FALSE);
}

void URIEntry::setFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if !PROFILE_MOBILE
    elm_object_focus_set(m_entry, EINA_TRUE);
#endif
}

bool URIEntry::hasFocus() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return elm_object_focus_get(m_entry) == EINA_TRUE ? true : false;
}

void URIEntry::setDisabled(bool disabled)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (disabled) {
        clearFocus();
    }
    elm_object_disabled_set(getContent(), disabled ? EINA_TRUE : EINA_FALSE);
}

void URIEntry::_uri_entry_double_clicked(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);
    elm_entry_select_all(self->m_entry);
}

void URIEntry::_uri_entry_selection_changed(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);
    self->m_entrySelectionState = SelectionState::SELECTION_KEEP;
}

void URIEntry::_uri_entry_longpressed(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);
    self->m_entryContextMenuOpen = true;
    self->m_entrySelectionState = SelectionState::SELECTION_KEEP;

}

#if PROFILE_MOBILE
void URIEntry::_uri_right_icon_clicked(void* data, Evas_Object* /*obj*/, const char* /*emission*/, const char* /*source*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = static_cast<URIEntry*>(data);
    switch (self->m_rightIconType) {
    case RightIconType::CANCEL:
        elm_entry_entry_set(self->m_entry, "");
        elm_object_signal_emit(self->m_entry_layout, "hide_icon", "ui");
        break;
    case RightIconType::SECURE:
        self->secureIconClicked();
        break;
    default:
        BROWSER_LOGW("[%s:%d] Unknown icon type!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void URIEntry::showCancelIcon()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_rightIconType = RightIconType::CANCEL;
    bool isEntryEmpty = elm_entry_is_empty(m_entry);
    if(!isEntryEmpty)
        elm_object_signal_emit(m_entry_layout, "show_cancel_icon", "ui");
    else
        elm_object_signal_emit(m_entry_layout, "hide_icon", "ui");
}

void URIEntry::updateSecureIcon()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bool show = false, secure = false;
    if (m_URI.find(HTTPS_SCHEME) == 0) {
        show = true;
        auto valid = isValidCert(m_URI);
        if (valid)
            secure = *valid;
        else
            BROWSER_LOGW("[%s:%d] Wrong isValidCert result!", __PRETTY_FUNCTION__, __LINE__);
    }
    showSecureIcon(show, secure);
}

void URIEntry::showSecureIcon(bool show, bool secure)
{
    BROWSER_LOGD("[%s:%d] [ show : %d, secure : %d] ", __PRETTY_FUNCTION__, __LINE__, show, secure);

    m_rightIconType = RightIconType::SECURE;
    m_securePageIcon = secure;
    m_showSecureIcon = show;
    if (show) {
        if (secure)
            elm_object_signal_emit(m_entry_layout, "show,secure,icon", "");
        else
            elm_object_signal_emit(m_entry_layout, "show,unsecure,icon", "");
    }
    else {
        elm_object_signal_emit(m_entry_layout, "hide_icon", "ui");
    }
}
#endif

}
}
