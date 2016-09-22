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

#include "WebsiteHistoryItemTitleTv.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>
#include <services/HistoryUI/HistoryDeleteManager.h>

namespace tizen_browser {
namespace base_ui {

boost::signals2::signal<void(const WebsiteHistoryItemDataPtr)>
WebsiteHistoryItemTitleTv::signalButtonClicked;

WebsiteHistoryItemTitleTv::WebsiteHistoryItemTitleTv(
        WebsiteHistoryItemDataPtr websiteHistoryItemData,
        HistoryDeleteManagerPtrConst historyDeleteManager)
    : m_websiteHistoryItemData(websiteHistoryItemData)
    , m_buttonSelect(nullptr)
    , m_imageClear(nullptr)
    , m_layoutMain(nullptr)
    , m_boxMainHorizontal(nullptr)
    , m_historyDeleteManager(historyDeleteManager)
{
}

WebsiteHistoryItemTitleTv::~WebsiteHistoryItemTitleTv()
{
    deleteCallbacks();
}

Evas_Object* WebsiteHistoryItemTitleTv::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemTitle");
    evas_object_size_hint_align_set(m_layoutMain, 0.0, 0.0);

    m_boxMainHorizontal = elm_box_add(parent);
    tools::EflTools::setExpandHints(m_boxMainHorizontal);
    elm_box_align_set(m_boxMainHorizontal, 0.0, 0.0);
    elm_box_horizontal_set(m_boxMainHorizontal, EINA_TRUE);
    elm_object_part_content_set(m_layoutMain, "boxMainHorizontal",
            m_boxMainHorizontal);

    m_buttonSelect = elm_button_add(parent);
    elm_object_part_content_set(m_layoutMain, "buttonSelect",
            m_buttonSelect);
    evas_object_color_set(m_buttonSelect, 0, 0, 0, 0);
    elm_object_style_set(m_buttonSelect, "anchor");

    m_imageClear = createImageClear(parent, edjeFilePath);
    elm_object_part_content_set(m_layoutMain, "imageClear",
            m_imageClear);

    Evas_Object* layoutIcon = createLayoutIcon(parent, edjeFilePath);
    Evas_Object* layoutSummary = createLayoutSummary(parent, edjeFilePath);
    elm_box_pack_end(m_boxMainHorizontal, layoutIcon);
    elm_box_pack_end(m_boxMainHorizontal, layoutSummary);

    evas_object_show(m_buttonSelect);
    evas_object_show(layoutIcon);
    evas_object_show(layoutSummary);
    evas_object_show(m_layoutMain);

    initCallbacks();
    return m_layoutMain;
}

void WebsiteHistoryItemTitleTv::initCallbacks()
{
    evas_object_smart_callback_add(m_buttonSelect, "clicked",
            _buttonSelectClicked, &m_websiteHistoryItemData);
    evas_object_smart_callback_add(m_buttonSelect, "focused",
            _buttonSelectFocused, this);
    evas_object_smart_callback_add(m_buttonSelect, "unfocused",
            _buttonSelectUnfocused, this);
}

void WebsiteHistoryItemTitleTv::_buttonSelectClicked(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data) return;
    WebsiteHistoryItemDataPtr* websiteHistoryItemData =
            static_cast<WebsiteHistoryItemDataPtr*> (data);
    signalButtonClicked(*websiteHistoryItemData);
}

void WebsiteHistoryItemTitleTv::_buttonSelectFocused(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data) return;
    WebsiteHistoryItemTitleTv* self = static_cast<WebsiteHistoryItemTitleTv*>(data);
    Evas_Object* layoutMain = self->getLayoutMain();
    elm_object_signal_emit(layoutMain, "buttonSelectFocused", "ui");
    if (self->getDeleteManager()->getDeleteMode())
        elm_object_signal_emit(layoutMain, "showImageClear", "ui");
}

void WebsiteHistoryItemTitleTv::_buttonSelectUnfocused(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data) return;
    WebsiteHistoryItemTitleTv* self =
            static_cast<WebsiteHistoryItemTitleTv*>(data);
    Evas_Object* layoutMain = self->getLayoutMain();
    elm_object_signal_emit(layoutMain, "buttonSelectUnfocused", "ui");
}

Evas_Object* WebsiteHistoryItemTitleTv::createLayoutIcon(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutItemIcon");

    if (m_websiteHistoryItemData->favIcon) {
        m_imageFavIcon = m_websiteHistoryItemData->favIcon->getEvasImage(parent);
        elm_object_part_content_set(layout, "swallowFavIcon", m_imageFavIcon);
    }

    evas_object_show(m_imageFavIcon);

    return layout;
}

Evas_Object* WebsiteHistoryItemTitleTv::createLayoutSummary(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layout);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutItemSummary");

    Evas_Object* boxVertical = elm_box_add(parent);
    tools::EflTools::setExpandHints(boxVertical);
    elm_box_horizontal_set(boxVertical, EINA_FALSE);
    elm_object_part_content_set(layout, "boxMainVertical", boxVertical);

    Evas_Object* layoutTextSummaryTitle = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layoutTextSummaryTitle);
    elm_layout_file_set(layoutTextSummaryTitle, edjeFilePath.c_str(), "layoutTextSummaryTitle");
    elm_object_text_set(layoutTextSummaryTitle,
            m_websiteHistoryItemData->websiteTitle.c_str());

    Evas_Object* layoutTextSummaryDomain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layoutTextSummaryDomain);
    elm_layout_file_set(layoutTextSummaryDomain, edjeFilePath.c_str(), "layoutTextSummaryTitle");
    elm_object_text_set(layoutTextSummaryDomain,
            m_websiteHistoryItemData->websiteDomain.c_str());

    elm_box_pack_end(boxVertical, layoutTextSummaryTitle);
    elm_box_pack_end(boxVertical, layoutTextSummaryDomain);

    evas_object_show(m_buttonSelect);
    evas_object_show(layout);
    evas_object_show(layoutTextSummaryTitle);
    evas_object_show(layoutTextSummaryDomain);

    return layout;
}

Evas_Object* WebsiteHistoryItemTitleTv::createImageClear(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* imageClear = elm_image_add(parent);
    elm_image_file_set(imageClear, edjeFilePath.c_str(), "groupImageClear");
    return imageClear;
}

void WebsiteHistoryItemTitleTv::deleteCallbacks()
{
    evas_object_smart_callback_del(m_buttonSelect, "clicked", NULL);
}

void WebsiteHistoryItemTitleTv::setFocusChain(Evas_Object* obj)
{
    elm_object_focus_allow_set(m_buttonSelect, EINA_TRUE);
    elm_object_focus_custom_chain_append(obj, m_buttonSelect, NULL);
}

}
}
