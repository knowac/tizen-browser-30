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

#ifndef PROGRESSIVEWEBAPP_H
#define PROGRESSIVEWEBAPP_H

#include <string>
#include <map>

namespace tizen_browser {
namespace base_ui {

//TODO: this is temporaty struct. It will be replaced by enum from engine API when it'll be ready.
struct pwaInfo {
    std::string     id;
    std::string     decodedIcon; // needs to src, type, sizes.
    std::string     uri;
    std::string     name;
    std::string     shortName;
    int             orientation; // needs to portrait-primary, portrait-secondary, landscape-primary, landscape-secondary.
    int             displayMode; // needs to fullscreen, standalone, minimal-ui, browser, and so on.
    long            themeColor;
    long            backgroundColor;
};

class ProgressiveWebApp
{
public:
    ProgressiveWebApp();
    ~ProgressiveWebApp();
    void preparePWAParameters(const std::string& uri);
    pwaInfo getPWAinfo() {return m_pwaInfoStruct;}

private:
    void parse_uri(const std::string& uri);
    void fillPWAstruct(const std::map<std::string, std::string> &pwaParametersMap);

    pwaInfo m_pwaInfoStruct;
    std::map<std::string, std::string> m_uriPartsMap;
};


}   // namespace tizen_browser
}   // namespace base_ui

#endif // PROGRESSIVEWEBAPP_H
