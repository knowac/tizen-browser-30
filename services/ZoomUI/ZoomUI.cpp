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


#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "ZoomUI.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"

#define DX 50
#define iDX -50

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(ZoomUI, "org.tizen.browser.zoomui")

ZoomUI::ZoomUI()
    : m_layout(nullptr)
    , m_zoom_slider(nullptr)
    , m_zoom_menu(nullptr)
    , m_parent(nullptr)
    , m_current_translation_x(0)
    , m_current_translation_y(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("ZoomUI/ZoomUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

ZoomUI::~ZoomUI() {}

void ZoomUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* ZoomUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if(!m_layout)
        createLayout(m_parent);
    return m_layout;
}

void ZoomUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(m_layout);
    evas_object_show(m_zoom_menu);
    evas_object_show(m_zoom_slider);
    int zoomFactor = *(getZoom());
    elm_slider_value_set(m_zoom_slider, calculateSliderValue(zoomFactor));
}

void ZoomUI::hideUI()
{
    evas_object_hide(m_zoom_slider);
    evas_object_hide(m_zoom_menu);
    evas_object_hide(m_layout);
}

void ZoomUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init(parent);
    if (!m_layout)
        createLayout(parent);
    showUI();
    elm_object_focus_set(m_zoom_slider, EINA_TRUE);
}

bool ZoomUI::isVisible()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (evas_object_visible_get(m_layout) || (*getZoom() != ZOOM_DEFAULT && *getZoom() != 0))
        return true;
    else
        return false;
}

void ZoomUI::createLayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "zoom-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    createZoomSlider();
}

void ZoomUI::createZoomSlider()
{
    m_zoom_menu = elm_layout_add(m_layout);
    elm_layout_file_set(m_zoom_menu, m_edjFilePath.c_str(), "zoom-menu");
    evas_object_size_hint_weight_set(m_zoom_menu, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_zoom_menu, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_zoom_slider = elm_slider_add(m_zoom_menu);
    evas_object_size_hint_weight_set(m_zoom_slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_zoom_slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(m_zoom_slider, "changed", _zoom_slider_changed, this);
    elm_object_style_set(m_zoom_slider, "default");
    elm_slider_horizontal_set(m_zoom_slider, EINA_TRUE);
    elm_slider_min_max_set(m_zoom_slider, 1, 6);
    elm_slider_step_set(m_zoom_slider, 0.2);
    int zoomFactor = *(getZoom());
    elm_slider_value_set(m_zoom_slider, calculateSliderValue(zoomFactor));
    elm_slider_indicator_show_set(m_zoom_slider, EINA_FALSE);

    elm_object_part_content_set(m_zoom_menu, "slider_swallow", m_zoom_slider);
    evas_object_event_callback_add(m_zoom_slider, EVAS_CALLBACK_KEY_DOWN, _zoom_value_confirmed, this);
}

void ZoomUI::clearItems()
{
    evas_object_del(m_layout);
    setZoom(ZOOM_DEFAULT);
}

void ZoomUI::_zoom_slider_changed(void *data, Evas_Object *obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(data && obj) {
        int val = elm_slider_value_get(obj);
        int zoomLevel = ZOOM_DEFAULT;    
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);

        switch (val) {
            case 1: zoomLevel = ZOOM_50;
                    break;
            case 2: zoomLevel = ZOOM_75;
                    break;
            case 3: zoomLevel = ZOOM_DEFAULT;
                    break;
            case 4: zoomLevel = ZOOM_150;
                    break;
            case 5: zoomLevel = ZOOM_200;
                    break;
            case 6: zoomLevel = ZOOM_300;
                    break;
            default:BROWSER_LOGD("[%s:%d] Warning: Unhandled zoom level", __PRETTY_FUNCTION__, __LINE__);
                    break;
        }
        zoomUI->setZoom(zoomLevel);
    }
}

void ZoomUI::_zoom_value_confirmed(void* data, Evas*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Event_Key_Down* ev = static_cast<Evas_Event_Key_Down*>(event_info);

    if (!data || !ev || !ev->keyname)
        return;

    ZoomUI* self = static_cast<ZoomUI*>(data);
    if ((std::string(ev->keyname) == "Return") || (std::string(ev->keyname) == "Up") || (std::string(ev->keyname) == "Down")) {
        int val = (int)elm_slider_value_get(self->m_zoom_slider);
        BROWSER_LOGD("[%s:%d] val: %d", __PRETTY_FUNCTION__, __LINE__, val);
        evas_object_hide(self->m_zoom_menu);
    }
}

void ZoomUI::escapeZoom()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (isVisible()) {
        setZoom(ZoomUI::ZOOM_DEFAULT);
        hideUI();
    }
}

int ZoomUI::calculateSliderValue(int zoom)
{
    BROWSER_LOGD("[%s:%d] zoom factor: %d", __PRETTY_FUNCTION__, __LINE__, zoom);
    int rv = 1; //for zoom < ZOOM_75
    if (zoom >= ZOOM_300)
        rv = 6;
    else if (zoom >= ZOOM_200)
        rv = 5;
    else if (zoom >= ZOOM_150)
        rv = 4;
    else if (zoom >= ZOOM_100)
        rv = 3;
    else if (zoom >= ZOOM_75)
        rv = 2;

    return rv;
}


}
}
