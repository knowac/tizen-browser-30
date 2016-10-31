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

#include "browser_config.h"
#include "QuickAccessItem.h"

#include <string>
#include <Evas.h>

namespace tizen_browser{
namespace services{

QuickAccessItem::QuickAccessItem()
    : m_id(0)
    , m_url("")
    , m_title("")
    , m_color(0)
    , m_order(0)
    , m_has_favicon(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

QuickAccessItem::QuickAccessItem(
        int id,
        const std::string &url,
        const std::string &title,
        int color,
        int order,
        bool hasFavicon)
    : m_id(id)
    , m_url(url)
    , m_title(title)
    , m_color(color)
    , m_order(order)
    , m_has_favicon(hasFavicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void QuickAccessItem::setFavicon(tools::BrowserImagePtr favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_has_favicon = true;
    m_favicon = favicon;
}

tools::BrowserImagePtr QuickAccessItem::getFavicon() const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_favicon;
}

}//end namespace storage
}//end namespace tizen_browser
