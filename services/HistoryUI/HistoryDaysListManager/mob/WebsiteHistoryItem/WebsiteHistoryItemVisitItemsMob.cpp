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
#include <EflTools.h>

#include "app_i18n.h"
#include "../../HistoryDayItemData.h"
#include "WebsiteHistoryItemVisitItemsMob.h"
#include "BrowserLogger.h"

namespace tizen_browser {
namespace base_ui {

boost::signals2::signal<void(const WebsiteVisitItemDataPtr, bool)> WebsiteHistoryItemVisitItemsMob::signalButtonClicked;

const int WebsiteHistoryItemVisitItemsMob::GESTURE_MOMENTUM_MIN = 2000;

WebsiteHistoryItemVisitItemsMob::WebsiteHistoryItemVisitItemsMob(WebsiteVisitItemDataPtr websiteVisitItem)
    : m_eflObjectsDeleted(nullptr)
    , m_layoutMain(nullptr)
    , m_boxMainVertical(nullptr)
{
    VisitItemObjects obj;
    obj.websiteVisitItemData = websiteVisitItem;
    m_websiteVisitItem = obj;
}

WebsiteHistoryItemVisitItemsMob::~WebsiteHistoryItemVisitItemsMob()
{
    // clear all widgets aded by this class
    if (!m_eflObjectsDeleted)
        evas_object_del(m_layoutMain);
}

Evas_Object* WebsiteHistoryItemVisitItemsMob::init(Evas_Object* parent,
    const std::string& edjeFilePath)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain, edjeFilePath.c_str(),
        "layoutWebsiteHistoryItemVisitItems");

    m_boxMainVertical = elm_box_add(m_layoutMain);
    tools::EflTools::setExpandHints(m_boxMainVertical);
    elm_object_part_content_set(m_layoutMain,
        "boxMainVertical", m_boxMainVertical);

    LayoutVisitItemObjects layoutObjects = createLayoutVisitItem(
        m_layoutMain, edjeFilePath, m_websiteVisitItem.websiteVisitItemData);
    m_websiteVisitItem.layoutVisitItemObjects = layoutObjects;
    elm_box_pack_end(m_boxMainVertical, layoutObjects.layout);

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutMain);

    initCallbacks();
    return m_layoutMain;
}

void WebsiteHistoryItemVisitItemsMob::setEflObjectsAsDeleted()
{
    m_eflObjectsDeleted = true;
}

WebsiteHistoryItemVisitItemsMob::LayoutVisitItemObjects
WebsiteHistoryItemVisitItemsMob::createLayoutVisitItem(
    Evas_Object* parent, const std::string& edjeFilePath,
    WebsiteVisitItemDataPtr websiteVisitItemData)
{
    Evas_Object* layout = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layout);
    elm_layout_file_set(layout, edjeFilePath.c_str(),
        "layoutWebsiteHistoryVisitItem");

    Evas_Object* boxMain = elm_box_add(parent);
    tools::EflTools::setExpandHints(boxMain);
    elm_box_horizontal_set(boxMain, EINA_TRUE);

    Evas_Object* layoutContent = createLayoutContent(parent, edjeFilePath, websiteVisitItemData);
    Evas_Object* layoutButtonDelete = createLayoutButtonDelete(parent, edjeFilePath);
    elm_box_pack_end(boxMain, layoutContent);

    elm_object_part_content_set(layout, "main", boxMain);

    Evas_Object* layerGesture = elm_gesture_layer_add(parent);
    elm_gesture_layer_attach(layerGesture, layout);

    evas_object_show(layoutContent);
    evas_object_show(layout);

    WebsiteHistoryItemVisitItemsMob::LayoutVisitItemObjects ret;
    ret.layout = layout;
    ret.buttonSelect = elm_object_part_content_get(layoutContent, "buttonSelect");
    ret.layerGesture = layerGesture;
    ret.boxMain= boxMain;
    ret.layoutButtonDelete = layoutButtonDelete;
    ret.buttonDelete= elm_object_part_content_get(layoutButtonDelete, "buttonSelect");
    ret.clickBlocked = false;
    return ret;
}

Evas_Object* WebsiteHistoryItemVisitItemsMob::createLayoutContent(Evas_Object* parent,
    const std::string& edjeFilePath, WebsiteVisitItemDataPtr websiteVisitItemData)
{
    Evas_Object* layoutContent = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layoutContent);
    elm_layout_file_set(layoutContent, edjeFilePath.c_str(),
        "layoutMainContent");

    elm_object_part_text_set(layoutContent, "textTitle",
        websiteVisitItemData->historyItem->getTitle().c_str());

    elm_object_part_text_set(layoutContent, "textUrl",
        websiteVisitItemData->historyItem->getUrl().c_str());

    // TODO Replace with std::time_t to_time_t(ptime pt) in the future
    std::time_t rawtime(websiteVisitItemData->historyItem->getLastVisitAsTimeT());
    char buffer[80];
    struct tm ts_ret;

    if (localtime_r(&rawtime, &ts_ret)==NULL){
        BROWSER_LOGE("[%s:%d] Warning: Unhandled localtime_r", __PRETTY_FUNCTION__, __LINE__);
        return  nullptr;
    }
    std::strftime(buffer,80,"%R",&ts_ret);

    elm_object_part_text_set(layoutContent, "textTime", buffer);

    Evas_Object* buttonSelect = elm_button_add(parent);
    elm_object_part_content_set(layoutContent, "buttonSelect", buttonSelect);
    elm_object_style_set(buttonSelect, "invisible_button");

    evas_object_show(buttonSelect);
    evas_object_show(layoutContent);

    return layoutContent;
}

Evas_Object* WebsiteHistoryItemVisitItemsMob::createLayoutButtonDelete(Evas_Object* parent,
    const std::string& edjeFilePath)
{
    Evas_Object* lay = elm_layout_add(parent);
    evas_object_size_hint_align_set(lay, EVAS_HINT_FILL, 0.5);
    elm_layout_file_set(lay, edjeFilePath.c_str(), "layoutButtonDelete");
    elm_object_text_set(lay, _("IDS_BR_SK_CLEAR"));

    Evas_Object* buttonDelete = elm_button_add(parent);
    elm_object_part_content_set(lay, "buttonSelect", buttonDelete);
    elm_object_style_set(buttonDelete, "invisible_button");

    return lay;
}

void WebsiteHistoryItemVisitItemsMob::initCallbacks()
{
    evas_object_smart_callback_add(
        m_websiteVisitItem.layoutVisitItemObjects.buttonSelect, "clicked",
        _buttonSelectClicked, &m_websiteVisitItem);
    evas_object_smart_callback_add(
        m_websiteVisitItem.layoutVisitItemObjects.buttonDelete, "clicked",
        _buttonDeleteClicked, &m_websiteVisitItem);
    elm_gesture_layer_cb_add(
        m_websiteVisitItem.layoutVisitItemObjects.layerGesture,
        ELM_GESTURE_N_LINES, ELM_GESTURE_STATE_MOVE, _gestureOccured,
        &m_websiteVisitItem);
}

void WebsiteHistoryItemVisitItemsMob::_buttonSelectClicked(void* data,
    Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data)
        return;
    VisitItemObjects* visitItemObject = static_cast<VisitItemObjects*>(data);
    if (visitItemObject->layoutVisitItemObjects.clickBlocked) {
        visitItemObject->layoutVisitItemObjects.clickBlocked = false;
        return;
    }
    signalButtonClicked((*visitItemObject).websiteVisitItemData, false);
}

void WebsiteHistoryItemVisitItemsMob::_buttonDeleteClicked(void* data,
    Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data)
        return;
    VisitItemObjects* visitItemObject = static_cast<VisitItemObjects*>(data);
    signalButtonClicked((*visitItemObject).websiteVisitItemData, true);
}

Evas_Event_Flags WebsiteHistoryItemVisitItemsMob::_gestureOccured(void *data,
    void *event_info)
{
    Evas_Event_Flags flag = EVAS_EVENT_FLAG_NONE;
    if (!data)
        return flag;
    VisitItemObjects* visitItemObject = static_cast<VisitItemObjects*>(data);
    auto info = static_cast<Elm_Gesture_Line_Info*>(event_info);
    if (info->momentum.mx != 0 || info->momentum.my != 0) {
        // prevents click event, when gesture occurred
        visitItemObject->layoutVisitItemObjects.clickBlocked = true;
        // ignore too small gestures
        if (abs(info->momentum.mx) < GESTURE_MOMENTUM_MIN)
            return flag;
        if (info->momentum.mx < 0)
            showButtonDelete(
                visitItemObject->layoutVisitItemObjects.layoutButtonDelete,
                visitItemObject->layoutVisitItemObjects.boxMain, true);
        else if (info->momentum.mx > 0)
            showButtonDelete(
                visitItemObject->layoutVisitItemObjects.layoutButtonDelete,
                visitItemObject->layoutVisitItemObjects.boxMain, false);
    }
    return flag;
}

void WebsiteHistoryItemVisitItemsMob::showButtonDelete(
    Evas_Object* layoutButtonDelete, Evas_Object* box, bool show)
{
    if (evas_object_visible_get(layoutButtonDelete) == show)
        return;
    if (show) {
        evas_object_show(layoutButtonDelete);
        elm_box_pack_end(box, layoutButtonDelete);
    } else {
        evas_object_hide(layoutButtonDelete);
        elm_box_unpack(box, layoutButtonDelete);
    }
}

bool WebsiteHistoryItemVisitItemsMob::contains(
    WebsiteVisitItemDataPtrConst websiteVisitItemData)
{
    if (m_websiteVisitItem.websiteVisitItemData == websiteVisitItemData)
        return true;
    return false;
}

void WebsiteHistoryItemVisitItemsMob::removeItem(
    WebsiteVisitItemDataPtrConst websiteVisitItemData)
{
    if (m_websiteVisitItem.websiteVisitItemData == websiteVisitItemData) {
        elm_box_unpack(m_boxMainVertical,
            m_websiteVisitItem.layoutVisitItemObjects.layout);
        evas_object_del(m_websiteVisitItem.layoutVisitItemObjects.layout);
    }
}

int WebsiteHistoryItemVisitItemsMob::getVisitItemsId()
{
    return m_websiteVisitItem.websiteVisitItemData->historyItem->getId();
}

}
}
