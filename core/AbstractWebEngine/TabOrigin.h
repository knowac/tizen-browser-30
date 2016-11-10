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

#ifndef ORIGIN_H_
#define ORIGIN_H_


namespace tizen_browser {
namespace basic_webengine {
/*
 * Container for webview origin id value
 */
class TabOrigin
{
public:
    static const int UNKNOWN = -1;
    static const int QUICKACCESS = -2;
    TabOrigin() : m_value(UNKNOWN) { }
    TabOrigin(int from) : m_value(from) { }
    int getValue() const { return m_value; }
    void setValue(int value) { m_value = value; }
    bool isFromWebView() const { return m_value >= 0; }
    bool isFromQuickAccess() const { return m_value == QUICKACCESS; }
private:
    int m_value;

};


}
}

#endif