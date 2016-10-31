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
#include <thread>
#include <future>
#include <functional>
#include <appcore-common.h>

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
#include "UrlHistoryList/UrlHistoryList.h"
#include "NotificationPopup.h"
#include "ContentPopup_mob.h"
#include "RadioPopup.h"
#include "Tools/GeneralTools.h"
#include "Tools/SnapshotType.h"
#include "SettingsPrettySignalConnector.h"
#include "net_connection.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const std::string ResetBrowserPopupMsg = "Do you really want to reset Browser?" \
                                         " If you press Reset, delete all data" \
                                         " and return to initial setting.";
const int SCALE_FACTOR = 720;

SimpleUI::SimpleUI()
    : AbstractMainWindow()
    , m_webPageUI()
    , m_bookmarkFlowUI()
    , m_findOnPageUI()
    , m_bookmarkManagerUI()
    , m_quickAccess()
    , m_historyUI()
    , m_settingsUI()
    , m_tabUI()
    , m_initialised(false)
    , m_tabLimit(0)
    , m_favoritesLimit(0)
    , m_wvIMEStatus(false)
#if PWA
    , m_pwa()
    , m_alreadyOpenedPWA(false)
#endif
    , m_manualRotation(false)
    , m_current_angle(0)
    , m_temp_angle(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_init(0, nullptr);

    main_window = elm_win_util_standard_add("browserApp", "browserApp");

    int width;
    elm_win_screen_size_get(main_window, nullptr, nullptr, &width, nullptr);
    double config_scale_value = (double)(width)/SCALE_FACTOR;
    config::Config::getInstance().set(
            "scale", static_cast<double>(elm_config_scale_get()/config_scale_value));
    m_tabLimit = boost::any_cast<int>(config::Config::getInstance().get("TAB_LIMIT"));
    m_favoritesLimit = boost::any_cast<int>(config::Config::getInstance().get("FAVORITES_LIMIT"));

    elm_win_conformant_set(main_window, EINA_TRUE);
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");

    setMainWindow(main_window);
    m_viewManager.init(main_window);

    elm_win_resize_object_add(main_window, m_viewManager.getConformant());
    evas_object_show(main_window);

    if (elm_win_wm_rotation_supported_get(main_window)) {
        rotationType(rotationLock::noLock);
        evas_object_smart_callback_add(main_window, "wm,rotation,changed", __orientation_changed, this);
        enableManualRotation(false);
    } else
        BROWSER_LOGW("[%s:%d] Device does not support rotation.", __PRETTY_FUNCTION__, __LINE__);

    // TODO Unify the virtual keyboard behavior. For now webview entry and url entry have the separate ways to
    // determine if keyboard has been shown. I think it is possible to unify it with below callbacks.
    evas_object_smart_callback_add(m_viewManager.getConformant(), "virtualkeypad,state,on", onUrlIMEOpened, this);
    evas_object_smart_callback_add(m_viewManager.getConformant(), "virtualkeypad,state,off",onUrlIMEClosed, this);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_functionViewPrepare();
#if DUMMY_BUTTON
    m_webPageUI->createDummyButton();
#endif
    m_webEngine->resume();
    if (m_findOnPageUI && m_findOnPageUI->isVisible())
        m_findOnPageUI->show_ime();
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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

void SimpleUI::setMostVisitedFrequencyValue(
    std::shared_ptr<services::HistoryItem> historyItem, int visitFrequency)
{
    m_historyService->setMostVisitedFrequency(historyItem->getId(), visitFrequency);
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getHistory()
{
    return m_historyService->getHistoryToday();
}

void SimpleUI::prepareServices()
{
    loadUIServices();
    loadModelServices();

    auto futureModelSig =
        std::async(std::launch::async, [this](){connectModelSignals();});
    auto futureUISig
        = std::async(std::launch::async, [this](){connectUISignals();});
    futureModelSig.get();
    futureUISig.get();

    // initModelServices() needs to be called after initUIServices()
    initUIServices();
    initModelServices();
}
#if PWA
std::string SimpleUI::preparePWA(const std::string& url)
{
    std::string startUrl;
    if (!strncmp(url.c_str(), "browser_shortcut::", strlen("browser_shortcut::"))) {
        BROWSER_LOGD("Progressive web app");
        m_pwa.preparePWAParameters(url);
        startUrl = m_pwa.getPWAinfo().uri;
        if (startUrl.empty())
            return std::string();
        BROWSER_LOGD("Display mode: %d", m_pwa.getPWAinfo().displayMode);
        m_webPageUI->setDisplayMode(
            static_cast<WebPageUI::WebDisplayMode>(
                m_pwa.getPWAinfo().displayMode));

        if (m_pwa.getPWAinfo().orientation ==  WebPageUI::WebScreenOrientationLockPortrait)
            rotationType(rotationLock::portrait);
        else if (m_pwa.getPWAinfo().orientation == WebPageUI::WebScreenOrientationLockLandscape)
            rotationType(rotationLock::landscape);
        return startUrl;
    }
    return std::string();
}
#endif

int SimpleUI::exec(const std::string& _url, const std::string& _caller, const std::string& _operation)
{
    BROWSER_LOGD(
        "[%s] _url=%s, _caller=%s, _operation=%s, initialised=%d",
        __func__,
        _url.c_str(),
        _caller.c_str(),
        _operation.c_str(),
        m_initialised);
    std::string url = _url;
    std::string operation = _operation;
    m_caller = _caller;
    m_alreadyOpenedExecURL = false;
    m_functionViewPrepare = [url, operation, this]() mutable {
        if (!m_initialised) {
            if (m_window.get()) {
                prepareServices();

                //Push first view to stack.
                pushViewToStack(m_webPageUI);

                // Register H/W back key callback
                m_platformInputManager->registerHWKeyCallback(m_viewManager.getContent());
            }

            if (url.empty()) {
                BROWSER_LOGD("[%s]: restore last session", __func__);
                switchViewToQuickAccess();
                restoreLastSession();
            }
            m_initialised = true;
        }
        std::string pwaUrl = std::string();
#if PWA
        // Progressive web app
        pwaUrl = preparePWA(url);

        if ((!pwaUrl.empty() && m_webEngine->getState() != basic_webengine::State::SECRET) ||
            (pwaUrl.empty() && m_webEngine->getState() == basic_webengine::State::SECRET))
            changeEngineState();

        m_webPageUI->updateEngineStateUI();
#endif

        if (!pwaUrl.empty() || (!url.empty() && !m_alreadyOpenedExecURL)) {
            BROWSER_LOGD("[%s]: open new tab", __func__);
            std::string newUrl = url;
            if (!operation.compare(APP_CONTROL_OPERATION_SEARCH)) {
                newUrl = m_webPageUI->getURIEntry().rewriteURI(url);
                popStackTo(m_webPageUI);
            }
            auto taburl = pwaUrl.empty() ? newUrl : pwaUrl;
#if PWA
            // Allow for only one instance of PWA
            if (m_alreadyOpenedPWA)
                m_webEngine->closeTab();
            if (!pwaUrl.empty())
                popStackTo(m_webPageUI);
            m_alreadyOpenedPWA = !pwaUrl.empty();
#endif

            openNewTab(taburl);
            m_alreadyOpenedExecURL = true;
        }
    };
    BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());
    return 0;
}

#if PWA
void SimpleUI::countCheckUrl()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int id, ret = 0;
    id = m_historyService->getHistoryId(m_webEngine->getURI());

    if (id)
        ret = m_historyService->getHistoryCnt(id);

    if (ret == CONNECT_COUNT)
        pwaPopupRequest();
    else
        BROWSER_LOGD("[%s:%d] url count : %d", __PRETTY_FUNCTION__, __LINE__, ret);
}
#endif

void SimpleUI::faviconChanged(tools::BrowserImagePtr favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webEngine->isLoading())
        m_historyService->updateHistoryItemFavicon(m_webEngine->getURI(), favicon);
}

void SimpleUI::restoreLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto vec(m_tabService->getAllTabs());
    for (const auto& i : *vec) {
        openNewTab(
            i.getUrl(),
            i.getTitle(),
            boost::optional<int>(i.getId().get()),
            false,
            i.getOrigin());
    }
}

void SimpleUI::loadUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI =
        std::dynamic_pointer_cast<base_ui::WebPageUI, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.webpageui"));

    m_quickAccess =
        std::dynamic_pointer_cast<base_ui::QuickAccess, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.quickaccess"));

    m_historyUI =
        std::dynamic_pointer_cast<base_ui::HistoryUI, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.historyui"));

    m_tabUI =
        std::dynamic_pointer_cast<base_ui::TabUI, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.tabui"));

    auto futureSettings(std::async(std::launch::async, [this](){
        m_settingsManager =
            std::dynamic_pointer_cast<SettingsManager, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.settingsui"));
    }));
    auto futureBookmarkFlow(std::async(std::launch::async, [this](){
        m_bookmarkFlowUI =
            std::dynamic_pointer_cast<base_ui::BookmarkFlowUI, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkflowui"));
    }));
    auto futureFindOnPage(std::async(std::launch::async, [this](){
        m_findOnPageUI =
            std::dynamic_pointer_cast<base_ui::FindOnPageUI, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.findonpageui"));
    }));
    auto futureBookmarksMan(std::async(std::launch::async, [this](){
        m_bookmarkManagerUI =
            std::dynamic_pointer_cast<base_ui::BookmarkManagerUI, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkmanagerui"));
    }));
    futureSettings.get();
    futureBookmarkFlow.get();
    futureFindOnPage.get();
    futureBookmarksMan.get();
}

void SimpleUI::loadModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine =
        std::dynamic_pointer_cast
        <basic_webengine::AbstractWebEngine, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    m_historyService =
        std::dynamic_pointer_cast
        <services::HistoryService, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));

    m_platformInputManager =
        std::dynamic_pointer_cast
        <services::PlatformInputManager, core::AbstractService>(
            core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));

    auto futureStorage(std::async(std::launch::async, [this](){
        m_storageService = std::dynamic_pointer_cast
            <services::StorageService, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.storageservice"));
    }));
    auto futureTab(std::async(std::launch::async, [this](){
        m_tabService = std::dynamic_pointer_cast<
            services::TabService, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.tabservice"));
    }));
    auto futureFavourite(std::async(std::launch::async, [this](){
        m_favoriteService = std::dynamic_pointer_cast
            <interfaces::AbstractFavoriteService, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));
    }));
    auto futureCertificates(std::async(std::launch::async, [this](){
        m_certificateContents = std::dynamic_pointer_cast
            <services::CertificateContents, core::AbstractService>(
                core::ServiceManager::getInstance().getService("org.tizen.browser.certificateservice"));
    }));
    futureStorage.get();
    futureTab.get();
    futureFavourite.get();
    futureCertificates.get();
}

void SimpleUI::connectWebPageSignals()
{
    M_ASSERT(m_webPageUI.get());
    m_webPageUI->getURIEntry().uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
    m_webPageUI->backPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->backPage.connect(boost::bind(&basic_webengine::AbstractWebEngine::back, m_webEngine.get()));
    m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_webPageUI->showBookmarksUI.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this,
        m_favoriteService->getRoot(), BookmarkManagerState::Default));
    m_webPageUI->showHomePage.connect(boost::bind(&SimpleUI::showHomePage, this));
    m_webPageUI->forwardPage.connect(boost::bind(&basic_webengine::AbstractWebEngine::forward, m_webEngine.get()));
    m_webPageUI->showQuickAccess.connect(boost::bind(&SimpleUI::showQuickAccess, this));
    m_webPageUI->hideQuickAccess.connect(boost::bind(&QuickAccess::hideUI, m_quickAccess));
    m_webPageUI->getQuickAccessEditUI()->requestQuickAccessGengrid.connect(
        boost::bind(&QuickAccess::getQuickAccessGengrid, m_quickAccess.get()));
    m_webPageUI->getQuickAccessEditUI()->requestMostVisitedGengrid.connect(
        boost::bind(&QuickAccess::getMostVisitedGengrid, m_quickAccess.get()));
    m_webPageUI->getQuickAccessEditUI()->requestQuickAccessState.connect(
        boost::bind(&QuickAccess::getQuickAccessState, m_quickAccess.get()));
    m_webPageUI->getQuickAccessEditUI()->editingFinished.connect(
        boost::bind(&QuickAccess::editingFinished, m_quickAccess.get()));
    m_webPageUI->getQuickAccessEditUI()->editingFinished.connect(
        boost::bind(&WebPageUI::editingFinished, m_webPageUI.get()));
    m_webPageUI->getQuickAccessEditUI()->deleteSelectedMostVisitedItems.connect(
        boost::bind(&QuickAccess::deleteSelectedMostVisitedItems, m_quickAccess.get()));
    m_webPageUI->getQuickAccessEditUI()->closeUI.connect(
        boost::bind(&SimpleUI::popTheStack, this));
    m_webPageUI->focusWebView.connect(boost::bind(&basic_webengine::AbstractWebEngine::setFocus, m_webEngine.get()));
    m_webPageUI->unfocusWebView.connect(boost::bind(&basic_webengine::AbstractWebEngine::clearFocus, m_webEngine.get()));
    m_webPageUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this,
        m_favoriteService->getRoot(), BookmarkManagerState::Default));
    m_webPageUI->updateManualRotation.connect([this](){enableManualRotation(isManualRotation(m_viewManager.topOfStack()));});
    m_webPageUI->getWindow.connect(boost::bind(&SimpleUI::getMainWindow, this));
    m_webPageUI->isBookmark.connect(boost::bind(&SimpleUI::checkBookmark, this));
    m_webPageUI->isQuickAccess.connect(boost::bind(&SimpleUI::checkQuickAccess, this));
    m_webPageUI->deleteBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));
    m_webPageUI->showBookmarkFlowUI.connect(boost::bind(&SimpleUI::showBookmarkFlowUI, this));
    m_webPageUI->showFindOnPageUI.connect(boost::bind(&SimpleUI::showFindOnPageUI, this, std::string()));
    m_webPageUI->isFindOnPageVisible.connect(boost::bind(&FindOnPageUI::isVisible, m_findOnPageUI.get()));
    m_webPageUI->showSettingsUI.connect(boost::bind(&SettingsManager::showSettingsBaseUI, m_settingsManager.get()));
    m_webPageUI->addNewTab.connect(boost::bind(&SimpleUI::newTabClicked, this));
    m_webPageUI->getURIEntry().mobileEntryFocused.connect(boost::bind(&WebPageUI::mobileEntryFocused, m_webPageUI));
    m_webPageUI->getURIEntry().mobileEntryUnfocused.connect(boost::bind(&WebPageUI::mobileEntryUnfocused, m_webPageUI));
    m_webPageUI->qaOrientationChanged.connect(boost::bind(&QuickAccess::orientationChanged, m_quickAccess));
    m_webPageUI->getURIEntry().secureIconClicked.connect(boost::bind(&SimpleUI::showCertificatePopup, this));
    m_webPageUI->getURIEntry().isValidCert.connect(boost::bind(&services::CertificateContents::isValidCertificate, m_certificateContents, _1));
    m_webPageUI->getURIEntry().reloadPage.connect(
        boost::bind(&basic_webengine::AbstractWebEngine::reload, m_webEngine.get()));
    m_webPageUI->getURIEntry().stopLoadingPage.connect(
        boost::bind(&basic_webengine::AbstractWebEngine::stopLoading, m_webEngine.get()));
    m_webPageUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    m_webPageUI->switchToMobileMode.connect(boost::bind(&SimpleUI::switchToMobileMode, this));
    m_webPageUI->switchToDesktopMode.connect(boost::bind(&SimpleUI::switchToDesktopMode, this));
    m_webPageUI->quickAccessEdit.connect(boost::bind(&SimpleUI::editQuickAccess, this));
    m_webPageUI->deleteMostVisited.connect(boost::bind(&SimpleUI::deleteMostVisited, this));
    m_webPageUI->addToQuickAccess.connect(boost::bind(&SimpleUI::addQuickAccessItem, this, _1, _2));
    m_webPageUI->getTitle.connect(boost::bind(&basic_webengine::AbstractWebEngine::getTitle, m_webEngine.get()));
    m_webPageUI->getEngineState.connect(boost::bind(&basic_webengine::AbstractWebEngine::getState, m_webEngine.get()));
    m_webPageUI->requestCurrentPageForWebPageUI.connect(boost::bind(&SimpleUI::requestSettingsCurrentPage, this));
#if PWA
    m_webPageUI->pwaRequestManifest.connect(boost::bind(&basic_webengine::AbstractWebEngine::requestManifest, m_webEngine.get()));
    m_webPageUI->getCountCheckSignal.connect(boost::bind(&SimpleUI::countCheckUrl, this));
#endif
    m_webPageUI->isMostVisited.connect(boost::bind(&QuickAccess::isMostVisitedActive, m_quickAccess.get()));
}

void SimpleUI::connectQuickAccessSignals()
{
    M_ASSERT(m_quickAccess.get());
    m_quickAccess->openURLquickaccess.connect(boost::bind(&SimpleUI::openURLquickaccess, this, _1, _2));
    m_quickAccess->openURLhistory.connect(boost::bind(&SimpleUI::openURLhistory, this, _1, _2));
    m_quickAccess->getMostVisitedItems.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this));
    m_quickAccess->getQuickAccessItems.connect(boost::bind(&SimpleUI::onQuickAccessClicked, this));
    m_quickAccess->switchViewToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_quickAccess->addQuickAccessClicked.connect(boost::bind(&SimpleUI::onNewQuickAccessClicked, this));
    m_quickAccess->deleteQuickAccessItem.connect(boost::bind(&SimpleUI::onQuickAccessDeleted, this, _1));
    m_quickAccess->removeMostVisitedItem.connect(
        boost::bind(&SimpleUI::setMostVisitedFrequencyValue, this, _1, _2));
    m_quickAccess->sendSelectedMVItemsCount.connect(
        boost::bind(&WebPageUI::setMostVisitedSelectedItemsCountInEditMode, m_webPageUI.get(), _1));
    m_quickAccess->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
}

void SimpleUI::connectTabsSignals()
{
    M_ASSERT(m_tabUI.get());
    m_tabUI->closeTabUIClicked.connect(boost::bind(&SimpleUI::popTheStack, this));
    m_tabUI->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this));
    m_tabUI->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    m_tabUI->closeTabsClicked.connect(boost::bind(&SimpleUI::closeTabsClicked, this,_1));
    m_tabUI->getWindow.connect(boost::bind(&SimpleUI::getMainWindow, this));
    m_tabUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    m_tabUI->changeEngineState.connect(boost::bind(&SimpleUI::changeEngineState, this));
    m_tabUI->refetchTabUIData.connect(boost::bind(&SimpleUI::refetchTabUIData, this));
    m_tabUI->checkIfParamExistsInDB.connect(boost::bind(&storage::SettingsStorage::isDBParamPresent,
        &m_storageService->getSettingsStorage(), _1));
    m_tabUI->setDBBoolParamValue.connect(boost::bind(&storage::SettingsStorage::setSettingsBool,
        &m_storageService->getSettingsStorage(), _1, _2));
    m_tabUI->setDBStringParamValue.connect(boost::bind(&storage::SettingsStorage::setSettingsString,
        &m_storageService->getSettingsStorage(), _1, _2));
    m_tabUI->getDBBoolParamValue.connect(boost::bind(&storage::SettingsStorage::getSettingsBool,
        &m_storageService->getSettingsStorage(), _1, false));
    m_tabUI->getDBStringParamValue.connect(boost::bind(&storage::SettingsStorage::getSettingsText,
        &m_storageService->getSettingsStorage(), _1, ""));
    m_tabUI->showPasswordUI.connect(boost::bind(&SimpleUI::showPasswordUI, this));
    m_tabUI->showNoPasswordWarning.connect(boost::bind(&SimpleUI::onFirstSecretMode, this));
    m_tabUI->getPasswordUI()->closeUI.connect(boost::bind(&SimpleUI::popTheStack, this));
    m_tabUI->getPasswordUI()->setDBStringParamValue.connect(boost::bind(&storage::SettingsStorage::setSettingsString,
        &m_storageService->getSettingsStorage(), _1, _2));
    m_tabUI->getPasswordUI()->setDBBoolParamValue.connect(boost::bind(&storage::SettingsStorage::setSettingsBool,
        &m_storageService->getSettingsStorage(), _1, _2));
    m_tabUI->getPasswordUI()->getDBStringParamValue.connect(boost::bind(&storage::SettingsStorage::getSettingsText,
        &m_storageService->getSettingsStorage(), _1, ""));
    m_tabUI->getPasswordUI()->getDBBoolParamValue.connect(boost::bind(&storage::SettingsStorage::getSettingsBool,
        &m_storageService->getSettingsStorage(), _1, false));
    m_tabUI->getPasswordUI()->changeEngineState.connect(boost::bind(&SimpleUI::changeEngineState, this));
}

void SimpleUI::connectHistorySignals()
{
    M_ASSERT(m_historyUI.get());
    m_historyUI->clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryAllClicked, this));
    m_historyUI->signalDeleteHistoryItems.connect(boost::bind(&SimpleUI::onDeleteHistoryItems, this, _1));
    m_historyUI->closeHistoryUIClicked.connect(boost::bind(&SimpleUI::popTheStack, this));
    m_historyUI->signalHistoryItemClicked.connect(boost::bind(&SimpleUI::openURL, this, _1, _2, false));
    m_historyUI->getWindow.connect(boost::bind(&SimpleUI::getMainWindow, this));
}

void SimpleUI::connectBookmarkFlowSignals()
{
    M_ASSERT(m_bookmarkFlowUI.get());
    m_bookmarkFlowUI->closeBookmarkFlowClicked.connect(boost::bind(&SimpleUI::popTheStack, this));
    m_bookmarkFlowUI->saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    m_bookmarkFlowUI->editBookmark.connect(boost::bind(&SimpleUI::editBookmark, this, _1));
    m_bookmarkFlowUI->showSelectFolderUI.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this,
        _1, BookmarkManagerState::SelectFolder));
    m_bookmarkFlowUI->addToQuickAccess.connect(boost::bind(&SimpleUI::addQuickAccessItem, this, _1, _2));
}

void SimpleUI::connectBookmarkManagerSignals()
{
    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::popTheStack, this));
    m_bookmarkManagerUI->folderSelected.connect(boost::bind(&BookmarkFlowUI::setFolder, m_bookmarkFlowUI.get(), _1));
    m_bookmarkManagerUI->getWindow.connect(boost::bind(&SimpleUI::getMainWindow, this));
    m_bookmarkManagerUI->bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
    m_bookmarkManagerUI->bookmarkItemEdit.connect(boost::bind(&SimpleUI::onBookmarkEdit, this, _1));
    m_bookmarkManagerUI->bookmarkItemOrderEdited.connect(boost::bind(&SimpleUI::onBookmarkOrderEdited, this, _1));
    m_bookmarkManagerUI->bookmarkItemDeleted.connect(boost::bind(&SimpleUI::onBookmarkDeleted, this, _1));
    m_bookmarkManagerUI->newFolderItemClicked.connect(boost::bind(&SimpleUI::onNewFolderClicked, this, _1));
    m_bookmarkManagerUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    m_bookmarkManagerUI->getHistoryGenlistContent.connect(boost::bind(&SimpleUI::showHistoryUI, this, _1, _2, _3));
    m_bookmarkManagerUI->removeSelectedItemsFromHistory.connect(boost::bind(&HistoryUI::removeSelectedHistoryItems, m_historyUI.get()));
    m_bookmarkManagerUI->isEngineSecretMode.connect(boost::bind(&basic_webengine::AbstractWebEngine::isSecretMode, m_webEngine.get()));
}

void SimpleUI::connectFindOnPageSignals()
{
    M_ASSERT(m_findOnPageUI.get());
    m_findOnPageUI->closeFindOnPageUIClicked.connect(boost::bind(&SimpleUI::closeFindOnPageUI, this));
    m_findOnPageUI->startFindingWord.connect(boost::bind(&SimpleUI::findWord, this, _1));
}

void SimpleUI::connectSettingsSignals()
{
    M_ASSERT(m_settingsManager.get());

    // SETTINGS OVERALL
    m_settingsManager->connectOpenSignals();
    SPSC.closeSettingsUIClicked.connect(
        boost::bind(&SimpleUI::popTheStack, this));
    SPSC.showSettings.connect(
        boost::bind(&SimpleUI::showSettings, this,_1));
    SPSC.getWebEngineSettingsParam.connect(
        boost::bind(
            &basic_webengine::AbstractWebEngine::getSettingsParam,
            m_webEngine.get(),
            _1));
    SPSC.getWebEngineSettingsParamString.connect(
        boost::bind(
            &storage::SettingsStorage::getParamString,
            &m_storageService->getSettingsStorage(),
            _1));

    SPSC.setWebEngineSettingsParam.connect(
        boost::bind(
            &basic_webengine::AbstractWebEngine::setSettingsParam,
            m_webEngine.get(),
            _1,
            _2));
    SPSC.setWebEngineSettingsParam.connect(
        boost::bind(&storage::SettingsStorage::setParam,
            &m_storageService->getSettingsStorage(),
            _1,
            _2));
    SPSC.setWebEngineSettingsParamString.connect(
        boost::bind(
            &storage::SettingsStorage::setParamString,
            &m_storageService->getSettingsStorage(),
            _1,
            _2));

    SPSC.isLandscape.connect(
        boost::bind(&SimpleUI::isLandscape, this));
    SPSC.settingsBaseShowRadioPopup.connect(
        boost::bind(&SimpleUI::onDefSearchEngineClicked, this));
    SPSC.settingsSaveContentToRadioPopup.connect(
        boost::bind(&SimpleUI::onSaveContentToClicked, this));

    // SETTINGS HOME PAGE SIGNALS
    m_settingsManager->init(m_viewManager.getContent());
    SPSC.requestCurrentPage.connect(
        boost::bind(&SimpleUI::requestSettingsCurrentPage, this));
    SPSC.showTextPopup.connect(
        boost::bind(&SimpleUI::selectSettingsOtherPageChange, this));

    // SETTINGS DELETE DATA
    SPSC.deleteSelectedDataClicked.connect(
        boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));

    SPSC.userAgentItemClicked.connect(
        boost::bind(&SimpleUI::settingsOverrideUseragent, this,_1));
}

void SimpleUI::connectWebEngineSignals()
{
    m_webEngine->minimizeBrowser.connect(boost::bind(&SimpleUI::minimizeBrowser, this));
    m_webEngine->uriChanged.connect(boost::bind(&URIEntry::changeUri, &m_webPageUI->getURIEntry(), _1));
    m_webEngine->downloadStarted.connect(boost::bind(&SimpleUI::downloadStarted, this, _1));
    m_webEngine->backwardEnableChanged.connect(boost::bind(&WebPageUI::setBackButtonEnabled, m_webPageUI.get(), _1));
    m_webEngine->forwardEnableChanged.connect(boost::bind(&WebPageUI::setForwardButtonEnabled, m_webPageUI.get(), _1));
    m_webEngine->loadStarted.connect(boost::bind(&SimpleUI::loadStarted, this));
    m_webEngine->loadProgress.connect(boost::bind(&SimpleUI::progressChanged,this,_1));
    m_webEngine->loadFinished.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->loadStop.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->tabCreated.connect(boost::bind(&SimpleUI::tabCreated, this));
    m_webEngine->checkIfCreate.connect(boost::bind(&SimpleUI::checkIfCreate, this));
    m_webEngine->tabClosed.connect(boost::bind(&SimpleUI::engineTabClosed,this,_1));
    m_webEngine->IMEStateChanged.connect(boost::bind(&SimpleUI::setwvIMEStatus, this, _1));
    m_webEngine->switchToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webEngine->favIconChanged.connect(boost::bind(&SimpleUI::faviconChanged, this, _1));
    m_webEngine->windowCreated.connect(boost::bind(&SimpleUI::windowCreated, this));
    m_webEngine->createTabId.connect(boost::bind(&SimpleUI::onCreateTabId, this));
    m_webEngine->snapshotCaptured.connect(boost::bind(&SimpleUI::onSnapshotCaptured, this, _1, _2));
    m_webEngine->redirectedWebPage.connect(boost::bind(&SimpleUI::redirectedWebPage, this, _1, _2));
    m_webEngine->rotatePrepared.connect(boost::bind(&SimpleUI::rotatePrepared, this));
    m_webEngine->switchToQuickAccess.connect(boost::bind(&SimpleUI::switchViewToQuickAccess, this));
    m_webEngine->setCertificatePem.connect(boost::bind(&services::CertificateContents::saveCertificateInfo, m_certificateContents, _1, _2));
    m_webEngine->setWrongCertificatePem.connect(boost::bind(&services::CertificateContents::saveWrongCertificateInfo, m_certificateContents, _1, _2));
    m_webEngine->fullscreenModeSet.connect(boost::bind(&WebPageUI::fullscreenModeSet, m_webPageUI.get(), _1));
    m_webEngine->confirmationRequest.connect(boost::bind(&SimpleUI::handleConfirmationRequest, this, _1));
    m_webEngine->getRotation.connect(boost::bind(&SimpleUI::getRotation, this));
    m_webEngine->openFindOnPage.connect(boost::bind(&SimpleUI::showFindOnPageUI, this, _1));
    m_webEngine->closeFindOnPage.connect(boost::bind(&SimpleUI::closeFindOnPageUI, this));
    m_webEngine->unsecureConnection.connect(boost::bind(&SimpleUI::showUnsecureConnectionPopup, this));
    m_webEngine->registerHWKeyCallback.connect(boost::bind(&SimpleUI::registerHWKeyCallback, this));
    m_webEngine->unregisterHWKeyCallback.connect(boost::bind(&SimpleUI::unregisterHWKeyCallback, this));
}

void SimpleUI::connectHistoryServiceSignals()
{
    m_historyService->historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));
}

void SimpleUI::connectTabServiceSignals()
{
    m_tabService->generateThumb.connect(boost::bind(&SimpleUI::onGenerateThumb, this, _1));
    m_tabService->generateFavicon.connect(boost::bind(&SimpleUI::onGenerateFavicon, this, _1));
}

void SimpleUI::connectPlatformInputSignals()
{
    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));
    m_platformInputManager->menuButtonPressed.connect(boost::bind(&SimpleUI::onMenuButtonPressed, this));
    m_platformInputManager->XF86BackPressed.connect(boost::bind(&SimpleUI::onXF86BackPressed, this));
}

void SimpleUI::connectCertificateSignals()
{
    m_certificateContents->getHostCertList.connect(boost::bind(&storage::CertificateStorage::getHostCertList, &m_storageService->getCertificateStorage()));
    m_certificateContents->addOrUpdateCertificateEntry.connect(boost::bind(&storage::CertificateStorage::addOrUpdateCertificateEntry, &m_storageService->getCertificateStorage(), _1, _2, _3));
}

void SimpleUI::connectStorageSignals()
{
    m_storageService->getSettingsStorage().setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine::setSettingsParam, m_webEngine.get(), _1, _2));
}

void SimpleUI::connectUISignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    connectWebPageSignals();
    connectQuickAccessSignals();
    connectTabsSignals();
    connectHistorySignals();
    connectSettingsSignals();
    connectBookmarkFlowSignals();
    connectFindOnPageSignals();
    connectBookmarkManagerSignals();
}

void SimpleUI::connectModelSignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    connectWebEngineSignals();
    connectHistorySignals();
    connectTabServiceSignals();
    connectPlatformInputSignals();
    connectCertificateSignals();
    connectStorageSignals();
}

void SimpleUI::initUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto viewManager(m_viewManager.getContent());

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->init(viewManager);

    auto webPageUI(m_webPageUI->getContent());
    M_ASSERT(m_quickAccess.get());
    m_quickAccess->init(webPageUI);

    M_ASSERT(m_tabUI.get());
    m_tabUI->init(viewManager);

    M_ASSERT(m_historyUI.get());
    m_historyUI->init(viewManager);

    M_ASSERT(m_bookmarkFlowUI.get());
    m_bookmarkFlowUI->init(viewManager);

    M_ASSERT(m_findOnPageUI.get());
    m_findOnPageUI->init(webPageUI);

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->init(viewManager);
}

void SimpleUI::initModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webEngine.get());
    M_ASSERT(m_webPageUI.get());
    m_webEngine->init(m_webPageUI->getContent());

    M_ASSERT(m_storageService->getSettingsStorage());
    m_storageService->getSettingsStorage().initWebEngineSettingsFromDB();

    M_ASSERT(m_platformInputManager);
    m_platformInputManager->init(m_window.get());

    M_ASSERT(m_certificateContents.get());
    m_certificateContents->init();
}

void SimpleUI::pushViewToStack(const sAUI& view)
{
    m_viewManager.pushViewToStack(view);
    enableManualRotation(isManualRotation(view));
    if (appcore_flush_memory() == -1)
        BROWSER_LOGW("[%s] appcore_flush_memory error!", __PRETTY_FUNCTION__);
}

void SimpleUI::popTheStack()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.popTheStack();
    enableManualRotation(isManualRotation(m_viewManager.topOfStack()));
    if (appcore_flush_memory() == -1)
        BROWSER_LOGW("[%s] appcore_flush_memory error!", __PRETTY_FUNCTION__);
}

void SimpleUI::popStackTo(const sAUI& view)
{
    m_viewManager.popStackTo(view);
    enableManualRotation(isManualRotation(view));
    if (appcore_flush_memory() == -1)
        BROWSER_LOGW("[%s] appcore_flush_memory error!", __PRETTY_FUNCTION__);
}

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

void SimpleUI::switchViewToWebPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_webEngine->isSuspended())
        m_webEngine->resume();
    m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), m_webEngine->getURI(), m_webEngine->isLoading());
}

void SimpleUI::switchToTab(const basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->switchToTab(tabId);
    m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), m_webEngine->getURI(), m_webEngine->isLoading());
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
    m_webEngine->suspend();
    popStackTo(m_webPageUI);
}

void SimpleUI::openNewTab(const std::string &uri, const std::string& title,
        const boost::optional<int> adaptorId, bool desktopMode,
        basic_webengine::TabOrigin origin)
{
    BROWSER_LOGD("[%s:%d] uri =%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    basic_webengine::TabId tab = m_webEngine->addTab(uri,
            adaptorId, title, desktopMode, origin);
    if (tab == basic_webengine::TabId::NONE) {
        BROWSER_LOGW("[%s:%d] New tab is not created!", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    switchToTab(tab);
}

void SimpleUI::closeTab()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto tabId = m_webEngine->currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const basic_webengine::TabId& id)
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

bool SimpleUI::checkQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_storageService->getQuickAccessStorage().quickAccessItemExist(m_webEngine->getURI());
}

void SimpleUI::openURLhistory(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode)
{
    openURL(historyItem->getUrl(), historyItem->getTitle(), desktopMode);
}

void SimpleUI::openURLquickaccess(services::SharedQuickAccessItem quickaccessItem, bool desktopMode)
{
    openURL(quickaccessItem->getUrl(), quickaccessItem->getTitle(), desktopMode);
}

void SimpleUI::openURL(const std::string& url)
{
    // TODO: desktop mode should be checked in WebView or QuickAcces
    // (depends on which view is active)
    openURL(url, "", m_quickAccess->isDesktopMode());
}

void SimpleUI::openURL(const std::string& url, const std::string& title, bool desktopMode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_webPageUI) {
        popStackTo(m_webPageUI);
        if (tabsCount() == 0 || m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
            openNewTab(url, title, boost::none, desktopMode, basic_webengine::TabOrigin::QUICKACCESS);
        else {
            m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), url, false);
            m_webEngine->setURI(url);
            m_webPageUI->getURIEntry().clearFocus();
        }
    } else {
        BROWSER_LOGW("[%s:%d] No m_webPageUI object!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::onClearHistoryAllClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyService->clearAllHistory();
}

void SimpleUI::onDeleteHistoryItems(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyService->deleteHistoryItem(id);
}

void SimpleUI::onMostVisitedClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_quickAccess->setMostVisitedItems(getMostVisitedItems());
}

void SimpleUI::onQuickAccessClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->setQuickAccessItems(m_storageService->getQuickAccessStorage().getQuickAccessList());
}

void SimpleUI::onBookmarkClicked(services::SharedBookmarkItem bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (bookmarkItem->is_folder())
        m_bookmarkManagerUI->addBookmarkItems(bookmarkItem, m_favoriteService->getAllBookmarkItems(bookmarkItem->getId()));
    else {
        openURL(bookmarkItem->getAddress());
    }
}

void SimpleUI::onBookmarkEdit(services::SharedBookmarkItem bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (bookmarkItem->is_folder()) {
        InputPopup *inputPopup =
            InputPopup::createPopup(
                m_viewManager.getContent(),
                "Edit Folder name",
                "Edit folder name?",
                bookmarkItem->getTitle(),
                _("IDS_BR_SK_DONE"),
                _("IDS_BR_SK_CANCEL_ABB"));
        services::SharedBookmarkItemList badWords =
            m_favoriteService->getFolders(bookmarkItem->getParent());
        for (auto it = badWords.begin(); it != badWords.end(); ++it)
            inputPopup->addBadWord((*it)->getTitle());
        inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onEditFolderPopupClicked, this, _1, bookmarkItem));
        inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
        inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
        inputPopup->show();
    } else {
        m_bookmarkFlowUI->setURL(bookmarkItem->getAddress());
        m_bookmarkFlowUI->setState(true);
        m_bookmarkFlowUI->setTitle(bookmarkItem->getTitle());
        m_bookmarkFlowUI->setFolder(m_favoriteService->getBookmarkItem(bookmarkItem->getParent()));
        pushViewToStack(m_bookmarkFlowUI);
    }
}

void SimpleUI::onBookmarkOrderEdited(services::SharedBookmarkItem bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_favoriteService->editBookmark(bookmarkItem->getId(), "", "", -1, bookmarkItem->getOrder());
}

void SimpleUI::onBookmarkDeleted(services::SharedBookmarkItem bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_favoriteService->deleteBookmark(bookmarkItem->getId());
}

void SimpleUI::onNewFolderClicked(int parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup =
        InputPopup::createPopup(
            m_viewManager.getContent(),
            _("IDS_BR_SK3_CREATE_FOLDER"),
            "",
            "New Folder #",
                //TODO: Missing translation
            "Create",
            _("IDS_BR_SK_CANCEL_ABB"));
    services::SharedBookmarkItemList badWords = m_favoriteService->getFolders(parent);
    for (auto it = badWords.begin(); it != badWords.end(); ++it)
        inputPopup->addBadWord((*it)->getTitle());
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onNewFolderPopupClick, this, _1, parent));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    inputPopup->show();
}

void SimpleUI::onNewFolderPopupClick(const std::string& folder_name, int parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_favoriteService->folderExists(folder_name, parent)) {
        BROWSER_LOGD("[%s:%d] Folder already exists.", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    services::SharedBookmarkItem folder = m_favoriteService->addFolder(folder_name, parent);
    if (m_viewManager.topOfStack() == m_bookmarkManagerUI)
        m_bookmarkManagerUI->addBookmarkItemCurrentFolder(folder);
}

void SimpleUI::onNewQuickAccessClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto inputPopup = InputPopup::createPopup(
        m_viewManager.getContent(),
        "Add to Quick access",
        "",
        "",
        _("IDS_BR_OPT_ADD"),
        _("IDS_BR_SK_CANCEL_ABB"));
    // TODO Add missing translations
    inputPopup->setTip(_("Enter web address"));
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::addQuickAccessItem, this, _1, ""));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    inputPopup->show();
}

void SimpleUI::addQuickAccessItem(const string &urlArg, const string &titleArg)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bool showQA = false;
    std::string url = urlArg;
    std::string title = titleArg;

    if (!url.compare(0, tools::PROTOCOL_HTTP.length(), tools::PROTOCOL_HTTP))
        url = std::string(tools::PROTOCOL_HTTP) + url;

    if (titleArg.empty()) {
        title = urlArg;
        showQA = true;
    }

    if (!title.compare(0, tools::PROTOCOL_HTTP.length(), tools::PROTOCOL_HTTP))
        title = title.substr(tools::PROTOCOL_HTTP.length(), std::string::npos);
    else if (!title.compare(0, tools::PROTOCOL_HTTPS.length(), tools::PROTOCOL_HTTPS))
        title = title.substr(tools::PROTOCOL_HTTPS.length(), std::string::npos);
    else if (!title.compare(0, tools::PROTOCOL_FTP.length(), tools::PROTOCOL_FTP))
        title = title.substr(tools::PROTOCOL_FTP.length(), std::string::npos);

    //TODO: add support for reorder and color
    if (m_webPageUI->stateEquals(WPUState::MAIN_WEB_PAGE)) {
        tools::BrowserImagePtr favicon = m_webEngine->getFavicon();
        if (favicon) {
            m_storageService->getQuickAccessStorage().addQuickAccessItem(
                url, title, 0, 0, true, favicon, favicon->getWidth(), favicon->getHeight());
        } else {
            m_storageService->getQuickAccessStorage().addQuickAccessItem(
                url, title, 0, 0, false, nullptr, 0, 0);
        }
    } else {
        m_storageService->getQuickAccessStorage().addQuickAccessItem(
            url, title, 0, 0, false, nullptr, 0, 0);
    }

    if (showQA)
        showQuickAccess();

    //TODO: display toast message
}

void SimpleUI::onQuickAccessDeleted(services::SharedQuickAccessItem quickaccessItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_storageService->getQuickAccessStorage().deleteQuickAccessItem(quickaccessItem->getId());
}

void SimpleUI::editQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->editQuickAccess();
    pushViewToStack(m_webPageUI->getQuickAccessEditUI());
}

void SimpleUI::deleteMostVisited()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->deleteMostVisited();
    pushViewToStack(m_webPageUI->getQuickAccessEditUI());
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

void SimpleUI::onRemoveFoldersClicked(services::SharedBookmarkItemList items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = items.begin(); it != items.end(); ++it) {
        m_storageService->getFoldersStorage().removeNumberInFolder((*it)->getParent());
        m_favoriteService->deleteBookmark((*it)->getAddress());
    }
    items.clear();
}

void SimpleUI::onEditFolderPopupClicked(const std::string& newName, services::SharedBookmarkItem item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_favoriteService->editBookmark(item->getId(), "", newName, item->getParent());
    services::SharedBookmarkItem parentItem = m_favoriteService->getBookmarkItem(item->getParent());
    if (m_viewManager.topOfStack() == m_bookmarkManagerUI)
        m_bookmarkManagerUI->addBookmarkItems(nullptr, m_favoriteService->getAllBookmarkItems(parentItem->getId()));
}

void SimpleUI::onDeleteFolderPopupClicked(PopupButtons button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (button == DELETE && m_storageService->getFoldersStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = m_storageService->getFoldersStorage().getFolderId(m_folder_name);
        onRemoveFoldersClicked(m_favoriteService->getBookmarks(id));
        m_storageService->getFoldersStorage().deleteFolder(id);
    }
}

void SimpleUI::onUrlIMEOpened(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimpleUI* self = reinterpret_cast<SimpleUI*>(data);
    self->m_webPageUI->showBottomBar(false);
    self->setwvIMEStatus(true);
}

void SimpleUI::onUrlIMEClosed(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimpleUI* self = reinterpret_cast<SimpleUI*>(data);
    self->m_webPageUI->showBottomBar(true);
    self->setwvIMEStatus(false);
}

void SimpleUI::onSnapshotCaptured(std::shared_ptr<tools::BrowserImage> snapshot, tools::SnapshotType snapshot_type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (snapshot_type) {
    case tools::SnapshotType::ASYNC_LOAD_FINISHED:
        if (m_webEngine->isSecretMode()) {
            m_tabService->saveThumbCache(m_webEngine->currentTabId(), snapshot);
        } else {
            m_historyService->updateHistoryItemSnapshot(m_webEngine->getURI(), snapshot);
            m_tabService->updateTabItemSnapshot(m_webEngine->currentTabId(), snapshot);
        }
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

void SimpleUI::onGenerateFavicon(basic_webengine::TabId tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_tabService->updateTabItemFavicon(tabId, m_webEngine->getFavicon());
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
    if (m_findOnPageUI->isVisible())
        closeFindOnPageUI();
    else
    if (m_wvIMEStatus) {    // if IME opened
        return;
    } else if (m_popupVector.size() > 0) {
        m_popupVector.back()->onBackPressed();
    } else if (m_viewManager.topOfStack() == m_bookmarkManagerUI) {
        m_bookmarkManagerUI->onBackPressed();
    } else if (m_viewManager.topOfStack() == m_webPageUI->getQuickAccessEditUI()) {
        m_webPageUI->getQuickAccessEditUI()->backPressed();
    } else if (m_viewManager.topOfStack() == nullptr) {
        switchViewToQuickAccess();
    } else if ((m_viewManager.topOfStack() == m_webPageUI)) {
        if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
            if (m_quickAccess->canBeBacked(m_webEngine->tabsCount())) {
                m_quickAccess->backButtonClicked();
            } else {
                minimizeBrowser();
            }
        } else {
            m_webEngine->backButtonClicked();
        }
    } else {
        popTheStack();
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

void SimpleUI::onMenuButtonPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto view = dynamic_cast<interfaces::AbstractContextMenu*>(m_viewManager.topOfStack().get());
    if (view)
        view->showContextMenu();
}

bool SimpleUI::isManualRotation(const sAUI& view)
{
    WebPageUI *webPageUI = dynamic_cast<WebPageUI*>(view.get());
    return (webPageUI && webPageUI->stateEquals(WPUState::MAIN_WEB_PAGE));
}

void SimpleUI::enableManualRotation(bool enable)
{
    m_manualRotation = enable;
    BROWSER_LOGD("[%s:%d]: %d", __PRETTY_FUNCTION__, __LINE__, m_manualRotation);
    elm_win_wm_rotation_manual_rotation_done_set(main_window,
        m_manualRotation ? EINA_TRUE : EINA_FALSE);
}

void SimpleUI::rotatePrepared()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_manualRotation && elm_win_wm_rotation_manual_rotation_done_get(main_window)) {
        elm_win_wm_rotation_manual_rotation_done(main_window);
        m_webPageUI->orientationChanged();
    }
}

void SimpleUI::onRotation()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_current_angle = m_temp_angle;
    m_webEngine->orientationChanged();
    if (!m_manualRotation) {
        if (m_bookmarkFlowUI)
            m_bookmarkFlowUI->orientationChanged();
        if (m_settingsUI)
            m_settingsUI->orientationChanged();
        if (m_bookmarkManagerUI)
            m_bookmarkManagerUI->orientationChanged();
        if (m_webPageUI)
            m_webPageUI->orientationChanged();
        if (m_tabUI)
            m_tabUI->orientationChanged();

        if (!m_popupVector.empty())
            m_popupVector.back()->orientationChanged();
    }
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

void SimpleUI::rotationType(rotationLock lock)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    switch (lock) {
        case rotationLock::portrait: {
            const int rots[] = {0};
            elm_win_wm_rotation_available_rotations_set(main_window, rots, sizeof(rots)/sizeof(int));
            break;
        }
        case rotationLock::landscape: {
            const int rots[] = {90};
            elm_win_wm_rotation_available_rotations_set(main_window, rots, sizeof(rots)/sizeof(int));
            break;
        }
        case rotationLock::noLock: {
            const int rots[] = {0, 90, 180, 270};
            elm_win_wm_rotation_available_rotations_set(main_window, rots, sizeof(rots)/sizeof(int));
            break;
        }
        default:
            BROWSER_LOGW("[%s:%d] Unknown rotationLock case!", __PRETTY_FUNCTION__, __LINE__);
    }
}

Evas_Object* SimpleUI::getMainWindow()
{
    return main_window;
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
        	connection_type_e connection_type;
        	connection_h handle;

        	if (connection_create(&handle) < 0) {
        		BROWSER_LOGD("Fail to create network handle");
        	}
        	if (connection_get_type(handle, &connection_type) < 0) {
        		BROWSER_LOGD("Fail to get download type");
        		if (connection_destroy(handle) < 0) {
        			BROWSER_LOGD("Fail to get download type");
        		}
        	}

        	if (connection_type == CONNECTION_TYPE_BT){
                popup->show(_("IDS_BR_POP_STARTING_DOWNLOAD_ING"), _("Unable to use tethering."), false);
        	}
        	else{
        		popup->show(_("IDS_BR_POP_STARTING_DOWNLOAD_ING"), false);
        	}

        	if (connection_destroy(handle) < 0) {
        		BROWSER_LOGD("Fail to destroy network handle");
        	}
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

void SimpleUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->loadStarted();
    if (m_findOnPageUI->isVisible())
        closeFindOnPageUI();
}

void SimpleUI::progressChanged(double progress)
{
    m_webPageUI->progressChanged(progress);
}

void SimpleUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->loadFinished();
    if (!m_webEngine->isSecretMode()) {
        m_tabService->updateTabItem(
            m_webEngine->currentTabId(),
            m_webEngine->getURI(),
            m_webEngine->getTitle(),
            m_webEngine->getOrigin());
        m_historyService->addHistoryItem(
            m_webEngine->getURI(),
            m_webEngine->getTitle(),
            m_webEngine->getFavicon());
    }
}

void SimpleUI::filterURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] url=%s", __PRETTY_FUNCTION__, __LINE__, url.c_str());

    if (url == HomePageURL) {
        m_webPageUI->getURIEntry().changeUri("");
        switchViewToQuickAccess();
    } else if (!url.empty()) {
        openURL(url);
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

void SimpleUI::scrollView(const int& dx, const int& dy)
{
    m_webEngine->scrollView(dx, dy);
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

void SimpleUI::showTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    pushViewToStack(m_tabUI);

    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) &&
        m_webEngine->tabsCount() > 0 &&
        m_webEngine->isLoading())
        onGenerateThumb(m_webEngine->currentTabId());
}

void SimpleUI::refetchTabUIData() {
    auto tabsContents = m_webEngine->getTabContents();
    m_tabService->fillThumbs(tabsContents);
    m_tabService->fillFavicons(tabsContents);
    m_tabUI->addTabItems(tabsContents, m_webEngine->isSecretMode());
}

void SimpleUI::newTabClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!checkIfCreate())
        return;
    showHomePage();
}

void SimpleUI::tabClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switchToTab(tabId);
    popStackTo(m_webPageUI);
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

#if PWA
void SimpleUI::pwaPopupRequest()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::pwaPopupButtonClicked, this, _1));
    popup->setTitle(m_webEngine->getTitle());
    popup->setMessage(_("IDS_BR_OPT_ADD_TO_HOME_SCREEN_ABB"));
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}

void SimpleUI::pwaPopupButtonClicked(const PopupButtons& button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (button) {
        case OK:
            BROWSER_LOGD("[%s:%d] pwaPopup create !", __PRETTY_FUNCTION__, __LINE__);
            m_webEngine->requestManifest();
            break;
        case CANCEL:
            BROWSER_LOGD("[%s:%d] pwaPopup deny !", __PRETTY_FUNCTION__, __LINE__);
            break;
        default:
            BROWSER_LOGW("[%s:%d] Unknown button type!", __PRETTY_FUNCTION__, __LINE__);
    }
}
#endif

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
            showCertificatePopup(certPopupData->cert->getURI(), certPopupData->cert->getPem(), services::CertificateContents::UNSECURE_HOST_UNKNOWN);
            break;
        }
        default:
            BROWSER_LOGW("[%s:%d] Unknown button type!", __PRETTY_FUNCTION__, __LINE__);
    }
}

Evas_Object* SimpleUI::showHistoryUI(Evas_Object* parent, SharedNaviframeWrapper naviframe, bool removeMode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyUI->setNaviframe(naviframe);
    auto ret = m_historyUI->createDaysList(parent, removeMode);
    m_historyUI->addHistoryItems(
        m_historyService->getHistoryToday(), HistoryPeriod::HISTORY_TODAY);
    m_historyUI->addHistoryItems(
        m_historyService->getHistoryYesterday(), HistoryPeriod::HISTORY_YESTERDAY);
    m_historyUI->addHistoryItems(
        m_historyService->getHistoryLastWeek(), HistoryPeriod::HISTORY_LASTWEEK);
    m_historyUI->addHistoryItems(
        m_historyService->getHistoryLastMonth(), HistoryPeriod::HISTORY_LASTMONTH);
    m_historyUI->addHistoryItems(
        m_historyService->getHistoryOlder(), HistoryPeriod::HISTORY_OLDER);
    return ret;
}

void SimpleUI::showSettings(unsigned s)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_settingsManager->init(m_viewManager.getContent());
    m_settingsUI = m_settingsManager->getView(static_cast<SettingsMainOptions>(s));
    pushViewToStack(m_settingsUI);
}

void SimpleUI::onDefSearchEngineClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto popup = RadioPopup::createPopup(m_viewManager.getContent());
    popup->setTitle(_(Translations::SettingsDefaultSearchEngineTitle.c_str()));
    popup->addRadio(RadioButtons::GOOGLE);
    popup->addRadio(RadioButtons::YAHOO);
    popup->addRadio(RadioButtons::BING);
    auto stateString = []() -> std::string {
        auto sig =
            SPSC.getWebEngineSettingsParamString(
                basic_webengine::WebEngineSettings::DEFAULT_SEARCH_ENGINE);
        return (sig && !sig->empty()) ?
            *sig :
            Translations::Google;
    }();
    auto state = RadioPopup::translateButtonState(stateString);
    popup->setState(state);
    popup->radioButtonClicked.connect(
        [&,this](const RadioButtons& button){
        SPSC.setSearchEngineSubText(
            static_cast<int>(button));
        dismissPopup(popup);
    });
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(
        boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}

void SimpleUI::onSaveContentToClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto popup = RadioPopup::createPopup(m_viewManager.getContent());
    SPSC.settingsSaveContentRadioPopupPtr(popup);
    popup->setTitle(_(Translations::SettingsAdvancedSaveContentTitle.c_str()));
    popup->addRadio(RadioButtons::DEVICE);
    popup->addRadio(RadioButtons::SD_CARD);
    auto stateString = []() -> std::string {
        auto sig =
            SPSC.getWebEngineSettingsParamString(
                basic_webengine::WebEngineSettings::SAVE_CONTENT_LOCATION);
        return (sig && !sig->empty()) ?
            *sig :
            Translations::Device;
    }();
    auto state = RadioPopup::translateButtonState(stateString);
    popup->setState(state);
    popup->radioButtonClicked.connect(
        [&,this](const RadioButtons& button){
        SPSC.setContentDestination(static_cast<int>(button));
        dismissPopup(popup);
        SPSC.settingsSaveContentRadioPopupPtr(nullptr);
    });
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(
        boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}

std::string SimpleUI::requestSettingsCurrentPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_webEngine->getURI();
}

void SimpleUI::selectSettingsOtherPageChange()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto popup = InputPopup::createPopup(m_viewManager.getContent());
    popup->setTitle(_("IDS_BR_MBODY_SET_HOMEPAGE"));
    popup->setCancelButtonText(_("IDS_BR_BUTTON_CANCEL"));
    popup->setOkButtonText(_("IDS_BR_BUTTON_SET"));
    popup->setTip(_("IDS_BR_BODY_WEB_ADDRESS"));
    popup->button_clicked.connect(
        [this](const std::string& otherPage){
        if (!otherPage.empty()) {
            SPSC.setWebEngineSettingsParamString(
                basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE,
                otherPage);
            m_settingsUI->updateButtonMap();
        }
    });
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(
        [this](interfaces::AbstractPopup* popup){
            dismissPopup(popup);
            m_settingsUI->updateButtonMap();
        }
    );
    popup->show();
}

void SimpleUI::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_webEngine->switchToMobileMode();
        popStackTo(m_webPageUI);
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

void SimpleUI::showBookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string uri = m_webEngine->getURI();
    m_bookmarkFlowUI->setURL(uri);

    bool state = m_favoriteService->bookmarkExists(uri);
    m_bookmarkFlowUI->setState(state);

    if (state) {
        tizen_browser::services::BookmarkItem bookmarkItem;
        m_favoriteService->getItem(uri, &bookmarkItem);
        m_bookmarkFlowUI->setTitle(bookmarkItem.getTitle());
        m_bookmarkFlowUI->setFolder(m_favoriteService->getBookmarkItem(bookmarkItem.getParent()));
    } else {
        m_bookmarkFlowUI->setTitle(m_webEngine->getTitle());
        m_bookmarkFlowUI->setFolder(m_favoriteService->getRoot());
    }

    pushViewToStack(m_bookmarkFlowUI);
}

void SimpleUI::showBookmarkManagerUI(std::shared_ptr<services::BookmarkItem> parent,
    BookmarkManagerState state)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_bookmarkManagerUI->setState(state);
    pushViewToStack(m_bookmarkManagerUI);
    m_bookmarkManagerUI->addBookmarkItems(parent,
        m_favoriteService->getAllBookmarkItems(parent->getId()));
}

void SimpleUI::showHomePage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto stateString = []() -> std::string {
        auto sig =
            SettingsPrettySignalConnector::Instance().
                getWebEngineSettingsParamString(
                    basic_webengine::WebEngineSettings::CURRENT_HOME_PAGE);
        return (sig && !sig->empty()) ?
            *sig :
            SettingsHomePage::DEF_HOME_PAGE;
    }();
    auto it = stateString.find(Translations::CurrentPage);
    if (!stateString.compare(SettingsHomePage::QUICK_PAGE)) {
        switchViewToQuickAccess();
        m_quickAccess->showQuickAccess();
        return;
    } else if (!stateString.compare(SettingsHomePage::MOST_VISITED_PAGE)) {
        switchViewToQuickAccess();
        m_quickAccess->showMostVisited();
        return;
    } else if (it != std::string::npos) {
        stateString.erase(it, Translations::CurrentPage.length());
    }
    auto url = m_webPageUI->getURIEntry().rewriteURI(stateString);
    popStackTo(m_webPageUI);
    openNewTab(url);
}

void SimpleUI::redirectedWebPage(const std::string& oldUrl, const std::string& newUrl)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("Redirect from %s to %s", oldUrl.c_str(), newUrl.c_str());
    m_historyService->clearURLHistory(oldUrl);
}

void SimpleUI::settingsDeleteSelectedData(const std::map<int, bool>& options)
{
    BROWSER_LOGD("[%s]: Deleting selected data", __func__);
    M_ASSERT(m_viewManager);
    bool isSelected = false;
    for (auto& it : options) {
        if (it.second) {
            isSelected = true;
            break;
        }
    }
    if (isSelected) {
        TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
        popup->addButton(OK);
        popup->addButton(CANCEL);
        popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteSelectedDataButton, this, _1, options));
        popup->setTitle("Delete");
        popup->setMessage("The selected web browsing data will be deleted.");
        popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
        popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
        popup->show();
    }
}

void SimpleUI::onDeleteSelectedDataButton(const PopupButtons& button, const std::map<int, bool>& options)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (button == OK) {
        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Delete Web Browsing Data");

        for (auto& it : options) {
            if (it.first == BROWSING_HISTORY && it.second) {
                BROWSER_LOGD("clear history" );
                m_historyService->clearAllHistory();
            } else if (it.first == CACHE && it.second) {
                BROWSER_LOGD("clear cache");
                m_webEngine->clearCache();
            } else if (it.first == COOKIES_AND_SITE && it.second) {
                BROWSER_LOGD("clear cookies");
                m_webEngine->clearCookies();
            } else if (it.first == PASSWORDS && it.second) {
                BROWSER_LOGD("clear passwords");
                m_webEngine->clearPasswordData();
            } else if (it.first == DEL_PERS_AUTO_FILL && it.second) {
                BROWSER_LOGD("clear autofill forms");
                m_webEngine->clearFormData();
                SPSC.autoFillCleared();
            } else if (it.first == LOCATION && it.second) {
                BROWSER_LOGD("clear location");
            }
        }
        //TODO: No certificates removal in guidelines. Probably it should be done
        // in the future
        popup->dismiss();
    }
}

void SimpleUI::tabLimitPopupButtonClicked(PopupButtons button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (button == CLOSE_TAB) {
        BROWSER_LOGD("[%s]: CLOSE TAB", __func__);
        closeTab();
    }
}

void SimpleUI::tabCreated()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int tabs = m_webEngine->tabsCount();
    m_webPageUI->setTabsNumber(tabs);
}

bool SimpleUI::checkIfCreate()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int tabs = m_webEngine->tabsCount();

    if (tabs >= m_tabLimit) {
        TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
        popup->addButton(OK);
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1));
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

void SimpleUI::updateView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int tabs = m_webEngine->tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (m_viewManager.topOfStack() == m_webPageUI) {
        if (tabs == 0) {
            switchViewToQuickAccess();
        } else if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
            switchViewToWebPage();
        }
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::changeEngineState()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine->changeState();
#if PWA
    if (!m_alreadyOpenedPWA)
#endif
        m_webPageUI->switchViewToQuickAccess(m_quickAccess->getContent());
    updateView();

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

void SimpleUI::engineTabClosed(const basic_webengine::TabId& id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabService->removeTab(id);

    int tabs = m_webEngine->tabsCount();
    m_webPageUI->setTabsNumber(tabs);
    if (tabs == 0) {
        m_webPageUI->switchViewToQuickAccess(m_quickAccess->getContent());
    } else
        switchViewToWebPage();
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    m_webEngine->searchOnWebsite(text, flags);
}

void SimpleUI::showPasswordUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    pushViewToStack(m_tabUI->getPasswordUI());
}

void SimpleUI::onFirstSecretMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->addButton(OK);
    popup->setTitle(_("IDS_BR_HEADER_DONT_USE_PASSWORD_ABB"));
    popup->setMessage("To protect your Secret mode data, create a password. "
        "If you Use Secret mode without creating a password, you will not be able to "
        "prevent others from viewing your browser and search history, bookmarks, And saved pages.");
    popup->buttonClicked.connect(boost::bind(&TabUI::switchToSecretFirstTime, m_tabUI.get()));
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
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
            m_favoriteService->addBookmark(m_webEngine->getURI(), bookmark_update.title, std::string(),
                m_webEngine->getSnapshotData(THUMB_WIDTH, THUMB_HEIGHT, tools::SnapshotType::ASYNC_BOOKMARK),
                m_webEngine->getFavicon(), bookmark_update.folder_id);
        }
    }
}

void SimpleUI::editBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_favoriteService) {
        if (m_webEngine && !bookmark_update.old_url.empty()) {
            services::BookmarkItem oldItem;
            m_favoriteService->getItem(bookmark_update.old_url, &oldItem);
            m_favoriteService->editBookmark(oldItem.getId(), bookmark_update.url,
                bookmark_update.title, bookmark_update.folder_id);
            m_bookmarkManagerUI->addBookmarkItems(nullptr, m_favoriteService->getAllBookmarkItems(
                bookmark_update.folder_id));
        }
    }
}

void SimpleUI::deleteBookmark()
{
    std::string uri(m_webEngine->getURI());
    bool ret(true);
    if (m_favoriteService->bookmarkExists(uri))
        ret = m_favoriteService->deleteBookmark(uri);
    // TODO add translations
    auto text(
        ret ?
        "Bookmark deletion has failed!" :
        "Bookmark has been deleted successfully.");
    auto toast(tools::EflTools::createToastPopup(
        m_viewManager.getContent(),
        3.0,
        text));
    evas_object_show(toast);
}

void SimpleUI::settingsOverrideUseragent(const std::string& userAgent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        NotificationPopup *popup =
            NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Open a webpage to perform this operation.");
        popup->dismiss();
        popTheStack();
        return;
    }

    if (userAgent.empty()) {
        auto inputPopup = InputPopup::createPopup(
            m_viewManager.getContent(),
            "Override UserAgent", "",
            "",
            _("IDS_BR_SK_DONE"),
            _("IDS_BR_SK_CANCEL_ABB"));
        inputPopup->button_clicked.connect(
            boost::bind(&SimpleUI::onOverrideUseragentButton, this, _1));
        inputPopup->popupShown.connect(
            boost::bind(&SimpleUI::showPopup, this, _1));
        inputPopup->popupDismissed.connect(
            boost::bind(&SimpleUI::dismissPopup, this, _1));
        inputPopup->show();
    }
    else {
        onOverrideUseragentButton(userAgent);
    }
}

void SimpleUI::onOverrideUseragentButton(const std::string& newUA)
{
    BROWSER_LOGD("[%s]: Overriding useragent", __func__);
    NotificationPopup *popup =
        NotificationPopup::createNotificationPopup(m_viewManager.getContent());
    m_webEngine->setUserAgent(newUA);
    popup->show("UserAgent updated..");
    popup->dismiss();
    popTheStack();
}

}
}
