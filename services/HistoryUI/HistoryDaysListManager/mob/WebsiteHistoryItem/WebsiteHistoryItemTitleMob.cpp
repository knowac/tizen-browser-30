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

#include "WebsiteHistoryItemTitleMob.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>
#include "app_i18n.h"

namespace tizen_browser {
namespace base_ui {

boost::signals2::signal<void(const WebsiteHistoryItemDataPtr, bool)> WebsiteHistoryItemTitleMob::signalButtonClicked;

const int WebsiteHistoryItemTitleMob::GESTURE_MOMENTUM_MIN = 2000;

WebsiteHistoryItemTitleMob::WebsiteHistoryItemTitleMob(
        WebsiteHistoryItemDataPtr websiteHistoryItemData)
    : m_websiteHistoryItemData(websiteHistoryItemData)
    , m_buttonSelect(nullptr)
    , m_imageFavIcon(nullptr)
    , m_imageFavIconMask(nullptr)
    , m_layoutMain(nullptr)
    , m_boxMain(nullptr)
    , m_layoutContent(nullptr)
    , m_layoutButtonDelete(nullptr)
    , m_boxContentHorizontal(nullptr)
{
}

WebsiteHistoryItemTitleMob::~WebsiteHistoryItemTitleMob()
{
}

Evas_Object* WebsiteHistoryItemTitleMob::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemTitle");

    m_boxMain = elm_box_add(parent);
    tools::EflTools::setExpandHints(m_boxMain);
    elm_box_horizontal_set(m_boxMain, EINA_TRUE);

    m_layoutContent = createLayoutContent(parent, edjeFilePath);
    m_layoutButtonDelete = createLayoutButtonDelete(parent, edjeFilePath);
    elm_box_pack_end(m_boxMain, m_layoutContent);

    elm_object_part_content_set(m_layoutMain, "main", m_boxMain);

    m_layerGesture = elm_gesture_layer_add(parent);
    elm_gesture_layer_attach(m_layerGesture, m_layoutMain);

    evas_object_show(m_layoutContent);
    evas_object_show(m_layoutMain);

    initCallbacks();
    return m_layoutMain;
}

void WebsiteHistoryItemTitleMob::initCallbacks()
{
    evas_object_smart_callback_add(m_buttonSelect, "clicked",
            _buttonSelectClicked, this);
    evas_object_smart_callback_add(m_buttonDelete, "clicked",
            _buttonDeleteClicked, this);
    elm_gesture_layer_cb_add(m_layerGesture, ELM_GESTURE_N_LINES,
            ELM_GESTURE_STATE_MOVE, _gestureOccured, this);
}

void WebsiteHistoryItemTitleMob::_buttonSelectClicked(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data)
        return;
    WebsiteHistoryItemTitleMob* self =
            static_cast<WebsiteHistoryItemTitleMob*>(data);
    if (self->getClickBlock()) {
        self->setClickBlock(false);
        return;
    }
    signalButtonClicked(self->getWebsiteHistoryItemDataPtr(), false);
}

void WebsiteHistoryItemTitleMob::_buttonDeleteClicked(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data)
        return;
    WebsiteHistoryItemTitleMob* self =
            static_cast<WebsiteHistoryItemTitleMob*>(data);
    if (self->getClickBlock()) {
        self->setClickBlock(false);
        return;
    }
    signalButtonClicked(self->getWebsiteHistoryItemDataPtr(), true);
}

Evas_Event_Flags WebsiteHistoryItemTitleMob::_gestureOccured(void *data,
        void *event_info)
{
    Evas_Event_Flags flag = EVAS_EVENT_FLAG_NONE;
    if (!data)
        return flag;
    WebsiteHistoryItemTitleMob* self =
            static_cast<WebsiteHistoryItemTitleMob*>(data);
    auto info = static_cast<Elm_Gesture_Line_Info*>(event_info);
    if (info->momentum.mx != 0 || info->momentum.my != 0) {
        // prevents click event, when gesture occurred
        self->setClickBlock(true);
        // ignore too small gestures
        if (abs(info->momentum.mx) < GESTURE_MOMENTUM_MIN)
            return flag;
        if (info->momentum.mx < 0)
            self->showButtonDelete(true);
        else if (info->momentum.mx > 0)
            self->showButtonDelete(false);
    }
    return flag;
}

void WebsiteHistoryItemTitleMob::showButtonDelete(bool show)
{
    if (evas_object_visible_get(m_layoutButtonDelete) == show)
        return;
    if (show)
        evas_object_show(m_layoutButtonDelete);
    else
        evas_object_hide(m_layoutButtonDelete);

    if (show)
        elm_box_pack_end(m_boxMain, m_layoutButtonDelete);
    else
        elm_box_unpack(m_boxMain, m_layoutButtonDelete);
}

void WebsiteHistoryItemTitleMob::_title_mouse_down(void* data, Evas*, Evas_Object*, void*)
{
    WebsiteHistoryItemTitleMob* self = static_cast<WebsiteHistoryItemTitleMob*>(data);
    elm_object_signal_emit(self->m_imageFavIconMask, "favicon_mask_selected", "ui");
}

void WebsiteHistoryItemTitleMob::_title_mouse_up(void* data, Evas*, Evas_Object*, void*)
{
    WebsiteHistoryItemTitleMob* self = static_cast<WebsiteHistoryItemTitleMob*>(data);
    elm_object_signal_emit(self->m_imageFavIconMask, "favicon_mask_default", "ui");
}

Evas_Object* WebsiteHistoryItemTitleMob::createLayoutContent(
        Evas_Object* parent, const std::string& edjeFilePath)
{
    Evas_Object* lay = elm_layout_add(parent);
    tools::EflTools::setExpandHints(lay);
    elm_layout_file_set(lay, edjeFilePath.c_str(),
            "layoutMainContent");

    evas_object_event_callback_add(lay, EVAS_CALLBACK_MOUSE_DOWN, _title_mouse_down, this);
    evas_object_event_callback_add(lay, EVAS_CALLBACK_MOUSE_UP, _title_mouse_up, this);

    m_boxContentHorizontal = elm_box_add(parent);
    elm_box_align_set(m_boxContentHorizontal, 0.0, 0.0);
    elm_box_horizontal_set(m_boxContentHorizontal, EINA_TRUE);
    elm_object_part_content_set(lay, "boxMainHorizontal",
            m_boxContentHorizontal);

    m_buttonSelect = elm_button_add(parent);
    elm_object_part_content_set(lay, "buttonSelect", m_buttonSelect);
    elm_object_style_set(m_buttonSelect, "invisible_button");

    Evas_Object* layoutIcon = createLayoutIcon(parent, edjeFilePath);
    Evas_Object* layoutSummary = createLayoutSummary(parent, edjeFilePath);
    elm_box_pack_end(m_boxContentHorizontal, layoutIcon);
    elm_box_pack_end(m_boxContentHorizontal, layoutSummary);

    evas_object_show(layoutIcon);
    evas_object_show(layoutSummary);

    return lay;
}

Evas_Object* WebsiteHistoryItemTitleMob::createLayoutButtonDelete(
        Evas_Object* parent, const std::string& edjeFilePath)
{
    Evas_Object* lay = elm_layout_add(parent);
    evas_object_size_hint_align_set(lay, EVAS_HINT_FILL,
    0.5);
    elm_layout_file_set(lay, edjeFilePath.c_str(),
            "layoutButtonDelete");
    elm_object_text_set(lay, _("IDS_BR_SK_CLEAR"));

    m_buttonDelete = elm_button_add(parent);
    elm_object_part_content_set(lay, "buttonSelect", m_buttonDelete);
    elm_object_style_set(m_buttonDelete, "invisible_button");

    return lay;
}

Evas_Object* WebsiteHistoryItemTitleMob::createLayoutIcon(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutItemIcon");

    if (m_websiteHistoryItemData->favIcon) {
        m_imageFavIcon = m_websiteHistoryItemData->favIcon->getEvasImage(parent);
        elm_object_part_content_set(layout, "swallowFavIcon", m_imageFavIcon);
    }

    m_imageFavIconMask = elm_layout_add(parent);
    elm_layout_file_set(m_imageFavIconMask, edjeFilePath.c_str(), "groupImageFaviconMask");
    elm_object_part_content_set(layout, "swallowFavIconMask",
            m_imageFavIconMask);

    evas_object_show(m_imageFavIcon);

    return layout;
}

Evas_Object* WebsiteHistoryItemTitleMob::createLayoutSummary(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layout);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutTextSummaryTitle");
    elm_object_text_set(layout, m_websiteHistoryItemData->websiteTitle.c_str());

    return layout;
}

}
}
