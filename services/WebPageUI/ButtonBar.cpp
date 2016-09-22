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

#include <Elementary.h>
#include <iostream>
#include "BrowserLogger.h"

#include "Config.h"
#include "ButtonBar.h"

namespace tizen_browser {
namespace base_ui {

#if !PROFILE_MOBILE
double ButtonBar::tooltipHideTimeout = 0.;
double ButtonBar::tooltipShowDelay = 0.;
Ecore_Timer* ButtonBar::m_tooltipHideTimer = NULL;
Ecore_Timer* ButtonBar::m_tooltipShowTimer = NULL;
#endif

ButtonBar::ButtonBar(Evas_Object* parent, const std::string& edjFile, const std::string& groupName)
{
#if !PROFILE_MOBILE
    ButtonBar::tooltipHideTimeout =  boost::any_cast <double> (tizen_browser::config::Config::getInstance().get("TOOLTIP_HIDE_TIMEOUT"));
#endif

    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append(edjFile);
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_layout = elm_layout_add(parent);
    Eina_Bool layoutSetResult = elm_layout_file_set(m_layout, edjFilePath.c_str(), groupName.c_str());
    if (!layoutSetResult)
        throw std::runtime_error("Layout file not found: " + edjFilePath);
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
}
ButtonBar::~ButtonBar()
{
#if !PROFILE_MOBILE
    if (ButtonBar::m_tooltipShowTimer) {
        ecore_timer_del(ButtonBar::m_tooltipShowTimer);
        ButtonBar::m_tooltipShowTimer = NULL;
    }
    if (ButtonBar::m_tooltipHideTimer) {
        ecore_timer_del(ButtonBar::m_tooltipHideTimer);
        ButtonBar::m_tooltipHideTimer = NULL;
    }
#endif
}

void ButtonBar::addAction(sharedAction action, const std::string& buttonName)
{
    ActionButton actionButton;
    std::shared_ptr<EAction> eAction = std::make_shared<EAction>(action);
    actionButton.eAction = eAction;
    Evas_Object* button = elm_button_add(m_layout);

    evas_object_event_callback_add(button, EVAS_CALLBACK_MOUSE_IN, __cb_mouse_in, NULL);
    evas_object_event_callback_add(button, EVAS_CALLBACK_MOUSE_OUT, __cb_mouse_out, NULL);
#if !PROFILE_MOBILE
    evas_object_smart_callback_add(button, "focused", __cb_focus_in, NULL);
    evas_object_smart_callback_add(button, "unfocused", __cb_focus_out, NULL);
#endif


    elm_object_style_set(button, action->getIcon().c_str());
    elm_object_disabled_set(button, action->isEnabled() ? EINA_FALSE : EINA_TRUE);
    evas_object_smart_callback_add(button, "clicked", EAction::callbackFunction, eAction.get());
    evas_object_show(button);
#if !PROFILE_MOBILE
    if (action->isEnabled()) {
        elm_object_tooltip_text_set(button, eAction->getAction()->getToolTip().c_str());
        elm_object_tooltip_orient_set(button, ELM_TOOLTIP_ORIENT_BOTTOM);
        elm_object_tooltip_style_set(button, "browserTip");
    }
#endif

    elm_object_part_content_set(m_layout, buttonName.c_str(), button);

    actionButton.button = button;
    m_buttonsMap[buttonName] = actionButton;
    m_actionsMap[buttonName] = action;
    registerEnabledChangedCallback(action, buttonName);
}

void ButtonBar::registerEnabledChangedCallback(sharedAction action, const std::string& buttonName)
{
    action->enabledChanged.connect(boost::bind(&ButtonBar::onEnabledChanged, this, buttonName, action));
}

void ButtonBar::onEnabledChanged(const std::string& buttonName, sharedAction action)
{
    // if action match action assigned to button, refresh button
    if (m_actionsMap[buttonName] == action) {
        refreshButton(buttonName);
    }

}

void ButtonBar::setActionForButton(const std::string& buttonName, sharedAction newAction)
{
    ActionButton actionButton;
    Evas_Object* button = m_buttonsMap[buttonName].button;
    std::shared_ptr<EAction> eAction = std::make_shared<EAction>(newAction);

    elm_object_style_set(button, newAction->getIcon().c_str());

    evas_object_smart_callback_del(button, "clicked", EAction::callbackFunction);
    evas_object_smart_callback_add(button, "clicked", EAction::callbackFunction, eAction.get());


    if (elm_object_focus_get(button)) {
        elm_object_signal_emit(button, "elm,action,focus", "elm");
    }

    m_buttonsMap[buttonName].eAction = eAction;
    m_actionsMap[buttonName] = newAction;

    refreshButton(buttonName);

}

void ButtonBar::refreshButton(const std::string& buttonName)
{
    Evas_Object* button = m_buttonsMap[buttonName].button;
    if (m_actionsMap[buttonName]->isEnabled()) {
        elm_object_disabled_set(button, EINA_FALSE);
#if !PROFILE_MOBILE
        elm_object_tooltip_text_set(button, m_actionsMap[buttonName]->getToolTip().c_str());
        elm_object_tooltip_orient_set(button, ELM_TOOLTIP_ORIENT_BOTTOM);
        elm_object_tooltip_style_set(button, "browserTip");
#endif
    } else {
        elm_object_disabled_set(button, EINA_TRUE);
#if !PROFILE_MOBILE
        elm_object_tooltip_unset(button);
#endif
    }
}

Evas_Object* ButtonBar::getContent()
{
    return m_layout;
}

Evas_Object* ButtonBar::getButton(const std::string& buttonName)
{
    return m_buttonsMap[buttonName].button;
}

void ButtonBar::clearFocus()
{
    for (auto it = m_buttonsMap.begin(); it != m_buttonsMap.end(); ++it) {
        elm_object_focus_set((*it).second.button, EINA_FALSE);
    }
}

void ButtonBar::setDisabled(bool disabled)
{
    if (disabled) {
        clearFocus();
    }
    elm_object_disabled_set(getContent(), disabled ? EINA_TRUE : EINA_FALSE);
}

#if !PROFILE_MOBILE
void ButtonBar::__cb_focus_in(void*, Evas_Object* obj, void*)
{
    //BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, reinterpret_cast<int>(obj));

    if (ButtonBar::m_tooltipHideTimer) {
        ecore_timer_del(ButtonBar::m_tooltipHideTimer);
        ButtonBar::m_tooltipHideTimer = NULL;
    }
    if (ButtonBar::m_tooltipShowTimer) {
        ecore_timer_del(ButtonBar::m_tooltipShowTimer);
        ButtonBar::m_tooltipShowTimer = NULL;
    }
    ButtonBar::m_tooltipHideTimer = ecore_timer_add(ButtonBar::tooltipHideTimeout, ButtonBar::__cb_tooltip_hide_timeout, obj);
    //emulate native behaviour
    //tooltip works better with some delay
    ButtonBar::m_tooltipShowTimer = ecore_timer_add(elm_config_tooltip_delay_get(), ButtonBar::__cb_tooltip_show_delay, obj);
}

void ButtonBar::__cb_focus_out(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ButtonBar::m_tooltipHideTimer) {
        ecore_timer_del(ButtonBar::m_tooltipHideTimer);
        ButtonBar::m_tooltipHideTimer = NULL;
    }
    if (ButtonBar::m_tooltipShowTimer) {
        ecore_timer_del(ButtonBar::m_tooltipShowTimer);
        ButtonBar::m_tooltipShowTimer = NULL;
    }
    elm_object_tooltip_hide(obj);
}
#endif

void ButtonBar::__cb_mouse_in(void* /*data*/, Evas* /*e*/, Evas_Object* obj, void* /*event_info*/)
{
    //BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, reinterpret_cast<int>(obj));
    elm_object_focus_set(obj, EINA_TRUE);
}

void ButtonBar::__cb_mouse_out(void* /*data*/, Evas* /*e*/, Evas_Object* obj, void* /*event_info*/)
{
    elm_object_focus_set(obj, EINA_FALSE);
}

#if !PROFILE_MOBILE
Eina_Bool ButtonBar::__cb_tooltip_hide_timeout(void* data)
{
    Evas_Object* button = static_cast<Evas_Object*>(data);

    if (ButtonBar::m_tooltipHideTimer) {
        ecore_timer_del(ButtonBar::m_tooltipHideTimer);
        ButtonBar::m_tooltipHideTimer = NULL;
    }

    elm_object_tooltip_hide(button);
    return ECORE_CALLBACK_CANCEL;
}


Eina_Bool ButtonBar::__cb_tooltip_show_delay(void* data)
{
    Evas_Object* button = static_cast<Evas_Object*>(data);

    if (ButtonBar::m_tooltipShowTimer) {
        ecore_timer_del(ButtonBar::m_tooltipShowTimer);
        ButtonBar::m_tooltipShowTimer = NULL;
    }

    elm_object_tooltip_show(button);
    return ECORE_CALLBACK_CANCEL;
}
#endif

}
}
