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

/*
 * WebView.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: p.rafalski
 */

#include "WebView.h"

#include <EWebKit.h>
#include <EWebKit_internal.h>

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <Elementary.h>
#include <Evas.h>

#include "URIschemes.h"
#include "app_i18n.h"
#include "AbstractWebEngine/AbstractWebEngine.h"
#include "app_common.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "EflTools.h"
#include "GeneralTools.h"
#include "Tools/WorkQueue.h"
#include "ServiceManager.h"
#include <shortcut_manager.h>
#include <string>

#include <device/haptic.h>
#include <Ecore.h>

#if PWA
#include <sys/stat.h>
#include <fcntl.h>
#include <tzplatform_config.h>
#endif

#define certificate_crt_path CERTS_DIR
#define APPLICATION_NAME_FOR_USER_AGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.69 safari/537.36 "

//TODO: temporary user agent for mobile display, change to proper one
#define APPLICATION_NAME_FOR_USER_AGENT_MOBILE "Mozilla/5.0 (Linux; Tizen 3.0; SAMSUNG TM1) AppleWebKit/537.36 (KHTML, like Gecko) SamsungBrowser/1.0 Chrome/47.0.2526.69 Mobile safari/537.36"

Ecore_Timer* m_haptic_timer_id =NULL;
haptic_device_h m_haptic_handle;
haptic_effect_h m_haptic_effect;

#define FIND_WORD_MAX_COUNT 1000

using namespace tizen_browser::tools;

namespace tizen_browser {
namespace basic_webengine {
namespace webengine_service {

const std::string WebView::COOKIES_PATH = "cookies";
#if PWA
std::string WebView::s_pwaData = "";
std::string WebView::s_name = "";
std::string WebView::s_start_url = "";
std::string WebView::s_icon = "";
#define DOWNLOAD_PATH tzplatform_getenv(TZ_USER_DOWNLOADS)
#endif

struct SnapshotItemData {
    WebView * web_view;
    tizen_browser::tools::SnapshotType snapshot_type;
};

WebView::WebView(Evas_Object * obj, TabId tabId, const std::string& title, bool incognitoMode)
    : m_parent(obj)
    , m_tabId(tabId)
    , m_ewkView(nullptr)
    , m_ewkContext(nullptr)
    , m_title(title)
    , m_redirectedURL("")
    , m_isLoading(false)
    , m_loadError(false)
    , m_suspended(false)
    , m_private(incognitoMode)
    , m_fullscreen(false)
    , m_downloadControl(nullptr)
{
}

WebView::~WebView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_ewkView) {
        unregisterCallbacks();
        evas_object_del(m_ewkView);
    }
    delete m_downloadControl;
}

void WebView::init(bool desktopMode, TabOrigin origin)
{
    m_ewkView = m_private ? ewk_view_add_in_incognito_mode(evas_object_evas_get(m_parent)) :
                            ewk_view_add_with_context(evas_object_evas_get(m_parent), ewk_context_default_get());

    m_ewkContext = ewk_view_context_get(m_ewkView);
    if (m_ewkContext)
        ewk_cookie_manager_accept_policy_set(ewk_context_cookie_manager_get(m_ewkContext), EWK_COOKIE_ACCEPT_POLICY_ALWAYS);

    evas_object_data_set(m_ewkView, "_container", this);
    BROWSER_LOGD("[%s:%d] self=%p", __PRETTY_FUNCTION__, __LINE__, this);

    evas_object_color_set(m_ewkView, 255, 255, 255, 255);
    evas_object_size_hint_weight_set(m_ewkView, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_ewkView, EVAS_HINT_FILL, EVAS_HINT_FILL);
    if (desktopMode) {
        switchToDesktopMode();
    } else {
        switchToMobileMode();
    }
    m_origin = origin;

    ewk_context_cache_model_set(m_ewkContext, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);
    std::string path = app_get_data_path() + COOKIES_PATH;
    ewk_cookie_manager_persistent_storage_set(ewk_context_cookie_manager_get(m_ewkContext),  path.c_str(), EWK_COOKIE_PERSISTENT_STORAGE_SQLITE);

    setupEwkSettings();
    registerCallbacks();
    m_downloadControl = new DownloadControl();
    orientationChanged();
    resume();
}

void WebView::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<int> signal = getRotation();
    if (signal && *signal != -1) {
        int angle = *signal;
        if ((angle % 180)== 90)
            angle -= 180;
        ewk_view_orientation_send(m_ewkView, angle);
    }
}

void cancel_vibration()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_haptic_timer_id) {
        ecore_timer_del(m_haptic_timer_id);
        m_haptic_timer_id = NULL;
    }

    if (m_haptic_handle) {
        device_haptic_stop(m_haptic_handle, m_haptic_effect);
        device_haptic_close(m_haptic_handle);
        m_haptic_handle = NULL;
    }
}

Eina_Bool __vibration_timeout_cb(void * /*data*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_haptic_timer_id = NULL;
    cancel_vibration();

    return ECORE_CALLBACK_CANCEL;
}

void __vibration_cb(uint64_t vibration_time, void * /*data*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    cancel_vibration();

    if (device_haptic_open(0, &m_haptic_handle) != DEVICE_ERROR_NONE) {
        return;
    }

    const uint64_t duration = vibration_time;
    device_haptic_vibrate(m_haptic_handle, duration, 100, &m_haptic_effect);
    double in = (double)((double)(duration) / (double)(1000));
    m_haptic_timer_id = ecore_timer_add(in, __vibration_timeout_cb, NULL);
}

void __vibration_cancel_cb(void * /*data*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    cancel_vibration();
}

void WebView::registerCallbacks()
{
    evas_object_smart_callback_add(m_ewkView, "load,started", __loadStarted, this);
    evas_object_smart_callback_add(m_ewkView, "load,finished", __loadFinished, this);
    evas_object_smart_callback_add(m_ewkView, "load,progress", __loadProgress, this);
    evas_object_smart_callback_add(m_ewkView, "load,error", __loadError, this);

    evas_object_smart_callback_add(m_ewkView, "title,changed", __titleChanged, this);
    evas_object_smart_callback_add(m_ewkView, "url,changed", __urlChanged, this);
    evas_object_smart_callback_add(m_ewkView, "back,forward,list,changed", __backForwardListChanged, this);

    evas_object_smart_callback_add(m_ewkView, "create,window", __newWindowRequest, this);
    evas_object_smart_callback_add(m_ewkView, "close,window", __closeWindowRequest, this);

    evas_object_smart_callback_add(m_ewkView, "policy,response,decide", __policy_response_decide_cb, this);
    evas_object_smart_callback_add(m_ewkView, "policy,navigation,decide", __policy_navigation_decide_cb, this);

    evas_object_smart_callback_add(m_ewkView, "request,certificate,confirm", __requestCertificationConfirm, this);
    evas_object_smart_callback_add(m_ewkView, "ssl,certificate,changed", __setCertificatePem, this);

    evas_object_smart_callback_add(m_ewkView, "icon,received", __faviconChanged, this);

    evas_object_smart_callback_add(m_ewkView, "editorclient,ime,closed", __IMEClosed, this);
    evas_object_smart_callback_add(m_ewkView, "editorclient,ime,opened", __IMEOpened, this);

    evas_object_smart_callback_add(m_ewkView, "load,provisional,started", __load_provisional_started, this);
    evas_object_smart_callback_add(m_ewkView, "load,provisional,redirect", __load_provisional_redirect, this);

    evas_object_smart_callback_add(m_ewkView, "contextmenu,customize", __contextmenu_customize_cb, this);
    evas_object_smart_callback_add(m_ewkView, "contextmenu,selected", __contextmenu_selected_cb, this);
    evas_object_smart_callback_add(m_ewkView, "fullscreen,enterfullscreen", __fullscreen_enter_cb, this);
    evas_object_smart_callback_add(m_ewkView, "fullscreen,exitfullscreen", __fullscreen_exit_cb, this);
    ewk_context_vibration_client_callbacks_set(m_ewkContext, __vibration_cb, __vibration_cancel_cb, this);

    evas_object_smart_callback_add(m_ewkView, "rotate,prepared", __rotate_prepared_cb, this);
}

void WebView::unregisterCallbacks()
{
    evas_object_smart_callback_del_full(m_ewkView, "load,started", __loadStarted, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,finished", __loadFinished, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,progress", __loadProgress, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,error", __loadError, this);

    evas_object_smart_callback_del_full(m_ewkView, "title,changed", __titleChanged, this);
    evas_object_smart_callback_del_full(m_ewkView, "url,changed", __urlChanged, this);
    evas_object_smart_callback_del_full(m_ewkView, "back,forward,list,changed", __backForwardListChanged, this);

    evas_object_smart_callback_del_full(m_ewkView, "create,window", __newWindowRequest, this);
    evas_object_smart_callback_del_full(m_ewkView, "close,window", __closeWindowRequest, this);

    evas_object_smart_callback_del_full(m_ewkView, "policy,response,decide", __policy_response_decide_cb, this);
    evas_object_smart_callback_del_full(m_ewkView, "policy,navigation,decide", __policy_navigation_decide_cb, this);

    evas_object_smart_callback_del_full(m_ewkView, "request,certificate,confirm", __requestCertificationConfirm, this);

    evas_object_smart_callback_del_full(m_ewkView, "icon,received", __faviconChanged, this);

    evas_object_smart_callback_del_full(m_ewkView, "editorclient,ime,closed", __IMEClosed, this);
    evas_object_smart_callback_del_full(m_ewkView, "editorclient,ime,opened", __IMEOpened, this);

    evas_object_smart_callback_del_full(m_ewkView, "load,provisional,started", __load_provisional_started, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,provisional,redirect", __load_provisional_redirect, this);

    evas_object_smart_callback_del_full(m_ewkView, "contextmenu,customize", __contextmenu_customize_cb,this);
    evas_object_smart_callback_del_full(m_ewkView, "contextmenu,selected", __contextmenu_selected_cb, this);
    evas_object_smart_callback_del_full(m_ewkView, "fullscreen,enterfullscreen", __fullscreen_enter_cb, this);
    evas_object_smart_callback_del_full(m_ewkView, "fullscreen,exitfullscreen", __fullscreen_exit_cb, this);
    ewk_context_vibration_client_callbacks_set(m_ewkContext, NULL, NULL, this);

    evas_object_smart_callback_del_full(m_ewkView, "rotate,prepared", __rotate_prepared_cb, this);
}

void WebView::setupEwkSettings()
{
    Ewk_Settings * settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_uses_keypad_without_user_action_set(settings, EINA_FALSE);
}

std::map<std::string, std::vector<std::string> > WebView::parse_uri(const char *uriToParse)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string uri = uriToParse;
    std::map<std::string, std::vector<std::string> > uri_parts;
    std::string::size_type mainDelimiter;
    mainDelimiter = uri.find_first_of("?");
    if (mainDelimiter != std::string::npos) {
        uri_parts["url"].push_back(uri.substr(0, mainDelimiter));
        std::string argsString = uri.substr(mainDelimiter+1, std::string::npos);
        const char *delimiter = "&";
        std::vector<std::string> argsVector;
        auto i = 0;
        auto pos = argsString.find(delimiter);
        if (pos != std::string::npos) {
            while (pos != std::string::npos) {
                argsVector.push_back(argsString.substr(i, pos-i));
                i = ++pos;
                pos = argsString.find(delimiter, pos);
                if (pos == std::string::npos)
                    argsVector.push_back(argsString.substr(i, argsString.length()));
            }
        } else
            argsVector.push_back(argsString.substr(i, argsString.length()));

        const char *valueDelimiter = "=";
        const char *itemsDelimiter = ";";
        for (auto item : argsVector) {
            pos = item.find(valueDelimiter);
            if (pos != std::string::npos) {
                std::string key = item.substr(0, pos);
                std::string value = item.substr(pos+1, std::string::npos);

                auto pos2 = value.find(itemsDelimiter);
                auto j = 0;
                if (pos2 != std::string::npos) {
                    while (pos2 != std::string::npos) {
                        uri_parts[key].push_back(value.substr(j, pos2-j));
                        j = ++pos2;
                        pos2 = value.find(itemsDelimiter, pos2);
                        if (pos2 == std::string::npos)
                            uri_parts[key].push_back(value.substr(j, value.length()));
                    }
                } else
                    uri_parts[key].push_back(value);
            }
        }
    } else {
        uri_parts["url"].push_back(uri.substr(0, mainDelimiter));
    }
    return uri_parts;
}

Eina_Bool WebView::handle_scheme(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!strncmp(uri, HTTP_SCHEME, strlen(HTTP_SCHEME)))
        return EINA_FALSE;
    else if (!strncmp(uri, HTTPS_SCHEME, strlen(HTTPS_SCHEME)))
        return EINA_FALSE;
    else if (!strncmp(uri, FILE_SCHEME, strlen(FILE_SCHEME)))
        return EINA_FALSE;
    else if (!strncmp(uri, TIZENSTORE_SCHEME, strlen(TIZENSTORE_SCHEME))) {
        launch_tizenstore(uri);
        return EINA_TRUE;
    }
    else if (!strncmp(uri, MAILTO_SCHEME, strlen(MAILTO_SCHEME))) {
        launch_email(uri);
        return EINA_TRUE;
    }
    else if (!strncmp(uri, TEL_SCHEME, strlen(TEL_SCHEME))) {
        launch_dialer(uri);
        return EINA_TRUE;
    }
    else if (!strncmp(uri, TELTO_SCHEME, strlen(TELTO_SCHEME))) {
        std::string request_uri = std::string(TEL_SCHEME) + std::string(uri + strlen(TELTO_SCHEME));
        launch_dialer(request_uri.c_str());
        return EINA_TRUE;
    }
    else if (!strncmp(uri, CALLTO_SCHEME, strlen(CALLTO_SCHEME))) {
        std::string request_uri = std::string(TEL_SCHEME) + std::string(uri + strlen(CALLTO_SCHEME));
        launch_dialer(request_uri.c_str());
        return EINA_TRUE;
    }
    else if (!strncmp(uri, SMS_SCHEME, strlen(SMS_SCHEME))) {
        launch_message(uri);
        return EINA_TRUE;
    }
    else if (!strncmp(uri, SMSTO_SCHEME, strlen(SMSTO_SCHEME))) {
        std::string request_uri = std::string(SMS_SCHEME) + std::string(uri + strlen(SMSTO_SCHEME));
        launch_message(request_uri.c_str());
        return EINA_TRUE;
    }

    return EINA_FALSE;
}

Eina_Bool WebView::launch_email(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::map<std::string, std::vector<std::string> >uri_parts = this->parse_uri(uri);

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGE("[%s:%d] Fail to app_control_create", __PRETTY_FUNCTION__, __LINE__);
        return EINA_FALSE;
    }
    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_COMPOSE) < 0) {
        BROWSER_LOGE("Fail to app_control_set_operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    auto it = uri_parts.find("url");
    if (it != uri_parts.end()) {
        if (app_control_set_uri(app_control, it->second.front().c_str()) < 0) {
            BROWSER_LOGE("Fail to app_control_set_uri");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    } else
        return EINA_FALSE;

    it = uri_parts.find("subject");
    if (it != uri_parts.end()) {
        if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_SUBJECT, it->second.front().c_str()) < 0) {
            BROWSER_LOGE("Fail to app_control_add_extra_data");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    it = uri_parts.find("body");
    if (it != uri_parts.end()) {
        if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TEXT, it->second.front().c_str()) < 0) {
            BROWSER_LOGE("Fail to app_control_add_extra_data");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    it = uri_parts.find("cc");
    if (it != uri_parts.end()) {
        size_t size = it->second.size();
        const char* cc[size];
        int i = 0;
        for(auto item : it->second) {
            cc[i] = item.c_str();
            ++i;
        }
        if (app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_CC, cc, size) < 0) {
            BROWSER_LOGE("Fail to app_control_add_extra_data_array");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    it = uri_parts.find("bcc");
    if (it != uri_parts.end()) {
        size_t size = it->second.size();
        const char* bcc[size];
        int i = 0;
        for(auto item : it->second) {
            bcc[i] = item.c_str();
            ++i;
        }
        if (app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_BCC, bcc, size) < 0) {
            BROWSER_LOGE("Fail to app_control_add_extra_data_array");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGE("Fail to app_control_send_launch_request");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    app_control_destroy(app_control);

    return EINA_TRUE;
}

Eina_Bool WebView::launch_contact(const char *uri, const char *protocol)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::map<std::string, std::vector<std::string> >uri_parts = this->parse_uri(uri);

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGE("Fail to create app_control handle");
        return EINA_FALSE;
    }
    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_ADD) < 0) {
        BROWSER_LOGE("Fail to app_control_set_operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    if (app_control_set_mime(app_control, "application/vnd.tizen.contact") < 0) {
        BROWSER_LOGE("Fail to app_control_set_mime");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    auto it = uri_parts.find("url");
    if (it != uri_parts.end()) {
        if (!strcmp(protocol, "tel")) {
            if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_PHONE,
                                           it->second.front().c_str()) < 0) {
                BROWSER_LOGE("Fail to app_control_add_extra_data");
                app_control_destroy(app_control);
                return EINA_FALSE;
            }
        } else if (strcmp(protocol, "mailto") == 0) {
            if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_EMAIL,
                                           it->second.front().c_str()) < 0) {
                BROWSER_LOGE("Fail to app_control_add_extra_data");
                app_control_destroy(app_control);
                return EINA_FALSE;
            }
        } else
            BROWSER_LOGE("Not supported protocol!");
    }
    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGE("Fail to launch app_control operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    app_control_destroy(app_control);

    return EINA_TRUE;
}

Eina_Bool WebView::launch_dialer(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGE("Fail to create app_control handle");
        return EINA_FALSE;
    }
    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_DIAL) < 0) {
        BROWSER_LOGE("Fail to app_control_set_operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    if (app_control_set_uri(app_control, uri) < 0) {
        BROWSER_LOGE("app_control_set_uri is failed.");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGE("Fail to launch app_control operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    app_control_destroy(app_control);

    return EINA_TRUE;
}

Eina_Bool WebView::launch_message(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::map<std::string, std::vector<std::string> >uri_parts = this->parse_uri(uri);

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGE("Fail to create app_control handle");
        return EINA_FALSE;
    }
    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_COMPOSE) < 0) {
        BROWSER_LOGE("Fail to set app_control operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    auto it = uri_parts.find("url");
    if (it != uri_parts.end()) {
        if (app_control_set_uri(app_control, it->second.front().c_str()) < 0) {
            BROWSER_LOGE("Fail to app_control_set_uri");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    else
        return EINA_FALSE;

    it = uri_parts.find("subject");
    if (it != uri_parts.end()) {
        if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_SUBJECT, it->second.front().c_str()) < 0) {
            BROWSER_LOGE("Fail to app_control_add_extra_data");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    it = uri_parts.find("body");
    if (it != uri_parts.end()) {
        if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TEXT, it->second.front().c_str()) < 0) {
            BROWSER_LOGE("Fail to app_control_add_extra_data");
            app_control_destroy(app_control);
            return EINA_FALSE;
        }
    }
    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGE("Fail to launch app_control operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    app_control_destroy(app_control);

    return EINA_TRUE;
}

Eina_Bool WebView::launch_tizenstore(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    app_control_h app_control = NULL;
    if (app_control_create(&app_control) < 0) {
        BROWSER_LOGE("Fail to create app_control handle");
        return EINA_FALSE;
    }
    if (!app_control) {
        BROWSER_LOGE("Fail to create app_control handle");
        return EINA_FALSE;
    }
    if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
        BROWSER_LOGE("Fail to set app_control operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    if (app_control_set_uri(app_control, uri) < 0) {
        BROWSER_LOGE("Fail to set uri operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    if (app_control_set_app_id(app_control, TIZENSTORE_APP_ID) < 0) {
        BROWSER_LOGE("Fail to app_control_set_app_id");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
        BROWSER_LOGE("Fail to launch app_control operation");
        app_control_destroy(app_control);
        return EINA_FALSE;
    }
    app_control_destroy(app_control);

    return EINA_TRUE;
}

Evas_Object * WebView::getLayout()
{
    return m_ewkView;
}

#if !DUMMY_BUTTON
Evas_Object * WebView::getWidget()
{
    return ewk_view_widget_get(m_ewkView);
}
#endif

void WebView::setURI(const std::string & uri)
{
    BROWSER_LOGD("[%s:%d] uri=%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    m_faviconImage.reset();
    ewk_view_url_set(m_ewkView, uri.c_str());
    m_loadError = false;
}

std::string WebView::getURI(void)
{
    BROWSER_LOGD("[%s:%d] uri=%s", __PRETTY_FUNCTION__, __LINE__, ewk_view_url_get(m_ewkView));
    return fromChar(ewk_view_url_get(m_ewkView));
}

#if PWA
void WebView::requestManifest(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ewk_view_request_manifest(m_ewkView, dataSetManifest, this);
}

void WebView::dataSetManifest(Evas_Object* view, Ewk_View_Request_Manifest* manifest, void* data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (view && data) {
        WebView * self = reinterpret_cast<WebView *>(data);

        uint8_t* theme_r = nullptr; uint8_t* theme_g = nullptr;
        uint8_t* theme_b = nullptr; uint8_t* theme_a = nullptr;
        uint8_t* bg_r = nullptr; uint8_t* bg_g = nullptr;
        uint8_t* bg_b = nullptr; uint8_t* bg_a = nullptr;

        const char* short_name(ewk_manifest_short_name_get(manifest));
        const char* name(ewk_manifest_name_get(manifest));
        const char* start_url(ewk_manifest_start_url_get(manifest));
        const char* icon_src(ewk_manifest_icons_src_get(manifest, 0));
        int orientation_type = ewk_manifest_orientation_type_get(manifest);
        int display_mode = ewk_manifest_web_display_mode_get(manifest);
        long theme_color = ewk_manifest_theme_color_get(manifest, theme_r, theme_g, theme_b, theme_a);
        long background_color = ewk_manifest_background_color_get(manifest, bg_r, bg_g, bg_b, bg_a);
        size_t icon_count = ewk_manifest_icons_count_get(manifest);

        std::string str_short_name = "";
        std::string str_icon_src = "";

        if (short_name)
            str_short_name = short_name;
        if (name)
            s_name = name;
        if (start_url)
            s_start_url = start_url;
        if (icon_src)
            str_icon_src = icon_src;

        std::string retVal("browser_shortcut:://");
        retVal.append("pwa_shortName:"); retVal.append(str_short_name.c_str()); retVal.append(",");
        retVal.append("pwa_name:"); retVal.append(s_name.c_str()); retVal.append(",");
        retVal.append("pwa_uri:"); retVal.append(s_start_url.c_str()); retVal.append(",");
        retVal.append("pwa_orientation:"); retVal.append(std::to_string(orientation_type)); retVal.append(",");
        retVal.append("pwa_displayMode:"); retVal.append(std::to_string(display_mode)); retVal.append(",");
        retVal.append("pwa_themeColor:"); retVal.append(std::to_string(theme_color)); retVal.append(",");
        retVal.append("theme_r:"); retVal.append(std::to_string((int)theme_r)); retVal.append(",");
        retVal.append("theme_g:"); retVal.append(std::to_string((int)theme_g)); retVal.append(",");
        retVal.append("theme_b:"); retVal.append(std::to_string((int)theme_b)); retVal.append(",");
        retVal.append("theme_a:"); retVal.append(std::to_string((int)theme_a)); retVal.append(",");
        retVal.append("pwa_backgroundColor:"); retVal.append(std::to_string(background_color)); retVal.append(",");
        retVal.append("bg_r:"); retVal.append(std::to_string((int)bg_r)); retVal.append(",");
        retVal.append("bg_g:"); retVal.append(std::to_string((int)bg_g)); retVal.append(",");
        retVal.append("bg_b:"); retVal.append(std::to_string((int)bg_b)); retVal.append(",");
        retVal.append("bg_a:"); retVal.append(std::to_string((int)bg_a)); retVal.append(",");
        retVal.append("icon_count:"); retVal.append(std::to_string(icon_count)); retVal.append(",");
        retVal.append("icon_src:"); retVal.append(str_icon_src.c_str()); retVal.append(",");

        BROWSER_LOGD("[%s:%d] retVal : %s", __PRETTY_FUNCTION__, __LINE__, retVal.c_str());
        s_pwaData = retVal;

        size_t len = strlen(icon_src);
        auto result = str_icon_src.substr(str_icon_src.find_last_of("/"), len);
        s_icon = DOWNLOAD_PATH + result;
        self->request_file_download(icon_src, s_icon, __download_result_cb, NULL);
        BROWSER_LOGD("[%s:%d] dataSetManifest callback function end!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void WebView::makeShortcut(const std::string& name, const std::string& pwaData, const std::string& icon)
{
    if (shortcut_add_to_home(name.c_str(),LAUNCH_BY_URI,pwaData.c_str(),
            icon.c_str(),0,result_cb,nullptr) != SHORTCUT_ERROR_NONE)
        BROWSER_LOGE("[%s:%d] Fail to add to homescreen", __PRETTY_FUNCTION__, __LINE__);
    else
        BROWSER_LOGE("[%s:%d] Success to add to homescreen", __PRETTY_FUNCTION__, __LINE__);
}

int WebView::result_cb(int ret, void *data) {

    if (data) {
        BROWSER_LOGD("[%s:%d] ret : %d, data : %s", __PRETTY_FUNCTION__, __LINE__, ret, data);
    } else {
        BROWSER_LOGW("[%s] result_cb_data = nullptr", __PRETTY_FUNCTION__);
    }
    return 0;
}

void WebView::request_file_download(const char *uri, const std::string& file_path, download_finish_callback cb, void *data)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("uri = [%s], file_path = [%s]", uri, file_path.c_str());

    SoupSession *soup_session = nullptr;
    SoupMessage *soup_msg = nullptr;

    soup_session = soup_session_async_new();
    g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, nullptr);

    soup_msg = soup_message_new("GET", uri);
    download_request *request = new(std::nothrow) download_request(strdup(file_path.c_str()), cb, data);
    soup_session_queue_message(soup_session, soup_msg, __file_download_finished_cb, static_cast<void*>(request));

    g_object_unref(soup_session);
}

void WebView::__file_download_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
    BROWSER_LOGD("[%s:%d] session : %s", __PRETTY_FUNCTION__, __LINE__, session);

    if (data) {
        download_request *request = static_cast<download_request*>(data);
        SoupBuffer *body = soup_message_body_flatten(msg->response_body);
        int fd = 0;
        if (body->data && (body->length > 0) && ((fd = open(request->file_path.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) >= 0)) {
            unsigned int write_len = write(fd, body->data, body->length);
            close(fd);
            if (write_len == body->length)
                request->cb(request->file_path.c_str(), request->data);
            else
                unlink(request->file_path.c_str());
        }
        g_object_unref(msg);
        soup_buffer_free(body);
        delete request;
    }
    makeShortcut(s_name, s_pwaData, s_icon);
}

void WebView::__download_result_cb(const std::string& file_path, void *data)
{
    BROWSER_LOGD("[%s:%d] file_path = [%s], data = [%s]", file_path.c_str(), data);
    BROWSER_LOGD("[%s:%d] complete !", __PRETTY_FUNCTION__, __LINE__);
}
#endif

std::string WebView::getTitle(void)
{
    return m_title;
}

std::string WebView::getUserAgent()
{
    return fromChar(ewk_view_user_agent_get(m_ewkView));
}

void WebView::setUserAgent(const std::string& ua)
{
    ewk_view_user_agent_set(m_ewkView, ua.c_str());
}

void WebView::suspend()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_ewkView);

    ewk_view_suspend(m_ewkView);
    m_suspended = true;
}

void WebView::resume()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_ewkView);

    ewk_view_resume(m_ewkView);
    m_suspended = false;
}

void WebView::stopLoading(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_isLoading = false;
    ewk_view_stop(m_ewkView);
    loadStop();
}

void WebView::reload(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_isLoading = true;
    if(m_loadError)
    {
        m_loadError = false;
        ewk_view_url_set(m_ewkView, ewk_view_url_get(m_ewkView));
    }
    else
        ewk_view_reload(m_ewkView);
}

void WebView::back(void)
{
    m_loadError = false;
    ewk_view_back(m_ewkView);
}

void WebView::forward(void)
{
    m_loadError = false;
    ewk_view_forward(m_ewkView);
}

bool WebView::isBackEnabled(void)
{
    return ewk_view_back_possible(m_ewkView);
}

bool WebView::isForwardEnabled(void)
{
    return ewk_view_forward_possible(m_ewkView);
}

bool WebView::isLoading()
{
    return m_isLoading;
}

bool WebView::isLoadError() const
{
    return m_loadError;
}

void WebView::confirmationResult(WebConfirmationPtr confirmation)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (confirmation->getConfirmationType() == WebConfirmation::ConfirmationType::CertificateConfirmation) {
        //FIXME: https://bugs.tizen.org/jira/browse/TT-229
        CertificateConfirmationPtr cert = std::dynamic_pointer_cast<CertificateConfirmation, WebConfirmation>(confirmation);

        // The below line doesn't serve any purpose now, but it may become
        // relevant when implementing https://bugs.tizen.org/jira/browse/TT-229
        Ewk_Certificate_Policy_Decision *request = m_confirmationCertificatenMap[cert];
        Eina_Bool result;

        if (cert->getResult() == WebConfirmation::ConfirmationResult::Confirmed)
            result = EINA_TRUE;
        else if (cert->getResult() == WebConfirmation::ConfirmationResult::Rejected)
            result = EINA_FALSE;
        else {
            BROWSER_LOGE("Wrong ConfirmationResult");
            return;
        }

        // set certificate confirmation
        ewk_certificate_policy_decision_allowed_set(request, result);
        resume();

        // remove from map
        m_confirmationCertificatenMap.erase(cert);
    } else {
        BROWSER_LOGW("[%s:%d] Unknown WebConfirmation::ConfirmationType!", __PRETTY_FUNCTION__, __LINE__);
    }
}

tools::BrowserImagePtr WebView::captureSnapshot(int targetWidth, int targetHeight, bool async,
        tizen_browser::tools::SnapshotType snapshot_type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_ewkView);
    M_ASSERT(targetWidth);
    M_ASSERT(targetHeight);
    Evas_Coord vw, vh;
    evas_object_geometry_get(m_ewkView, nullptr, nullptr, &vw, &vh);
    if (vw == 0 || vh == 0)
        return std::make_shared<BrowserImage>();

    double scale = targetWidth / (double)(vw * getZoomFactor());
    double scale_max, scale_min;
    ewk_view_scale_range_get(m_ewkView, &scale_min, &scale_max);
    if (scale < scale_min)
        scale = scale_min;
    else if (scale > scale_max)
        scale = scale_max;

    Eina_Rectangle area;
    double snapshotProportions = (double)(targetWidth) /(double)(targetHeight);
    double webkitProportions = (double)(vw) /(double)(vh);
    if (webkitProportions >= snapshotProportions) {
        // centring position of screenshot
        area.x = (vw*getZoomFactor()/2) - (vh*getZoomFactor()*snapshotProportions/2);
        area.y = 0;
        area.w = vh*getZoomFactor()*snapshotProportions;
        area.h = vh*getZoomFactor();
    }
    else {
        area.x = 0;
        area.y = 0;
        area.w = vw*getZoomFactor();
        area.h = vw*getZoomFactor()/snapshotProportions;
    }
    if (area.w == 0 || area.h == 0)
        return std::make_shared<BrowserImage>();

    BROWSER_LOGD("[%s:%d] Before snapshot (screenshot) - look at the time of taking snapshot below",__func__, __LINE__);

    if (async) {
        SnapshotItemData *snapshot_data = new SnapshotItemData();
        snapshot_data->web_view = this;
        snapshot_data->snapshot_type = snapshot_type;
        bool result = ewk_view_screenshot_contents_get_async(m_ewkView, area, scale, evas_object_evas_get(m_ewkView), __screenshotCaptured, snapshot_data);
        if (!result)
            BROWSER_LOGD("[%s:%d] ewk_view_screenshot_contents_get_async API failed", __func__, __LINE__);
    } else {
        Evas_Object *snapshot = ewk_view_screenshot_contents_get(m_ewkView, area, scale, evas_object_evas_get(m_ewkView));
        BROWSER_LOGD("[%s:%d] Snapshot (screenshot) catched, evas pointer: %p",__func__, __LINE__, snapshot);
        if (snapshot)
            return std::make_shared<tools::BrowserImage>(snapshot);
    }

    return std::make_shared<BrowserImage>();
}

void WebView::__screenshotCaptured(Evas_Object* image, void* data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    SnapshotItemData *snapshot_data = static_cast<SnapshotItemData*>(data);
    snapshot_data->web_view->snapshotCaptured(std::make_shared<tools::BrowserImage>(image), snapshot_data->snapshot_type);
}

void WebView::__newWindowRequest(void *data, Evas_Object *, void *out)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView * self = reinterpret_cast<WebView *>(data);
    BROWSER_LOGD("[%s:%d] self=%p", __PRETTY_FUNCTION__, __LINE__, self);
    BROWSER_LOGD("Window creating in tab: %s", self->getTabId().toString().c_str());
    std::shared_ptr<basic_webengine::AbstractWebEngine>  m_webEngine;
    m_webEngine = std::dynamic_pointer_cast
    <
        basic_webengine::AbstractWebEngine,tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));
    M_ASSERT(m_webEngine);

    /// \todo: Choose newly created tab.
    TabId id(TabId::NONE);
    TabId currentTabId = m_webEngine->currentTabId();
    if (currentTabId != (id = m_webEngine->addTab(std::string(),
                                                                 boost::none,
                                                                 std::string(),
                                                                 self->isDesktopMode(),
                                                                 currentTabId.get()))) {
        BROWSER_LOGD("Created tab: %s", id.toString().c_str());
        Evas_Object* tab_ewk_view = m_webEngine->getTabView(id);
        *static_cast<Evas_Object**>(out) = tab_ewk_view;

        // switch to a new tab
        m_webEngine->switchToTab(id);
        m_webEngine->windowCreated();
    }
}

void WebView::__closeWindowRequest(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebView * self = reinterpret_cast<WebView *>(data);
    std::shared_ptr<AbstractWebEngine> m_webEngine =
                std::dynamic_pointer_cast
                <basic_webengine::AbstractWebEngine,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));
    m_webEngine->closeTab(self->getTabId());
}

void WebView::__loadStarted(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    WebView * self = reinterpret_cast<WebView *>(data);

    BROWSER_LOGD("%s:%d\n\t %s", __func__, __LINE__, ewk_view_url_get(self->m_ewkView));

    self->m_isLoading = true;
    self->loadStarted();
}

void WebView::__loadFinished(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView * self = reinterpret_cast<WebView *>(data);

    self->m_isLoading = false;
    self->m_loadProgress = 1;

    self->loadFinished();
    self->loadProgress(self->m_loadProgress);

    self->captureSnapshot(boost::any_cast<int>(config::Config::getInstance().get(CONFIG_KEY::HISTORY_TAB_SERVICE_THUMB_WIDTH)),
            boost::any_cast<int>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::HISTORY_TAB_SERVICE_THUMB_HEIGHT)),
            true, tools::SnapshotType::ASYNC_LOAD_FINISHED);
}

void WebView::__loadProgress(void * data, Evas_Object * /* obj */, void * event_info)
{
    WebView * self = reinterpret_cast<WebView *>(data);
    if (!self->isLoading())
        return;

    self->m_loadProgress = *(double *)event_info;
    self->loadProgress(self->m_loadProgress);
}

void WebView::__loadError(void* data, Evas_Object * obj, void* ewkError)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView *self = reinterpret_cast<WebView*>(data);
    Ewk_Error *error = reinterpret_cast<Ewk_Error*>(ewkError);
    Ewk_Error_Type errorType = ewk_error_type_get(error);

    BROWSER_LOGD("[%s:%d] ewk_error_type: %d ",
                 __PRETTY_FUNCTION__, __LINE__, errorType);

    BROWSER_LOGD("[%s:%d] emiting signal ", __PRETTY_FUNCTION__, __LINE__);
    int errorCode = ewk_error_code_get(error);
    if(errorCode == EWK_ERROR_NETWORK_STATUS_CANCELLED)
    {
        BROWSER_LOGD("Stop signal emitted");
        BROWSER_LOGD("Error description: %s", ewk_error_description_get(error));
        evas_object_smart_callback_call(obj, "load,stop", nullptr);
    }
    else
    {
        self->loadError();
        self->m_loadError=true;
    }
}

void WebView::__titleChanged(void * data, Evas_Object * obj, void * /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto self = reinterpret_cast<WebView *>(data);
    self->m_title = fromChar(ewk_view_title_get(obj));
}

void WebView::__urlChanged(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView * self = reinterpret_cast<WebView *>(data);
    BROWSER_LOGD("URL changed for tab: %s", self->getTabId().toString().c_str());
    self->uriChanged(self->getURI());
}

void WebView::__backForwardListChanged(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView * self = reinterpret_cast<WebView *>(data);
    self->backwardEnableChanged(self->isBackEnabled());
    self->forwardEnableChanged(self->isForwardEnabled());
}

void WebView::__faviconChanged(void* data, Evas_Object*, void*)
{
    if(data)
    {
        WebView * self = static_cast<WebView *>(data);
        Evas_Object * favicon = ewk_context_icon_database_icon_object_add(self->m_ewkContext, ewk_view_url_get(self->m_ewkView),evas_object_evas_get(self->m_ewkView));
        if (favicon) {
            BROWSER_LOGD("[%s:%d] Favicon received", __PRETTY_FUNCTION__, __LINE__);
            self->m_faviconImage = std::make_shared<tools::BrowserImage>(favicon);
            evas_object_del(favicon);
            self->favIconChanged(self->m_faviconImage);
        }
    }
}

void WebView::__IMEClosed(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s", __func__);
    WebView * self = reinterpret_cast<WebView *>(data);
    self->IMEStateChanged(false);
}

void WebView::__IMEOpened(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s", __func__);
    WebView * self = reinterpret_cast<WebView *>(data);
    self->IMEStateChanged(true);
}

void WebView::__load_provisional_started(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebView * self = reinterpret_cast<WebView*>(data);
    self->setRedirectedURL(self->getURI());
}

void WebView::__load_provisional_redirect(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebView * self = reinterpret_cast<WebView*>(data);
    if (!self->getRedirectedURL().empty())
        self->redirectedWebPage(self->getRedirectedURL(), self->getURI());
    self->setRedirectedURL("");
}

void WebView::__requestCertificationConfirm(void * data , Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView * self = reinterpret_cast<WebView *>(data);
    Ewk_Certificate_Policy_Decision *request = reinterpret_cast<Ewk_Certificate_Policy_Decision *>(event_info);
    if (!request) {
        BROWSER_LOGW("[%s:%d] Wrong event_info!", __PRETTY_FUNCTION__, __LINE__);
        return;
    }

    int error = ewk_certificate_policy_decision_error_get(request);
    if (error == EWK_CERTIFICATE_POLICY_DECISION_ERROR_PINNED_KEY_NOT_IN_CHAIN) {
        ewk_certificate_policy_decision_allowed_set(request, EINA_FALSE);
        BROWSER_LOGW("[%s:%d] EWK_CERTIFICATE_POLICY_DECISION_ERROR_PINNED_KEY_NOT_IN_CHAIN", __PRETTY_FUNCTION__, __LINE__);
        self->unsecureConnection();
        return;
    }

    self->suspend();
    ewk_certificate_policy_decision_suspend(request);

    std::string url = tools::extractDomain(self->m_loadingURL);

    ///\todo add translations
    std::string message = (boost::format("There are problems with the security certificate for this site.<br>%1%") % url).str();

    CertificateConfirmationPtr c = std::make_shared<CertificateConfirmation>(self->m_tabId, url, message);
    const char *pem = ewk_certificate_policy_decision_certificate_pem_get(request);
    c->setPem(std::string(pem));
    c->setData(reinterpret_cast<void*>(request));

    // store
    self->m_confirmationCertificatenMap[c] = request;
    self->confirmationRequest(c);
}

void WebView::__setCertificatePem(void* data , Evas_Object* /* obj */, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = reinterpret_cast<WebView *>(data);
    auto certInfo = static_cast<const Ewk_Certificate_Info*>(event_info);
    if (!certInfo) {
        BROWSER_LOGW("[%s:%d] Wrong event_info!", __PRETTY_FUNCTION__, __LINE__);
        return;
    }

    std::string url = tools::extractDomain(self->getURI());
    const char* pem = ewk_certificate_info_pem_get(certInfo);
    if (pem) {
        Eina_Bool valid = ewk_certificate_info_is_context_secure(certInfo);
        if (valid)
            self->setCertificatePem(url, std::string(pem));
        else
            self->setWrongCertificatePem(url, std::string(pem));
    }
}

context_menu_type WebView::_get_menu_type(Ewk_Context_Menu *menu)
{
    int count = ewk_context_menu_item_count(menu);
    bool text = false;
    bool link = false;
    bool image = false;
    bool selection_mode = false;
    bool call_number = false;
    bool email_address = false;
    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, i);
        Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
        const char *link_url = ewk_context_menu_item_image_url_get(item);
        BROWSER_LOGD("tag=%d", tag);

        if (link_url && !strncmp(MAILTO_SCHEME, link_url, strlen(MAILTO_SCHEME)))
            email_address = true;
        if (link_url && !strncmp(TEL_SCHEME, link_url, strlen(TEL_SCHEME)))
            call_number = true;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE)
            selection_mode = true;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_CLIPBOARD)
            return INPUT_FIELD;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB)
            text = true;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW || tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD)
            link = true;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD)
            image = true;
    }

    if (email_address && selection_mode)
        return EMAIL_LINK;
    if (call_number && selection_mode)
        return TEL_LINK;
    if (text && !link)
        return TEXT_ONLY;
    if (link && !image)
        return TEXT_LINK;
    if (image && !link)
        return IMAGE_ONLY;
    if(selection_mode && image && link)
        return TEXT_IMAGE_LINK;
    if (image && link)
        return IMAGE_LINK;

    return UNKNOWN_MENU;
}

void WebView::_show_context_menu_text_link(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item * item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }

    /* Open in new window */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW,_("IDS_BR_OPT_OPEN_IN_NEW_WINDOW_ABB"), true);   //TODO: missing translation
    /* Save link */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, _("IDS_BR_BODY_SAVE_LINK"), true);
    /* Copy link address */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_LINK"), true);  //TODO: missing translation
    /*Text Selection Mode */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, _("IDS_BR_OPT_SELECT_TEXT"), true);
}

void WebView::_show_context_menu_email_address(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item * item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }

    /* Send email */
    ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_EMAIL, "Send email", true);  //TODO: missing translation
    /* Add to contact */
    ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT, "Add to contacts", true);  //TODO: missing translation
    /* Copy link address */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_LINK_URL"), true);  //TODO: missing translation
}

void WebView::_show_context_menu_call_number(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item * item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }

    /* Call */
    ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_CALL, "Call", true);  //TODO: missing translation
    /* Send message */
    ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_MESSAGE, "Send message", true);  //TODO: missing translation
    /* Add to contact */
    ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT, "Add to contacts", true);  //TODO: missing translation
    /* Copy link address */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_LINK_URL"), true);  //TODO: missing translation
}

void WebView::_show_context_menu_text_only(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    const char *selected_text = ewk_view_text_selection_text_get(m_ewkView);
    bool text_selected = false;
    if (selected_text && strlen(selected_text) > 0)
        text_selected = true;

    for (int  i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }

    /* Select all */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL, _("IDS_BR_OPT_SELECT_ALL"), true);
    /* Copy */
    if (text_selected == true) {
        ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY, _("IDS_BR_OPT_COPY"), true);
    }
    /* Share*/
    if (text_selected == true) {
        ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SHARE, _("IDS_BR_OPT_SHARE"), true);
    }
    /* Web Search*/
    if (text_selected == true) {
        ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB, _("IDS_BR_OPT_WEB_SEARCH"), true);
    }
    /* Find on page*/
    if (text_selected == true) {
        ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE, _("IDS_BR_OPT_FIND_ON_PAGE_ABB"), true);
    }
}

void WebView::_show_context_menu_image_only(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }
    /* Save image */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, _("IDS_BR_OPT_SAVE_IMAGE"), true);                 //TODO: missing translation
    /* Copy image */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_IMAGE"), true);
    /* View Image */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, _("IDS_BR_BODY_VIEW_IMAGE"), true);               //TODO: missing translation
}

void WebView::_show_context_menu_text_image_link(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    for (int  i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }

    /* Open in new window */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW,_("IDS_BR_OPT_OPEN_IN_NEW_WINDOW_ABB"), true);	//TODO: missing translation
    /* Save link */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, _("IDS_BR_OPT_SAVE_LINK"), true);
    /* Copy link address */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_LINK"), true);  //TODO: missing translation
    /*Text Selection Mode */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, _("IDS_BR_OPT_SELECT_TEXT"), true);

}

void WebView::_show_context_menu_image_link(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int count = ewk_context_menu_item_count(menu);

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        ewk_context_menu_item_remove(menu, item);
    }

    /* Open link in new window */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, _("IDS_BR_OPT_OPEN_IN_NEW_WINDOW_ABB"), true);               //TODO: missing translation
    /* Save link */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, _("IDS_BR_OPT_SAVE_LINK"), true);
    /* Copy link */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_LINK"), true);              //TODO: missing translation
    /* Save image */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, _("IDS_BR_OPT_SAVE_IMAGE"), true);                 //TODO: missing translation
   /* copy image */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, _("IDS_BR_OPT_COPY_IMAGE"), true);
    /* View image  */
    ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, _("IDS_BR_BODY_VIEW_IMAGE"), true);                                //TODO: missing translation
}

void WebView::_customize_context_menu(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    context_menu_type menu_type = _get_menu_type(menu);
    BROWSER_LOGD("menu_type=%d", menu_type);

    if (menu_type == UNKNOWN_MENU || menu_type == INPUT_FIELD)
        return;

    switch (menu_type) {
        case TEXT_ONLY:
            _show_context_menu_text_only(menu);
        break;

        case TEXT_LINK:
            _show_context_menu_text_link(menu);
        break;

        case EMAIL_LINK:
            _show_context_menu_email_address(menu);
        break;

        case TEL_LINK:
            _show_context_menu_call_number(menu);
        break;

        case IMAGE_ONLY:
            _show_context_menu_image_only(menu);
        break;

        case IMAGE_LINK:
            _show_context_menu_image_link(menu);
        break;

        case TEXT_IMAGE_LINK:
            _show_context_menu_text_image_link(menu);
        break;

        default:
            BROWSER_LOGD("[%s:%d] Warning: Unhandled button.", __PRETTY_FUNCTION__, __LINE__);
        break;
    }
}

void WebView::__contextmenu_customize_cb(void *data, Evas_Object * /* obj */, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Ewk_Context_Menu *menu = reinterpret_cast<Ewk_Context_Menu*>(event_info);
    WebView * self = reinterpret_cast<WebView *>(data);

    self->_customize_context_menu(menu);
}

void WebView::__contextmenu_selected_cb(void *data, Evas_Object */*obj*/, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView * self = reinterpret_cast<WebView *>(data);
    Ewk_Context_Menu_Item *item = reinterpret_cast<Ewk_Context_Menu_Item*>(event_info);
    Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

    const char *link_url = ewk_context_menu_item_link_url_get(item);
    const char *selected_text = ewk_view_text_selection_text_get(self->m_ewkView);

    if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEND_EMAIL) {
        self->handle_scheme(link_url);
    } else if (tag == CUSTOM_CONTEXT_MENU_ITEM_CALL) {
        self->handle_scheme(link_url);
    } else if (tag == CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE) {
        self->findOnPage(selected_text);
    } else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEND_MESSAGE) {
        if (link_url && !strncmp(TEL_SCHEME, link_url, strlen(TEL_SCHEME))) {
            std::string::size_type pos = std::string::npos;
            std::string source = std::string(link_url);
            while ((pos = source.find(TEL_SCHEME)) != std::string::npos)
                source.replace(pos, strlen(TEL_SCHEME), SMS_SCHEME);
            self->handle_scheme(source.c_str());
        }
    } else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT) {
        if (link_url && !strncmp(TEL_SCHEME, link_url, strlen(TEL_SCHEME))) {
            self->launch_contact(link_url + strlen(TEL_SCHEME), "tel");
        } else if (link_url && !strncmp(MAILTO_SCHEME, link_url, strlen(MAILTO_SCHEME))) {
            size_t source_end_pos = 0;
            std::string source = std::string(link_url);
            if (source.find("?") != std::string::npos) {
                source_end_pos = source.find("?");
                source = source.substr(0, source_end_pos);
            }
            self->launch_contact(source.c_str() + strlen(MAILTO_SCHEME), "mailto");
        }
    }
}

void WebView::__fullscreen_enter_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto self = static_cast<WebView*>(data);
    self->m_fullscreen = true;
    self->fullscreenModeSet(self->m_fullscreen);
}

void WebView::__fullscreen_exit_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto self = static_cast<WebView*>(data);
    self->m_fullscreen = false;
    self->fullscreenModeSet(self->m_fullscreen);
}

void WebView::__rotate_prepared_cb(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto *self = static_cast<WebView *>(data);
        self->rotatePrepared();
    } else {
            BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void WebView::setFocus()
{
    ewk_view_focus_set(m_ewkView, EINA_TRUE);
}

void WebView::clearFocus()
{
    ewk_view_focus_set(m_ewkView, EINA_FALSE);
}

bool WebView::hasFocus() const
{
    return ewk_view_focus_get(m_ewkView) == EINA_TRUE ? true : false;
}

double WebView::getZoomFactor() const
{
    if(EINA_UNLIKELY(m_ewkView == nullptr)) {
        return 1.0;
    }

    return ewk_view_page_zoom_get(m_ewkView);
}

void WebView::setZoomFactor(double zoomFactor)
{
    if(m_ewkView) {
        //using zoomFactor = 0 sets zoom "fit to screen"

        if(zoomFactor != getZoomFactor())
            ewk_view_page_zoom_set(m_ewkView, zoomFactor);
    }
}

void WebView::scrollView(const int& dx, const int& dy)
{
    ewk_view_scroll_by(m_ewkView, dx, dy);
}

void WebView::findWord(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!word || strlen(word) == 0) {
        ewk_view_text_find_highlight_clear(m_ewkView);
        return;
    }

    evas_object_smart_callback_del(m_ewkView, "text,found", found_cb);
    evas_object_smart_callback_add(m_ewkView, "text,found", found_cb, data);

    Ewk_Find_Options find_option = (Ewk_Find_Options)(EWK_FIND_OPTIONS_CASE_INSENSITIVE | EWK_FIND_OPTIONS_WRAP_AROUND
                    | EWK_FIND_OPTIONS_SHOW_FIND_INDICATOR | EWK_FIND_OPTIONS_SHOW_HIGHLIGHT);

    if (!forward)
        find_option = (Ewk_Find_Options)(find_option | EWK_FIND_OPTIONS_BACKWARDS);

    ewk_view_text_find(m_ewkView, word, find_option, FIND_WORD_MAX_COUNT);
}

void WebView::ewkSettingsAutoFittingSet(bool value)
{
    BROWSER_LOGD("[%s:%d:%d] ", __PRETTY_FUNCTION__, __LINE__, value);
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_auto_fitting_set(settings, value);
}

void WebView::ewkSettingsLoadsImagesSet(bool value)
{
    BROWSER_LOGD("[%s:%d:%d] ", __PRETTY_FUNCTION__, __LINE__, value);
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_loads_images_automatically_set(settings, value);
}

void WebView::ewkSettingsJavascriptEnabledSet(bool value)
{
    BROWSER_LOGD("[%s:%d:%d] ", __PRETTY_FUNCTION__, __LINE__, value);
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_javascript_enabled_set(settings, value);
}

void WebView::ewkSettingsFormCandidateDataEnabledSet(bool value)
{
    BROWSER_LOGD("[%s:%d:%d] ", __PRETTY_FUNCTION__, __LINE__, value);
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_form_candidate_data_enabled_set(settings, value);
}

void WebView::ewkSettingsAutofillPasswordFormEnabledSet(bool value)
{
    BROWSER_LOGD("[%s:%d:%d] ", __PRETTY_FUNCTION__, __LINE__, value);
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_autofill_password_form_enabled_set(settings, value);
}

void WebView::ewkSettingsScriptsCanOpenNewPagesEnabledSet(bool value)
{
    BROWSER_LOGD("[%s:%d:%d] ", __PRETTY_FUNCTION__, __LINE__, value);
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_scripts_can_open_windows_set(settings, value);
}

bool WebView::clearTextSelection() const {
    return ewk_view_text_selection_clear(m_ewkView);
}

bool WebView::exitFullScreen() const {
    return ewk_view_fullscreen_exit(m_ewkView);
}

void WebView::ewkSettingsFormProfileDataEnabledSet(bool value)
{
    Ewk_Settings* settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_form_profile_data_enabled_set(settings, value);
}

const TabId& WebView::getTabId() {
    return m_tabId;
}

tools::BrowserImagePtr WebView::getFavicon()
{
    BROWSER_LOGD("%s:%d, TabId: %s", __PRETTY_FUNCTION__, __LINE__, m_tabId.toString().c_str());
    return m_faviconImage;
}

void WebView::clearCache()
{
    BROWSER_LOGD("Clearing cache");
    M_ASSERT(m_ewkContext);
    if (m_ewkContext) {
        ewk_context_cache_clear(m_ewkContext);
        ewk_context_application_cache_delete_all(m_ewkContext);
    }
}

void WebView::clearCookies()
{
    BROWSER_LOGD("Clearing cookies");
    M_ASSERT(m_ewkContext);
    if (m_ewkContext)
        ewk_cookie_manager_cookies_clear(ewk_context_cookie_manager_get(m_ewkContext));
}

void WebView::clearPrivateData()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_ewkContext);
    if (m_ewkContext) {
        ewk_context_cache_clear(m_ewkContext);
        ewk_context_web_storage_delete_all(m_ewkContext);
        ewk_cookie_manager_cookies_clear(ewk_context_cookie_manager_get(m_ewkContext));
        ewk_context_application_cache_delete_all(m_ewkContext);
    }
}

void WebView::clearPasswordData()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_ewkContext);
    if (m_ewkContext)
        ewk_context_form_password_data_delete_all(m_ewkContext);
    else
        BROWSER_LOGD("[%s:%d] Warning: no context", __PRETTY_FUNCTION__, __LINE__);
}

void WebView::clearFormData()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_ewkContext);
    if (m_ewkContext) {
        ewk_context_form_candidate_data_delete_all(m_ewkContext);
    } else
        BROWSER_LOGD("[%s:%d] Warning: no context", __PRETTY_FUNCTION__, __LINE__);
}

void WebView::searchOnWebsite(const std::string & searchString, int flags)
{
    ///\todo: it should be "0" instead of "1024" for unlimited match count but it doesn't work properly in WebKit
    Eina_Bool result = ewk_view_text_find(m_ewkView, searchString.c_str(), static_cast<Ewk_Find_Options>(flags), 1024);
    BROWSER_LOGD("Ewk search; word: %s, result: %d", searchString.c_str(), result);
}

void WebView::switchToDesktopMode() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ewk_view_user_agent_set(m_ewkView, APPLICATION_NAME_FOR_USER_AGENT);
    m_desktopMode = true;
}

void WebView::switchToMobileMode() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ewk_view_user_agent_set(m_ewkView, APPLICATION_NAME_FOR_USER_AGENT_MOBILE);
    m_desktopMode = false;
}

bool WebView::isDesktopMode() const {
    return m_desktopMode;
}

void WebView::__policy_response_decide_cb(void *data, Evas_Object * /* obj */, void *event_info)
{
    WebView *wv = (WebView *)data;

    Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;
    Ewk_Policy_Decision_Type policy_type = ewk_policy_decision_type_get(policy_decision);

    wv->m_status_code = ewk_policy_decision_response_status_code_get(policy_decision);
    wv->m_is_error_page = EINA_FALSE;

    switch (policy_type) {
    case EWK_POLICY_DECISION_USE:
        BROWSER_LOGD("[%s:%d] policy_use", __PRETTY_FUNCTION__, __LINE__);
        ewk_policy_decision_use(policy_decision);
        break;

    case EWK_POLICY_DECISION_DOWNLOAD: {
        BROWSER_LOGD("[%s:%d] policy_download", __PRETTY_FUNCTION__, __LINE__);
        const char *uri = ewk_policy_decision_url_get(policy_decision);
        const char *content_type = ewk_policy_decision_response_mime_get(policy_decision);
        const Eina_Hash *headers = ewk_policy_decision_response_headers_get(policy_decision);
        app_control_h app_control = NULL;
        if (app_control_create(&app_control) < 0) {
            BROWSER_LOGE("[%s:%d] Fail to app_control_create", __PRETTY_FUNCTION__, __LINE__);
            return;
         }

        if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
            BROWSER_LOGE("[%s:%d] Fail to app_control_set_operation", __PRETTY_FUNCTION__, __LINE__);
            app_control_destroy(app_control);
            return;
         }

        BROWSER_LOGD("[%s:%d] uri: %s", __PRETTY_FUNCTION__, __LINE__, uri);
        if (app_control_set_uri(app_control, uri) < 0) {
            BROWSER_LOGE("[%s:%d] Fail to app_control_set_uri", __PRETTY_FUNCTION__, __LINE__);
            app_control_destroy(app_control);
            return;
         }

        BROWSER_LOGD("[%s:%d] content_type: %s", __PRETTY_FUNCTION__, __LINE__, content_type);
        if (app_control_set_mime(app_control, content_type) < 0) {
            BROWSER_LOGE("[%s:%d] Fail to app_control_set_mime", __PRETTY_FUNCTION__, __LINE__);
            app_control_destroy(app_control);
            return;
         }

        const char *content_dispotision = (const char *)eina_hash_find(headers, "Content-Disposition");
        BROWSER_LOGD("[%s:%d] Content-disposition: %s", __PRETTY_FUNCTION__, __LINE__, content_dispotision);
        if (content_dispotision && (strstr(content_dispotision, "attachment") != NULL)){
            wv->m_downloadControl->handle_download_request(uri, content_type);
            app_control_destroy(app_control);
            ewk_policy_decision_ignore(policy_decision);
            break;
         }

        if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_APP_NOT_FOUND) {
            BROWSER_LOGD("[%s:%d] app_control_send_launch_request returns APP_CONTROL_ERROR_APP_NOT_FOUND", __PRETTY_FUNCTION__, __LINE__);
            wv->m_downloadControl->handle_download_request(uri, content_type);
         }
        app_control_destroy(app_control);
        ewk_policy_decision_ignore(policy_decision);
        break;
    }
    case EWK_POLICY_DECISION_IGNORE:
    default:
        BROWSER_LOGD("[%s:%d] policy_ignore", __PRETTY_FUNCTION__, __LINE__);
        ewk_policy_decision_ignore(policy_decision);
        break;
    }
}

void WebView::__policy_navigation_decide_cb(void *data, Evas_Object * /*obj*/, void *event_info)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    WebView *wv = (WebView *)data;

    Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;
    const char *uri = ewk_policy_decision_url_get(policy_decision);
    wv->m_loadingURL = std::string(uri);
    BROWSER_LOGD("uri = [%s]", uri);

    Eina_Bool is_scheme_handled = wv->handle_scheme(uri);

    if (is_scheme_handled) {
        BROWSER_LOGD("Scheme handled");
        ewk_policy_decision_ignore(policy_decision);
        if (!wv->isBackEnabled())
            wv = NULL;
        return;
    }
    ewk_policy_decision_use(policy_decision);
}

} /* namespace webengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */

