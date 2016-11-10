/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
 * NaviframeWrapper.cpp
 *
 *  Created on: Aug 4, 2016
 *      Author: m.kawonczyk@samsung.com
 */

#include "NaviframeWrapper.h"

#include "BrowserAssert.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

NaviframeWrapper::NaviframeWrapper(Evas_Object* parent)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_bottom_box(nullptr)
{
    M_ASSERT(m_parent);
    m_layout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
    elm_layout_theme_set(m_layout, "naviframe", "item/basic", "default");
}

NaviframeWrapper::~NaviframeWrapper()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_left_button)
        deleteLeftButton();
    if (m_prev_button)
        deletePrevButton();
    if (m_right_button)
        deleteRightButton();
    if (m_bottom_box)
        deleteBottomBar();
    evas_object_del(m_layout);
}

Evas_Object* NaviframeWrapper::getLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);
    return m_layout;
}

void NaviframeWrapper::setTitle(std::string title)
{
    elm_object_translatable_part_text_set(m_layout, "elm.text.title", title.c_str());
}

void NaviframeWrapper::unsetContent()
{
    elm_object_part_content_unset(m_layout, "elm.swallow.content");
}

void NaviframeWrapper::setContent(Evas_Object *content)
{
    elm_object_part_content_set(m_layout, "elm.swallow.content", content);
}

void NaviframeWrapper::addPrevButton(Evas_Smart_Cb callback, void *data)
{
    deletePrevButton();
    m_prev_button = elm_button_add(m_layout);
    m_prev_button_callback = callback;
    elm_object_part_content_set(m_layout, "elm.swallow.prev_btn", m_prev_button);
    elm_object_style_set(m_prev_button, "tizen_view/prev_btn");
    evas_object_smart_callback_add(m_prev_button, "clicked", m_prev_button_callback, data);
    evas_object_size_hint_weight_set(m_prev_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_prev_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void NaviframeWrapper::deletePrevButton()
{
    elm_object_item_part_content_unset(m_layout, "elm.swallow.prev_btn");
    if (m_prev_button) {
        evas_object_smart_callback_del(m_prev_button, "clicked", m_prev_button_callback);
        evas_object_del(m_prev_button);
        m_prev_button = nullptr;
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::setPrevButtonVisible(bool visible)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, visible);
    if (m_prev_button) {
        if (visible) {
            evas_object_show(m_prev_button);
            elm_object_signal_emit(m_layout, "elm,state,prev_btn,show", "elm");
        } else {
            evas_object_hide(m_prev_button);
            elm_object_signal_emit(m_layout, "elm,state,prev_btn,hide", "elm");
        }
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::addLeftButton(Evas_Smart_Cb callback, void *data)
{
    deleteLeftButton();
    m_left_button = elm_button_add(m_layout);
    m_left_button_callback = callback;
    elm_object_part_content_set(m_layout, "title_left_btn", m_left_button);
    elm_object_style_set(m_left_button, "naviframe/title_left");
    evas_object_smart_callback_add(m_left_button, "clicked", m_left_button_callback, data);
    evas_object_size_hint_weight_set(m_left_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_left_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void NaviframeWrapper::deleteLeftButton()
{
    if (m_left_button) {
        evas_object_smart_callback_del(m_left_button, "clicked", m_left_button_callback);
        evas_object_del(m_left_button);
        m_left_button = nullptr;
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::setLeftButtonText(std::string text)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, text.c_str());
    if (m_left_button)
        elm_object_translatable_text_set(m_left_button, text.c_str());
    else
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
}

void NaviframeWrapper::setLeftButtonVisible(bool visible)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, visible);
    if (m_left_button) {
        if (visible) {
            evas_object_show(m_left_button);
            elm_object_signal_emit(m_layout, "elm,state,title_left_btn,show", "elm");
        } else {
            evas_object_hide(m_left_button);
            elm_object_signal_emit(m_layout, "elm,state,title_left_btn,hide", "elm");
        }
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::setLeftButtonEnabled(bool enabled)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, enabled);
    if (m_left_button) {
        if (enabled)
            elm_object_signal_emit(m_left_button, "elm,state,enabled", "elm");
        else
            elm_object_signal_emit(m_left_button, "elm,state,disabled", "elm");
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::addRightButton(Evas_Smart_Cb callback, void *data)
{
    deleteRightButton();
    m_right_button = elm_button_add(m_layout);
    m_right_button_callback = callback;
    elm_object_part_content_set(m_layout, "title_right_btn", m_right_button);
    elm_object_style_set(m_right_button, "naviframe/title_right");
    evas_object_smart_callback_add(m_right_button, "clicked", m_right_button_callback, data);
    evas_object_size_hint_weight_set(m_right_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_right_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void NaviframeWrapper::deleteRightButton()
{
    if (m_right_button) {
        evas_object_smart_callback_del(m_right_button, "clicked", m_right_button_callback);
        evas_object_del(m_right_button);
        m_right_button = nullptr;
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::setRightButtonText(std::string text)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, text.c_str());
    if (m_right_button)
        elm_object_translatable_text_set(m_right_button, text.c_str());
    else
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
}

void NaviframeWrapper::setRightButtonVisible(bool visible)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, visible);
    if (m_right_button) {
        if (visible) {
            evas_object_show(m_right_button);
            elm_object_signal_emit(m_layout, "elm,state,title_right_btn,show", "elm");
        } else {
            evas_object_hide(m_right_button);
            elm_object_signal_emit(m_layout, "elm,state,title_right_btn,hide", "elm");
        }
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::setRightButtonEnabled(bool enabled)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, enabled);
    if (m_right_button) {
        if (enabled)
            elm_object_signal_emit(m_right_button, "elm,state,enabled", "elm");
        else
            elm_object_signal_emit(m_right_button, "elm,state,disabled", "elm");
    } else {
        BROWSER_LOGW("[%s] Button does not exist!", __PRETTY_FUNCTION__);
    }
}

void NaviframeWrapper::addButtonToBottomBar(std::string text, Evas_Smart_Cb callback, void *data)
{
    if (!m_bottom_box)
        createBottomBar();
    Evas_Object* button = elm_button_add(m_bottom_box);
    elm_object_style_set(button, "bottom");
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(button, "elm.text", text.c_str());
    elm_object_signal_emit(button, "elm,state,text,visible", "elm");
    evas_object_smart_callback_add(button, "clicked", callback, data);

    elm_box_pack_end(m_bottom_box, button);
    m_bottom_buttons.push_back(button);
    evas_object_show(button);

}

void NaviframeWrapper::setEnableButtonInBottomBar(int number, bool enabled)
{
    BROWSER_LOGD("[%s:%d] %d %d", __PRETTY_FUNCTION__, __LINE__, number, enabled);
    if (enabled)
        elm_object_signal_emit(m_bottom_buttons[number], "elm,state,enabled", "elm");
    else
        elm_object_signal_emit(m_bottom_buttons[number], "elm,state,disabled", "elm");
}

void NaviframeWrapper::setBottomButtonText(int number, const std::string& text)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, text.c_str());
    elm_object_part_text_set(m_bottom_buttons[number], "elm.text", text.c_str());
}

void NaviframeWrapper::setVisibleBottomBar(bool visible)
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, visible);
    if (!m_bottom_box)
        createBottomBar();
    if (visible) {
        evas_object_show(m_bottom_box);
        elm_object_signal_emit(m_layout, "elm,state,toolbar,show", "elm");
    } else {
        evas_object_hide(m_bottom_box);
        elm_object_signal_emit(m_layout, "elm,state,toolbar,hide", "elm");
    }
}

void NaviframeWrapper::createBottomBar(Evas_Object* layout, std::string swallow_name)
{
    if (m_bottom_box)
        deleteBottomBar();
    m_bottom_box = elm_box_add(m_layout);
    elm_box_horizontal_set(m_bottom_box, EINA_TRUE);
    elm_box_padding_set(m_bottom_box, 32, 0);
    evas_object_size_hint_weight_set(m_bottom_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    if (layout) {
        elm_object_part_content_set(m_layout, "toolbar", layout);
        elm_object_part_content_set(layout, swallow_name.c_str(), m_bottom_box);
    } else {
        elm_object_part_content_set(m_layout, "toolbar", m_bottom_box);
    }
}

void NaviframeWrapper::deleteBottomBar()
{
    if (m_bottom_box) {
        elm_box_clear(m_bottom_box);
        m_bottom_buttons.clear();
        evas_object_del(m_bottom_box);
        m_bottom_box = nullptr;
    } else {
        BROWSER_LOGW("[%s] Box does not exist!", __PRETTY_FUNCTION__);
    }
}


}
}
