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

#ifndef __ABSTRACT_WEBENGINE_H__
#define __ABSTRACT_WEBENGINE_H__ 1

#include <boost/signals2/signal.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <Evas.h>

#include "SnapshotType.h"

#include "BrowserImage.h"
#include "../ServiceManager/Debug/Lifecycle.h"
#include "../ServiceManager/AbstractService.h"

#include "TabId.h"
#include "TabOrigin.h"
#include "WebConfirmation.h"
#include "WebEngineSettings.h"
#include "State.h"

namespace tizen_browser {
namespace basic_webengine {
/***
 * AbstractWebEngine defines interface for WebEngine object.
 * It is designed for multiple tabs operations.
 * It is designed to be only way for communication with WebEngine. It should NOT return WebEngine object.
 */
#ifndef NDEBUG
class AbstractWebEngine: public tizen_browser::core::AbstractService, ShowLifeCycle<AbstractWebEngine>
#else
class AbstractWebEngine: public tizen_browser::core::AbstractService
#endif
{
public:
    /**
     * \todo this function should return nonEFL object in future
     * Remember that there must be at least 1 tab created to return layout
     * @return pointer Evas_Object for current WebView.
     */
    virtual Evas_Object* getLayout() = 0;

    /**
     * Remember that there must be at least 1 tab created to return layout
     * @return pointer to Evas_Object widget connected with a current WebView.
     */
    virtual Evas_Object* getWidget() = 0;

    /**
     * Initialize WebEngine.
     * @param guiParent GUI parent object
     */
    virtual void init(Evas_Object* guiParent) = 0;

    /**
     * Preinitialize WebView parameters.
     */
    virtual void preinitializeWebViewCache() = 0;

    /**
     * Set URI address for current tab.
     * @param uri
     */
    virtual void setURI(const std::string & uri) = 0;

    /**
     * @return uri address for current tab.
     */
    virtual std::string getURI(void) const = 0;

#if PWA
    virtual void requestManifest(void) = 0;
#endif

    /**
     * @return title of page opened in current tab.
     */
    virtual std::string getTitle(void) const = 0;

    /**
     * @return Current tab origin.
     */
    virtual TabOrigin getOrigin(void) const = 0;

    /**
     * @return Current UserAgent string of the browser.
     */
    virtual std::string getUserAgent(void) const = 0;

    /**
     * @return Override Browser's UserAgent.
     */
    virtual void setUserAgent(const std::string& ua) = 0;

    /**
     * Suspend current webview.
     */
    virtual void suspend(void) = 0;

    /**
     * Resume current webview.
     */
    virtual void resume(void) = 0;

    /**
     * @return true if current webview is suspended, false otherwise
     */
    virtual bool isSuspended(void) const = 0;

    /**
     * Stop loading current page.
     */
    virtual void stopLoading(void) = 0;

    /**
     * Reload current page.
     */
    virtual void reload(void) = 0;

    /**
     * Go to previous page (if possible)
     */
    virtual void back(void) = 0;

    /**
     * Go to next page (if possible)
     */
    virtual void forward(void) = 0;

    /**
     * @return true if it is possible to go back, false otherwise
     */
    virtual bool isBackEnabled(void) const = 0;

    /**
     * @return true if it is possible to go forward, false otherwise
     */
    virtual bool isForwardEnabled(void) const = 0;

    /**
     * @return true if page is still loading, false otherwise
     */
    virtual bool isLoading() const = 0;

    /**
     * @return number of tabs
     */
    virtual int tabsCount() const = 0;

    /**
     * @return TabId of current tab
     */
    virtual TabId currentTabId() const = 0;

    /**
     * Destroy active WebViews.
     */
    virtual void destroyTabs() = 0;

    /**
     *  @return vector of metadata for all opened tabs
     */
    virtual std::vector<std::shared_ptr<TabContent>> getTabContents() const = 0;

    /**
     * Adds new tab. If it first tab in WebEngine switch to this tab.
     * @param uri if not empty opens specified uri
     * @param openerId id of the tab which content (Evas_Object) should be set
     * for the created tab
     * @param tabId Tab id of the new tab. If boost::none, tab's id will be
     * generated
     * @param desktopMode true if desktop mode, false if mobile mode
     * @return TabId of created tab
     */
    virtual TabId addTab(
            const std::string& uri = std::string(),
            const boost::optional<int> tabId = boost::none,
            const std::string& title = std::string(),
            bool desktopMode = true,
            TabOrigin origin = TabOrigin::UNKNOWN) = 0;

    /**
     * @param tab id
     * @return returns underlaying UI component
     */
    virtual Evas_Object* getTabView(TabId id) = 0;

    /**
     * Switch current tab for tab with tabId
     * @param tabId of tab
     * @return true if tab changed, false otherwise
     */
    virtual bool switchToTab(TabId tabId) = 0;

    /**
     * Close current tab. Switch tab to other (not defined which).
     * Should NOT close last tab (there should be only at least 1 tab)
     * @return true if tab closed successfully, false otherwise
     */
    virtual bool closeTab() = 0;

    /**
     * Close tab specified with id parameter. Switch tab to other (not defined which).
     * Should NOT close last tab (there should be only at least 1 tab)
     * @return true if tab closed successfully, false otherwise
     */
    virtual bool closeTab(TabId id) = 0;

    /**
     * Process confirmation result
     * \param web confirmation with request and result value
     */
    virtual void confirmationResult(WebConfirmationPtr) = 0;

    /**
     * Gets snapshot data as void* for current tab
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(int width = 324, int height = 177,
            tizen_browser::tools::SnapshotType snapshot_type = tizen_browser::tools::SnapshotType::SYNC) = 0;

    /**
     * Gets snapshot data as void* for tab provided as parameter
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(TabId id, int width, int height,
            bool async, tizen_browser::tools::SnapshotType snapshot_type) = 0;

    /**
     * Get the state of secret mode
     *
     * /param id of snapshot
     * /return state of private mode
     */
    virtual bool isSecretMode() = 0;

    virtual bool isLoadError() const = 0;

    /**
     * \brief Sets Focus to URI entry.
     */
    virtual void setFocus()=0;

    /**
     * @brief Remove focus form URI
     */
    virtual void clearFocus()=0;

    /**
     * @brief check if URI is focused
     */
    virtual bool hasFocus() const = 0;

    /**
     * @brief return current zoom factor in percentage.
     *
     * @return real zoom, also for "fit to screen" mode
     */
    virtual int getZoomFactor()const = 0;


    /**
     * @brief Sets zoom factor in percentage
     *
     * @param zoomFactor in percentage of default zoom, 0 means autofit
     */
    virtual void setZoomFactor(int zoomFactor) = 0;

    /**
     * @brief Clear cache of WebView
     */

    virtual void clearCache() = 0;

    /**
     * @brief Clear cookies of WebView
     */
    virtual void clearCookies() = 0;

    /**
     * @brief Clear private data of WebView
     */
    virtual void clearPrivateData() = 0;
    /**
     * @brief Clear saved Password of WebView
     */
    virtual void clearPasswordData() = 0;

    /**
    * @brief Clear Form Data of WebView
    */
    virtual void clearFormData() = 0;

    /**
     * @brief Disconnect signals from current webview.
     */
    virtual void disconnectCurrentWebViewSignals() = 0;

    /**
     * @brief Connect signals to current web view.
     */
    virtual void connectCurrentWebViewSignals() = 0;

    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    virtual void searchOnWebsite(const std::string &, int flags) = 0;

    /**
     * @brief Get favicon of current page loaded
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon() = 0;

    /**
     * @brief back or exit when back key is pressed
     */
    virtual void backButtonClicked() = 0;

#if PROFILE_MOBILE
    /**
     * @brief clear text selection or exit full screen when more key is pressed
     */
    virtual void moreKeyPressed() = 0;
#endif

    /**
     * @brief Switch current view to mobile mode
     */
    virtual void switchToMobileMode() = 0;

    /**
     * @brief Switch current view to desktop mode
     */
    virtual void switchToDesktopMode() = 0;

    /**
     * @brief Check if desktop mode is enabled for current view
     *
     * @return true if desktop mode is enabled
     */
    virtual bool isDesktopMode() const = 0;

    /**
     * Sets an absolute scroll of the given view.
     *
     * Both values are from zero to the contents size minus the viewport
     * size.
     *
     * @param x horizontal position to scroll
     * @param y vertical position to scroll
     */
    virtual void scrollView(const int& dx, const int& dy) = 0;

    /**
     * Slot to which generated tab's id is passed.
     */
    virtual void onTabIdCreated(int tabId) = 0;

    /**
     * @brief Searches for word in the current page.
     *
     * @param enabled The input word entered by user in Find on page entry.
     * @param forward If true, search forward, else search backward.
     * @param found_cb Callback function invoked when "text,found" event is triggered.
     * @param data User data.
     */
    virtual void findWord(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data) = 0;

    /**
     * @brief Get settings param.
     */
    virtual bool getSettingsParam(WebEngineSettings param) = 0;

    /**
     * @brief Set bool settings param value.
     */
    virtual void setSettingsParam(WebEngineSettings param, bool value) = 0;

    /**
     * @brief Reset WebView settings
     */
    virtual void resetSettingsParam() = 0;

    /**
     * @brief Informs WebEngine that device orientation is changed.
     */
    virtual void orientationChanged() = 0;

    /**
     * @brief set next state
     */
    virtual void changeState() = 0;

    /**
     * @brief Get current state of the engine
     */
    virtual State getState() = 0;

    /**
     * Ask browser to minimize itself
     */
    boost::signals2::signal<void ()> minimizeBrowser;

    /**
     * FavIcon of current page changed
     */
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::tools::BrowserImage>)> favIconChanged;

    /**
     * Title of current page changed
     * \param new title
     */
    boost::signals2::signal<void (const std::string&)> titleChanged;

    /**
     * URI of current page changed
     * \param new URI
     */
    boost::signals2::signal<void (const std::string &)> uriChanged;

    /**
     * Possibility of go forward changed
     * \param bool true if it is possible to go forward, false otherwise
     */
    boost::signals2::signal<void (bool)> forwardEnableChanged;

    /**
     * Possibility of go back changed
     * \param bool true if it is possible to go back, false otherwise
     */
    boost::signals2::signal<void (bool)> backwardEnableChanged;

    /**
     * File downoad started
     */
    boost::signals2::signal<void (int)> downloadStarted;

    /**
     * Page load finished
     */
    boost::signals2::signal<void ()> loadFinished;

    /**
     * Page load started
     */
    boost::signals2::signal<void ()> loadStarted;

    /**
     * Load progress changed
     * \param double 0..1 of progress
     */
    boost::signals2::signal<void (double)> loadProgress;

    /**
     * Page load stopped.
     */
    boost::signals2::signal<void ()> loadStop;

    /**
     * Page load error.
     */
    boost::signals2::signal<void ()> loadError;

    /**
     * Current tab changed
     * \param TabId of new tab
     */
    boost::signals2::signal<void (TabId)> currentTabChanged;

    /**
    * New tab was created. It could be explicit call (by user) or tab could be opened from JavaScript.
    */
    boost::signals2::signal<void ()> tabCreated;

    /**
    * Checks if tab can be created.
    */
    boost::signals2::signal<bool ()> checkIfCreate;

    /**
     * Tab closed
     * \param TabId of closed tab
     */
    boost::signals2::signal<void (TabId)> tabClosed;

    /**
     * Open find on page
     */
    boost::signals2::signal<void (const std::string&)> openFindOnPage;

    /**
     * Close find on page
     */
    boost::signals2::signal<void ()> closeFindOnPage;

    /**
     * Confirmation Request
     */
    boost::signals2::signal<void (basic_webengine::WebConfirmationPtr)> confirmationRequest;

    /**
     * All links to RSS/Atom channels were gathered from webpage.
     */
    boost::signals2::signal<void (std::vector<std::string>)> gotFeedLinks;

    /**
     * Status of IME
     * \param bool true if IME is opened, false otherwise
     */
    boost::signals2::signal<void (bool)> IMEStateChanged;

    /**
     * Switch view to actual web page
     */
    boost::signals2::signal<void ()> switchToWebPage;

    /**
     * Signal to switch to window after it is created
     */
    boost::signals2::signal<void ()> windowCreated;

    /**
     * Generate id for the new tab.
     */
    boost::signals2::signal<void()> createTabId;

    /**
     *  Set valid certificate for host.
     */
    boost::signals2::signal<void (const std::string&, const std::string&)> setCertificatePem;

    /**
     *  Set invalid certificate for host.
     */
    boost::signals2::signal<void (const std::string&, const std::string&)> setWrongCertificatePem;

    /**
     * Async signal to save snapshot after it is generated.
     */
    boost::signals2::signal<void(std::shared_ptr<tizen_browser::tools::BrowserImage>,
            tizen_browser::tools::SnapshotType snapshot_type)> snapshotCaptured;

    /**
     * Async signal to inform the redirection has started.
     */
    boost::signals2::signal<void (const std::string&, const std::string&)> redirectedWebPage;

    /**
     * Switch to quick access when tere is no page to witch we can return on back keys.
     */
    boost::signals2::signal<void()> switchToQuickAccess;

    /**
     * Switch fullscreenmode.
     */
    boost::signals2::signal<void(bool)> fullscreenModeSet;

#if PWA
    boost::signals2::signal<void (std::string)> resultDataManifest;

    boost::signals2::signal<void (std::string)> iconDownload;
#endif

#if PROFILE_MOBILE
    /**
     * Register H/W back key callback for the current webview
     */
    boost::signals2::signal<void()> registerHWKeyCallback;

    /**
     * Unregister H/W back key callback for the current webview
     */
    boost::signals2::signal<void()> unregisterHWKeyCallback;

    /**
     * Gets rotation angle value.
     */
    boost::signals2::signal<int()> getRotation;

    /**
     * Unsecure connection to https host, do not even ask to confirm, just inform.
     */
    boost::signals2::signal<void()> unsecureConnection;
#endif
};

} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* __ABSTRACT_WEBENGINE_H__ */
