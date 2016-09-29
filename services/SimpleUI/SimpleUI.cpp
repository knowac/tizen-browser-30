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

#include "browser_config.h"

#include <boost/any.hpp>
#include <memory>
#include <algorithm>
#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <Edje.h>
#include <Evas.h>
#include <app.h>
#include "Config.h"
#include "app_i18n.h"
#include "TabService.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "AbstractWebEngine.h"
#include "TabId.h"
#include "BrowserImage.h"
#include "SimpleUI.h"
#include "WebPageUIStatesManager.h"
#include "BookmarkItem.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "HistoryItem.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "DetailPopup.h"
#include "UrlHistoryList/UrlHistoryList.h"
#include "NotificationPopup.h"
#include "ContentPopup_mob.h"
#include "Tools/GeneralTools.h"
#include "Tools/SnapshotType.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const std::string ResetBrowserPopupMsg = "Do you really want to reset Browser?" \
                                         " If you press Reset, delete all data" \
                                         " and return to initial setting.";
const int SCALE_FACTOR =
#if PROFILE_MOBILE
        720;
#else
        1920;
#endif

SimpleUI::SimpleUI()
    : AbstractMainWindow()
    , m_webPageUI()
    , m_moreMenuUI()
#if PROFILE_MOBILE
    , m_bookmarkFlowUI()
    , m_findOnPageUI()
#endif
    , m_bookmarkManagerUI()
    , m_quickAccess()
    , m_historyUI()
    , m_settingsUI()
    , m_tabUI()
    , m_initialised(false)
    , m_tabLimit(0)
    , m_favoritesLimit(0)
    , m_wvIMEStatus(false)
#if PROFILE_MOBILE
    , m_current_angle(0)
    , m_temp_angle(0)
#endif
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_init(0, nullptr);

    main_window = elm_win_util_standard_add("browserApp", "browserApp");

    int width;
    elm_win_screen_size_get(main_window, nullptr, nullptr, &width, nullptr);
    double config_scale_value = (double)(width)/SCALE_FACTOR;
    tizen_browser::config::Config::getInstance().set(
            "scale", static_cast<double>(elm_config_scale_get()/config_scale_value));

    elm_win_conformant_set(main_window, EINA_TRUE);
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");

    setMainWindow(main_window);
    m_viewManager.init(main_window);

    elm_win_resize_object_add(main_window, m_viewManager.getConformant());
    evas_object_show(main_window);
#if PROFILE_MOBILE
    if (elm_win_wm_rotation_supported_get(main_window)) {
        static const int rots[] = {0, 90, 180, 270};
        elm_win_wm_rotation_available_rotations_set(main_window, rots, (sizeof(rots) / sizeof(int)));
        evas_object_smart_callback_add(main_window, "wm,rotation,changed", __orientation_changed, this);
    } else
        BROWSER_LOGW("[%s:%d] Device does not support rotation.", __PRETTY_FUNCTION__, __LINE__);

    // TODO Unify the virtual keyboard behavior. For now webview entry and url entry have the separate ways to
    // determine if keyboard has been shown. I think it is possible to unify it with below callbacks.
    evas_object_smart_callback_add(m_viewManager.getConformant(), "virtualkeypad,state,on", onUrlIMEOpened, this);
    evas_object_smart_callback_add(m_viewManager.getConformant(), "virtualkeypad,state,off",onUrlIMEClosed, this);
#endif
}

SimpleUI::~SimpleUI() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_del(m_window.get());
}

void SimpleUI::suspend()
{
    m_webEngine->suspend();
}

void SimpleUI::resume()
{
    m_webEngine->preinitializeWebViewCache();
    m_webEngine->resume();
#if PROFILE_MOBILE
    if (m_findOnPageUI && m_findOnPageUI->isVisible())
        m_findOnPageUI->show_ime();
#endif
}

void SimpleUI::destroyUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->destroyTabs();
}

std::string SimpleUI::edjePath(const std::string &file)
{
    return std::string(EDJE_DIR) + file;
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getMostVisitedItems()
{
    return m_historyService->getMostVisitedHistoryItems();
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getHistory()
{
    return m_historyService->getHistoryToday();
}

int SimpleUI::exec(const std::string& _url, const std::string& _caller)
{
    BROWSER_LOGD("[%s] _url=%s, _caller=%s, initialised=%d", __func__, _url.c_str(), _caller.c_str(), m_initialised);
    std::string url = _url;
    m_caller = _caller;

    if(!m_initialised){
        if (m_window.get()) {
            m_tabLimit = boost::any_cast <int> (tizen_browser::config::Config::getInstance().get("TAB_LIMIT"));
            m_favoritesLimit = boost::any_cast <int> (tizen_browser::config::Config::getInstance().get("FAVORITES_LIMIT"));


            loadUIServices();
            loadModelServices();

            connectModelSignals();
            connectUISignals();

            // initModelServices() needs to be called after initUIServices()
            initUIServices();
            initModelServices();

            //Push first view to stack.
            m_viewManager.pushViewToStack(m_webPageUI.get());
#if PROFILE_MOBILE
            // Register H/W back key callback
            m_platformInputManager->registerHWKeyCallback(m_viewManager.getContent());
#endif
        }

        if (url.empty())
        {
            BROWSER_LOGD("[%s]: restore last session", __func__);
            switchViewToQuickAccess();
            restoreLastSession();
        }
        m_initialised = true;
    }

    if (!url.empty())
    {
        BROWSER_LOGD("[%s]: open new tab", __func__);
        openNewTab(url);
    }

    BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());
    return 0;
}

void SimpleUI::titleChanged(const std::string& title)
{
    m_moreMenuUI->setWebTitle(title);
}

void SimpleUI::faviconChanged(tools::BrowserImagePtr favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webEngine->isLoading())
        m_historyService->updateHistoryItemFavicon(m_webEngine->getURI(), favicon);
}

void SimpleUI::restoreLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto vec = m_tabService->getAllTabs();
    for (const basic_webengine::TabContent& i : *vec) {
        openNewTab(i.getUrl(), i.getTitle(), boost::optional<int>(i.getId().get()), false, false, i.getOrigin());
    }
}


//TODO: Move all service creation here:
void SimpleUI::loadUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::WebPageUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webpageui"));

    m_quickAccess =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::QuickAccess,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.quickaccess"));

    m_tabUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::TabUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.tabui"));

    m_historyUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::HistoryUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyui"));

    m_settingsUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::SettingsUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.settingsui"));

    m_moreMenuUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::MoreMenuUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.moremenuui"));

    m_bookmarkDetailsUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::BookmarkDetailsUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkdetailsui"));
#if PROFILE_MOBILE
    m_bookmarkFlowUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::BookmarkFlowUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkflowui"));

    m_findOnPageUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::FindOnPageUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.findonpageui"));
#else
    m_zoomUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::ZoomUI, tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.zoomui"));
#endif
    m_bookmarkManagerUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::BookmarkManagerUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkmanagerui"));
}

void SimpleUI::connectUISignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    m_viewManager.isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->getURIEntry().uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
//    m_webPageUI->getURIEntry().uriEntryEditingChangedByUser.connect(boost::bind(&SimpleUI::onURLEntryEditedByUser, this, _1));
//    m_webPageUI->getUrlHistoryList()->openURL.connect(boost::bind(&SimpleUI::onOpenURL, this, _1));
//    m_webPageUI->getUrlHistoryList()->uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
    m_webPageUI->backPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->backPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::showMoreMenu, this));
    m_webPageUI->forwardPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, m_webEngine.get()));
    m_webPageUI->stopLoadingPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, m_webEngine.get()));
    m_webPageUI->showQuickAccess.connect(boost::bind(&SimpleUI::showQuickAccess, this));
    m_webPageUI->hideQuickAccess.connect(boost::bind(&QuickAccess::hideUI, m_quickAccess));
    m_webPageUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_webPageUI->focusWebView.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::setFocus, m_webEngine.get()));
    m_webPageUI->unfocusWebView.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::clearFocus, m_webEngine.get()));
#if PROFILE_MOBILE
    m_webPageUI->hideMoreMenu.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    m_webPageUI->getURIEntry().mobileEntryFocused.connect(boost::bind(&WebPageUI::mobileEntryFocused, m_webPageUI));
    m_webPageUI->getURIEntry().mobileEntryUnfocused.connect(boost::bind(&WebPageUI::mobileEntryUnfocused, m_webPageUI));
    m_webPageUI->qaOrientationChanged.connect(boost::bind(&QuickAccess::orientationChanged, m_quickAccess));
    m_webPageUI->getURIEntry().secureIconClicked.connect(boost::bind(&SimpleUI::showCertificatePopup, this));
    m_webPageUI->getURIEntry().isValidCert.connect(boost::bind(&services::CertificateContents::isValidCertificate, m_certificateContents, _1));
    m_webEngine->fullscreenModeSet.connect(boost::bind(&WebPageUI::fullscreenModeSet, m_webPageUI.get(), _1));
#endif

    M_ASSERT(m_quickAccess.get());
    m_quickAccess->getDetailPopup().openURL.connect(boost::bind(&SimpleUI::onOpenURL, this, _1, _2));
    m_quickAccess->openURL.connect(boost::bind(&SimpleUI::onOpenURL, this, _1, _2));
    m_quickAccess->mostVisitedTileClicked.connect(boost::bind(&SimpleUI::onMostVisitedTileClicked, this, _1, _2));
    m_quickAccess->getMostVisitedItems.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this));
    m_quickAccess->getBookmarksItems.connect(boost::bind(&SimpleUI::onBookmarkButtonClicked, this));
    m_quickAccess->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_quickAccess->switchViewToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
#if PROFILE_MOBILE
    m_quickAccess->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(m_tabUI.get());
    m_tabUI->closeTabUIClicked.connect(boost::bind(&SimpleUI::closeTabUI, this));
    m_tabUI->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this));
    m_tabUI->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    m_tabUI->closeTabsClicked.connect(boost::bind(&SimpleUI::closeTabsClicked, this,_1));
    m_tabUI->isIncognito.connect(boost::bind(&SimpleUI::isIncognito, this, _1));
#if PROFILE_MOBILE
    m_tabUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    bool desktop_ua = false;
#else
    bool desktop_ua = true;
#endif
    m_tabUI->newIncognitoTabClicked.connect(boost::bind(&SimpleUI::openNewTab, this, "", "", boost::none, desktop_ua, true, basic_webengine::TabOrigin::UNKNOWN));
    m_tabUI->tabsCount.connect(boost::bind(&SimpleUI::tabsCount, this));

    M_ASSERT(m_historyUI.get());
    m_historyUI->clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryAllClicked, this));
    m_historyUI->signalDeleteHistoryItems.connect(boost::bind(&SimpleUI::onDeleteHistoryItems, this, _1));
    m_historyUI->closeHistoryUIClicked.connect(boost::bind(&SimpleUI::closeHistoryUI, this));
    m_historyUI->signalHistoryItemClicked.connect(boost::bind(&SimpleUI::onOpenURL, this, _1, _2, desktop_ua));

    M_ASSERT(m_settingsUI.get());
    m_settingsUI->closeSettingsUIClicked.connect(boost::bind(&SimpleUI::closeSettingsUI, this));
    m_settingsUI->deleteSelectedDataClicked.connect(boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));
    m_settingsUI->resetMostVisitedClicked.connect(boost::bind(&SimpleUI::settingsResetMostVisited, this));
    m_settingsUI->resetBrowserClicked.connect(boost::bind(&SimpleUI::settingsResetBrowser, this));
#if PROFILE_MOBILE
    m_settingsUI->userAgentItemClicked.connect(boost::bind(&SimpleUI::settingsOverrideUseragent, this, _1));
    m_settingsUI->getWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::getSettingsParam, m_webEngine.get(), _1));
    m_settingsUI->setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::setSettingsParam, m_webEngine.get(), _1, _2));
    m_settingsUI->setWebEngineSettingsParam.connect(boost::bind(&storage::SettingsStorage::setParam, &m_storageService->getSettingsStorage(), _1, _2));
    m_settingsUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(m_moreMenuUI.get());
    m_moreMenuUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_moreMenuUI->historyUIClicked.connect(boost::bind(&SimpleUI::showHistoryUI, this));
    m_moreMenuUI->settingsClicked.connect(boost::bind(&SimpleUI::showSettingsUI, this));
    m_moreMenuUI->closeMoreMenuClicked.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    m_moreMenuUI->switchToMobileMode.connect(boost::bind(&SimpleUI::switchToMobileMode, this));
    m_moreMenuUI->switchToDesktopMode.connect(boost::bind(&SimpleUI::switchToDesktopMode, this));
    m_moreMenuUI->isBookmark.connect(boost::bind(&SimpleUI::checkBookmark, this));
    m_moreMenuUI->bookmarkFlowClicked.connect(boost::bind(&SimpleUI::showBookmarkFlowUI, this, _1));
#if PROFILE_MOBILE
    m_moreMenuUI->findOnPageClicked.connect(boost::bind(&SimpleUI::showFindOnPageUI, this, std::string()));
    m_moreMenuUI->isRotated.connect(boost::bind(&SimpleUI::isLandscape, this));
    m_webPageUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#else
    m_moreMenuUI->zoomUIClicked.connect(boost::bind(&SimpleUI::showZoomUI, this));
#endif

    M_ASSERT(m_bookmarkDetailsUI.get());
    m_bookmarkDetailsUI->bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
    m_bookmarkDetailsUI->closeBookmarkDetailsClicked.connect(boost::bind(&SimpleUI::closeBookmarkDetailsUI, this));
#if PROFILE_MOBILE
    m_bookmarkDetailsUI->editFolderButtonClicked.connect(boost::bind(&SimpleUI::onEditFolderClicked, this, _1));
    m_bookmarkDetailsUI->deleteFolderButtonClicked.connect(boost::bind(&SimpleUI::onDeleteFolderClicked, this, _1));
    m_bookmarkDetailsUI->removeFoldersButtonClicked.connect(boost::bind(&SimpleUI::onRemoveFoldersClicked, this, _1));

    M_ASSERT(m_bookmarkFlowUI.get());
    m_bookmarkFlowUI->addFolder.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    m_bookmarkFlowUI->closeBookmarkFlowClicked.connect(boost::bind(&SimpleUI::closeBookmarkFlowUI, this));
    m_bookmarkFlowUI->saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    m_bookmarkFlowUI->editBookmark.connect(boost::bind(&SimpleUI::editBookmark, this, _1));
    m_bookmarkFlowUI->removeBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));
    m_bookmarkFlowUI->isRotated.connect(boost::bind(&SimpleUI::isLandscape, this));

    M_ASSERT(m_findOnPageUI.get());
    m_findOnPageUI->closeFindOnPageUIClicked.connect(boost::bind(&SimpleUI::closeFindOnPageUI, this));
    m_findOnPageUI->startFindingWord.connect(boost::bind(&SimpleUI::findWord, this, _1));
#endif

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::closeBookmarkManagerUI, this));
    m_bookmarkManagerUI->customFolderClicked.connect(boost::bind(&SimpleUI::onBookmarkCustomFolderClicked, this, _1));
    m_bookmarkManagerUI->allFolderClicked.connect(boost::bind(&SimpleUI::onBookmarkAllFolderClicked, this));
    m_bookmarkManagerUI->specialFolderClicked.connect(boost::bind(&SimpleUI::onBookmarkSpecialFolderClicked, this));
#if PROFILE_MOBILE
    m_bookmarkManagerUI->newFolderItemClicked.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    m_bookmarkManagerUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#else
    M_ASSERT(m_zoomUI.get());
    m_zoomUI->setZoom.connect(boost::bind(&SimpleUI::setZoomFactor, this, _1));
    m_zoomUI->getZoom.connect(boost::bind(&SimpleUI::getZoomFactor, this));
    m_zoomUI->scrollView.connect(boost::bind(&SimpleUI::scrollView, this, _1, _2));
#endif
}

void SimpleUI::loadModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine =
        std::dynamic_pointer_cast
        <basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    m_storageService =
        std::dynamic_pointer_cast
        <tizen_browser::services::StorageService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.storageservice"));

    m_favoriteService =
        std::dynamic_pointer_cast
        <tizen_browser::interfaces::AbstractFavoriteService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

    m_historyService =
        std::dynamic_pointer_cast
        <tizen_browser::services::HistoryService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));

    m_tabService = std::dynamic_pointer_cast<
            tizen_browser::services::TabService,
            tizen_browser::core::AbstractService>(
            tizen_browser::core::ServiceManager::getInstance().getService(
                    "org.tizen.browser.tabservice"));

    m_platformInputManager =
        std::dynamic_pointer_cast
        <tizen_browser::services::PlatformInputManager,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));

    m_certificateContents =
        std::dynamic_pointer_cast
        <tizen_browser::services::CertificateContents, tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.certificateservice"));
}

void SimpleUI::initUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->init(m_viewManager.getContent());

    M_ASSERT(m_quickAccess.get());
    m_quickAccess->init(m_webPageUI->getContent());

    M_ASSERT(m_tabUI.get());
    m_tabUI->init(m_viewManager.getContent());

    M_ASSERT(m_historyUI.get());
    m_historyUI->init(m_viewManager.getContent());

    M_ASSERT(m_moreMenuUI.get());
#if PROFILE_MOBILE
    m_moreMenuUI->init(m_webPageUI->getContent());
#else
    m_moreMenuUI->init(m_viewManager.getContent());
#endif

    M_ASSERT(m_settingsUI.get());
    m_settingsUI->init(m_viewManager.getContent());

    M_ASSERT(m_bookmarkDetailsUI.get());
    m_bookmarkDetailsUI->init(m_viewManager.getContent());

#if PROFILE_MOBILE
    M_ASSERT(m_bookmarkFlowUI.get());
    m_bookmarkFlowUI->init(m_viewManager.getContent());
    m_bookmarkFlowUI->setFoldersId(m_storageService->getFoldersStorage().AllFolder, m_storageService->getFoldersStorage().SpecialFolder);

    M_ASSERT(m_findOnPageUI.get());
    m_findOnPageUI->init(m_webPageUI->getContent());
#else
    M_ASSERT(m_zoomUI.get());
    m_zoomUI->init(m_viewManager.getContent());
#endif

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->init(m_viewManager.getContent());
    m_bookmarkManagerUI->setFoldersId(m_storageService->getFoldersStorage().AllFolder, m_storageService->getFoldersStorage().SpecialFolder);
}

void SimpleUI::initModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webEngine);
    M_ASSERT(m_webPageUI->getContent());
    m_webEngine->init(m_webPageUI->getContent());

#if PROFILE_MOBILE
    M_ASSERT(m_storageService->getSettingsStorage());
    M_ASSERT(m_storageService->getFoldersStorage());
    m_storageService->getSettingsStorage().initWebEngineSettingsFromDB();
#endif

    M_ASSERT(m_favoriteService);
    m_favoriteService->getBookmarks();

    M_ASSERT(m_platformInputManager);
    m_platformInputManager->init(m_window.get());

    M_ASSERT(m_certificateContents.get());
    m_certificateContents->init();
}

void SimpleUI::connectModelSignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine->minimizeBrowser.connect(boost::bind(&SimpleUI::minimizeBrowser, this));
    m_webEngine->uriChanged.connect(boost::bind(&URIEntry::changeUri, &m_webPageUI->getURIEntry(), _1));
    m_webEngine->downloadStarted.connect(boost::bind(&SimpleUI::downloadStarted, this, _1));
    m_webEngine->backwardEnableChanged.connect(boost::bind(&WebPageUI::setBackButtonEnabled, m_webPageUI.get(), _1));
    m_webEngine->forwardEnableChanged.connect(boost::bind(&WebPageUI::setForwardButtonEnabled, m_webPageUI.get(), _1));
    m_webEngine->loadStarted.connect(boost::bind(&SimpleUI::loadStarted, this));
    m_webEngine->loadProgress.connect(boost::bind(&SimpleUI::progressChanged,this,_1));
    m_webEngine->loadFinished.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->loadStop.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->loadError.connect(boost::bind(&SimpleUI::loadError, this));
    m_webEngine->tabCreated.connect(boost::bind(&SimpleUI::tabCreated, this));
    m_webEngine->checkIfCreate.connect(boost::bind(&SimpleUI::checkIfCreate, this));
    m_webEngine->tabClosed.connect(boost::bind(&SimpleUI::tabClosed,this,_1));
    m_webEngine->IMEStateChanged.connect(boost::bind(&SimpleUI::setwvIMEStatus, this, _1));
    m_webEngine->switchToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webEngine->titleChanged.connect(boost::bind(&SimpleUI::titleChanged, this, _1));
    m_webEngine->favIconChanged.connect(boost::bind(&SimpleUI::faviconChanged, this, _1));
    m_webEngine->windowCreated.connect(boost::bind(&SimpleUI::windowCreated, this));
    m_webEngine->createTabId.connect(boost::bind(&SimpleUI::onCreateTabId, this));
    m_webEngine->snapshotCaptured.connect(boost::bind(&SimpleUI::onSnapshotCaptured, this, _1, _2));
    m_webEngine->redirectedWebPage.connect(boost::bind(&SimpleUI::redirectedWebPage, this, _1, _2));
    m_webEngine->switchToQuickAccess.connect(boost::bind(&SimpleUI::switchViewToQuickAccess, this));
    m_webEngine->setCertificatePem.connect(boost::bind(&services::CertificateContents::saveCertificateInfo, m_certificateContents, _1, _2));
    m_webEngine->setWrongCertificatePem.connect(boost::bind(&services::CertificateContents::saveWrongCertificateInfo, m_certificateContents, _1, _2));
#if PROFILE_MOBILE
    m_webEngine->uriChanged.connect(boost::bind(&SimpleUI::webEngineURLChanged, this, _1));
    m_webEngine->confirmationRequest.connect(boost::bind(&SimpleUI::handleConfirmationRequest, this, _1));
    m_webEngine->getRotation.connect(boost::bind(&SimpleUI::getRotation, this));
    m_webEngine->openFindOnPage.connect(boost::bind(&SimpleUI::showFindOnPageUI, this, _1));
    m_webEngine->closeFindOnPage.connect(boost::bind(&SimpleUI::closeFindOnPageUI, this));
    m_webEngine->unsecureConnection.connect(boost::bind(&SimpleUI::showUnsecureConnectionPopup, this));
#endif

    m_favoriteService->bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
    m_favoriteService->bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));

    m_historyService->historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));

    m_tabService->generateThumb.connect(boost::bind(&SimpleUI::onGenerateThumb, this, _1));

    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));
#if !PROFILE_MOBILE
    m_platformInputManager->escapePressed.connect(boost::bind(&SimpleUI::onEscapePressed, this));
    m_platformInputManager->redPressed.connect(boost::bind(&SimpleUI::onRedKeyPressed, this));
    m_platformInputManager->yellowPressed.connect(boost::bind(&SimpleUI::onYellowKeyPressed, this));
#endif

    m_certificateContents->getHostCertList.connect(boost::bind(&storage::CertificateStorage::getHostCertList, &m_storageService->getCertificateStorage()));
    m_certificateContents->addOrUpdateCertificateEntry.connect(boost::bind(&storage::CertificateStorage::addOrUpdateCertificateEntry, &m_storageService->getCertificateStorage(), _1, _2, _3));

#if PROFILE_MOBILE
    m_storageService->getSettingsStorage().setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::setSettingsParam, m_webEngine.get(), _1, _2));
    m_platformInputManager->menuButtonPressed.connect(boost::bind(&SimpleUI::onMenuButtonPressed, this));
    m_platformInputManager->XF86BackPressed.connect(boost::bind(&SimpleUI::onXF86BackPressed, this));
    m_webEngine->registerHWKeyCallback.connect(boost::bind(&SimpleUI::registerHWKeyCallback, this));
    m_webEngine->unregisterHWKeyCallback.connect(boost::bind(&SimpleUI::unregisterHWKeyCallback, this));
#endif
}

#if PROFILE_MOBILE
void SimpleUI::registerHWKeyCallback()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_platformInputManager->registerHWKeyCallback(m_webEngine->getLayout());
}

void SimpleUI::unregisterHWKeyCallback()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_platformInputManager->unregisterHWKeyCallback(m_webEngine->getLayout());
}
#endif


void SimpleUI::switchViewToWebPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webEngine->isSuspended())
        m_webEngine->resume();
    m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), m_webEngine->getURI());
    m_webPageUI->toIncognito(m_webEngine->isPrivateMode(m_webEngine->currentTabId()));
}

void SimpleUI::switchToTab(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webEngine->currentTabId() != tabId) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_webEngine->switchToTab(tabId);
    }
    if(m_webEngine->isLoadError()){
        BROWSER_LOGD("[%s:%d] LOAD ERROR!", __PRETTY_FUNCTION__, __LINE__);
        loadError();
    return;
    }
    BROWSER_LOGD("[%s:%d] swiching to web_view ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
}

void SimpleUI::showQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->showUI();
}

void SimpleUI::switchViewToQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI->switchViewToQuickAccess(m_quickAccess->getContent());
    m_webEngine->disconnectCurrentWebViewSignals();
    m_viewManager.popStackTo(m_webPageUI.get());
}

void SimpleUI::switchViewToIncognitoPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->switchViewToIncognitoPage();
    m_viewManager.popStackTo(m_webPageUI.get());
}

void SimpleUI::openNewTab(const std::string &uri, const std::string& title,
        const boost::optional<int> adaptorId, bool desktopMode,
        bool incognitoMode, basic_webengine::TabOrigin origin)
{
    BROWSER_LOGD("[%s:%d] uri =%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    tizen_browser::basic_webengine::TabId tab = m_webEngine->addTab(uri,
            nullptr, adaptorId, title, desktopMode, incognitoMode, origin);
    if (tab == basic_webengine::TabId::NONE) {
        BROWSER_LOGW("[%s:%d] New tab is not created!", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    switchToTab(tab);
    m_webPageUI->toIncognito(incognitoMode);
    if (incognitoMode)
        switchViewToIncognitoPage();
}

void SimpleUI::closeTab()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto tabId = m_webEngine->currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const tizen_browser::basic_webengine::TabId& id)
{
    BROWSER_LOGD("[%s:%d] id: %d", __PRETTY_FUNCTION__, __LINE__, id.get());
    m_tabService->removeTab(id);
    m_webEngine->closeTab(id);
    updateView();
}

bool SimpleUI::checkBookmark()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
        return false;

    if(m_favoriteService->bookmarkExists(m_webEngine->getURI())) {
        BROWSER_LOGD("[%s] There is bookmark for this site [%s], set indicator on!", __func__, m_webEngine->getURI().c_str());
        return true;
    }
    else {
        BROWSER_LOGD("[%s] There is no bookmark for this site [%s], set indicator off", __func__, m_webEngine->getURI().c_str());
        return false;
    }
}
// Consider removing these functions
void SimpleUI::onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem>)
{
    if (m_moreMenuUI) {
        m_moreMenuUI->changeBookmarkStatus(true);
        m_moreMenuUI->createToastPopup( (std::string(m_webEngine->getTitle()) + std::string(" added to bookmark")).c_str() );
    }
}

void SimpleUI::onBookmarkRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    if (m_moreMenuUI) {
        m_moreMenuUI->changeBookmarkStatus(false);
        m_moreMenuUI->createToastPopup( (std::string(m_webEngine->getTitle()) + std::string(" removed from bookmark")).c_str() );
    }
}

void SimpleUI::onOpenURL(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode)
{
    onOpenURL(historyItem->getUrl(), historyItem->getTitle(), desktopMode);
}

void SimpleUI::onOpenURL(const std::string& url)
{
    // TODO: desktop mode should be checked in WebView or QuickAcces
    // (depends on which view is active)
    onOpenURL(url, "", m_quickAccess->isDesktopMode());
}

void SimpleUI::onOpenURL(const std::string& url, const std::string& title, bool desktopMode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_webPageUI) {
        m_viewManager.popStackTo(m_webPageUI.get());
        if (tabsCount() == 0 || m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
            openNewTab(url, title, boost::none, desktopMode, false, basic_webengine::TabOrigin::QUICKACCESS);
        else {
            m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), title);
            m_webEngine->setURI(url);
            m_webPageUI->getURIEntry().clearFocus();
        }
    } else {
        BROWSER_LOGW("[%s:%d] No m_webPageUI object!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::onMostVisitedTileClicked(std::shared_ptr< services::HistoryItem > historyItem, int itemsNumber)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->openDetailPopup(historyItem, m_historyService->getHistoryItemsByURL(historyItem->getUrl(), itemsNumber));
}

void SimpleUI::onClearHistoryAllClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyService->clearAllHistory();
}

void SimpleUI::onDeleteHistoryItems(
        std::shared_ptr<const std::vector<int>> itemIds)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (const int& id : *itemIds)
        m_historyService->deleteHistoryItem(id);
}

void SimpleUI::onMostVisitedClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_quickAccess->setMostVisitedItems(getMostVisitedItems());
}

void SimpleUI::onBookmarkButtonClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_quickAccess->setBookmarksItems(m_favoriteService->getBookmarks(-1));
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string bookmarkAddress = bookmarkItem->getAddress();

    if (tabsCount() == 0)
        openNewTab(bookmarkAddress);
    else {
        std::string bookmarkTitle = bookmarkItem->getTitle();
        m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), bookmarkAddress);
        m_webEngine->setURI(bookmarkAddress);
        m_webPageUI->getURIEntry().clearFocus();
        closeBookmarkManagerUI();
    }
    m_viewManager.popStackTo(m_webPageUI.get());
}

void SimpleUI::onNewFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    InputPopup *inputPopup = InputPopup::createPopup(m_viewManager.getContent(), "New Folder", "Add New Folder?",
                                                          "New Folder #", _("IDS_BR_OPT_ADD"), _("IDS_BR_SK_CANCEL_ABB"), true);
#else
    InputPopup *inputPopup = InputPopup::createPopup(m_viewManager.getContent(), "New Folder", "Add new folder for adding to bookmark?",
                                                          "Folder #", _("IDS_BR_SK_CANCEL_ABB"), "Add to Bookmark", false);
#endif
    services::SharedBookmarkFolderList badWords = m_storageService->getFoldersStorage().getFolders();
    for (auto it = badWords.begin(); it != badWords.end(); ++it)
        inputPopup->addBadWord((*it)->getName());
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onNewFolderPopupClick, this, _1));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    inputPopup->show();
}

void SimpleUI::onNewFolderPopupClick(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_storageService->getFoldersStorage().ifFolderExists(folder_name)) {
        BROWSER_LOGD("[%s:%d] Folder already exists.", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    unsigned int id = m_storageService->getFoldersStorage().addFolder(folder_name);
#if PROFILE_MOBILE
    SharedBookmarkFolder folder = m_storageService->getFoldersStorage().getFolder(id);
    SharedBookmarkFolderList list;
    list.push_back(folder);
    if (m_viewManager.topOfStack() == m_bookmarkManagerUI.get()) {
        m_bookmarkManagerUI->addCustomFolders(list);
    } else if (m_viewManager.topOfStack() == m_bookmarkFlowUI.get()) {
        m_bookmarkFlowUI->addCustomFolders(list);
        m_bookmarkFlowUI->setFolder(folder->getId(), folder->getName());
    }
#else
    BookmarkUpdate update;
    update.folder_id = id;
    M_UNUSED(id);
    addBookmark(update);
#endif
}
#if PROFILE_MOBILE
void SimpleUI::onEditFolderClicked(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = InputPopup::createPopup(m_viewManager.getContent(), "Edit Folder name", "Edit folder name?",
                                                        folder_name, _("IDS_BR_SK_DONE"), _("IDS_BR_SK_CANCEL_ABB"), true);
    services::SharedBookmarkFolderList badWords = m_storageService->getFoldersStorage().getFolders();
    for (auto it = badWords.begin(); it != badWords.end(); ++it)
        inputPopup->addBadWord((*it)->getName());
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onEditFolderPopupClicked, this, _1));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    m_folder_name = folder_name;
    inputPopup->show();
}

void SimpleUI::onDeleteFolderClicked(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->addButton(DELETE);
    popup->addButton(CANCEL);
    popup->setTitle(_("IDS_BR_SK_DELETE"));
    popup->setMessage("<b>Delete '" + folder_name + "'?</b><br>If you delete this Folder, All Bookmarks in the folder will also be deleted.");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteFolderPopupClicked, this, _1));
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    m_folder_name = folder_name;
    popup->show();
}

void SimpleUI::onRemoveFoldersClicked(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = items.begin(); it != items.end(); ++it) {
        m_storageService->getFoldersStorage().removeNumberInFolder((*it)->getDir());
        m_favoriteService->deleteBookmark((*it)->getAddress());
    }
    items.clear();
}

void SimpleUI::onEditFolderPopupClicked(const std::string& newName)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_storageService->getFoldersStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = m_storageService->getFoldersStorage().getFolderId(m_folder_name);
        m_storageService->getFoldersStorage().updateFolderName(id, newName);
        m_bookmarkDetailsUI->onBackPressed();
    }
}

void SimpleUI::onDeleteFolderPopupClicked(PopupButtons button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (button == DELETE && m_storageService->getFoldersStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = m_storageService->getFoldersStorage().getFolderId(m_folder_name);
        onRemoveFoldersClicked(m_favoriteService->getBookmarks(id));
        m_storageService->getFoldersStorage().deleteFolder(id);
        m_bookmarkDetailsUI->onBackPressed();
    }
}

void SimpleUI::onUrlIMEOpened(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimpleUI* self = reinterpret_cast<SimpleUI*>(data);
    self->setwvIMEStatus(true);
}

void SimpleUI::onUrlIMEClosed(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimpleUI* self = reinterpret_cast<SimpleUI*>(data);
    self->setwvIMEStatus(false);
}
#endif

void SimpleUI::onSnapshotCaptured(std::shared_ptr<tools::BrowserImage> snapshot, tools::SnapshotType snapshot_type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (snapshot_type) {
    case tools::SnapshotType::ASYNC_LOAD_FINISHED:
        m_historyService->updateHistoryItemSnapshot(m_webEngine->getURI(), snapshot);
        m_tabService->updateTabItemSnapshot(m_webEngine->currentTabId(), snapshot);
        break;
    case tools::SnapshotType::ASYNC_TAB:
        m_tabService->updateTabItemSnapshot(m_webEngine->currentTabId(), snapshot);
        break;
    case tools::SnapshotType::ASYNC_BOOKMARK:
        m_favoriteService->updateBookmarkItemSnapshot(m_webEngine->getURI(), snapshot);
        break;
    case tools::SnapshotType::SYNC:
        BROWSER_LOGE("Synchronized snapshot in asynchronized workflow");
        break;
    default:
        BROWSER_LOGW("Snapshot type is not supported in asynchronized workflow");
    }
}

void SimpleUI::onGenerateThumb(basic_webengine::TabId tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    const int THUMB_WIDTH = boost::any_cast<int>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::HISTORY_TAB_SERVICE_THUMB_WIDTH));
    const int THUMB_HEIGHT = boost::any_cast<int>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::HISTORY_TAB_SERVICE_THUMB_HEIGHT));
    tools::BrowserImagePtr snapshotImage = m_webEngine->getSnapshotData(tabId, THUMB_WIDTH, THUMB_HEIGHT, false, tools::SnapshotType::SYNC);
    m_tabService->updateTabItemSnapshot(tabId, snapshotImage);
}

void SimpleUI::onCreateTabId()
{
    int id = m_tabService->createTabId();
    m_webEngine->onTabIdCreated(id);
}

void SimpleUI::onHistoryRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
}

void SimpleUI::onReturnPressed(MenuButton *m)
{
    BROWSER_LOGD("[%s]", __func__);
    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m->hidePopup();
}

void SimpleUI::setwvIMEStatus(bool status)
{
    BROWSER_LOGD("[%s]", __func__);
    m_wvIMEStatus = status;
}

void SimpleUI::onXF86BackPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_wvIMEStatus && m_webPageUI->getURIEntry().hasFocus())
        m_webPageUI->getURIEntry().clearFocus();
}

void SimpleUI::onBackPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    if (m_findOnPageUI->isVisible())
        closeFindOnPageUI();
    else
#else
    if (m_zoomUI->isVisible()) {
        m_zoomUI->escapeZoom();
    } else
#endif
    if (m_wvIMEStatus) {    // if IME opened
        return;
    } else if (m_popupVector.size() > 0) {
        m_popupVector.back()->onBackPressed();
#if PROFILE_MOBILE
    } else if (m_moreMenuUI->isVisible()) {
        m_moreMenuUI->hideUI();
#endif
    } else if ((m_viewManager.topOfStack() == m_tabUI.get()) && m_tabUI->isEditMode()) {
        m_tabUI->onBackKey();
    } else if ((m_viewManager.topOfStack() == m_bookmarkDetailsUI.get())) {
        m_bookmarkDetailsUI->onBackPressed();
    } else if (m_viewManager.topOfStack() == m_bookmarkManagerUI.get()) {
        m_viewManager.popTheStack();
    } else if (m_viewManager.topOfStack() == nullptr) {
        switchViewToQuickAccess();
    } else if ((m_viewManager.topOfStack() == m_webPageUI.get())) {
        if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
            if (m_quickAccess->canBeBacked(m_webEngine->tabsCount())) {
                m_quickAccess->backButtonClicked();
            } else {
                minimizeBrowser();
            }
        } else {
            m_webEngine->backButtonClicked();
        }
#if PROFILE_MOBILE
    } else if ((m_viewManager.topOfStack() == m_settingsUI.get()) && m_settingsUI->isSubpage()) {
        m_settingsUI->onBackKey();
#endif
    } else {
        m_viewManager.popTheStack();
    }
}

void SimpleUI::showPopup(interfaces::AbstractPopup* popup)
{
    BROWSER_LOGD("[%s]", __func__);
    m_popupVector.push_back(popup);
}

void SimpleUI::dismissPopup(interfaces::AbstractPopup* popup)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::vector<interfaces::AbstractPopup*>::reverse_iterator it = m_popupVector.rbegin();
    for (; it != m_popupVector.rend(); ++it) {
        if (popup == *it) {
            delete *it;
            m_popupVector.erase(--it.base());
            if (m_popupVector.size() > 0) {
                interfaces::AbstractPopup* tmp = m_popupVector.back();
                m_popupVector.pop_back();   // pop_back last popup because it'll be pushed while showing
                tmp->show();
            }
            break;
        }
    }
}

#if !PROFILE_MOBILE
void SimpleUI::onEscapePressed()
{
    BROWSER_LOGD("[%s]", __func__);
    m_zoomUI->escapeZoom();
}
#else
void SimpleUI::onMenuButtonPressed()
{
    BROWSER_LOGD("[%s]", __func__);
    showMoreMenu();
}

void SimpleUI::onRotation()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_current_angle = m_temp_angle;
    elm_win_rotation_with_resize_set(main_window, m_current_angle);
    m_bookmarkDetailsUI->setLandscape((m_current_angle % 180) == 0);
    m_moreMenuUI->resetContent();
    m_bookmarkFlowUI->resetContent();
    m_settingsUI->orientationChanged();
    m_bookmarkManagerUI->orientationChanged();
    m_webPageUI->orientationChanged();
    m_tabUI->orientationChanged();
    m_webEngine->orientationChanged();
    if (!m_popupVector.empty())
        m_popupVector.back()->orientationChanged();
}

void SimpleUI::__orientation_changed(void* data, Evas_Object*, void*)
{
    SimpleUI* simpleUI = static_cast<SimpleUI*>(data);
    int event_angle = elm_win_rotation_get(simpleUI->main_window);
    if (simpleUI->m_current_angle != event_angle) {
        simpleUI->m_temp_angle = event_angle;
        BROWSER_LOGD("[%s:%d] previous angle: [%d] event angle: [%d]", __PRETTY_FUNCTION__, __LINE__,
                        simpleUI->m_current_angle, simpleUI->m_temp_angle);
        simpleUI->onRotation();
    }
}

bool SimpleUI::isLandscape()
{
    return elm_win_rotation_get(main_window) % 180;
}

int SimpleUI::getRotation()
{
    return elm_win_rotation_get(main_window);
}
#endif

void SimpleUI::reloadEnable(bool enable)
{
    m_webPageUI->setReloadButtonEnabled(enable);
}

void SimpleUI::downloadStarted(int status)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());

    switch(status)
    {
        case DOWNLOAD_UNABLE_TO_DOWNLOAD:
            popup->show(_("IDS_BR_HEADER_UNABLE_TO_DOWNLOAD_ABB"), false);
            break;
        case DOWNLOAD_STARTING_DOWNLOAD:
            popup->show(_("IDS_BR_POP_STARTING_DOWNLOAD_ING"), false);
            break;
        case DOWNLOAD_SAVEDPAGES:
            popup->show(_("IDS_BR_OPT_SAVEDPAGES"), false);
            break;
        case DOWNLOAD_FAIL:
            popup->show(_("IDS_BR_POP_FAIL"), false);
            break;
        case DOWNLOAD_ONLY_HTTP_OR_HTTPS_URLS:
            popup->show(_("IDS_BR_POP_ONLY_HTTP_OR_HTTPS_URLS_CAN_BE_DOWNLOADED"), false);
            break;
        default:
            break;
    }
    popup->dismiss();
}

void SimpleUI::stopEnable(bool enable)
{
    m_webPageUI->setStopButtonEnabled(enable);
}

void SimpleUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->loadStarted();
#if PROFILE_MOBILE
    if (m_findOnPageUI->isVisible())
        closeFindOnPageUI();
#endif
}

void SimpleUI::progressChanged(double progress)
{
    m_webPageUI->progressChanged(progress);
}

void SimpleUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->loadFinished();
#if PROFILE_MOBILE
    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId())) {
        m_tabService->updateTabItem(m_webEngine->currentTabId(),
                m_webEngine->getURI(),
                m_webEngine->getTitle(),
                m_webEngine->getOrigin());
        m_historyService->addHistoryItem(m_webEngine->getURI(),
                                                 m_webEngine->getTitle(),
                                                 m_webEngine->getFavicon());
    }
#endif
}

void SimpleUI::loadError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if !PROFILE_MOBILE
    m_webPageUI->switchViewToErrorPage();
#endif
}

void SimpleUI::filterURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] url=%s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    //check for special urls (like:  'about:home')
    //if there will be more addresses may be we should
    //create some kind of std::man<std::string url, bool *(doSomethingWithUrl)()>
    //and then just map[url]() ? m_webEngine->setURI(url) : /*do nothing*/;;
    if(/*url.empty() ||*/ url == HomePageURL){
        m_webPageUI->getURIEntry().changeUri("");
    } else if (!url.empty()){

    //check if url is in favorites

    //check if url is in blocked

    //no filtering

        if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
            openNewTab(url, "", boost::none, false, false, basic_webengine::TabOrigin::QUICKACCESS);
        else
            m_webEngine->setURI(url);

        if (m_webEngine->isPrivateMode(m_webEngine->currentTabId()) ||
                m_webPageUI->stateEquals(WPUState::MAIN_ERROR_PAGE))
            switchViewToWebPage();
    }
    m_webPageUI->getURIEntry().clearFocus();
}

void SimpleUI::onURLEntryEditedByUser(const std::shared_ptr<std::string> editedUrlPtr)
{
    string editedUrl(*editedUrlPtr);
    int historyItemsVisibleMax =
            m_webPageUI->getUrlHistoryList()->getItemsNumberMax();
    int minKeywordLength =
            m_webPageUI->getUrlHistoryList()->getKeywordLengthMin();
    std::shared_ptr<services::HistoryItemVector> result =
            m_historyService->getHistoryItemsByKeywordsString(editedUrl,
                    historyItemsVisibleMax, minKeywordLength, true);
    m_webPageUI->getUrlHistoryList()->onURLEntryEditedByUser(editedUrl, result);
}

#if !PROFILE_MOBILE
void SimpleUI::onRedKeyPressed()
{
    m_webPageUI->onRedKeyPressed();
}

void SimpleUI::onYellowKeyPressed()
{
    m_webPageUI->onYellowKeyPressed();
}

void SimpleUI::showZoomUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(! m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_viewManager.popStackTo(m_webPageUI.get());
        m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::closeZoomUI, this));
        m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::closeZoomUI, this));
        m_zoomUI->show(m_window.get());
    }
}

void SimpleUI::closeZoomUI()
{
    M_ASSERT(m_zoomUI);
    m_zoomUI->hideUI();
}

void SimpleUI::setZoomFactor(int level)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->setZoomFactor(level);
}

int SimpleUI::getZoomFactor()
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, m_webEngine->getZoomFactor());
    return m_webEngine->getZoomFactor();
}
#endif

void SimpleUI::scrollView(const int& dx, const int& dy)
{
    m_webEngine->scrollView(dx, dy);
}

#if PROFILE_MOBILE
void SimpleUI::webEngineURLChanged(const std::string url)
{
    BROWSER_LOGD("webEngineURLChanged:%s", url.c_str());
    if (m_moreMenuUI->isVisible())
        m_moreMenuUI->updateBookmarkButton();
}

void SimpleUI::showFindOnPageUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_findOnPageUI);
    m_findOnPageUI->show();
    if (!str.empty())
        m_findOnPageUI->set_text(str.c_str());
}

void SimpleUI::findWord(const struct FindData& fdata)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->findWord(fdata.input_str, fdata.forward, fdata.func, fdata.data);
}

void SimpleUI::closeFindOnPageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_findOnPageUI);
    if (m_findOnPageUI)
        m_findOnPageUI->hideUI();
}
#endif

void SimpleUI::showTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_tabUI.get());

    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) && m_webEngine->tabsCount() > 0 && m_webEngine->isLoading())
        onGenerateThumb(m_webEngine->currentTabId());
    std::vector<basic_webengine::TabContentPtr> tabsContents =
            m_webEngine->getTabContents();
    m_tabService->fillThumbs(tabsContents);
    m_tabUI->addTabItems(tabsContents);
}

void SimpleUI::closeTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_tabUI.get())
        m_viewManager.popTheStack();
}

void SimpleUI::newTabClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!checkIfCreate())
        return;

    switchViewToQuickAccess();
}

void SimpleUI::tabClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.popStackTo(m_webPageUI.get());
    m_webPageUI->toIncognito(m_webEngine->isPrivateMode(tabId));
    switchToTab(tabId);
}

bool SimpleUI::isIncognito(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__, __func__);
    return m_webEngine->isPrivateMode(tabId);
}

void SimpleUI::closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->closeTab(tabId);
}

int SimpleUI::tabsCount()
{
    return m_webEngine->tabsCount();
}

#if PROFILE_MOBILE
void SimpleUI::handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (webConfirmation->getConfirmationType() == basic_webengine::WebConfirmation::ConfirmationType::CertificateConfirmation) {
        if (m_webPageUI->stateEquals(WPUState::MAIN_WEB_PAGE)) {
            auto cert = std::dynamic_pointer_cast<basic_webengine::CertificateConfirmation, basic_webengine::WebConfirmation>(webConfirmation);
            auto type = m_certificateContents->isCertExistForHost(cert->getURI());
            if (type == services::CertificateContents::UNSECURE_HOST_ALLOWED) {
                webConfirmation->setResult(tizen_browser::basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
                m_webEngine->confirmationResult(webConfirmation);
            } else {
                m_webPageUI->getURIEntry().changeUri(webConfirmation->getURI());
                TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
                popup->addButton(BACK_TO_SAFETY, true, true);
                popup->addButton(VIEW_CERTIFICATE, false);
                popup->addButton(CONTINUE);
                auto popupData = std::make_shared<CertificatePopupData>();
                popupData->cert = cert;
                popup->buttonClicked.connect(boost::bind(&SimpleUI::certPopupButtonClicked, this, _1, popupData));
                popup->setTitle(_("IDS_BR_HEADER_SITE_NOT_TRUSTED_ABB"));
                popup->setMessage(_("IDS_BR_BODY_SECURITY_CERTIFICATE_PROBLEM_MSG"));
                popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
                popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
                popup->show();
            }
        }
    } else {
        BROWSER_LOGW("[%s:%d] Unknown WebConfirmation::ConfirmationType!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::showCertificatePopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string uri = tools::extractDomain(m_webEngine->getURI());
    services::CertificateContents::HOST_TYPE type = m_certificateContents->isCertExistForHost(uri);
    std::string pem = m_storageService->getCertificateStorage().getPemForURI(uri);
    showCertificatePopup(uri, pem, type);
}

void SimpleUI::showUnsecureConnectionPopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->addButton(OK);
    popup->setTitle("Unsecure connection!");
    popup->setMessage("The page which you're trying to open cannot be displayed, unsecure connection detected.");
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}

void SimpleUI::showCertificatePopup(const std::string& host, const std::string& pem, services::CertificateContents::HOST_TYPE type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto popup = ContentPopup::createPopup(m_viewManager.getContent());
    popup->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    m_certificateContents->initUI(popup->getMainLayout());
    m_certificateContents->setCurrentTabCertData(host, pem, type);
    popup->setContent(m_certificateContents->getContent());
    popup->addButton(OK);
    popup->setTitle(_("IDS_BR_HEADER_SECURITY_CERTIFICATE"));
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}
#endif

void SimpleUI::certPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::shared_ptr<CertificatePopupData> certPopupData = std::dynamic_pointer_cast<CertificatePopupData, PopupData>(popupData);
    switch (button) {
        case OK:
            break;
        case CONTINUE:
        {
            certPopupData->cert->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
            m_webEngine->confirmationResult(certPopupData->cert);
            std::string uri = certPopupData->cert->getURI();
            std::string pem = certPopupData->cert->getPem();
            m_certificateContents->saveWrongCertificateInfo(uri, pem);
            break;
        }
        case BACK_TO_SAFETY:
            certPopupData->cert->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Rejected);
            m_webEngine->confirmationResult(certPopupData->cert);
            break;
        case VIEW_CERTIFICATE:
        {
#if PROFILE_MOBILE
            showCertificatePopup(certPopupData->cert->getURI(), certPopupData->cert->getPem(), services::CertificateContents::UNSECURE_HOST_UNKNOWN);
#endif
            break;
        }
        default:
            BROWSER_LOGW("[%s:%d] Unknown button type!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::showHistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_historyUI.get());
    m_historyUI->addHistoryItems(m_historyService->getHistoryToday(),
            HistoryPeriod::HISTORY_TODAY);
    m_historyUI->addHistoryItems(m_historyService->getHistoryYesterday(),
            HistoryPeriod::HISTORY_YESTERDAY);
    m_historyUI->addHistoryItems(m_historyService->getHistoryLastWeek(),
            HistoryPeriod::HISTORY_LASTWEEK);
    m_historyUI->addHistoryItems(m_historyService->getHistoryLastMonth(),
            HistoryPeriod::HISTORY_LASTMONTH);
    m_historyUI->addHistoryItems(m_historyService->getHistoryOlder(),
            HistoryPeriod::HISTORY_OLDER);
}

void SimpleUI::closeHistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_historyUI.get())
        m_viewManager.popTheStack();
}

void SimpleUI::showSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_settingsUI.get());
}

void SimpleUI::closeSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_settingsUI.get())
        m_viewManager.popTheStack();
}

void SimpleUI::showMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_ASSERT(m_webPageUI);
    if (m_moreMenuUI->isVisible())
        m_moreMenuUI->hideUI();
    else {
        m_moreMenuUI->shouldShowFindOnPage(!m_webEngine->getURI().empty());
        m_moreMenuUI->blockThumbnails(m_webPageUI->stateEquals(WPUState::QUICK_ACCESS));
        m_webEngine->moreKeyPressed();
        m_moreMenuUI->showUI();
    }
#else
    bool desktopMode = m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) ? m_quickAccess->isDesktopMode() : m_webEngine->isDesktopMode();
    m_moreMenuUI->setDesktopMode(desktopMode);
    m_viewManager.pushViewToStack(m_moreMenuUI.get());
    m_moreMenuUI->showCurrentTab();

    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_moreMenuUI->setFavIcon(m_webEngine->getFavicon());
        m_moreMenuUI->setWebTitle(m_webEngine->getTitle());
        m_moreMenuUI->setURL(m_webEngine->getURI());
    }
    else {
        m_moreMenuUI->setHomePageInfo();
    }
#endif
}

void SimpleUI::closeMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_ASSERT(m_webPageUI);
    if (m_moreMenuUI->isVisible())
        m_moreMenuUI->hideUI();
#else
    if (m_viewManager.topOfStack() == m_moreMenuUI.get())
        m_viewManager.popTheStack();
    else
        BROWSER_LOGD("[%s:%d] WARNING!!! closeMoreMenu is not topOfStack", __PRETTY_FUNCTION__, __LINE__);
#endif
}

void SimpleUI::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_webEngine->switchToMobileMode();
        m_viewManager.popStackTo(m_webPageUI.get());
        m_webEngine->reload();
    } else {
        m_quickAccess->setDesktopMode(false);
    }
}

void SimpleUI::switchToDesktopMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_webEngine->switchToDesktopMode();
        m_webEngine->reload();
    } else {
        m_quickAccess->setDesktopMode(true);
    }
}

void SimpleUI::showBookmarkFlowUI(bool state)
{
#if !PROFILE_MOBILE
    if (state) {
        deleteBookmark();
        return;
    }
#endif
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    m_viewManager.pushViewToStack(m_bookmarkFlowUI.get());
    std::string uri = m_webEngine->getURI();
    m_bookmarkFlowUI->setURL(uri);
    m_bookmarkFlowUI->setState(state);
    tizen_browser::services::BookmarkItem item;
    if(m_favoriteService->bookmarkExists(uri) && m_favoriteService->getItem(uri, &item))
        m_bookmarkFlowUI->setTitle(item.getTitle());
    else
        m_bookmarkFlowUI->setTitle(m_webEngine->getTitle());
    m_bookmarkFlowUI->addCustomFolders(m_storageService->getFoldersStorage().getFolders());
    unsigned int id = state ? item.getDir() : m_storageService->getFoldersStorage().SpecialFolder;
    m_bookmarkFlowUI->setFolder(id, m_storageService->getFoldersStorage().getFolderName(id));
#else
    BookmarkFlowUI *bookmarkFlow = BookmarkFlowUI::createPopup(m_viewManager.getContent());
    bookmarkFlow->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    bookmarkFlow->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    bookmarkFlow->addFolder.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    bookmarkFlow->saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    bookmarkFlow->show();
    bookmarkFlow->addNewFolder();
    bookmarkFlow->addCustomFolders(m_storageService->getFoldersStorage().getFolders());
#endif
}
#if PROFILE_MOBILE
void SimpleUI::closeBookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_bookmarkFlowUI.get())
        m_viewManager.popTheStack();
}
#endif

void SimpleUI::showBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_bookmarkManagerUI.get());
#if PROFILE_MOBILE
    m_bookmarkManagerUI->addNewFolder();
#endif
    m_bookmarkManagerUI->addCustomFolders(m_storageService->getFoldersStorage().getFolders());

    m_bookmarkManagerUI->showUI();
}

void SimpleUI::redirectedWebPage(const std::string& oldUrl, const std::string& newUrl)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("Redirect from %s to %s", oldUrl.c_str(), newUrl.c_str());
    m_historyService->clearURLHistory(oldUrl);
}

void SimpleUI::onBookmarkAllFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_bookmarkDetailsUI.get());
    m_bookmarkDetailsUI->addBookmarks(m_favoriteService->getBookmarks(tizen_browser::services::ALL_BOOKMARKS_ID),
            _("IDS_BR_BODY_ALL"));
    m_bookmarkDetailsUI->showUI();
}

void SimpleUI::onBookmarkSpecialFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_bookmarkDetailsUI.get());
    m_bookmarkDetailsUI->addBookmarks(m_favoriteService->getBookmarks(m_storageService->getFoldersStorage().SpecialFolder),
                                    m_storageService->getFoldersStorage().getFolderName(m_storageService->getFoldersStorage().SpecialFolder));
    m_bookmarkDetailsUI->showUI();
}

void SimpleUI::onBookmarkCustomFolderClicked(int folderId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_bookmarkDetailsUI.get());
    m_bookmarkDetailsUI->addBookmarks(m_favoriteService->getBookmarks(folderId), m_storageService->getFoldersStorage().getFolderName(folderId));
    m_bookmarkDetailsUI->showUI();
}

void SimpleUI::closeBookmarkDetailsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_bookmarkDetailsUI.get())
        m_viewManager.popTheStack();
    showBookmarkManagerUI();
}

void SimpleUI::closeBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_bookmarkManagerUI.get())
    m_viewManager.popTheStack();
}

void SimpleUI::settingsDeleteSelectedData(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting selected data", __func__);
    M_ASSERT(m_viewManager);
    if((str.find("CACHE")    != std::string::npos)  ||
       (str.find("COOKIES")  != std::string::npos)  ||
       (str.find("HISTORY")  != std::string::npos)  ||
       (str.find("PASSWORD") != std::string::npos)  ||
       (str.find("FORMDATA") != std::string::npos)) {
#if PROFILE_MOBILE
           TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
           popup->addButton(OK);
           popup->addButton(CANCEL);
#else
           SimplePopup* popup = SimplePopup::createPopup(m_viewManager.getContent());
           popup->addButton(OK);
           popup->addButton(CANCEL);
#endif
           popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteSelectedDataButton, this, _1, str));
           popup->setTitle("Delete");
           popup->setMessage("The selected web browsing data will be deleted.");
           popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
           popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
           popup->show();
    }
}

void SimpleUI::onDeleteSelectedDataButton(const PopupButtons& button, const std::string& dataText)
{
    BROWSER_LOGD("[%s]: TYPE : %s", __func__, dataText.c_str());
    if(button == OK){
        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Delete Web Browsing Data");

        if (dataText.find("CACHE") != std::string::npos)
            m_webEngine->clearCache();
        if (dataText.find("COOKIES") != std::string::npos)
            m_webEngine->clearCookies();
        if (dataText.find("HISTORY") != std::string::npos)
            m_historyService->clearAllHistory();
        if (dataText.find("PASSWORD") != std::string::npos)
            m_webEngine->clearPasswordData();
        if (dataText.find("FORMDATA") != std::string::npos)
            m_webEngine->clearFormData();

        popup->dismiss();
    }
}

void SimpleUI::settingsResetMostVisited()
{
    BROWSER_LOGD("[%s]: Deleting most visited sites", __func__);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
    popup->show("Resetting most visited sites.");
    onDeleteMostVisitedButton(nullptr);
    popup->dismiss();
}

void SimpleUI::onDeleteMostVisitedButton(std::shared_ptr< PopupData > /*popupData*/)
{
    BROWSER_LOGD("[%s]: Deleting most visited", __func__);
    m_historyService->cleanMostVisitedHistoryItems();
}

void SimpleUI::settingsResetBrowser()
{
    BROWSER_LOGD("[%s]: Resetting browser", __func__);
#if PROFILE_MOBILE
    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->addButton(RESET);
    popup->addButton(CANCEL);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, nullptr));
#else
    SimplePopup* popup = SimplePopup::createPopup(m_viewManager.getContent());
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, _2));
#endif
    popup->setTitle("Reset Browser");
    popup->setMessage(ResetBrowserPopupMsg);
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}

void SimpleUI::onResetBrowserButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if (button == OK || button == RESET) {
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Resetting browser", __func__);

        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Reset Browser");

        m_webEngine->clearPrivateData();
        m_historyService->clearAllHistory();
        m_favoriteService->deleteAllBookmarks();
        m_webEngine->clearPasswordData();
        m_webEngine->clearFormData();

        // Close all openend tabs
        auto openedTabs = m_webEngine->getTabContents();
        for (const auto& it : openedTabs)
            m_webEngine->closeTab(it.get()->getId());
        m_tabService->clearAll();
        m_certificateContents->clear();
        m_storageService->getCertificateStorage().deleteAllEntries();
        m_storageService->getFoldersStorage().deleteAllFolders();
#if PROFILE_MOBILE
        m_webEngine->resetSettingsParam();
        m_storageService->getSettingsStorage().resetSettings();
#endif
        //TODO: add here any missing functionality that should be cleaned.

        popup->dismiss();
    }
}

#if PROFILE_MOBILE
void SimpleUI::settingsOverrideUseragent(const std::string& userAgent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Open a webpage to perform this operation.");
        popup->dismiss();
        m_settingsUI->onBackKey();
        return;
    }

    if (userAgent.empty()) {
        std::string currentUserAgent = m_webEngine->getUserAgent();
        InputPopup *inputPopup = InputPopup::createPopup(m_viewManager.getContent(), "Override UserAgent", "",
                                                        currentUserAgent, _("IDS_BR_SK_DONE"), _("IDS_BR_SK_CANCEL_ABB"), true);
        inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onOverrideUseragentButton, this, _1));
        inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
        inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
        inputPopup->show();
    }
    else {
        onOverrideUseragentButton(userAgent);
    }
}

void SimpleUI::onOverrideUseragentButton(const std::string& newUA)
{
    BROWSER_LOGD("[%s]: Overriding useragent", __func__);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
    m_webEngine->setUserAgent(newUA);
    popup->show("UserAgent updated..");
    popup->dismiss();
    m_settingsUI->onBackKey();
}
#endif

#if PROFILE_MOBILE
void SimpleUI::tabLimitPopupButtonClicked(PopupButtons button)
#else
void SimpleUI::tabLimitPopupButtonClicked(PopupButtons button, std::shared_ptr< PopupData > /*popupData */)
#endif
{
    if (button == CLOSE_TAB) {
        BROWSER_LOGD("[%s]: CLOSE TAB", __func__);
        closeTab();
    }
}

void SimpleUI::tabCreated()
{
    int tabs = m_webEngine->tabsCount();
    m_webPageUI->setTabsNumber(tabs);
}

bool SimpleUI::checkIfCreate()
{
    int tabs = m_webEngine->tabsCount();

    if (tabs >= m_tabLimit) {
#if PROFILE_MOBILE
        TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
        popup->addButton(OK);
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1));
#else
        SimplePopup *popup = SimplePopup::createPopup(m_viewManager.getContent());
        popup->addButton(OK);
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1, _2));
#endif
        popup->setTitle(_("Maximum tab count reached"));
        popup->setMessage("Close other tabs to open another new tab");
        popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
        popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
        popup->show();
        return false;
    }
    else
        return true;
}

void SimpleUI::updateView() {
    int tabs = m_webEngine->tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (tabs == 0) {
        switchViewToQuickAccess();
    } else if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        switchViewToWebPage();
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::windowCreated()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
}

void SimpleUI::minimizeBrowser()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_win_lower(main_window);
}

void SimpleUI::tabClosed(const tizen_browser::basic_webengine::TabId& id) {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabService->removeTab(id);
    updateView();
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    m_webEngine->searchOnWebsite(text, flags);
}

void SimpleUI::addBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_favoriteService) {
        if (m_webEngine && !m_webEngine->getURI().empty()) {
            const int THUMB_HEIGHT = boost::any_cast<int>(
                    tizen_browser::config::Config::getInstance().get(CONFIG_KEY::FAVORITESERVICE_THUMB_HEIGHT));
            const int THUMB_WIDTH = boost::any_cast<int>(
                    tizen_browser::config::Config::getInstance().get(CONFIG_KEY::FAVORITESERVICE_THUMB_WIDTH));
            m_favoriteService->addBookmark(m_webEngine->getURI(),
#if PROFILE_MOBILE
                                           bookmark_update.title,
#else
                                           m_webEngine->getTitle(),
#endif
                                           std::string(), m_webEngine->getSnapshotData(THUMB_WIDTH, THUMB_HEIGHT, tools::SnapshotType::ASYNC_BOOKMARK),
                                           m_webEngine->getFavicon(), bookmark_update.folder_id);
            m_storageService->getFoldersStorage().addNumberInFolder(bookmark_update.folder_id);
        }
    }
}

#if PROFILE_MOBILE
void SimpleUI::editBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_favoriteService) {
        if (m_webEngine && !m_webEngine->getURI().empty()) {
            services::BookmarkItem oldItem;
            m_favoriteService->getItem(m_webEngine->getURI(), &oldItem);
            if (m_favoriteService->editBookmark(m_webEngine->getURI(), bookmark_update.title, bookmark_update.folder_id)) {
                m_storageService->getFoldersStorage().removeNumberInFolder(oldItem.getDir());
                m_storageService->getFoldersStorage().addNumberInFolder(bookmark_update.folder_id);
            }
        }
    }
}
#endif

//TODO: Replace by direct call.
void SimpleUI::deleteBookmark()
{
    std::string uri = m_webEngine->getURI();
    tizen_browser::services::BookmarkItem item;
    if (m_favoriteService->bookmarkExists(uri) && m_favoriteService->getItem(uri, &item))
        m_storageService->getFoldersStorage().removeNumberInFolder(item.getDir());
    m_favoriteService->deleteBookmark(uri);
}
}
}
