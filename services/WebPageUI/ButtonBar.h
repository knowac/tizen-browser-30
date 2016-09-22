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

#ifndef BUTTONBAR_H
#define BUTTONBAR_H

#include <Evas.h>
#include <list>
#include <Ecore.h>

#include "Action.h"
#include "EAction.h"

namespace tizen_browser {
namespace base_ui {

class ButtonBar {
public:

    struct ActionButton {
        Evas_Object* button;
        std::shared_ptr<EAction> eAction;
    };

    ButtonBar(Evas_Object* parent, const std::string& edjFile, const std::string& groupName);
    ~ButtonBar();
    void addAction(sharedAction action, const std::string& buttonName);
    void setActionForButton(const std::string& buttonName, sharedAction newAction);
    /**
     * @brief register callback, used internally by addAction,
     *        should be called externally for all actions that will be assigned to button
     * @param action - action that will be used with buttonName
     * @param buttonName - name of button used in ButtonBar
     */
    void registerEnabledChangedCallback(sharedAction action, const std::string& buttonName);

    Evas_Object* getContent();
    Evas_Object* getButton(const std::string& buttonName);
    void clearFocus();
    void setDisabled(bool disabled);

private:
    //map button name to current action assigned to button
    std::map<std::string, sharedAction> m_actionsMap;
    //map button name to struct ActionButton which contains Evas_Object of button
    std::map<std::string, ActionButton> m_buttonsMap;
    Evas_Object* m_layout;
    void refreshButton(const std::string& buttonName);
    void onEnabledChanged(const std::string& buttonName, sharedAction action);
#if !PROFILE_MOBILE
    static Ecore_Timer* m_tooltipHideTimer;
    static Ecore_Timer* m_tooltipShowTimer;

    static double tooltipHideTimeout;
    static double tooltipShowDelay;
#endif

    static void __cb_mouse_in(void* data, Evas* e, Evas_Object* obj, void* event_info);
    static void __cb_mouse_out(void* data, Evas* e, Evas_Object* obj, void* event_info);
#if !PROFILE_MOBILE
    static void __cb_focus_in(void* data, Evas_Object* obj, void* event_info);
    static void __cb_focus_out(void* data, Evas_Object* obj, void* event_info);

    static Eina_Bool __cb_tooltip_hide_timeout(void* data);
    static Eina_Bool __cb_tooltip_show_delay(void* data);
#endif

};

}

}

#endif // BUTTONBAR_H
