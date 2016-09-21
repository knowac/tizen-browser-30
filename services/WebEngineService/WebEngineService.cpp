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

#include "browser_config.h"
#include "WebEngineService.h"

#include <Evas.h>
#include <memory>
#include <BrowserImage.h>
#include <app.h>

#include "AbstractWebEngine/TabId.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "Config/Config.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webengine_service {

EXPORT_SERVICE(WebEngineService, "org.tizen.browser.webengineservice")

WebEngineService::WebEngineService()
    : m_state(State::NORMAL)
    , m_initialised(false)
    , m_guiParent(nullptr)
    , m_stopped(false)
    , m_webViewCacheInitialized(false)
    , m_currentTabId(TabId::NONE)
    , m_currentWebView(nullptr)
    , m_stateStruct(&m_normalStateStruct)
    , m_tabIdCreated(-1)
    , m_downloadControl(nullptr)
{
    m_stateStruct->mostRecentTab.clear();
    m_stateStruct->tabs.clear();

#if PROFILE_MOBILE
    // init settings
    m_settings[WebEngineSettings::PAGE_OVERVIEW] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_PAGE_OVERVIEW));
    m_settings[WebEngineSettings::LOAD_IMAGES] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_LOAD_IMAGES));
    m_settings[WebEngineSettings::ENABLE_JAVASCRIPT] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_ENABLE_JAVASCRIPT));
    m_settings[WebEngineSettings::REMEMBER_FROM_DATA] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_REMEMBER_FROM_DATA));
    m_settings[WebEngineSettings::REMEMBER_PASSWORDS] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_REMEMBER_PASSWORDS));
    m_settings[WebEngineSettings::AUTOFILL_PROFILE_DATA] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_AUTOFILL_PROFILE_DATA));
    m_settings[WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_SCRIPTS_CAN_OPEN_PAGES));
#endif
}

WebEngineService::~WebEngineService()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void WebEngineService::destroyTabs()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_stateStruct->tabs.clear();
    if (m_currentWebView)
        m_currentWebView.reset();
}

Evas_Object * WebEngineService::getLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return nullptr;
    }
    return m_currentWebView->getLayout();
}

Evas_Object* WebEngineService::getWidget()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return nullptr;
    }
    return m_currentWebView->getWidget();
}

void WebEngineService::init(Evas_Object* guiParent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_initialised) {
        m_guiParent = guiParent;
        m_initialised = true;
    }
}

void WebEngineService::initializeDownloadControl(Ewk_Context* context)
{
    ewk_context_did_start_download_callback_set(context , _download_request_cb, this);
    m_downloadControl = std::make_shared<DownloadControl>();
}

void WebEngineService::preinitializeWebViewCache()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webViewCacheInitialized) {
        m_webViewCacheInitialized = true;
        Ewk_Context* context = ewk_context_default_get();

        initializeDownloadControl(context);

        Evas_Object* ewk_view = ewk_view_add_with_context(evas_object_evas_get(
                reinterpret_cast<Evas_Object *>(m_guiParent)), context);
        ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);
        ewk_view_orientation_send(ewk_view, 0);
        evas_object_del(ewk_view);
    }
}

void WebEngineService::connectSignals(std::shared_ptr<WebView> webView)
{
    M_ASSERT(webView);
    webView->favIconChanged.connect(boost::bind(&WebEngineService::_favIconChanged, this, _1));
    webView->titleChanged.connect(boost::bind(&WebEngineService::_titleChanged, this, _1));
    webView->uriChanged.connect(boost::bind(&WebEngineService::_uriChanged, this, _1));
    webView->loadFinished.connect(boost::bind(&WebEngineService::_loadFinished, this));
    webView->loadStarted.connect(boost::bind(&WebEngineService::_loadStarted, this));
    webView->loadStop.connect(boost::bind(&WebEngineService::_loadStop, this));
    webView->loadProgress.connect(boost::bind(&WebEngineService::_loadProgress, this, _1));
    webView->loadError.connect(boost::bind(&WebEngineService::_loadError, this));
    webView->forwardEnableChanged.connect(boost::bind(&WebEngineService::_forwardEnableChanged, this, _1));
    webView->backwardEnableChanged.connect(boost::bind(&WebEngineService::_backwardEnableChanged, this, _1));
    webView->confirmationRequest.connect(boost::bind(&WebEngineService::_confirmationRequest, this, _1));
    webView->IMEStateChanged.connect(boost::bind(&WebEngineService::_IMEStateChanged, this, _1));
    webView->snapshotCaptured.connect(boost::bind(&WebEngineService::_snapshotCaptured, this, _1, _2));
    webView->redirectedWebPage.connect(boost::bind(&WebEngineService::_redirectedWebPage, this, _1, _2));
    webView->setCertificatePem.connect(boost::bind(&WebEngineService::_setCertificatePem, this, _1, _2));
    webView->setWrongCertificatePem.connect(boost::bind(&WebEngineService::_setWrongCertificatePem, this, _1, _2));
    webView->fullscreenModeSet.connect([this](auto state){fullscreenModeSet(state);});
#if PWA
    webView->resultDataManifest.connect(boost::bind(&WebEngineService::_resultDataManifest, this, _1));
    webView->iconDownload.connect(boost::bind(&WebEngineService::_iconDownload, this, _1));
#endif
#if PROFILE_MOBILE
    webView->getRotation.connect(boost::bind(&WebEngineService::_getRotation, this));
    webView->unsecureConnection.connect(boost::bind(&WebEngineService::_unsecureConnection, this));
    webView->findOnPage.connect(boost::bind(&WebEngineService::_findOnPage, this, _1));
#endif
}

void WebEngineService::disconnectSignals(std::shared_ptr<WebView> webView)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(webView);
    webView->favIconChanged.disconnect(boost::bind(&WebEngineService::_favIconChanged, this));
    webView->titleChanged.disconnect(boost::bind(&WebEngineService::_titleChanged, this, _1));
    webView->uriChanged.disconnect(boost::bind(&WebEngineService::_uriChanged, this, _1));
    webView->loadFinished.disconnect(boost::bind(&WebEngineService::_loadFinished, this));
    webView->loadStarted.disconnect(boost::bind(&WebEngineService::_loadStarted, this));
    webView->loadStop.disconnect(boost::bind(&WebEngineService::_loadStop, this));
    webView->loadProgress.disconnect(boost::bind(&WebEngineService::_loadProgress, this, _1));
    webView->loadError.disconnect(boost::bind(&WebEngineService::_loadError, this));
    webView->forwardEnableChanged.disconnect(boost::bind(&WebEngineService::_forwardEnableChanged, this, _1));
    webView->backwardEnableChanged.disconnect(boost::bind(&WebEngineService::_backwardEnableChanged, this, _1));
    webView->confirmationRequest.disconnect(boost::bind(&WebEngineService::_confirmationRequest, this, _1));
    webView->IMEStateChanged.disconnect(boost::bind(&WebEngineService::_IMEStateChanged, this, _1));
    webView->redirectedWebPage.disconnect(boost::bind(&WebEngineService::_redirectedWebPage, this, _1, _2));
    webView->fullscreenModeSet.disconnect_all_slots();
#if PWA
    webView->resultDataManifest.disconnect(boost::bind(&WebEngineService::_resultDataManifest, this));
    webView->iconDownload.disconnect(boost::bind(&WebEngineService::_iconDownload, this));
#endif
#if PROFILE_MOBILE
    webView->getRotation.disconnect(boost::bind(&WebEngineService::_getRotation, this));
    webView->unsecureConnection.disconnect(boost::bind(&WebEngineService::_unsecureConnection, this));
    webView->findOnPage.disconnect(boost::bind(&WebEngineService::_findOnPage, this, _1));
#endif
}

void WebEngineService::disconnectCurrentWebViewSignals()
{
    if (m_currentWebView.get())
        disconnectSignals(m_currentWebView);
}

void WebEngineService::connectCurrentWebViewSignals()
{
    if (m_currentWebView)
        connectSignals(m_currentWebView);
    else
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
}

int WebEngineService::createTabId()
{
    m_tabIdCreated = -1;
    AbstractWebEngine::createTabId();
    if (m_tabIdCreated == -1) {
        BROWSER_LOGE("%s generated tab id == -1", __PRETTY_FUNCTION__);
    }
    return m_tabIdCreated;
}

void WebEngineService::onTabIdCreated(int tabId)
{
    m_tabIdCreated= tabId;
}

void WebEngineService::setURI(const std::string & uri)
{
    BROWSER_LOGD("[%s:%d] uri=%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_stopped = false;
    m_currentWebView->setURI(uri);
}

std::string WebEngineService::getURI() const
{
    M_ASSERT(m_currentWebView);
    if (m_currentWebView)
        return m_currentWebView->getURI();
    else
        return std::string("");
}

#if PWA
void WebEngineService::requestManifest()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currentWebView->requestManifest();
}

void WebEngineService::_resultDataManifest(std::string pwaData)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    resultDataManifest(pwaData);
}

void WebEngineService::_iconDownload(std::string download_uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_downloadControl->launch_download_app(download_uri.c_str());
}
#endif

bool WebEngineService::isLoadError() const
{
    if (!m_currentWebView) {
        BROWSER_LOGW("[%s:%d] no current webview!", __PRETTY_FUNCTION__, __LINE__);
        return false;
    }
    return m_currentWebView->isLoadError();
}


std::string WebEngineService::getTitle() const
{
    M_ASSERT(m_currentWebView);
    if (m_currentWebView) {
        if (m_stopped)
            return m_currentWebView->getURI();
        else
            return m_currentWebView->getTitle();
    } else
        return std::string("");
}

TabOrigin WebEngineService::getOrigin() const
{
    if (m_currentWebView)
        return m_currentWebView->getOrigin();
    else
        return TabOrigin::UNKNOWN;
}

std::string WebEngineService::getUserAgent() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return std::string();
    }
    return m_currentWebView->getUserAgent();
}

void WebEngineService::setUserAgent(const std::string& ua)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->setUserAgent(ua);
}

void WebEngineService::suspend()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->suspend();
#if PROFILE_MOBILE
        unregisterHWKeyCallback();
#endif
}

void WebEngineService::resume()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->resume();
#if PROFILE_MOBILE
        registerHWKeyCallback();
#endif
}

bool WebEngineService::isSuspended() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return false;
    }
    return m_currentWebView->isSuspended();
}

void WebEngineService::stopLoading(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_stopped = true;
    m_currentWebView->stopLoading();
}

void WebEngineService::reload(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_stopped = false;
    m_currentWebView->reload();
}

void WebEngineService::back(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_stopped = false;
    m_currentWebView->back();
#if PROFILE_MOBILE
    closeFindOnPage();
#endif
}

void WebEngineService::forward(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_stopped = false;
    m_currentWebView->forward();
#if PROFILE_MOBILE
    closeFindOnPage();
#endif
}

bool WebEngineService::isBackEnabled() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return false;
    }
    return m_currentWebView->isBackEnabled();
}

bool WebEngineService::isForwardEnabled() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return false;
    }
    return m_currentWebView->isForwardEnabled();
}

bool WebEngineService::isLoading() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return false;
    }
    return m_currentWebView->isLoading();
}

void WebEngineService::_favIconChanged(std::shared_ptr<tizen_browser::tools::BrowserImage> bi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    favIconChanged(bi);
}

void WebEngineService::_titleChanged(const std::string& title)
{
    titleChanged(title);
}

void WebEngineService::_uriChanged(const std::string & uri)
{
    uriChanged(uri);
}

void WebEngineService::_loadFinished()
{
    loadFinished();
}

void WebEngineService::_loadStarted()
{
    loadStarted();
}

void WebEngineService::_loadStop()
{
    loadStop();
}

void WebEngineService::_loadError()
{
    loadError();
}

void WebEngineService::_forwardEnableChanged(bool enable)
{
    forwardEnableChanged(enable);
}

void WebEngineService::_backwardEnableChanged(bool enable)
{
    backwardEnableChanged(enable);
}

void WebEngineService::_loadProgress(double d)
{
    loadProgress(d);
}

void WebEngineService::_confirmationRequest(WebConfirmationPtr c)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    confirmationRequest(c);
}

int WebEngineService::tabsCount() const
{
    return m_stateStruct->tabs.size();
}

TabId WebEngineService::currentTabId() const
{
    return m_currentTabId;
}

std::vector<TabContentPtr> WebEngineService::getTabContents() const {
    std::vector<TabContentPtr> result;
    for (auto const& tab : m_stateStruct->tabs) {
        auto tabContent =
            std::make_shared<TabContent>(
                tab.first,
                tab.second->getURI(),
                tab.second->getTitle(),
                tab.second->getOrigin(),
                m_state == State::SECRET);
        result.push_back(tabContent);
    }
    return result;
}

TabId WebEngineService::addTab(
    const std::string & uri,
    const boost::optional<int> tabId,
    const std::string& title,
    bool desktopMode,
    TabOrigin origin)
{
    if (!(*AbstractWebEngine::checkIfCreate()))
        return currentTabId();

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    TabId newTabId(0);
    if (m_state == State::NORMAL) {
        int newAdaptorId = -1;
        if (tabId) {
            newAdaptorId = *tabId;
        } else {
            // searching for next available tabId
            newAdaptorId = createTabId();
            if (newAdaptorId < 0)
                return TabId(TabId::NONE);
        }
        newTabId = TabId(newAdaptorId);
    } else {
        ++m_tabIdSecret;
        newTabId = TabId(m_tabIdSecret);
    }

    if (!m_webViewCacheInitialized) {
        initializeDownloadControl();
        m_webViewCacheInitialized = true;
    }
    WebViewPtr p = std::make_shared<WebView>(m_guiParent, newTabId, title, m_state == State::SECRET);
    p->init(desktopMode, origin);

    m_stateStruct->tabs[newTabId] = p;

#if PROFILE_MOBILE
    setWebViewSettings(p);
#endif

    if (!uri.empty()) {
        p->setURI(uri);
    }

    AbstractWebEngine::tabCreated();

    return newTabId;
}

Evas_Object* WebEngineService::getTabView(TabId id){
    if (m_stateStruct->tabs.find(id) == m_stateStruct->tabs.end()) {
        BROWSER_LOGW("[%s:%d] there is no tab of id %d", __PRETTY_FUNCTION__, __LINE__, id.get());
        return nullptr;
    }
    return m_stateStruct->tabs[id]->getLayout();
}

bool WebEngineService::switchToTab(tizen_browser::basic_webengine::TabId newTabId)
{
    BROWSER_LOGD("[%s:%d] newTabId=%s", __PRETTY_FUNCTION__, __LINE__, newTabId.toString().c_str());

    // if there was any running WebView
    if (m_currentWebView) {
        disconnectSignals(m_currentWebView);
        suspend();
    }

    if (m_stateStruct->tabs.find(newTabId) == m_stateStruct->tabs.end()) {
        BROWSER_LOGW("[%s:%d] there is no tab of id %d", __PRETTY_FUNCTION__, __LINE__, newTabId.get());
        return false;
    }
#if PROFILE_MOBILE
    closeFindOnPage();
#endif
    m_currentWebView = m_stateStruct->tabs[newTabId];
    m_currentTabId = newTabId;
    m_stateStruct->mostRecentTab.erase(
        std::remove(m_stateStruct->mostRecentTab.begin(),
            m_stateStruct->mostRecentTab.end(),
            newTabId),
        m_stateStruct->mostRecentTab.end());
    m_stateStruct->mostRecentTab.push_back(newTabId);

    connectSignals(m_currentWebView);
    resume();

    titleChanged(m_currentWebView->getTitle());
    uriChanged(m_currentWebView->getURI());
    forwardEnableChanged(m_currentWebView->isForwardEnabled());
    backwardEnableChanged(m_currentWebView->isBackEnabled());
    currentTabChanged(m_currentTabId);
#if PROFILE_MOBILE
    m_currentWebView->orientationChanged();
#endif

    return true;
}

bool WebEngineService::closeTab()
{
    BROWSER_LOGD("[%s:%d] closing tab=%s", __PRETTY_FUNCTION__, __LINE__, m_currentTabId.toString().c_str());
    bool res = closeTab(m_currentTabId);
    return res;
}

bool WebEngineService::closeTab(TabId id) {
    BROWSER_LOGD("[%s:%d] closing tab=%s", __PRETTY_FUNCTION__, __LINE__, id.toString().c_str());

    TabId closingTabId = id;
    bool res = true;
    if (closingTabId == TabId::NONE){
        return res;
    }
    m_stateStruct->tabs.erase(closingTabId);
    m_stateStruct->mostRecentTab.erase(
        std::remove(m_stateStruct->mostRecentTab.begin(),
            m_stateStruct->mostRecentTab.end(),
            closingTabId),
        m_stateStruct->mostRecentTab.end());

    if (closingTabId == m_currentTabId) {
        if (m_currentWebView)
            m_currentWebView.reset();
    }
    if (m_stateStruct->tabs.size() == 0) {
        m_currentTabId = TabId::NONE;
    }
    else if (closingTabId == m_currentTabId && m_stateStruct->mostRecentTab.size()){
        res = switchToTab(m_stateStruct->mostRecentTab.back());
    }

    tabClosed(closingTabId);
    return res;
}

void WebEngineService::confirmationResult(WebConfirmationPtr c)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // tabId MUST be set
    M_ASSERT(c && c->getTabId() != TabId());

    // check if still exists
    if (m_stateStruct->tabs.find(c->getTabId()) == m_stateStruct->tabs.end()) {
        return;
    }

    m_stateStruct->tabs[c->getTabId()]->confirmationResult(c);
}

bool WebEngineService::isSecretMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_state == State::SECRET;
}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebEngineService::getSnapshotData(int width, int height,
        tizen_browser::tools::SnapshotType snapshot_type)
{
    M_ASSERT(m_currentWebView);
    if (m_currentWebView)
        return m_currentWebView->captureSnapshot(width, height, snapshot_type != tools::SnapshotType::SYNC, snapshot_type);
    else
        return std::make_shared<tizen_browser::tools::BrowserImage>();
}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebEngineService::getSnapshotData(TabId id, int width, int height, bool async,
        tizen_browser::tools::SnapshotType snapshot_type){
    if (m_stateStruct->tabs.find(id) == m_stateStruct->tabs.end()) {
        BROWSER_LOGW("[%s:%d] there is no tab of id %d", __PRETTY_FUNCTION__, __LINE__, id.get());
        return std::shared_ptr<tizen_browser::tools::BrowserImage>();
    }
   return m_stateStruct->tabs[id]->captureSnapshot(width, height, async, snapshot_type);
}

void WebEngineService::setFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->setFocus();
}

void WebEngineService::clearFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->clearFocus();
}

bool WebEngineService::hasFocus() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return false;
    }
    return m_currentWebView->hasFocus();
}


std::shared_ptr<tizen_browser::tools::BrowserImage> WebEngineService::getFavicon()
{
    M_ASSERT(m_currentWebView);
    if (m_currentWebView) {
        if (m_stopped)
            return std::make_shared<tizen_browser::tools::BrowserImage>();
        else
            return m_currentWebView->getFavicon();
    } else
        return std::make_shared<tizen_browser::tools::BrowserImage>();
}

#if PROFILE_MOBILE
void WebEngineService::setWebViewSettings(std::shared_ptr<WebView> webView) {
    webView->ewkSettingsAutoFittingSet(m_settings[WebEngineSettings::PAGE_OVERVIEW]);
    webView->ewkSettingsLoadsImagesSet(m_settings[WebEngineSettings::LOAD_IMAGES]);
    webView->ewkSettingsJavascriptEnabledSet(m_settings[WebEngineSettings::ENABLE_JAVASCRIPT]);
    webView->ewkSettingsFormCandidateDataEnabledSet(m_settings[WebEngineSettings::REMEMBER_FROM_DATA]);
    webView->ewkSettingsAutofillPasswordFormEnabledSet(m_settings[WebEngineSettings::REMEMBER_PASSWORDS]);
    webView->ewkSettingsFormProfileDataEnabledSet(m_settings[WebEngineSettings::AUTOFILL_PROFILE_DATA]);
    webView->ewkSettingsScriptsCanOpenNewPagesEnabledSet(m_settings[WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES]);
}

void WebEngineService::orientationChanged()
{
    if (m_currentWebView)
        m_currentWebView->orientationChanged();
}

void WebEngineService::_unsecureConnection()
{
    unsecureConnection();
}

void WebEngineService::_findOnPage(const std::string& str)
{
    openFindOnPage(str);
}
#endif

int WebEngineService::getZoomFactor() const
{
    if (!m_currentWebView)
        return 0;
    return static_cast<int>(m_currentWebView->getZoomFactor()*100);

}

void WebEngineService::setZoomFactor(int zoomFactor)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->setZoomFactor(0.01*zoomFactor);

}

void WebEngineService::clearCache()
{
    for(const auto& it: m_stateStruct->tabs) {
        it.second->clearCache();
    }
}

void WebEngineService::clearCookies()
{
    for(const auto& it: m_stateStruct->tabs) {
        it.second->clearCookies();
    }
}

void WebEngineService::clearPrivateData()
{
    for(const auto& it: m_stateStruct->tabs) {
        it.second->clearPrivateData();
    }
}

void WebEngineService::clearPasswordData()
{
    for(const auto& it: m_stateStruct->tabs) {
        it.second->clearPasswordData();
    }
}

void WebEngineService::clearFormData()
{
    auto context = ewk_context_default_get();
    Eina_List *list = nullptr;
    void *item_data = nullptr;
    Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(context);

    EINA_LIST_FOREACH(entire_item_list, list, item_data) {
        if (item_data) {
            Ewk_Autofill_Profile *profile = static_cast<Ewk_Autofill_Profile*>(item_data);
            ewk_context_form_autofill_profile_remove(context, ewk_autofill_profile_id_get(profile));
        }
    }
    ewk_context_form_candidate_data_delete_all(ewk_context_default_get());
    for(const auto& it: m_stateStruct->tabs)
        it.second->clearFormData();
}

void WebEngineService::searchOnWebsite(const std::string & searchString, int flags)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->searchOnWebsite(searchString, flags);
}

void WebEngineService::_IMEStateChanged(bool enable)
{
    IMEStateChanged(enable);
}

void WebEngineService::_snapshotCaptured(std::shared_ptr<tizen_browser::tools::BrowserImage> image, tools::SnapshotType snapshot_type)
{
    snapshotCaptured(image, snapshot_type);
}

void WebEngineService::_redirectedWebPage(const std::string& oldUrl, const std::string& newUrl)
{
    redirectedWebPage(oldUrl, newUrl);
}

void WebEngineService::_setCertificatePem(const std::string& uri, const std::string& pem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    setCertificatePem(uri, pem);
}

void WebEngineService::_setWrongCertificatePem(const std::string& uri, const std::string& pem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    setWrongCertificatePem(uri, pem);
}

#if PROFILE_MOBILE
void WebEngineService::_download_request_cb(const char *download_uri, void *data)
{
     BROWSER_LOGD("[%s:%d] download_uri= [%s]", __PRETTY_FUNCTION__, __LINE__, download_uri);

     WebEngineService *wes = static_cast<WebEngineService *>(data);

     if (!strncmp(download_uri, "data:", strlen("data:"))) {
         wes->downloadStarted(DOWNLOAD_STARTING_DOWNLOAD);
         BROWSER_LOGD("[%s:%d] download start..", __PRETTY_FUNCTION__, __LINE__);

         if (wes->m_downloadControl->handle_data_scheme(download_uri) == EINA_TRUE) {
             BROWSER_LOGD("[%s:%d] saved..", __PRETTY_FUNCTION__, __LINE__);
             wes->downloadStarted(DOWNLOAD_SAVEDPAGES);
          } else {
             BROWSER_LOGD("[%s:%d] fail..", __PRETTY_FUNCTION__, __LINE__);
             wes->downloadStarted(DOWNLOAD_FAIL);
          }
     } else if (strncmp(download_uri, "http://", strlen("http://")) &&
                strncmp(download_uri, "https://", strlen("https://"))) {
         BROWSER_LOGD("[%s:%d] Only http or https URLs can be downloaded", __PRETTY_FUNCTION__, __LINE__);
         wes->downloadStarted(DOWNLOAD_ONLY_HTTP_OR_HTTPS_URLS);
         return;
     } else {
         wes->downloadStarted(wes->m_downloadControl->launch_download_app(download_uri) == EINA_TRUE);
     }
}


int WebEngineService::_getRotation()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<int> signal = getRotation();
    if (signal)
        return *signal;
    else
        return -1;
}

void WebEngineService::moreKeyPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }

    if (m_currentTabId == TabId::NONE || m_currentWebView->clearTextSelection())
        return;

    if (m_currentWebView->isFullScreen()) {
        m_currentWebView->exitFullScreen();
    }
}
#endif

void WebEngineService::backButtonClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }

#if PROFILE_MOBILE
    if (m_currentWebView->clearTextSelection())
        return;

    if (m_currentWebView->isFullScreen()) {
        m_currentWebView->exitFullScreen();
        return;
    }
#endif

    if (isBackEnabled()) {
        m_currentWebView->back();
    } else if (m_currentWebView->getOrigin().isFromWebView() &&
        m_stateStruct->tabs.find(m_currentWebView->getOrigin().getValue()) != m_stateStruct->tabs.end()) {
        int switchTo = m_currentWebView->getOrigin().getValue();
        closeTab();
        switchToTab(switchTo);
    } else if (m_currentWebView->getOrigin().isFromQuickAccess()) {
        closeTab();
        switchToQuickAccess();
    } else {
        minimizeBrowser();
    }
}

void WebEngineService::switchToDesktopMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->switchToDesktopMode();
}

void WebEngineService::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->switchToMobileMode();
}

bool WebEngineService::isDesktopMode() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return false;
    }
    return m_currentWebView->isDesktopMode();
}

void WebEngineService::scrollView(const int& dx, const int& dy)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);
    if (!m_currentWebView) {
        BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__,"m_currentWebView is null");
        return;
    }
    m_currentWebView->scrollView(dx, dy);
}

void WebEngineService::findWord(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data)
{
    if (m_currentWebView)
        m_currentWebView->findWord(word, forward, found_cb, data);
}

bool WebEngineService::getSettingsParam(WebEngineSettings param) {
    return m_settings.at(param);
}

void WebEngineService::setSettingsParam(WebEngineSettings param, bool value) {
    m_settings[param] = value;
    for(auto it = m_stateStruct->tabs.cbegin(); it != m_stateStruct->tabs.cend(); ++it) {
        switch (param) {
        case WebEngineSettings::PAGE_OVERVIEW:
            it->second->ewkSettingsAutoFittingSet(value);
            break;
        case WebEngineSettings::LOAD_IMAGES:
            it->second->ewkSettingsLoadsImagesSet(value);
            break;
        case WebEngineSettings::ENABLE_JAVASCRIPT:
            it->second->ewkSettingsJavascriptEnabledSet(value);
            break;
        case WebEngineSettings::REMEMBER_FROM_DATA:
            it->second->ewkSettingsFormCandidateDataEnabledSet(value);
            break;
        case WebEngineSettings::REMEMBER_PASSWORDS:
            it->second->ewkSettingsAutofillPasswordFormEnabledSet(value);
            break;
        case WebEngineSettings::AUTOFILL_PROFILE_DATA:
            it->second->ewkSettingsFormProfileDataEnabledSet(value);
            break;
        case WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES:
            it->second->ewkSettingsScriptsCanOpenNewPagesEnabledSet(value);
            break;
        default:
            BROWSER_LOGD("[%s:%d] Warning unknown param value!", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}

void WebEngineService::resetSettingsParam()
{
    setSettingsParam(WebEngineSettings::PAGE_OVERVIEW, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_PAGE_OVERVIEW)));
    setSettingsParam(WebEngineSettings::LOAD_IMAGES, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_LOAD_IMAGES)));
    setSettingsParam(WebEngineSettings::ENABLE_JAVASCRIPT, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_ENABLE_JAVASCRIPT)));
    setSettingsParam(WebEngineSettings::REMEMBER_FROM_DATA, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_REMEMBER_FROM_DATA)));
    setSettingsParam(WebEngineSettings::REMEMBER_PASSWORDS, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_REMEMBER_PASSWORDS)));
    setSettingsParam(WebEngineSettings::AUTOFILL_PROFILE_DATA, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_AUTOFILL_PROFILE_DATA)));
    setSettingsParam(WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES, boost::any_cast<bool>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_SCRIPTS_CAN_OPEN_PAGES)));
}

void WebEngineService::changeState()
{
    if (m_state == State::NORMAL) {
        m_state = State::SECRET;
        m_stateStruct = &m_secretStateStruct;
    } else {
        m_state = State::NORMAL;
        m_stateStruct = &m_normalStateStruct;
    }
}

} /* end of webengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */
