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

#include <Elementary.h>
#include <memory>
#include <boost/format.hpp>
#include "app_i18n.h"
#include "WebPageUI.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "BrowserAssert.h"
#include "UrlHistoryList/UrlHistoryList.h"
#include "WebPageUIStatesManager.h"
#include <string>
#include <app_control.h>

namespace tizen_browser {
namespace base_ui {

EXPORT_SERVICE(WebPageUI, "org.tizen.browser.webpageui")

#define MAX_SIGNAL_NAME 32
#define MAX_PROGRESS_LEVEL 20
#define PROGRESS_STEP 0.05

WebPageUI::WebPageUI()
    : m_parent(nullptr)
    , m_mainLayout(nullptr)
    , m_errorLayout(nullptr)
    , m_privateLayout(nullptr)
    , m_bookmarkManagerButton(nullptr)
    , m_statesMgr(std::make_shared<WebPageUIStatesManager>(WPUState::MAIN_WEB_PAGE))
    , m_URIEntry(new URIEntry(m_statesMgr))
    , m_urlHistoryList(std::make_shared<UrlHistoryList>(getStatesMgr()))
    , m_webviewLocked(false)
    , m_WebPageUIvisible(false)
    , m_pwaInfo(nullptr)
#if GESTURE
    , m_gestureLayer(nullptr)
#endif
    , m_uriBarHidden(false)
    , m_fullscreen(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

WebPageUI::~WebPageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void WebPageUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* WebPageUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_mainLayout) {
        createLayout();
    }
    return m_mainLayout;
}

UrlHistoryPtr WebPageUI::getUrlHistoryList()
{
    return m_urlHistoryList;
}

void WebPageUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto state = getEngineState();
    if (state) {
        switch (*state) {
            case basic_webengine::State::NORMAL:
                elm_object_signal_emit(m_mainLayout, "set_normal_mode", "ui");
                break;
            case basic_webengine::State::SECRET:
                elm_object_signal_emit(m_mainLayout, "set_secret_mode", "ui");
                break;
            default:
                BROWSER_LOGE("[%s:%d] Unknown state!", __PRETTY_FUNCTION__, __LINE__);
        }
        m_bottomButtonBar->setButtonsColor(*state == basic_webengine::State::SECRET);
        m_rightButtonBar->setButtonsColor(*state == basic_webengine::State::SECRET);
    } else {
        BROWSER_LOGE("[%s:%d] Wrong state value!", __PRETTY_FUNCTION__, __LINE__);
    }

    M_ASSERT(m_mainLayout);
    evas_object_show(m_mainLayout);

    evas_object_show(elm_object_part_content_get(m_mainLayout, "web_view"));

    evas_object_show(m_URIEntry->getContent());
    evas_object_show(elm_object_part_content_get(m_mainLayout, "bottom_toolbar"));
    evas_object_show(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_right"));

    if (m_statesMgr->equals(WPUState::QUICK_ACCESS)) {
        elm_object_signal_emit(m_mainLayout, "shiftback_uri", "ui");
        showQuickAccess();
    }

    m_WebPageUIvisible = true;

    elm_object_event_callback_add(m_bottomButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_add(m_rightButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_add(m_URIEntry->getContent(), _cb_down_pressed_on_urlbar, this);
#if GESTURE
    elm_gesture_layer_cb_add(m_gestureLayer, ELM_GESTURE_N_LINES, ELM_GESTURE_STATE_MOVE, _gesture_move, this);
    elm_gesture_layer_line_min_length_set(m_gestureLayer, SWIPE_MOMENTUM_TRESHOLD);
    elm_gesture_layer_line_distance_tolerance_set(m_gestureLayer, SWIPE_MOMENTUM_TRESHOLD);
#endif
}


void WebPageUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_mainLayout);
    m_URIEntry->loadFinished();
    evas_object_hide(m_mainLayout);

    if(m_statesMgr->equals(WPUState::QUICK_ACCESS))
        hideQuickAccess();

    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    evas_object_hide(m_URIEntry->getContent());
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "bottom_toolbar"));
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_right"));

    m_WebPageUIvisible = false;

    elm_object_event_callback_del(m_bottomButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_del(m_rightButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_del(m_URIEntry->getContent(), _cb_down_pressed_on_urlbar, this);
#if GESTURE
    elm_gesture_layer_cb_del(m_gestureLayer, ELM_GESTURE_N_LINES, ELM_GESTURE_STATE_MOVE, _gesture_move, this);
#endif
    hideFindOnPage();
}

void WebPageUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    showProgressBar();
    m_URIEntry->loadStarted();
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
}

void WebPageUI::progressChanged(double progress)
{
    if (progress == 1.0) {
        hideProgressBar();
    } else {
        int level = (int)(progress * MAX_PROGRESS_LEVEL);
        char signal_name[MAX_SIGNAL_NAME] = { 0 };
        snprintf(signal_name, MAX_SIGNAL_NAME, "update,progress,%.02f,signal", ((double)level * PROGRESS_STEP));
        elm_object_signal_emit(m_mainLayout, signal_name, "");
    }
}

bool WebPageUI::stateEquals(WPUState state) const
{
    return m_statesMgr->equals(state);
}

bool WebPageUI::stateEquals(std::initializer_list<WPUState> states) const
{
    return m_statesMgr->equals(states);
}

void WebPageUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_URIEntry->loadFinished();
    hideProgressBar();
    m_URIEntry->updateSecureIcon();
}

void WebPageUI::toIncognito(bool incognito)
{
    BROWSER_LOGD("[%s:%d,%d] ", __PRETTY_FUNCTION__, __LINE__, incognito);
    if (incognito) {
        elm_object_signal_emit(m_mainLayout, "incognito,true", "ui");
    }
    else {
        elm_object_signal_emit(m_mainLayout, "incognito,false", "ui");
    }
}

void WebPageUI::setMainContent(Evas_Object* content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(content);
    hideWebView();
    elm_object_part_content_set(m_mainLayout, "web_view", content);
#if GESTURE
    elm_gesture_layer_attach(m_gestureLayer, content);
#endif
    evas_object_smart_callback_add(content, "mouse,down", _content_clicked, this);
    evas_object_show(content);
}

void WebPageUI::switchViewToErrorPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_statesMgr->set(WPUState::MAIN_ERROR_PAGE);
    if (!m_errorLayout)
        createErrorLayout();
    setMainContent(m_errorLayout);
    evas_object_show(m_bottomButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    setErrorButtons();
}

void WebPageUI::switchViewToIncognitoPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_statesMgr->set(WPUState::MAIN_INCOGNITO_PAGE);
    toIncognito(true);
    if (!m_privateLayout)
        createPrivateLayout();
    setMainContent(m_privateLayout);
    orientationChanged();
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    setPrivateButtons();
    m_URIEntry->changeUri("");
}

void WebPageUI::switchViewToWebPage(Evas_Object* content, const std::string uri, bool loading)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_statesMgr->equals(WPUState::QUICK_ACCESS))
    {
        hideQuickAccess();
        m_statesMgr->set(WPUState::MAIN_WEB_PAGE);
    }
    setMainContent(content);
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    updateURIBar(uri, loading);
}

void WebPageUI::switchViewToQuickAccess(Evas_Object* content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_statesMgr->set(WPUState::QUICK_ACCESS);
    toIncognito(false);
    setMainContent(content);
    elm_object_signal_emit(m_mainLayout, "shiftback_uri", "ui");
    hideProgressBar();
    m_URIEntry->changeUri("");
    m_URIEntry->showSecureIcon(false, false);
    showQuickAccess();
}

void WebPageUI::faviconClicked(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI* self = reinterpret_cast<WebPageUI*>(data);
    if(!self->stateEquals({ WPUState::QUICK_ACCESS, WPUState::MAIN_ERROR_PAGE })) {
        self->getURIEntry().clearFocus();
    }
}

Eina_Bool WebPageUI::_cb_down_pressed_on_urlbar(void *data, Evas_Object */*obj*/, Evas_Object */*src*/, Evas_Callback_Type type, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI* self = reinterpret_cast<WebPageUI*>(data);
    if(type == EVAS_CALLBACK_KEY_DOWN) {
        Ecore_Event_Key *ev = static_cast<Ecore_Event_Key *>(event_info);
        const std::string keyName = ev->keyname;
        if(!keyName.compare("Down")){
            self->lockWebview();
        }
    }
    return EINA_FALSE;
}

void WebPageUI::setTabsNumber(int tabs)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (tabs == 0) {
        elm_object_part_text_set(m_bottomButtonBar->getContent(), "tabs_number", "");
    } else {
        elm_object_part_text_set(m_bottomButtonBar->getContent(), "tabs_number", (boost::format("%1%") % tabs).str().c_str());
    }
}

void WebPageUI::lockWebview()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(isWebPageUIvisible()) {
        if(m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
            elm_object_focus_custom_chain_unset(m_mainLayout);
            elm_object_focus_custom_chain_append(m_mainLayout, elm_object_part_content_get(m_mainLayout, "web_view"), NULL);
            m_webviewLocked = true;
        }
    }
}

void WebPageUI::lockUrlHistoryList()
{
    elm_object_focus_custom_chain_unset(m_mainLayout);
    elm_object_focus_custom_chain_append(m_mainLayout,
            getUrlHistoryList()->getLayout(), NULL);
    getUrlHistoryList()->listWidgetFocusedFromUri();
    elm_object_focus_set(getUrlHistoryList()->getLayout(), EINA_TRUE);
}

void WebPageUI::unlockUrlHistoryList()
{
    elm_object_focus_set(m_URIEntry->getEntryWidget(), EINA_TRUE);
    getUrlHistoryList()->onListWidgetFocusChange(false);
}

void WebPageUI::setFocusOnSuspend()
{
    elm_object_focus_set(m_rightButtonBar->getButton("tab_button"), EINA_TRUE);
}

void WebPageUI::fullscreenModeSet(bool state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto landscape = isLandscape();
    m_fullscreen = state;
    if (!state)
        elm_object_signal_emit(m_mainLayout, "show_uri_bar", "ui");
    else if (landscape && state) {
        (*landscape) ?
            elm_object_signal_emit(m_mainLayout, "hide_uri_bar_landscape", "ui") :
            elm_object_signal_emit(m_mainLayout, "hide_uri_bar_vertical", "ui");
    }
    showBottomBar(!state);
}

void WebPageUI::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto landscape = isLandscape();

    if (landscape) {
        if (*landscape) {
            elm_object_signal_emit(m_privateLayout, "show_incognito_landscape", "ui");
            elm_object_signal_emit(m_bottomButtonBar->getContent(), "landscape,mode", "");
            if (m_uriBarHidden)
                elm_object_signal_emit(m_mainLayout, "hide_uri_bar_landscape", "ui");
        } else {
            elm_object_signal_emit(m_privateLayout, "show_incognito_vertical", "ui");
            elm_object_signal_emit(m_bottomButtonBar->getContent(), "portrait,mode", "");
            if (m_uriBarHidden)
                elm_object_signal_emit(m_mainLayout, "hide_uri_bar_vertical", "ui");
        }
    }
    else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);

    if (m_statesMgr->equals(WPUState::QUICK_ACCESS)) {
        qaOrientationChanged();
    }
}

void WebPageUI::showContextMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    boost::optional<Evas_Object*> window = getWindow();
    if (window) {
        createContextMenu(*window);

        if (m_statesMgr->equals(WPUState::QUICK_ACCESS)) {
            //TODO: Add translation
            elm_ctxpopup_item_append(m_ctxpopup, "Edit Quick access", nullptr, _cm_edit_qa_clicked, this);
        } else if (m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_SHARE"), nullptr, _cm_share_clicked, this);
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_FIND_ON_PAGE"), nullptr, _cm_find_on_page_clicked, this);

            boost::optional<bool> bookmark = isBookmark();
            if (bookmark) {
                //TODO: Add translation
                if (*bookmark)
                    elm_ctxpopup_item_append(m_ctxpopup, "Remove from bookmarks", nullptr,
                        _cm_delete_bookmark_clicked, this);
                else
                    elm_ctxpopup_item_append(m_ctxpopup, "Add to Bookmarks", nullptr,
                        _cm_bookmark_flow_clicked, this);
            } else
                BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);

            //TODO: "dont add this item if it is already in a quick access
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_ADD_TO_QUICK_ACCESS"), nullptr, _cm_add_to_qa_clicked, this);

            if (!getDesktopMode())
                elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_BODY_DESKTOP_VIEW"), nullptr, _cm_desktop_view_page_clicked, this);
            else
                elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_BODY_MOBILE_VIEW"), nullptr, _cm_mobile_view_page_clicked, this);
        } else {
            BROWSER_LOGW("[%s] State not handled, context menu not shown", __PRETTY_FUNCTION__);
            return;
        }

        elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_BODY_SETTINGS"), nullptr, _cm_settings_clicked, this);
#if PWA
        elm_ctxpopup_item_append(m_ctxpopup, "Add to Homescreen", nullptr, _cm_add_to_hs_clicked, this);
#endif
        alignContextMenu(*window);
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void WebPageUI::_cm_edit_qa_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->quickAccessEdit();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_share_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);

    std::string uri = webPageUI->getURI();
    webPageUI->launch_share(uri.c_str());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_find_on_page_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->showFindOnPageUI();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_delete_bookmark_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->deleteBookmark();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_bookmark_flow_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->showBookmarkFlowUI();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_add_to_qa_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->addToQuickAccess();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_desktop_view_page_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->switchToDesktopMode();
        webPageUI->setDesktopMode(true);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_mobile_view_page_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->switchToMobileMode();
        webPageUI->setDesktopMode(false);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void WebPageUI::_cm_settings_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);
        webPageUI->showSettingsUI();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}
#if PWA
void WebPageUI::_cm_add_to_hs_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (data) {
        WebPageUI* webPageUI = static_cast<WebPageUI*>(data);
        webPageUI->m_pwaInfo = std::make_shared<pwaInfo>();
        _cm_dismissed(nullptr, webPageUI->m_ctxpopup, nullptr);

        // send request API.
        pwaRequestManifest();
    }
    else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}
#endif

std::string WebPageUI::getURI() {
    auto retVal = requestCurrentPageForWebPageUI();
    if(retVal && !(*retVal).empty()) {
        return *retVal;
    } else {
        return " ";
    }
}

void WebPageUI::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    // set white base background for button
    edje_color_class_set("elm/widget/button/default/bg-default", 255, 255, 255, 255,
                    255, 255, 255, 255,
                    255, 255, 255, 255);
    edje_color_class_set("elm/widget/button/default/bg-disabled", 255, 255, 255, 255,
                        255, 255, 255, 255,
                        255, 255, 255, 255);

    // create web layout
    m_mainLayout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_file_set(m_mainLayout, edjePath("WebPageUI/WebPageUI.edj").c_str(), "main_layout");

    createActions();

    // bottom buttons
    m_bottomButtonBar = std::unique_ptr<ButtonBar>(new ButtonBar(m_mainLayout, "WebPageUI/BottomButtonBar.edj", "bottom_button_bar"));
    m_bottomButtonBar->addAction(m_back, "prev_button");
    m_bottomButtonBar->addAction(m_forward, "next_button");
    m_bottomButtonBar->addAction(m_homePage, "home_button");
    m_bottomButtonBar->addAction(m_bookmarks, "bookmarks_button");
    m_bottomButtonBar->addAction(m_tabs, "tabs_button");

    // right buttons
    m_rightButtonBar = std::unique_ptr<ButtonBar>(new ButtonBar(m_mainLayout, "WebPageUI/RightButtonBar.edj", "right_button_bar"));
    m_rightButtonBar->addAction(m_addTab, "tab_button");

    //URL bar (Evas Object is shipped by URIEntry object)
    m_URIEntry->init(m_mainLayout);
    elm_object_part_content_set(m_mainLayout, "uri_entry", m_URIEntry->getContent());
    elm_object_part_content_set(m_mainLayout, "bottom_swallow", m_bottomButtonBar->getContent());
    elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_right", m_rightButtonBar->getContent());

    elm_layout_signal_callback_add(m_URIEntry->getContent(), "slide_websearch", "elm", faviconClicked, this);

//    elm_theme_extension_add(nullptr, edjePath("WebPageUI/UrlHistoryList.edj").c_str());
//    m_urlHistoryList->setMembers(m_mainLayout, m_URIEntry->getEntryWidget());
//    elm_object_part_content_set(m_mainLayout, "url_history_list", m_urlHistoryList->getLayout());

    connectActions();

#if GESTURE
    // will be attatch on every 'setMainContent'
    m_gestureLayer = elm_gesture_layer_add(m_mainLayout);
#endif
}

void WebPageUI::createErrorLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_errorLayout =  elm_layout_add(m_mainLayout);
    evas_object_size_hint_weight_set(m_errorLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_errorLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_errorLayout, edjePath("WebPageUI/ErrorMessage.edj").c_str(), "error_message");
}

void WebPageUI::createPrivateLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_privateLayout =  elm_layout_add(m_mainLayout);
    evas_object_size_hint_weight_set(m_privateLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_privateLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_privateLayout, edjePath("WebPageUI/PrivateMode.edj").c_str(), "inco_message");

    m_bookmarkManagerButton = elm_button_add(m_privateLayout);
    elm_object_style_set(m_bookmarkManagerButton, "invisible_button");
    evas_object_smart_callback_add(m_bookmarkManagerButton, "clicked", _bookmark_manager_clicked, this);
    evas_object_show(m_bookmarkManagerButton);

    elm_object_part_content_set(m_privateLayout, "bookmarkmanager_click", m_bookmarkManagerButton);
}

void WebPageUI::_bookmark_manager_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI*  webpageUI = static_cast<WebPageUI*>(data);
    webpageUI->bookmarkManagerClicked();
}

void WebPageUI::setContentFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (getURIEntry().hasFocus())
        getURIEntry().clearFocus();
}

void WebPageUI::showBottomBar(bool isShown)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, isShown);
    if (m_fullscreen) {
        elm_object_signal_emit(m_mainLayout, "hide,bottom", "");
        return;
    }
    if (isShown)
        elm_object_signal_emit(m_mainLayout, "show,bottom", "");
    else
        elm_object_signal_emit(m_mainLayout, "hide,bottom", "");
}

void WebPageUI::_content_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI*  webpageUI = static_cast<WebPageUI*>(data);
    webpageUI->setContentFocus();
}

void WebPageUI::createActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_back = sharedAction(new Action(_("IDS_BR_BUTTON_BACK_ABB")));
    m_back->setIcon("toolbar_prev");

    m_forward = sharedAction(new Action(_("IDS_BR_SK_NEXT")));
    m_forward->setIcon("toolbar_next");

    m_addTab = sharedAction(new Action("New tab"));
    m_addTab->setIcon("add_tab");

    m_homePage = sharedAction(new Action("Home"));
    m_homePage->setIcon("toolbar_home");

    m_bookmarks = sharedAction(new Action(_("IDS_BR_BODY_BOOKMARKS")));
    m_bookmarks->setIcon("toolbar_bookmark");

    m_tabs = sharedAction(new Action(_("IDS_BR_SK_TABS")));
    m_tabs->setIcon("toolbar_tabs");
}

void WebPageUI::connectActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //bottom bar
    m_back->triggered.connect(boost::bind(&WebPageUI::backPageConnect, this));
    m_forward->triggered.connect(boost::bind(&WebPageUI::forwardPageConnect, this));
    m_tabs->triggered.connect(WebPageUI::showTabUI);
    m_bookmarks->triggered.connect(WebPageUI::showBookmarksUI);
    m_homePage->triggered.connect(WebPageUI::showHomePage);

    //right bar
    m_addTab->triggered.connect(boost::bind(&WebPageUI::addNewTabConnect, this));
}

void WebPageUI::showProgressBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "show,progress,signal", "");
    elm_object_signal_emit(m_mainLayout, "update,progress,0.00,signal", "");
}

void WebPageUI::hideProgressBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "hide,progress,signal", "");
}

void WebPageUI::hideFindOnPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "hide_findonpage", "ui");
}

void WebPageUI::hideWebView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_hide(elm_object_part_content_unset(m_mainLayout, "web_view"));
}

void WebPageUI::setErrorButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_forward->setEnabled(false);
}

void WebPageUI::setPrivateButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_forward->setEnabled(false);
}

void WebPageUI::updateURIBar(const std::string& uri, bool loading)
{
    BROWSER_LOGD("[%s:%d] URI:%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    m_URIEntry->setPageLoading(loading);
    m_URIEntry->changeUri(uri);
    hideProgressBar();
}

std::string WebPageUI::edjePath(const std::string& file)
{
    return std::string(EDJE_DIR) + file;
}

#if GESTURE
Evas_Event_Flags WebPageUI::_gesture_move(void* data , void* event_info)
{
    auto info = static_cast<Elm_Gesture_Line_Info*>(event_info);
    if (info->momentum.n == WebPageUI::SINGLE_FINGER) {
        if ((info->angle > 330 || info->angle < 30) && info->momentum.my < -WebPageUI::SWIPE_MOMENTUM_TRESHOLD) {    // top direction
            auto self = static_cast<WebPageUI*>(data);
            self->gestureUp();
        } else if (info->angle > 150 && info->angle < 210 && info->momentum.my > WebPageUI::SWIPE_MOMENTUM_TRESHOLD) {    // bottom direction
            auto self = static_cast<WebPageUI*>(data);
            self->gestureDown();
        }
    }

    return EVAS_EVENT_FLAG_NONE;
}

void WebPageUI::gestureUp()
{
    if (!m_uriBarHidden) {
        m_uriBarHidden = true;
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

        boost::optional<bool> landscape = isLandscape();

        if (landscape) {
            if (*landscape)
                elm_object_signal_emit(m_mainLayout, "hide_uri_bar_landscape", "ui");
            else
                elm_object_signal_emit(m_mainLayout, "hide_uri_bar_vertical", "ui");
        } else
            BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
    }
}

void WebPageUI::gestureDown()
{
    if (m_uriBarHidden) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        elm_object_signal_emit(m_mainLayout, "show_uri_bar", "ui");
        m_uriBarHidden = false;
    }
}
#endif

void WebPageUI::mobileEntryFocused()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "enlarge_focused_uri", "ui");
}

void WebPageUI::mobileEntryUnfocused()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "decrease_unfocused_uri", "ui");

    // delay hiding on one efl loop iteration to enable genlist item selected callback to come
    ecore_timer_add(0.0, _hideDelay, this);
}

Eina_Bool WebPageUI::_hideDelay(void *data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<WebPageUI*>(data);
    self->m_urlHistoryList->hideWidget();
    return ECORE_CALLBACK_CANCEL;
}

void WebPageUI::setDisplayMode(WebPageUI::WebDisplayMode mode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (mode == WebDisplayMode::WebDisplayModeFullscreen)
        elm_object_signal_emit(m_mainLayout, "webview_fullscreen", "ui");
    else if (mode == WebDisplayMode::WebDisplayModeStandalone)
        BROWSER_LOGD("Not implemented");
    else if (mode == WebDisplayMode::WebDisplayModeMinimalUi)
        BROWSER_LOGD("Not implemented");
    else if (mode == WebDisplayMode::WebDisplayModeBrowser)
        elm_object_signal_emit(m_mainLayout, "webview_default", "ui");
}

void WebPageUI:: launch_share(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGD("Fail to create app_control handle");
        return ;
    }

    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_SHARE_TEXT) < 0) {
        BROWSER_LOGD("Fail to set app_control operation");
        app_control_destroy(app_control);
        return ;
    }

    if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TEXT, uri) < 0) {
        BROWSER_LOGD("Fail to set extra data : APP_CONTROL_DATA_TEXT");
        app_control_destroy(app_control);
        return ;
    }

    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGD("Fail to launch app_control operation");
        app_control_destroy(app_control);
        return ;
    }

    app_control_destroy(app_control);
    return ;
}

}   // namespace tizen_browser
}   // namespace base_ui
