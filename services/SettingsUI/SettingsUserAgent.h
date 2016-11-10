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

#ifndef SETTINGSUSERAGENT_MOB_H_
#define SETTINGSUSERAGENT_MOB_H_

#include "SettingsUI.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include <vconf.h>
#include "BrowserLogger.h"
#include "../SimpleUI/RadioPopup.h"
#include "Tools/EflTools.h"
#include "Tools/SettingsEnums.h"

namespace tizen_browser{
namespace base_ui{

class SettingsUserAgent
    : public SettingsUI
{
public:
    SettingsUserAgent(Evas_Object* parent);
    virtual ~SettingsUserAgent();
    virtual bool populateList(Evas_Object* genlist) override;
    virtual void updateButtonMap() override;
    Evas_Object* createRadioButton(Evas_Object* obj, ItemData* itd) override;
private:
    struct UserAgentItem {
        UserAgentItem(std::string&& n, std::string&& s)
            : name(n)
            , uaString(s)
        {}
        std::string name;
        std::string uaString;
    };

    struct RadioData {
        RadioData(SettingsUserAgent* suaUI, int id)
            : sua(suaUI)
            , radioId(id)
        {};
        SettingsUserAgent* sua;
        int radioId;
    };
    static void onGenlistClick(void* data, Evas_Object* obj, void* event_info);
    static void onRadioClick(void* data, Evas_Object* obj, void* event_info);

    static const int UA_ITEMS_COUNT = 21;
    std::array<UserAgentItem, UA_ITEMS_COUNT> m_uaList{{
        UserAgentItem{"Default", "Mozilla/5.0 (Linux; Tizen 3.0; SAMSUNG TM1) AppleWebKit/537.36 (KHTML, like Gecko) SamsungBrowser/1.0 Chrome/47.0.2526.69 Mobile safari/537.36"},
        UserAgentItem{"Mobile - Kiran", "Mozilla/5.0 (Linux; Tizen 2.3; SAMSUNG SM-Z130H) AppleWebKit/537.3 (KHTML, like Gecko) SamsungBrowser/1.0 Mobile Safari/537.3"},
        UserAgentItem{"Mobile - Tizen 2.4", "Mozilla/5.0 (Linux; Tizen 2.4.0; SAMSUNG TM1) AppleWebKit/537.3 (KHTML, like Gecko)SamsungBrowser/1.0 Mobile Safari/537.3"},
        UserAgentItem{"Mobile - Tizen 3.0", "Mozilla/5.0 (Linux; Tizen 3.0; tm1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.69 Mobile safari/537.36"},
        UserAgentItem{"Mobile - Android 4 SM-G900K  ", "Mozilla/5.0 (Linux; Android 4.4.2; SM-G900K Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.122 Mobile Safari/537.36"},
        UserAgentItem{"Mobile - Android 5 SM-G531H", "Mozilla/5.0 (Linux; Android 5.1.1; SAMSUNG SM-G531H Build/LMY48B) AppleWebKit/537.36 (KHTML, like Gecko) SamsungBrowser/3.3 Chrome/38.0.2125.102 Mobile Safari/537.36"},
        UserAgentItem{"Desktop - Firefox 22.0", "Mozilla/5.0 (Windows NT 6.1; rv:22.0) Gecko/20100101 Firefox/22.0"},
        UserAgentItem{"Desktop - Chrome 35.0", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.114 Safari/537.36"},
        UserAgentItem{"Desktop - Internet Explorer 10", "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/6.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; InfoPath.3; MS-RTC LM 8; Tablet PC 2.0; .NET4.0E)"},
        UserAgentItem{"Desktop - Opera 21.0", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.132 Safari/537.36 OPR/21.0.1432.67 (Edition Campaign 38)"},
        UserAgentItem{"Desktop - Safari 5.1.7", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/534.57.2 (KHTML, like Gecko) Version/5.1.7 Safari/534.57.2"},
        UserAgentItem{"Desktop - Safari 6.0", "Mozilla/5.0 (iPad; CPU OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5355d Safari/8536.25"},
        UserAgentItem{"Apple iOS 7.0 - mobile", "Mozilla/5.0 (iPhone; CPU iPhone OS 7_0 like Mac OS X) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11A465 Safari/9537.53"},
        UserAgentItem{"Apple iOS 6.0 - pad", "Mozilla/5.0 (iPad; CPU OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5376e Safari/8536.25"},
        UserAgentItem{"Apple iOS 6.0 - mobile", "Mozilla/5.0 (iPhone; CPU iPhone OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5376e Safari/8536.25"},
        UserAgentItem{"Apple iOS 5.0 - pad", "Mozilla/5.0 (iPad; CPU OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3"},
        UserAgentItem{"Apple iOS 5.0 - mobile", "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3"},
        UserAgentItem{"Galaxy S5", "Mozilla/5.0 (Linux; Android 4.4.2; en-us; SAMSUNG SM-G900K/KTU1AND8 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/1.6 Chrome/28.0.1500.94 Mobile Safari/537.36"},
        UserAgentItem{"Galaxy S4", "Mozilla/5.0 (Linux; Android 4.2.2; en-gb; SAMSUNG GT-I9500 Build/JDQ39) AppleWebKit/535.19 (KHTML, like Gecko) Version/1.0 Chrome/18.0.1025.308 Mobile Safari/535.19"},
        UserAgentItem{"Galaxy S note2", "Mozilla/5.0 (Linux; U; Android 4.3; ko-kr; SHV-E250K/KKUENC3 Build/JSS15J) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30"},
        UserAgentItem{"Custom UserAgent", ""}
    }};

    std::map<int, Evas_Object*> m_radioMap;
};
}
}

#endif /* SETTINGSUSERAGENT_MOB_H_ */
