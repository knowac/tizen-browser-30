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

#ifndef WEBPAGEUI_H
#define WEBPAGEUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include "AbstractService.h"
#include "AbstractUIComponent.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "ButtonBar.h"
#include "URIEntry.h"

namespace tizen_browser {
namespace base_ui {

class WebPageUIStatesManager;
enum class WPUState;
typedef std::shared_ptr<WebPageUIStatesManager> WPUStatesManagerPtr;
typedef std::shared_ptr<const WebPageUIStatesManager> WPUStatesManagerPtrConst;
class UrlHistoryList;
typedef std::shared_ptr<UrlHistoryList> UrlHistoryPtr;

class BROWSER_EXPORT WebPageUI
        : public tizen_browser::core::AbstractService
        , public tizen_browser::interfaces::AbstractUIComponent
#if PROFILE_MOBILE
        , public tizen_browser::interfaces::AbstractRotatable
#endif
{
public:
    WebPageUI();
    virtual ~WebPageUI();
    virtual std::string getName();
    virtual void init(Evas_Object* parent);
    virtual Evas_Object* getContent();
    UrlHistoryPtr getUrlHistoryList();
    virtual void showUI();
    virtual void hideUI();
#if DUMMY_BUTTON
    void createDummyButton();
#endif
#if PROFILE_MOBILE
    virtual void orientationChanged() override;
    void fullscreenModeSet(bool state);
#endif
    void loadStarted();
    void progressChanged(double progress);
    void loadFinished();
    WPUStatesManagerPtrConst getStatesMgr() {return m_statesMgr;}
    /**
     * @param state The state to compare
     * @returns True if manager's state equals to given state
     */
    bool stateEquals(WPUState state) const;
    /**
     * @param states The states to compare
     * @returns True if one of the given states equals to the manager's state
     */
    bool stateEquals(std::initializer_list<WPUState> states) const;
    bool isWebPageUIvisible() { return m_WebPageUIvisible; }
    void toIncognito(bool);
    void switchViewToErrorPage();
    void switchViewToWebPage(Evas_Object* content, const std::string uri);
    void switchViewToIncognitoPage();
    void switchViewToQuickAccess(Evas_Object* content);
    URIEntry& getURIEntry() const { return *m_URIEntry.get(); }
    void setTabsNumber(int tabs);
    void setBackButtonEnabled(bool enabled) { m_back->setEnabled(enabled); }
    void setForwardButtonEnabled(bool enabled) { m_forward->setEnabled(enabled); }
    void setReloadButtonEnabled(bool enabled) { m_reload->setEnabled(enabled); }
    void setStopButtonEnabled(bool enabled) { m_stopLoading->setEnabled(enabled); }
    void setMoreMenuButtonEnabled(bool enabled) { m_showMoreMenu->setEnabled(enabled); }
    void lockWebview();
    void lockUrlHistoryList();
    void unlockUrlHistoryList();
    void setFocusOnSuspend();
#if PROFILE_MOBILE
    void mobileEntryFocused();
    void mobileEntryUnfocused();
    void setContentFocus();
    static Eina_Bool _hideDelay(void *data);
#else
    void onRedKeyPressed();
    void onYellowKeyPressed();
#endif

    boost::signals2::signal<void ()> backPage;
    boost::signals2::signal<void ()> forwardPage;
    boost::signals2::signal<void ()> stopLoadingPage;
    boost::signals2::signal<void ()> reloadPage;
    boost::signals2::signal<void ()> showTabUI;
#if PROFILE_MOBILE
    boost::signals2::signal<void ()> updateManualRotation;
    boost::signals2::signal<void ()> hideMoreMenu;
    boost::signals2::signal<void ()> qaOrientationChanged;
#else
    boost::signals2::signal<void ()> showZoomNavigation;
#endif
    boost::signals2::signal<void ()> showMoreMenu;
    boost::signals2::signal<void ()> hideQuickAccess;
    boost::signals2::signal<void ()> showQuickAccess;
    boost::signals2::signal<void ()> bookmarkManagerClicked;
    boost::signals2::signal<void ()> focusWebView;
    boost::signals2::signal<void ()> unfocusWebView;

private:
    static void faviconClicked(void* data, Evas_Object* obj, const char* emission, const char* source);
    static Eina_Bool _cb_down_pressed_on_urlbar(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info);
    static void _bookmark_manager_clicked(void * data, Evas_Object *, void *);
#if DUMMY_BUTTON
    static void _dummy_button_focused(void *data, Evas_Object *, void *);
    static void _dummy_button_unfocused(void *data, Evas_Object *, void *);
#endif
#if PROFILE_MOBILE
    static void _more_menu_background_clicked(void* data, Evas_Object*, const char*, const char*);
    static void _content_clicked(void * data, Evas_Object *, void *);
#endif
#if PROFILE_MOBILE && GESTURE
    static Evas_Event_Flags _gesture_move(void *data, void *event_info);
#endif

    void createLayout();
    void createErrorLayout();
    void createPrivateLayout();
    void createActions();
    void connectActions();
    void showProgressBar();
    void hideProgressBar();
    void hideFindOnPage();
    void hideWebView();
    void setErrorButtons();
    void setPrivateButtons();
    void setMainContent(Evas_Object* content);
    void updateURIBar(const std::string& uri);
    std::string edjePath(const std::string& file);
#if !PROFILE_MOBILE
    void refreshFocusChain();
#endif
#if PROFILE_MOBILE && GESTURE
    void gestureUp();
    void gestureDown();
#endif

    // wrappers to call singal as a reaction to other signal
    void backPageConnect() { backPage(); }
    void forwardPageConnect() { forwardPage(); }
    void stopLoadingPageConnect() { stopLoadingPage(); }
    void reloadPageConnect() { reloadPage(); }
    void showTabUIConnect();
    void showMoreMenuConnect();

    Evas_Object* m_parent;
    Evas_Object* m_mainLayout;
#if DUMMY_BUTTON
    Evas_Object* m_dummy_button;
#endif
    Evas_Object* m_errorLayout;
    Evas_Object* m_privateLayout;
    Evas_Object* m_bookmarkManagerButton;

    std::unique_ptr<ButtonBar> m_leftButtonBar;
    std::unique_ptr<ButtonBar> m_rightButtonBar;
    std::unique_ptr<URIEntry> m_URIEntry;
    WPUStatesManagerPtr m_statesMgr;
    UrlHistoryPtr m_urlHistoryList;
    bool m_webviewLocked;
    bool m_WebPageUIvisible;

    sharedAction m_back;
    sharedAction m_forward;
    sharedAction m_stopLoading;
    sharedAction m_reload;
    sharedAction m_tab;
    sharedAction m_showMoreMenu;

#if PROFILE_MOBILE && GESTURE
    Evas_Object* m_gestureLayer;
    static const int SINGLE_FINGER = 1;
    static const int SWIPE_MOMENTUM_TRESHOLD = 400;
#endif
#if PROFILE_MOBILE
    bool m_uriBarHidden;
    bool m_fullscreen;
#endif
};


}   // namespace tizen_browser
}   // namespace base_ui

#endif // WEBPAGEUI_H
