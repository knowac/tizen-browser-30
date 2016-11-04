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

#ifndef __TEXT_POPUP_H__
#define __TEXT_POPUP_H__ 1

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

class TextPopup : public interfaces::AbstractPopup
{
public:
    static TextPopup* createPopup(Evas_Object* parent);
    static TextPopup* createPopup(Evas_Object* parent, const std::string& title, const std::string& message);

    void show();
    void dismiss();
    void onBackPressed();
#if PROFILE_MOBILE
    void orientationChanged() {};
#endif

    void setTitle(const std::string& title);
    void setMessage(const std::string& message);
    void setContent(Evas_Object* content);
    void addButton(const PopupButtons& button, bool dismissOnClick = true, bool defaultBackButton = false);
    void createLayout();
    boost::signals2::signal<void (PopupButtons)> buttonClicked;

    ~TextPopup();

private:
    TextPopup(Evas_Object* parent);
    TextPopup(Evas_Object* parent, const std::string& title, const std::string& message);

    static void _response_cb(void* data, Evas_Object* obj, void* event_info);

    struct Button {
        Button(PopupButtons type, bool dismissOnClick)
            : m_type(type)
            , m_dismissOnClick(dismissOnClick)
            , m_evasObject(nullptr)
        {};
        PopupButtons m_type;
        bool m_dismissOnClick;
        Evas_Object* m_evasObject;
    };

    Evas_Object* m_parent;
    Evas_Object* m_popup;
    std::vector<Button> m_buttons;
    std::string m_title;
    std::string m_message;
    std::string m_edjFilePath;
    PopupButtons m_defaultBackButton = NONE;
};

}

}

#endif //__TEXT_POPUP_H__
