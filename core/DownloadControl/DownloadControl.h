/*
 * Copyright 2015  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Minseok Choi <min7.choi@samsung.com>
 *
 */

#ifndef DOWNLOAD_CONTROL_H
#define DOWNLOAD_CONTROL_H

#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>
#include <Ecore.h>
#include <Eina.h>
#include <Elementary.h>
#include <glib.h>

#include "BrowserLogger.h"

#define unit_size 1024
#define default_device_storage_path "/home/owner/content/Downloads/"
//TODO Currently cannot save content on sd card. Check if it is correct path
//or the problem is with the download provider
#define defualt_sd_card_storage_path "/opt/storage/sdcard/Downloads/"

#define CALLBACK_DATA_0 "cb0"
#define CALLBACK_DATA_1 "cb1"
#define CALLBACK_DATA_2 "cb2"
#define CALLBACK_DATA_3 "cb3"

#define data_scheme "data:"
#define data_scheme_png_base64 "data:image/png;base64"
#define data_scheme_jpeg_base64 "data:image/jpeg;base64"
#define data_scheme_jpg_base64 "data:image/jpg;base64"
#define data_scheme_gif_base64 "data:image/gif;base64"
#define BROWSER_APP_NAME "org.tizen.browser"
#define sec_vt_app "org.tizen.vtmain"

#define BR_STRING_NO_NAME                       _("IDS_BR_BODY_NO_NAME")
#define BR_STRING_DOWNLOAD_COMPLETE             _("IDS_BR_POP_DOWNLOADCOMPLETE")
#define BR_STRING_INTERNET         _("IDS_BR_BODY_INTERNET")
#define BR_STRING_STREAMING_PLAYER _("IDS_BR_BODY_STREAMING")
#define BR_STRING_DOWNLOADING_ING   "Starting download..."

#define BROWSER_PACKAGE_NAME    "browser"
#define BROWSER_DOMAIN_NAME BROWSER_PACKAGE_NAME
#define SYSTEM_DOMAIN_NAME "sys_string"

#define efl_scale (elm_config_scale_get() / elm_app_base_scale_get())
#define default_download_item_name "download"

#define BROWSER_DATA_SCHEME_DOWNLOAD_ALLOW_MAX_COUNT 100000
#define APP_LIST_HEIGHT (230 * efl_scale)

typedef enum _download_popup_type {
    DOWNLOAD_UNABLE_TO_DOWNLOAD = 0,
    DOWNLOAD_STARTING_DOWNLOAD,
    DOWNLOAD_SAVEDPAGES,
    DOWNLOAD_FAIL,
    DOWNLOAD_ONLY_HTTP_OR_HTTPS_URLS
} download_popup_type;

struct popup_callback
{
public:
    popup_callback(Evas_Smart_Cb cb = NULL, void* fdata = NULL, void* udata = NULL) :
        func(cb),
        func_data(fdata),
        user_data(udata)
    {}

    ~popup_callback(){}

    Evas_Smart_Cb func;
    void *func_data;
    void *user_data;

private:
    // Don't copy me
    popup_callback(const popup_callback&);
    popup_callback& operator=(const popup_callback&);
};

class DownloadControl {
public:
    DownloadControl(void);
    ~DownloadControl(void);

    void init(Evas_Object *mainWindow);

    Eina_Bool launch_download_app(const char *uri);
    Eina_Bool launch_streaming_player(const char *uri);

    /* If know the content type, use this method to handle it. */
    void handle_download_request(const char *uri, const char *content_type);

    Eina_Bool handle_data_scheme(const char *uri);

private:
    /* String should be freed in caller side */
    char *get_file_size_str(const char *full_path);

    Eina_Bool _check_file_exist(const char *path);
    Eina_Bool _save_file(const char *raw_data, const char *path);
    Eina_Bool _update_contents_on_media_db(const char *path);   //

    /* should be freed from caller side */
    Eina_Bool _get_download_path(const char *extension, char **full_path, char **file_name);

    static void __sdp_download_finished_cb(const char *file_path, void *data);

    Evas_Object *brui_popup_add(Evas_Object *parent);
    void show_noti_popup(const char *msg);
    const char* getStorageType();

    std::string m_download_uri;
    Evas_Object *m_parent;
};

#endif /* DOWNLOAD_CONTROL_H */

