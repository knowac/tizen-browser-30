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
#include "AbstractContextMenu.h"
#include "AbstractService.h"
#include "AbstractUIComponent.h"
#include "AbstractRotatable.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "ButtonBar.h"
#include "URIEntry.h"
#include "AbstractWebEngine/State.h"

namespace tizen_browser {
namespace base_ui {

class WebPageUIStatesManager;
enum class WPUState;
class UrlHistoryList;
typedef std::shared_ptr<UrlHistoryList> UrlHistoryPtr;

class BROWSER_EXPORT WebPageUI
        : public interfaces::AbstractContextMenu
        , public tizen_browser::core::AbstractService
        , public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::interfaces::AbstractRotatable
{
public:
    typedef enum OrientationType {
        portrait_primary = 0,
        portrait_secondary,
        landscape_primary,
        landscape_secondary,
    } orientationType;

    typedef enum WebDisplayMode {
        WebDisplayModeUndefined = 0,
        WebDisplayModeBrowser,
        WebDisplayModeMinimalUi,
        WebDisplayModeStandalone,
        WebDisplayModeFullscreen,
        WebDisplayModeLast = WebDisplayModeFullscreen
    } webDisplayMode;

    struct pwaInfo {
        std::string     id;
        std::string     decodedIcon; // needs to src, type, sizes.
        std::string     uri;
        std::string     name;
        std::string     shortName;
        int             orientation; // needs to portrait-primary, portrait-secondary, landscape-primary, landscape-secondary.
        int             displayMode; // needs to fullscreen, standalone, minimal-ui, browser, and so on.
        long            themeColor;
        long            backgroundColor;
    };

    WebPageUI();
    virtual ~WebPageUI();
    virtual std::string getName();
    virtual void init(Evas_Object* parent);
    virtual Evas_Object* getContent();
    UrlHistoryPtr getUrlHistoryList();
    virtual void showUI();
    virtual void hideUI();
    void fullscreenModeSet(bool state);
    virtual void orientationChanged() override;
    //AbstractContextMenu interface implementation
    virtual void showContextMenu() override;

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
    void switchViewToWebPage(Evas_Object* content, const std::string uri, bool loading);
    void switchViewToIncognitoPage();
    void switchViewToQuickAccess(Evas_Object* content);
    URIEntry& getURIEntry() const { return *m_URIEntry.get(); }
    void setTabsNumber(int tabs);
    void setBackButtonEnabled(bool enabled) { m_back->setEnabled(enabled); }
    void setForwardButtonEnabled(bool enabled) { m_forward->setEnabled(enabled); }
    void lockWebview();
    void lockUrlHistoryList();
    void unlockUrlHistoryList();
    void setFocusOnSuspend();
    void mobileEntryFocused();
    void mobileEntryUnfocused();
    void setContentFocus();
    void showBottomBar(bool isShown);
    static Eina_Bool _hideDelay(void *data);
    void setDesktopMode(bool desktopMode) {m_desktopMode = desktopMode;}
    bool getDesktopMode() { return m_desktopMode; }
    void setDisplayMode(WebDisplayMode mode);

    std::string getURI();

    boost::signals2::signal<void ()> backPage;
    boost::signals2::signal<void ()> forwardPage;
    boost::signals2::signal<void ()> showTabUI;
    boost::signals2::signal<void ()> showBookmarksUI;
    boost::signals2::signal<void ()> showHomePage;
    boost::signals2::signal<void ()> qaOrientationChanged;
    boost::signals2::signal<void ()> hideQuickAccess;
    boost::signals2::signal<void ()> showQuickAccess;
    boost::signals2::signal<void ()> bookmarkManagerClicked;
    boost::signals2::signal<void ()> focusWebView;
    boost::signals2::signal<void ()> unfocusWebView;
    boost::signals2::signal<void ()> addNewTab;

    //AbstractContextMenu signals
    boost::signals2::signal<bool ()> isBookmark;
    boost::signals2::signal<void ()> deleteBookmark;
    boost::signals2::signal<void ()> showBookmarkFlowUI;
    boost::signals2::signal<void ()> showFindOnPageUI;
    boost::signals2::signal<void ()> showSettingsUI;
    boost::signals2::signal<void ()> switchToMobileMode;
    boost::signals2::signal<void ()> switchToDesktopMode;
    boost::signals2::signal<void ()> quickAccessEdit;
    boost::signals2::signal<void ()> addToQuickAccess;

    boost::signals2::signal<std::string ()> requestCurrentPageForWebPageUI;
    boost::signals2::signal<basic_webengine::State ()> getEngineState;
#if PWA
    boost::signals2::signal<void ()> pwaRequestManifest;
#endif

private:
    static void faviconClicked(void* data, Evas_Object* obj, const char* emission, const char* source);
    static Eina_Bool _cb_down_pressed_on_urlbar(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info);
    static void _bookmark_manager_clicked(void * data, Evas_Object *, void *);
    static void _more_menu_background_clicked(void* data, Evas_Object*, const char*, const char*);
    static void _content_clicked(void * data, Evas_Object *, void *);
#if GESTURE
    static Evas_Event_Flags _gesture_move(void *data, void *event_info);
#endif
    static void _cm_edit_qa_clicked(void*, Evas_Object*, void*);
    static void _cm_share_clicked(void*, Evas_Object*, void*);
    static void _cm_find_on_page_clicked(void*, Evas_Object*, void*);
    static void _cm_delete_bookmark_clicked(void*, Evas_Object*, void*);
    static void _cm_bookmark_flow_clicked(void*, Evas_Object*, void*);
    static void _cm_add_to_qa_clicked(void*, Evas_Object*, void*);
    static void _cm_desktop_view_page_clicked(void*, Evas_Object*, void*);
    static void _cm_mobile_view_page_clicked(void*, Evas_Object*, void*);
    static void _cm_settings_clicked(void*, Evas_Object*, void*);
#if PWA
    static void _cm_add_to_hs_clicked(void*, Evas_Object*, void*);
#endif
    static void launch_share(const char *uri);

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
    void updateURIBar(const std::string& uri, bool loading);
    std::string edjePath(const std::string& file);
#if GESTURE
    void gestureUp();
    void gestureDown();
#endif

    // wrappers to call singal as a reaction to other signal
    void backPageConnect() { backPage(); }
    void forwardPageConnect() { forwardPage(); }
    void addNewTabConnect() { addNewTab(); }

    Evas_Object* m_parent;
    Evas_Object* m_mainLayout;
    Evas_Object* m_errorLayout;
    Evas_Object* m_privateLayout;
    Evas_Object* m_bookmarkManagerButton;

    std::unique_ptr<ButtonBar> m_bottomButtonBar;
    std::unique_ptr<ButtonBar> m_rightButtonBar;
    WPUStatesManagerPtr m_statesMgr;
    std::unique_ptr<URIEntry> m_URIEntry;
    UrlHistoryPtr m_urlHistoryList;
    bool m_webviewLocked;
    bool m_WebPageUIvisible;
    bool m_desktopMode;

    sharedAction m_back;
    sharedAction m_forward;
    sharedAction m_addTab;
    sharedAction m_homePage;
    sharedAction m_bookmarks;
    sharedAction m_tabs;

    std::shared_ptr<pwaInfo> m_pwaInfo;

#if GESTURE
    Evas_Object* m_gestureLayer;
    static const int SINGLE_FINGER = 1;
    static const int SWIPE_MOMENTUM_TRESHOLD = 400;
#endif
    bool m_uriBarHidden;
    bool m_fullscreen;
};


}   // namespace tizen_browser
}   // namespace base_ui

#endif // WEBPAGEUI_H
