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

/*
 * TabId.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: p.rafalski
 */

#include "browser_config.h"
#include "BrowserImage.h"
#include "TabId.h"
#include "CapiWebErrorCodes.h"

namespace tizen_browser {
namespace basic_webengine {

TabId TabId::NONE(-1);

TabId::TabId(int id)
    : m_id(id)
{
}

TabId::TabId(const TabId & n)
{
    m_id = n.m_id;
}

TabId::~TabId()
{
}

std::string TabId::toString() const {
    return std::to_string(m_id);
}

TabContent::TabContent(TabId id,
        const std::string& url,
        const std::string& title,
        const TabOrigin& origin,
        tools::BrowserImagePtr thumbnail)
    : m_id(id)
    , m_url(url)
    , m_title(title)
    , m_origin(origin)
    , m_thumbnail(thumbnail)
{
}

TabContent::TabContent(const TabId& id, const std::string& url, const std::string& title, const TabOrigin& origin) :
        TabContent(id, url, title, origin, std::make_shared<tools::BrowserImage>())
{
}

TabId TabContent::getId() const
{
    return m_id;
}

std::string TabContent::getUrl() const
{
    return m_url;
}

std::string TabContent::getTitle() const
{
    return m_title;
}

TabOrigin TabContent::getOrigin() const
{
    return m_origin;
}

void TabContent::setThumbnail(tools::BrowserImagePtr thumbnail)
{
    m_thumbnail = thumbnail;
}

tools::BrowserImagePtr TabContent::getThumbnail() const
{
    return m_thumbnail;
}

} /* end of basic_webengine */
} /* end of tizen_browser */
