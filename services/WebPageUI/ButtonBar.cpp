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


ButtonBar::ButtonBar(Evas_Object* parent, const std::string& edjFile, const std::string& groupName)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append(edjFile);
    elm_theme_extension_add(NULL, m_edjFilePath.c_str());
    m_layout = elm_layout_add(parent);
    Eina_Bool layoutSetResult = elm_layout_file_set(m_layout, m_edjFilePath.c_str(), groupName.c_str());
    if (!layoutSetResult)
        throw std::runtime_error("Layout file not found: " + m_edjFilePath);
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
}

ButtonBar::~ButtonBar()
{

}

void ButtonBar::addAction(sharedAction action, const std::string& buttonName)
{
    ActionButton actionButton;
    std::shared_ptr<EAction> eAction = std::make_shared<EAction>(action);
    actionButton.eAction = eAction;
    Evas_Object* button = elm_button_add(m_layout);

    Evas_Object* box = elm_box_add(button);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_homogeneous_set(box, EINA_FALSE);

    // Set a source file to fetch pixel data
    Evas_Object *img = elm_layout_add(box);
    elm_layout_file_set(img, m_edjFilePath.c_str(), action->getIcon().c_str());
    evas_object_size_hint_min_set(img, 0, BUTTON_WITH_ICON_HEIGHT);
    elm_box_pack_end(box, img);
    evas_object_show(img);
    m_imgMap[buttonName] = img;

    Evas_Object *label = elm_label_add(box);
    elm_object_text_set(label, action->getText().c_str());
    elm_box_pack_end(box, label);
    evas_object_show(label);
    evas_object_show(box);
    elm_object_part_content_set(button, "elm.swallow.content", box);

    elm_object_disabled_set(button, action->isEnabled() ? EINA_FALSE : EINA_TRUE);
    evas_object_smart_callback_add(button, "clicked", EAction::callbackFunction, eAction.get());
    evas_object_show(button);
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
    elm_object_disabled_set(button, m_actionsMap[buttonName]->isEnabled() ? EINA_FALSE : EINA_TRUE);
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

void ButtonBar::setButtonsColor(bool secretMode)
{
    for (const auto& it : m_buttonsMap) {
        if (secretMode) {
            //TODO works, state is changed but get gray scale only, why?
            //elm_object_signal_emit(m_imgMap[it.first], "set_secret_mode", "ui");
            evas_object_color_set(it.second.button, 97, 97, 97, 255);
        } else {
            evas_object_color_set(it.second.button, 240, 240, 240, 255);
            elm_object_signal_emit(m_imgMap[it.first], "set_normal_mode", "ui");
        }
    }

}

}
}
