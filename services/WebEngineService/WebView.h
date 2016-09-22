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

#ifndef WEBVIEW_H_
#define WEBVIEW_H_

#include <boost/signals2/signal.hpp>
#include <string>
#include <Evas.h>
#include <ecore-1/Ecore.h>

#include <ewk_chromium.h>
#include "browser_config.h"
#include "Config.h"
#include "BrowserImage.h"
#include "SnapshotType.h"
#include "AbstractWebEngine/TabId.h"
#include "AbstractWebEngine/WebConfirmation.h"
#include "AbstractWebEngine/TabOrigin.h"

#if PROFILE_MOBILE
#include "DownloadControl/DownloadControl.h"
#include <app_control.h>
#include <app.h>
#include "AbstractRotatable.h"
#endif

#if PROFILE_MOBILE
typedef enum _context_menu_type {
    TEXT_ONLY = 0,
    INPUT_FIELD,
    TEXT_LINK,
    IMAGE_ONLY,
    IMAGE_LINK,
    EMAIL_LINK,
    TEL_LINK,
    TEXT_IMAGE_LINK,
    UNKNOWN_MENU
} context_menu_type;

typedef enum _custom_context_menu_item_tag {
    CUSTOM_CONTEXT_MENU_ITEM_BASE_TAG = EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG,
    CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE,
    CUSTOM_CONTEXT_MENU_ITEM_SHARE,
    CUSTOM_CONTEXT_MENU_ITEM_SEARCH,
    CUSTOM_CONTEXT_MENU_ITEM_SAVE_TO_KEEPIT,
    CUSTOM_CONTEXT_MENU_ITEM_CALL,
    CUSTOM_CONTEXT_MENU_ITEM_SEND_MESSAGE,
    CUSTOM_CONTEXT_MENU_ITEM_SEND_EMAIL,
    CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT,
} custom_context_menu_item_tag;
#endif

namespace tizen_browser {
namespace basic_webengine {
namespace webengine_service {

class WebView
#if PROFILE_MOBILE
        : public tizen_browser::interfaces::AbstractRotatable
#endif
{
public:
    WebView(Evas_Object *, TabId, const std::string& title, bool incognitoMode);
    virtual ~WebView();
    void init(bool desktopMode, TabOrigin origin, Evas_Object * view = NULL);

#if PROFILE_MOBILE
    virtual void orientationChanged() override;
#endif

    void setURI(const std::string &);
    std::string getURI(void);

    std::string getTitle(void);

    std::string getUserAgent(void);
    void setUserAgent(const std::string& ua);

    void suspend(void);
    void resume(void);
    bool isSuspended(void) const { return m_suspended; }

    void stopLoading(void);
    void reload(void);

    void back(void);
    void forward(void);

    bool isBackEnabled(void);
    bool isForwardEnabled(void);

    bool isLoading();
    bool isLoadError() const;

    std::map<std::string, std::vector<std::string> > parse_uri(const char *uriToParse);

    Evas_Object * getLayout();

    void confirmationResult(WebConfirmationPtr);

    /**
     * @brief Get the state of private mode
     *
     * @return state of private mode
     */
    bool isPrivateMode() {return m_private;}

    std::shared_ptr<tizen_browser::tools::BrowserImage> captureSnapshot(int width, int height, bool async,
            tizen_browser::tools::SnapshotType snapshot_type);
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

    /**
     * @brief get current real zoom factor from webkit
     */
    double getZoomFactor() const;

    /**
     * @brief set zoom factor of website
     */
    void setZoomFactor(double zoomFactor);

    void clearCache();
    void clearCookies();
    void clearPrivateData();
    void clearPasswordData();
    void clearFormData();

    /**
     * @return tab id
     */
    const TabId& getTabId();

    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    void searchOnWebsite(const std::string &, int);

    /**
     * @brief Change user agent to desktop type
     */
    void switchToDesktopMode();

    /**
     * @brief Change user agent to mobile type
     */
    void switchToMobileMode();

    /**
     * @brief Check if desktop mode is enabled
     *
     * @return true if desktop mode is enabled
     */
    bool isDesktopMode() const;

    /**
     * @brief Get favicon of URL
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon();

    /**
     * Sets an absolute scroll of the given view.
     *
     * Both values are from zero to the contents size minus the viewport
     * size.
     *
     * @param x horizontal position to scroll
     * @param y vertical position to scroll
     */
    void scrollView(const int& dx, const int& dy);

    TabOrigin getOrigin() { return m_origin; }

#if PROFILE_MOBILE
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
     * @brief Set auto fitting settings flag.
     */
    void ewkSettingsAutoFittingSet(bool value);

    /**
     * @brief Set load images settings flag.
     */
    void ewkSettingsLoadsImagesSet(bool value);

    /**
     * @brief Set javascript enabled settings flag.
     */
    void ewkSettingsJavascriptEnabledSet(bool value);

    /**
     * @brief Set form candidate data enabled settings flag.
     */
    void ewkSettingsFormCandidateDataEnabledSet(bool value);

    /**
     * @brief Set autofill password form enabled settings flag.
     */
    void ewkSettingsAutofillPasswordFormEnabledSet(bool value);

    /**
     * @brief Check if fullscreen mode is enabled.
     */
    bool isFullScreen() const { return m_fullscreen; };

    /**
     * @brief Clear text slection return true if some selection was cleared.
     */
    bool clearTextSelection() const;

    /**
     * @brief Exit full screen mode, return true if successful.
     */
    bool exitFullScreen() const;

    /**
     * @brief Set autofill profile data enabled settings flag.
     */
    void ewkSettingsFormProfileDataEnabledSet(bool value);
#endif

// signals
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::tools::BrowserImage>)> favIconChanged;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::tools::BrowserImage>, tizen_browser::tools::SnapshotType snapshot_type)> snapshotCaptured;
    boost::signals2::signal<void (const std::string&)> titleChanged;
    boost::signals2::signal<void (const std::string)> uriChanged;
    boost::signals2::signal<void (const std::string&)> findOnPage;

    boost::signals2::signal<void ()> loadFinished;
    boost::signals2::signal<void ()> loadStarted;
    boost::signals2::signal<void ()> loadStop;
    boost::signals2::signal<void ()> loadError;
    boost::signals2::signal<void (double)> loadProgress;

    boost::signals2::signal<void (bool)> forwardEnableChanged;
    boost::signals2::signal<void (bool)> backwardEnableChanged;

    boost::signals2::signal<void (WebConfirmationPtr)> confirmationRequest;

    boost::signals2::signal<void (bool)> IMEStateChanged;

    boost::signals2::signal<void ()> switchToWebPage;
    boost::signals2::signal<void (const std::string&, const std::string&)> setCertificatePem;
    boost::signals2::signal<void (const std::string&, const std::string&)> setWrongCertificatePem;

    boost::signals2::signal<void (const std::string&, const std::string&)> redirectedWebPage;
    boost::signals2::signal<void()> unsecureConnection;
    boost::signals2::signal<void(bool)> fullscreenModeSet;

protected:
    std::string getRedirectedURL() {return m_redirectedURL;};
    void setRedirectedURL(std::string url){m_redirectedURL = url;};

private:
    void registerCallbacks();
    void unregisterCallbacks();
    void setupEwkSettings();

    static void __newWindowRequest(void * data, Evas_Object *, void *out);
    static void __closeWindowRequest(void * data, Evas_Object *, void *);

#if  PROFILE_MOBILE
    context_menu_type _get_menu_type(Ewk_Context_Menu *menu);
    void _customize_context_menu(Ewk_Context_Menu *menu);
    void _show_context_menu_text_link(Ewk_Context_Menu *menu);
    void _show_context_menu_email_address(Ewk_Context_Menu *menu);
    void _show_context_menu_call_number(Ewk_Context_Menu *menu);
    void _show_context_menu_image_only(Ewk_Context_Menu *menu);
    void _show_context_menu_image_link(Ewk_Context_Menu *menu);
    void _show_context_menu_text_image_link(Ewk_Context_Menu *menu);
    void _show_context_menu_text_only(Ewk_Context_Menu *menu);

    static void __contextmenu_customize_cb(void *data, Evas_Object *obj, void *event_info);
    static void __contextmenu_selected_cb(void *data, Evas_Object *obj, void *event_info);
    static void __fullscreen_enter_cb(void *data, Evas_Object *obj, void *event_info);
    static void __fullscreen_exit_cb(void *data, Evas_Object *obj, void *event_info);

    Eina_Bool handle_scheme(const char *uri);
    Eina_Bool launch_email(const char *uri);
    Eina_Bool launch_contact(const char *uri, const char *protocol);
    Eina_Bool launch_dialer(const char *uri);
    Eina_Bool launch_message(const char *uri);
    Eina_Bool launch_tizenstore(const char *uri);
#endif

    // Load
    static void __loadStarted(void * data, Evas_Object * obj, void * event_info);
    static void __loadStop(void * data, Evas_Object * obj, void * event_info);
    static void __loadFinished(void * data, Evas_Object * obj, void * event_info);
    static void __loadProgress(void * data, Evas_Object * obj, void * event_info);
    static void __loadError(void* data, Evas_Object* obj, void *ewkError);

    static void __titleChanged(void * data, Evas_Object * obj, void * event_info);
    static void __urlChanged(void * data, Evas_Object * obj, void * event_info);

    static void __backForwardListChanged(void * data, Evas_Object * obj, void * event_info);

    // Favicon - from database
    static void __faviconChanged(void* data, Evas_Object*, void*);

    static void __IMEClosed(void * data, Evas_Object *obj, void *event_info);
    static void __IMEOpened(void * data, Evas_Object *obj, void *event_info);

    // redirect
    static void __load_provisional_started(void* data, Evas_Object*, void*);
    static void __load_provisional_redirect(void* data, Evas_Object*, void*);

    // confirmation requests
    static void __requestCertificationConfirm(void * data, Evas_Object * obj, void * event_info);
    static void __setCertificatePem(void * data, Evas_Object * obj, void * event_info);

    static void scriptLinkSearchCallback(Evas_Object *o, const char *value, void *data);

#if PROFILE_MOBILE
    // downloads
    static void __policy_response_decide_cb(void *data, Evas_Object *obj, void *event_info);
    static void __policy_navigation_decide_cb(void *data, Evas_Object *obj, void *event_info);
#endif

    // Screenshot capture
    static void __screenshotCaptured(Evas_Object* image, void* user_data);
private:
    Evas_Object * m_parent;
    TabId m_tabId;
    Evas_Object * m_ewkView;
    // ewk context of this web view
    Ewk_Context * m_ewkContext;
    std::string m_title;
    std::string m_redirectedURL;
    std::string m_loadingURL;
    std::shared_ptr<tizen_browser::tools::BrowserImage> m_faviconImage;
    bool m_isLoading;
    double m_loadProgress;
    bool m_loadError;
    // true if desktop view is enabled, false if mobile
    bool m_desktopMode;
    bool m_suspended;
    bool m_private;
    bool m_fullscreen;
    TabOrigin m_origin;

    std::map<CertificateConfirmationPtr, Ewk_Certificate_Policy_Decision *> m_confirmationCertificatenMap;

    static const std::string COOKIES_PATH;

#if PROFILE_MOBILE
    int m_status_code;
    Eina_Bool m_is_error_page;
    DownloadControl *m_downloadControl;
#endif
};

} /* namespace webengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */
#endif /* WEBVIEW_H_ */
