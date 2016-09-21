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

#include "ProgressiveWebApp.h"

#include "BrowserLogger.h"

namespace tizen_browser {
namespace base_ui {

ProgressiveWebApp::ProgressiveWebApp()
    : m_pwaInfoStruct()
    , m_uriPartsMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

ProgressiveWebApp::~ProgressiveWebApp()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ProgressiveWebApp::preparePWAParameters(const std::string &uri)
{
    parse_uri(uri);
    fillPWAstruct(m_uriPartsMap);
}

void ProgressiveWebApp::parse_uri(const std::string& uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string::size_type pos = uri.find("pwa_");
    std::string::size_type prevpos = pos;
    m_uriPartsMap["protocol"] = uri.substr(0, pos);
    while (pos != std::string::npos) {
        prevpos = pos;
        pos = uri.find("pwa_", pos+1);
        std::string tmp = uri.substr(prevpos, pos-prevpos-1);
        std::string::size_type delimiter = tmp.find(":");
        std::string first = tmp.substr(0, delimiter);
        std::string second = tmp.substr(delimiter+1, tmp.length());
        m_uriPartsMap[first] = second;
    }
}

void ProgressiveWebApp::fillPWAstruct(const std::map<std::string, std::string> &pwaParametersMap)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto it = pwaParametersMap.find("pwa_id");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.id = it->second;
    it = pwaParametersMap.find("pwa_uri");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.uri = it->second;
    it = pwaParametersMap.find("pwa_decodedIcon");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.decodedIcon = it->second;
    it = pwaParametersMap.find("pwa_name");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.name = it->second;
    it = pwaParametersMap.find("pwa_shortName");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.shortName = it->second;
    it = pwaParametersMap.find("pwa_orientation");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.orientation = std::stoi(it->second);
    it = pwaParametersMap.find("pwa_displayMode");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.displayMode = std::stoi(it->second);
    it = pwaParametersMap.find("pwa_themeColor");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.themeColor = std::stol(it->second);
    it = pwaParametersMap.find("pwa_backgroundColor");
    if (it != pwaParametersMap.end())
        m_pwaInfoStruct.backgroundColor = std::stol(it->second);
}

}   // namespace tizen_browser
}   // namespace base_ui
