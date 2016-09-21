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

#ifndef __POPUP_BUTTONS_H__
#define __POPUP_BUTTONS_H__ 1

#include <map>
#include "app_i18n.h"

namespace tizen_browser
{
namespace base_ui
{

    enum PopupButtons
    {
        NONE    = 0
       ,OK      = 1 << 1
       ,CANCEL  = 1 << 2
       ,YES     = 1 << 3
       ,NO      = 1 << 4
       ,CLOSE   = 1 << 5
       ,CONNECT = 1 << 6
       ,CONTINUE= 1 << 7
       ,CLOSE_TAB = 1 << 8
       ,RESET   = 1 << 9
       ,DELETE  = 1 << 10
       ,BACK_TO_SAFETY  = 1 << 11
       ,VIEW_CERTIFICATE  = 1 << 12
    };

    static std::map<PopupButtons, std::string> createTranslations()
    {
        std::map<PopupButtons, std::string> m;
        m[OK] = "IDS_BR_SK_OK";
        m[CANCEL] = "IDS_BR_BUTTON_CANCEL";
        m[YES] = "IDS_BR_SK_YES";
        m[NO] = "IDS_BR_SK_NO";
        m[CLOSE] = "IDS_BR_BUTTON_CLOSE";
        m[CONNECT] = "Connect";
        m[CONTINUE] = "IDS_BR_BUTTON_CONTINUE";
        m[CLOSE_TAB] = "Close tab";
        m[RESET] = "IDS_BR_BUTTON_RESET_ABB";
        m[DELETE] = "IDS_BR_SK_DELETE_ABB";
        m[BACK_TO_SAFETY] = "Back to safety";
        m[VIEW_CERTIFICATE] = "IDS_BR_OPT_VIEW_CERTIFICATE";

        return m;
    }

    static std::map<PopupButtons, std::string> buttonsTranslations = createTranslations();
}

}

#endif //__POPUP_BUTTONS_H__
