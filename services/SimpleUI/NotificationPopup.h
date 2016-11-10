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

#ifndef __NOTIFICATION_POPUP_H__
#define __NOTIFICATION_POPUP_H__

#include <Evas.h>
#include <eina-1/Eina.h>
#include <ecore-1/Ecore.h>
#include <Elementary.h>
#include <string>

namespace tizen_browser {
namespace base_ui {

class NotificationPopup
{
public:
    NotificationPopup();
    static NotificationPopup *createNotificationPopup(Evas_Object *parent);
    void show(const std::string &message, bool progressVisible = true);
    void show(const std::string &message, const std::string &message_add, bool progressVisible = true);
    void dismiss();
    static Eina_Bool _hide_cb(void *data);

private:
    void createLayout(bool progressVisible);

    std::string edjFilePath;
    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_progress;
    std::string m_message;
    Ecore_Timer *m_timer;
};

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */

#endif //__NOTIFICATION_POPUP_H__
