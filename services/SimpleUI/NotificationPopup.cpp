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

#include <Evas.h>
#include <Elementary.h>
#include "NotificationPopup.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"
#include <BrowserAssert.h>


static const float DEFAULT_POPUP_INTERVAL = 3.0;

namespace tizen_browser {
namespace base_ui {

NotificationPopup::NotificationPopup()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_progress(nullptr)
    , m_timer(nullptr)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/NotificationPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
}

NotificationPopup *NotificationPopup::createNotificationPopup(Evas_Object *parent)
{
    BROWSER_LOGD("[%s,%d]", __func__, __LINE__);
    NotificationPopup *raw_popup = new NotificationPopup();
    raw_popup->m_parent = parent;
    return raw_popup;
}

void NotificationPopup::show(const std::string& message, bool progressVisible)
{
    BROWSER_LOGD("[%s,%d]", __func__, __LINE__);
    m_message = message;
    createLayout(progressVisible);
    m_timer =  ecore_timer_add(DEFAULT_POPUP_INTERVAL, _hide_cb, this);
}

void NotificationPopup::show(const std::string& message, const std::string &message_add, bool progressVisible)
{
    BROWSER_LOGD("[%s,%d]", __func__, __LINE__);
    m_message = message + message_add;
    createLayout(progressVisible);
    m_timer =  ecore_timer_add(DEFAULT_POPUP_INTERVAL, _hide_cb, this);
}

void NotificationPopup::dismiss()
{
    BROWSER_LOGD("[%s,%d]", __func__, __LINE__);
    float pendingTime = ecore_timer_pending_get(m_timer);
    if(pendingTime > DEFAULT_POPUP_INTERVAL-1) {
        ecore_timer_interval_set(m_timer, 1.0);
        ecore_timer_reset(m_timer);
    }
    else {
        elm_object_part_content_unset(m_parent, "popup_content");
        evas_object_hide(m_layout);
        ecore_timer_del(m_timer);
    }
}

Eina_Bool NotificationPopup::_hide_cb(void *data)
{
    BROWSER_LOGD("[%s,%d]", __func__, __LINE__);
    NotificationPopup * np = static_cast<NotificationPopup*>(data);
    elm_object_part_content_unset(np->m_parent, "popup_content");
    evas_object_hide(np->m_layout);
    ecore_timer_del(np->m_timer);
    delete np;
    return ECORE_CALLBACK_CANCEL;
}

void NotificationPopup::createLayout(bool progressVisible)
{
    BROWSER_LOGD("[%s,%d]", __func__, __LINE__);
    M_ASSERT(m_parent);

    m_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "notification_popup_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(m_parent, "popup_content", m_layout);
    elm_layout_text_set(m_layout, "popup_text", m_message.c_str());

    if (progressVisible) {
        m_progress = elm_progressbar_add(m_layout);
        elm_object_part_content_set(m_layout, "progress_swallow", m_progress);
#if PROFILE_MOBILE
        elm_object_style_set(m_progress, "process_medium");
        elm_progressbar_unit_format_set(m_progress, "");
#endif
        //TODO: set correct progressbar theme when it will be available.
        elm_progressbar_horizontal_set(m_progress, EINA_TRUE);
        elm_progressbar_pulse_set(m_progress, EINA_TRUE);
        elm_progressbar_pulse(m_progress, EINA_TRUE);
        evas_object_show(m_progress);
    }
    evas_object_show(m_layout);
}


} /* end of namespace base_ui */
} /* end of namespace tizen_browser */

