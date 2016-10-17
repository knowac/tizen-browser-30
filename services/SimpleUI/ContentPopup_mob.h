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

#ifndef __MOBILE_POPUP_H__
#define __MOBILE_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>

#include "AbstractPopup.h"
#include "PopupButtons.h"
#include "WebConfirmation.h"

namespace tizen_browser
{

namespace base_ui
{

class ContentPopup : public interfaces::AbstractPopup
{
public:
    static ContentPopup* createPopup(Evas_Object* parent);
    static ContentPopup* createPopup(Evas_Object* parent, const std::string& title);

    void show();
    void dismiss();
    void onBackPressed();
    void orientationChanged() override { }

    void setTitle(const std::string& title);
    void setContent(Evas_Object* content);
    void addButton(const PopupButtons& button, bool dismissOnClick = true);
    boost::signals2::signal<void (PopupButtons)> buttonClicked;
    boost::signals2::signal<bool ()> isLandscape;

    ~ContentPopup();

    Evas_Object* getMainLayout() { return m_scroller; }
private:
    ContentPopup(Evas_Object* parent);
    ContentPopup(Evas_Object* parent, const std::string& title);
    void createLayout();

    static void _layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void _response_cb(void* data, Evas_Object* obj, void* event_info);

    struct Button {
        Button(PopupButtons type, bool dismissOnClick, Evas_Object* object)
            : m_type(type)
            , m_dismissOnClick(dismissOnClick)
            , m_evasObject(object)
        {};
        PopupButtons m_type;
        bool m_dismissOnClick;
        Evas_Object* m_evasObject;
    };

    Evas_Object* m_parent;
    Evas_Object* m_layout;
    Evas_Object* m_buttons_box;
    Evas_Object* m_scroller;
    Evas_Object* m_content;
    std::vector<Button> m_buttons;
    std::string m_title;
    std::string m_message;
    std::string m_edjFilePath;
    static const int MARGIN = 44;
};

}

}

#endif //__MOBILE_POPUP_H__
