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
 * NaviframeLWrapper.h
 *
 *  Created on: Aug 4, 2016
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef NAVIFRAMEWRAPPER_H
#define NAVIFRAMEWRAPPER_H

#include <Elementary.h>
#include <Ecore.h>
#include <Edje.h>
#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include <string>
#include <map>

namespace tizen_browser{
namespace base_ui{

class NaviframeWrapper
{
public:
    NaviframeWrapper(Evas_Object *parent);
    ~NaviframeWrapper();

    void show() { evas_object_show(m_layout); }
    void hide() { evas_object_hide(m_layout); }

    Evas_Object *getLayout();
    void setTitle(std::string title);
    void setContent(Evas_Object *content);

    void addPrevButton(Evas_Smart_Cb callback, void* data);
    void setPrevButtonVisible(bool visible);

    void addLeftButton(Evas_Smart_Cb callback, void* data);
    void setLeftButtonText(std::string text);
    void setLeftButtonVisible(bool visible);
    void setLeftButtonEnabled(bool enabled);

    void addRightButton(Evas_Smart_Cb callback, void* data);
    void setRightButtonText(std::string text);
    void setRightButtonVisible(bool visible);
    void setRightButtonEnabled(bool enabled);

    void createBottomBar(Evas_Object* layout = nullptr,
        std::string swallow_name = "elm.swallow.content");
    void addButtonToBottomBar(std::string text, Evas_Smart_Cb callback, void* data);
    void setEnableButtonInBottomBar(int number, bool enabled);
    void setBottomButtonText(int number, const std::string& text);
    void setVisibleBottomBar(bool visible);

protected:
    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_bottom_box;
    std::vector<Evas_Object*> m_bottom_buttons;
};

using SharedNaviframeWrapper = std::shared_ptr<NaviframeWrapper>;

}
}

#endif // NAVIFRAMEWRAPPER_H
