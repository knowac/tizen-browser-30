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

#ifndef WEBENGINESERVICE_H_
#define WEBENGINESERVICE_H_

#include <boost/noncopyable.hpp>
#include <string>
#include <Evas.h>
#include <memory>

#include "ServiceFactory.h"
#include "service_macros.h"

#include "AbstractWebEngine/AbstractWebEngine.h"
#include "AbstractWebEngine/TabIdTypedef.h"
#include "AbstractWebEngine/WebConfirmation.h"
#include "AbstractWebEngine/TabOrigin.h"
#include "AbstractWebEngine/State.h"
#include "SnapshotType.h"
#include "BrowserImage.h"
#include "DownloadControl/DownloadControl.h"
#include "WebView.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webengine_service {

class WebView;

using WebViewPtr = std::shared_ptr<WebView>;
using TabsMapPtr = std::shared_ptr<std::map<TabId, WebViewPtr > >;

class BROWSER_EXPORT WebEngineService : public AbstractWebEngine, boost::noncopyable
{
public:
    WebEngineService();
    virtual ~WebEngineService();
    virtual std::string getName();

    Evas_Object * getLayout();
#if !DUMMY_BUTTON
    Evas_Object * getWidget();
#endif
    void init(Evas_Object *guiParent);
    void preinitializeWebViewCache();

    void setURI(const std::string &);
    std::string getURI(void) const;
#if PWA
    void requestManifest(void);
#endif
    std::string getTitle(void) const;
    TabOrigin getOrigin(void) const;
    std::string getUserAgent(void) const;
    void setUserAgent(const std::string& ua);

    void suspend(void);
    void resume(void);
    bool isSuspended(void) const;

    void stopLoading(void);
    void reload(void);

    void back(void);
    void forward(void);

    bool isBackEnabled(void) const;
    bool isForwardEnabled(void) const;

    bool isLoading() const;

    void clearCache();
    void clearCookies();
    void clearPrivateData();
    void clearPasswordData();
    void clearFormData();

    int tabsCount() const;
    TabId currentTabId() const;

    void destroyTabs();

    /**
     * Get TabContent collection filled with TabID and titles.
     * Without thumbnails.
     */
    std::vector<TabContentPtr> getTabContents() const;

    /**
     * See AbstractWebEngine@addTab(const std::string&, const TabId*,
     * const boost::optional<int>, const std::string&, bool, bool)
     */
    TabId addTab(
            const std::string & uri = std::string(),
            const boost::optional<int> tabId = boost::none,
            const std::string& title = std::string(),
            bool desktopMode = true,
            TabOrigin origin = TabOrigin::UNKNOWN);
    Evas_Object* getTabView(TabId id);
    bool switchToTab(TabId);
    bool closeTab();
    bool closeTab(TabId);
    void confirmationResult(WebConfirmationPtr);

    /**
     * @brief Get snapshot of webpage
     *
     * @param width of snapshot
     * @param height of snapshot
     * @param type of snapshot
     * @return Shared pointer to BrowserImage
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(int width, int height, tizen_browser::tools::SnapshotType snapshot_type);

    std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(TabId id, int width, int height, bool async, tizen_browser::tools::SnapshotType snapshot_type);

    /**
     * @brief Get the state of secret mode
     *
     */
    bool isSecretMode();


    /**
     * @brief Check if current tab has load error.
     *
     */
    bool isLoadError() const;

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

    virtual int getZoomFactor() const;

    /**
     * @brief check if autofit is enabled
     */

    virtual void setZoomFactor(int zoomFactor);


    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    void searchOnWebsite(const std::string &, int flags);

    /**
     * @brief Get favicon of current page loaded
     */
    tools::BrowserImagePtr getFavicon();

    /**
     * @brief back or exit when back key is pressed
     */
    void backButtonClicked();

    void moreKeyPressed();
    void orientationChanged();

    void switchToMobileMode();
    void switchToDesktopMode();
    bool isDesktopMode() const;

    void scrollView(const int& dx, const int& dy);

    void onTabIdCreated(int tabId) override;

    /**
     * @brief Searches for word in the current page.
     *
     * @param enabled The input word entered by user in Find on page entry.
     * @param forward If true, search forward, else search backward.
     * @param found_cb Callback function invoked when "text,found" event is triggered.
     * @param data User data.
     */
    void findWord(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data);

    /**
     * @brief Get settings param.
     */
    bool getSettingsParam(WebEngineSettings param) override;

    /**
     * @brief Set bool settings param value.
     */
    void setSettingsParam(WebEngineSettings param, bool value) override;

    /**
     * @brief Reset WebView settings
     */
    virtual void resetSettingsParam() override;

    /**
     * @brief set next state
     */
    void changeState() override;

    /**
     * @brief Get current state of the engine
     */
    State getState() override { return m_state; }
private:
    // callbacks from WebView
    void _favIconChanged(std::shared_ptr<tizen_browser::tools::BrowserImage> bi);
    void _uriChanged(const std::string &);
    void _loadFinished();
    void _loadStarted();
    void _loadStop();
    void _loadError();
    void _forwardEnableChanged(bool);
    void _backwardEnableChanged(bool);
    void _loadProgress(double);
    void _confirmationRequest(WebConfirmationPtr) ;
    void _IMEStateChanged(bool);
    void _snapshotCaptured(std::shared_ptr<tizen_browser::tools::BrowserImage> snapshot, tools::SnapshotType snapshot_type);
    void _redirectedWebPage(const std::string& oldUrl, const std::string& newUrl);
    void _setCertificatePem(const std::string& uri, const std::string& pem);
    void _setWrongCertificatePem(const std::string& uri, const std::string& pem);

    int _getRotation();
    void setWebViewSettings(std::shared_ptr<WebView> webView);
    void _unsecureConnection();
    void _findOnPage(const std::string& str);
    static void _download_request_cb(const char *download_uri, void *data);

    /**
     * disconnect signals from specified WebView
     * \param WebView
     */
    void disconnectSignals(WebViewPtr);

    /**
     * connect signals of specified WebView
     * \param WebView
     */
    void connectSignals(WebViewPtr);

    int createTabId();
    void initializeDownloadControl(Ewk_Context* context = ewk_context_default_get());

private:
    struct StateStruct {
        std::map<TabId, WebViewPtr > tabs;
        std::vector<TabId> mostRecentTab;
        TabId currentTabId = TabId::NONE;
    };

    State m_state;
    bool m_initialised;
    Evas_Object* m_guiParent;
    bool m_stopped;
    bool m_webViewCacheInitialized;

    // current WebView
    WebViewPtr m_currentWebView;
    StateStruct m_normalStateStruct;
    StateStruct m_secretStateStruct;
    StateStruct* m_stateStruct;
    int m_tabIdCreated;
    int m_tabIdSecret;
    bool m_signalsConnected;

    std::map<WebEngineSettings, bool>  m_settings;
    std::shared_ptr<DownloadControl> m_downloadControl;
    Ewk_Context* m_defaultContext;
};

} /* end of webengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* WEBENGINESERVICE_H_ */
