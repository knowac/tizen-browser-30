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
 * SimpleUI.h
 *
 *  Created on: Mar 18, 2014
 *      Author: k.szalilow
 */

#ifndef SIMPLEUI_H_
#define SIMPLEUI_H_

#include <Evas.h>

#include "AbstractContextMenu.h"
#include "AbstractMainWindow.h"
#include "AbstractService.h"
#include "AbstractFavoriteService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

// components
#include "WebPageUI.h"
#include "AbstractWebEngine.h"
#include "TabOrigin.h"
#include "HistoryUI.h"
#include "FindOnPageUI.h"
#include "SettingsUI.h"
#include "SettingsMain.h"
#include "SettingsHomePage.h"
#include "SettingsPrivacy.h"
#include "SettingsManager.h"
#include "TextPopup_mob.h"
#include "QuickAccess.h"
#include "TabUI.h"
#include "TabId.h"
#include "HistoryService.h"
#include "TabServiceTypedef.h"
#include "BookmarkFlowUI.h"
#include "BookmarkManagerUI.h"
#include "PlatformInputManager.h"
#include "StorageService.h"
#include "CertificateContents.h"

// other
#include "Action.h"
#include "InputPopup.h"
#include "SimplePopup.h"
#include "ContentPopup_mob.h"
#include "WebConfirmation.h"
#include "ViewManager.h"
#include "MenuButton.h"
#include "NaviframeWrapper.h"
#if PWA
#include "ProgressiveWebApp.h"
#endif
#include <functional>
#include <future>

#define CONNECT_COUNT 2

namespace tizen_browser{
namespace base_ui{

template <>
void AbstractMainWindow<Evas_Object>::setMainWindow(Evas_Object * rawPtr)
{
    m_window = std::shared_ptr<Evas_Object>(rawPtr, evas_object_del);
}

class BROWSER_EXPORT SimpleUI : public AbstractMainWindow<Evas_Object>
{
public:
    SimpleUI(/*Evas_Object *window*/);
    virtual ~SimpleUI();
    virtual int exec(
        const std::string& _url,
        const std::string& _caller,
        const std::string& _operation) final override;
    virtual std::string getName() final override;
    void suspend();
    void resume();
    void destroyUI();

    enum class rotationLock {
        noLock = 0,
        portrait,
        landscape,
    };

private:
    // setup functions
    void prepareServices();
#if PWA
    std::string preparePWA(const std::string& url);
    void countCheckUrl();
#endif
    void loadUIServices();
    void connectUISignals();
    void loadModelServices();
    void initModelServices();
    void initUIServices();
    void connectModelSignals();
    void pushViewToStack(const sAUI& view);
    void popTheStack();
    void popStackTo(const sAUI& view);
    void faviconChanged(tools::BrowserImagePtr favicon);
    void restoreLastSession();
    Evas_Object* createWebLayout(Evas_Object* parent);
    Evas_Object* createErrorLayout(Evas_Object* parent);

    void forwardEnable(bool enable);

    void downloadStarted(int status);
    void loadFinished();
    void progressChanged(double progress);
    void loadStarted();

    void setErrorButtons();

    void bookmarkAdded();
    void bookmarkDeleted();

    void showQuickAccess();
    void switchViewToQuickAccess();
    void switchViewToWebPage();
    void updateView();
    void changeEngineState();
    void windowCreated();
    void minimizeBrowser();
    void openNewTab(const std::string &uri, const std::string& title =
            std::string(), const boost::optional<int> adaptorId = boost::none,
            bool desktopMode = false,
            basic_webengine::TabOrigin origin = basic_webengine::TabOrigin::UNKNOWN);
    void switchToTab(const tizen_browser::basic_webengine::TabId& tabId);
    void newTabClicked();
    void tabClicked(const tizen_browser::basic_webengine::TabId& tabId);
    void closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId);
    void tabCreated();
    bool checkIfCreate();
    void engineTabClosed(const basic_webengine::TabId& id);

    std::shared_ptr<services::HistoryItemVector> getHistory();
    std::shared_ptr<services::HistoryItemVector> getMostVisitedItems();
    void setMostVisitedFrequencyValue(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem,
        int visitFrequency);

    void onBookmarkClicked(services::SharedBookmarkItem bookmarkItem);
    void onNewQuickAccessClicked();
    void addQuickAccessItem(const std::string &urlArg, const std::string &titleArg);
    void onQuickAccessDeleted(services::SharedQuickAccessItem quickaccessItem);
    void editQuickAccess();
    void deleteMostVisited();
    void onBookmarkEdit(services::SharedBookmarkItem bookmarkItem);
    void onBookmarkOrderEdited(services::SharedBookmarkItem bookmarkItem);
    void onBookmarkDeleted(services::SharedBookmarkItem bookmarkItem);
    void onNewFolderClicked(int parent);
    void onNewFolderPopupClick(const std::string& folder_name, int parent);
    void onDeleteFolderClicked(const std::string& folder_name);
    void onRemoveFoldersClicked(services::SharedBookmarkItemList items);
    void onEditFolderPopupClicked(const std::string& newName, services::SharedBookmarkItem item);
    void onDeleteFolderPopupClicked(PopupButtons button);
    static void onUrlIMEOpened(void* data, Evas_Object*, void*);
    static void onUrlIMEClosed(void* data, Evas_Object*, void*);

    void onHistoryRemoved(const std::string& uri);
    void openURLhistory(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode);
    void openURLquickaccess(services::SharedQuickAccessItem quickaccessItem, bool desktopMode);
    /**
     * @brief Handles 'openUrlInNewTab' signals. Uses QuickAccess to indicate
     * desktop/mobile mode.
     * TODO: desktop mode should be checked in WebView or QuickAcces (depends
     * on which view is active)
     */
    void openURL(const std::string& url);
    void openURL(const std::string& url, const std::string& title, bool desktopMode);
    void onClearHistoryAllClicked();
    void onDeleteHistoryItems(int id);

    void onMostVisitedClicked();
    void onQuickAccessClicked();

    /**
     * @brief Handles 'generateThumb' signals.
     */
    void onGenerateThumb(basic_webengine::TabId tabId);
    void onGenerateFavicon(basic_webengine::TabId tabId);
    void onSnapshotCaptured(std::shared_ptr<tools::BrowserImage> snapshot, tools::SnapshotType snapshot_type);
    void onCreateTabId();

    void authPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void certPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData);

    void onActionTriggered(const Action& action);
    void onMenuButtonPressed();
    void handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation);
    void setwvIMEStatus(bool status);
#if PWA
    void pwaPopupRequest();
    void pwaPopupButtonClicked(const PopupButtons& button);
#endif

    sharedAction m_showBookmarkManagerUI;

    /**
     * \brief filters URL before it is passed to WebEngine.
     *
     * This function should be connected with m_simpleURI->uriChanged.
     * it is a good place to check for special urls (like "about:home"),
     * filter forbidden addresses and to check set favorite icon.
     */
    void filterURL(const std::string& url);

    // on uri entry widget "changed,user" signal
    void onURLEntryEditedByUser(const std::shared_ptr<std::string> editedUrlPtr);
    // on uri entry widget "changed" signal
    void onURLEntryEdited();

    void onmostHistoryvisitedClicked();
    void onBookmarkvisitedClicked();

    /**
     * @brief Check if the current page exists as a bookmark.
     *
     */
    bool checkBookmark();

    /**
     * @brief Check if the current page exists as a quick access.
     *
     */
    bool checkQuickAccess();

    /**
     * @brief Adds current page to bookmarks.
     *
     */
    void addBookmark(BookmarkUpdate bookmark_update);

    /**
     * @brief Edits currents page bookmark
     *
     */
    void editBookmark(BookmarkUpdate bookmark_update);

    /**
     * @brief Remove current page from bookmarks
     *
     * @param  ...
     * @return void
     */
    void deleteBookmark(void);

    void settingsOverrideUseragent(const std::string& userAgent);
    void onOverrideUseragentButton(const std::string& newUA);

    void scrollView(const int& dx, const int& dy);

    void showTabUI();
    void refetchTabUIData();
    void switchToMobileMode();
    void switchToDesktopMode();
    Evas_Object* showHistoryUI(Evas_Object* parent, SharedNaviframeWrapper naviframe, bool removeMode = false);
    void showSettings(unsigned);
    void onDefSearchEngineClicked();
    void onSaveContentToClicked();
    std::string requestSettingsCurrentPage();
    void selectSettingsOtherPageChange();

    void onEditOtherPagePopupClicked(const std::string& newName);
    void showBookmarkFlowUI();
    void showFindOnPageUI(const std::string& str);
    void showCertificatePopup();
    void showCertificatePopup(const std::string& host, const std::string& pem, services::CertificateContents::HOST_TYPE type);
    void showUnsecureConnectionPopup();

    void findWord(const struct FindData& fdata);
    void closeFindOnPageUI();

    void registerHWKeyCallback();
    void unregisterHWKeyCallback();

    bool isManualRotation(const sAUI& view);
    void enableManualRotation(bool enable);
    void rotatePrepared();
    void onRotation();
    bool isLandscape();
    int getRotation();
    void rotationType(rotationLock lock);
    void connectWebPageSignals();
    void connectQuickAccessSignals();
    void connectTabsSignals();
    void connectHistorySignals();
    void connectSettingsSignals();
    void connectBookmarkFlowSignals();
    void connectBookmarkManagerSignals();
    void connectFindOnPageSignals();
    void connectWebEngineSignals();
    void connectHistoryServiceSignals();
    void connectTabServiceSignals();
    void connectPlatformInputSignals();
    void connectCertificateSignals();
    void connectStorageSignals();

    static void __orientation_changed(void* data, Evas_Object*, void*);
    Evas_Object* getMainWindow();
    void showBookmarkManagerUI(std::shared_ptr<services::BookmarkItem> parent,
                               BookmarkManagerState state);
    void showHomePage();
    void redirectedWebPage(const std::string& oldUrl, const std::string& newUrl);

    void showPopup(interfaces::AbstractPopup* popup);
    void dismissPopup(interfaces::AbstractPopup* popup);

    void closeTab();
    void closeTab(const tizen_browser::basic_webengine::TabId& id);

    void settingsDeleteSelectedData(const std::map<int, bool>& option);
    void settingsResetMostVisited();
    void settingsResetBrowser();
    void onDeleteSelectedDataButton(const PopupButtons& button, const std::map<int, bool>& options);
    void onDeleteMostVisitedButton(std::shared_ptr<PopupData> popupData);
    void onResetBrowserButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void tabLimitPopupButtonClicked(PopupButtons button);
    int tabsCount();

    void onReturnPressed(MenuButton *m);
    void onXF86BackPressed();
    void onBackPressed();

    void searchWebPage(std::string &text, int flags);
    void showPasswordUI();
    void onFirstSecretMode();

    std::string edjePath(const std::string &);

    std::vector<interfaces::AbstractPopup*> m_popupVector;

    std::shared_ptr<WebPageUI> m_webPageUI;
    std::shared_ptr<basic_webengine::AbstractWebEngine>  m_webEngine;
    std::shared_ptr<interfaces::AbstractFavoriteService> m_favoriteService;
    std::shared_ptr<services::HistoryService> m_historyService;
    services::TabServicePtr m_tabService;

    std::shared_ptr<BookmarkFlowUI> m_bookmarkFlowUI;
    std::shared_ptr<FindOnPageUI> m_findOnPageUI;
    std::shared_ptr<services::CertificateContents> m_certificateContents;
    std::shared_ptr<BookmarkManagerUI> m_bookmarkManagerUI;
    std::shared_ptr<QuickAccess> m_quickAccess;
    std::shared_ptr<HistoryUI> m_historyUI;
    std::shared_ptr<SettingsManager> m_settingsManager;
    std::shared_ptr<SettingsUI> m_settingsUI;
    std::shared_ptr<TabUI> m_tabUI;
    std::shared_ptr<services::PlatformInputManager> m_platformInputManager;
    std::shared_ptr<services::StorageService> m_storageService;
    bool m_initialised;
    std::string m_caller;
    int m_tabLimit;
    int m_favoritesLimit;
    bool m_wvIMEStatus;
    std::string m_folder_name;

    //helper object used to view management
    ViewManager m_viewManager;
    Evas_Object *main_window;
#if PWA
    ProgressiveWebApp m_pwa;
    bool m_alreadyOpenedPWA;
#endif
    SharedNaviframeWrapper m_QAEditNaviframe;
    Evas_Object *m_conformant;
    bool m_manualRotation;
    int m_current_angle;
    int m_temp_angle;
    std::function<void()> m_functionViewPrepare;
    std::future<void> m_futureView;
    bool m_alreadyOpenedExecURL;
};

}
}

#endif /* SIMPLEUI_H_ */
