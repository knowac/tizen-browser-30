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

#ifndef ZOOMUI_H
#define ZOOMUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT ZoomUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    ZoomUI();
    ~ZoomUI();

    //AbstractUIComponent interface methods
    void init(Evas_Object* parent);
    Evas_Object* getContent();
    void showUI();
    void hideUI();
    std::string getName();

    void show(Evas_Object* parent);
    void clearItems();
    bool isVisible();
    void escapeZoom();

    boost::signals2::signal<void ()> closeZoomUI;
    boost::signals2::signal<void (int)> setZoom;
    boost::signals2::signal<int ()> getZoom;
    boost::signals2::signal<void (const int&, const int&)> scrollView;
private:
    void createLayout(Evas_Object* parent);
    void createZoomSlider();
    int calculateSliderValue(int zoom);

    static void _zoom_slider_changed(void * data, Evas_Object * obj, void * event_info);
    static void _zoom_value_confirmed(void* data, Evas* e, Evas_Object* obj, void* event_info);

    std::string m_edjFilePath;
    Evas_Object* m_layout;
    Evas_Object* m_zoom_slider;
    Evas_Object* m_zoom_menu;
    Evas_Object* m_parent;

    int m_current_translation_x;
    int m_current_translation_y;

    enum ZoomLevel {
        ZOOM_50 = 50,
        ZOOM_75 = 75,
        ZOOM_100 = 100,
        ZOOM_150 = 150,
        ZOOM_200 = 200,
        ZOOM_300 = 300, 

        ZOOM_MIN = ZOOM_50,
        ZOOM_DEFAULT = ZOOM_100,
        ZOOM_MAX = ZOOM_300
    };
    enum ArrowType {
        LEFT = 1,
        RIGHT = 2,
        UP = 3,
        DOWN = 4
    };
};

}
}

#endif // ZOOMUI_H
